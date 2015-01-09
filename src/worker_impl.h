#if !defined(NODEMAGICK_WORKER_HEADER)
#  error 'worker_impl.h' is not supposed to be included directly. Include 'worker.h' instead.
#endif

using namespace node;
using namespace v8;

#define NODEMAGICK_WORKER_BATCH_LOCK(name) lock name(&mutex)

namespace NodeMagick {

  using namespace std;

  template <class T>
  Worker<T>::Worker()
      : mutex( uv_mutex_t() ), request( uv_work_t() )
      , isBusy(false), isOnHold(false)
      , isBatch(false), isPersistentBatch(false)
      , batch( vector<T*>() ), cursor( 0 ), cursorStop( 0 )
      , currentIndex( kStorageMinKey ), storageIndex( kStorageMinKey - 1 )
      , errmsg( NULL ) {
    NanScope();
    uv_mutex_init(&mutex);
    Local<Object> obj = NanNew<Object>();
    NanAssignPersistent(storage, obj);
  }

  template <class T>
  Worker<T>::~Worker() {
    BatchReset();
    if ( ! storage.IsEmpty() )
      NanDisposePersistent( storage );
    delete errmsg;
    uv_mutex_destroy(&mutex);
  }

  template <class T>
  uint32_t Worker<T>::SaveObject(const Handle<Object> &object) {
    NanScope();
    NanNew(storage)->Set(++storageIndex, object);
    return storageIndex;
  }

  template <class T>
  NAN_INLINE void Worker<T>::DisposeStorageItems(uint32_t minIndex) {
    NanScope();
    Local<Object> handle = NanNew(storage);
    if ( minIndex < kStorageMinKey ) {
      minIndex = kStorageMinKey;
    }
    for( uint32_t i = max(minIndex, currentIndex); i <= storageIndex; ++i )
      handle->Delete(i);
    storageIndex = minIndex - 1;
    if ( currentIndex > minIndex )
      currentIndex = minIndex;
  }

  template <class T>
  NAN_INLINE void Worker<T>::Lock()                { isBusy = true;   }
  template <class T>
  NAN_INLINE void Worker<T>::Unlock()              { isBusy = false;  }
  template <class T>
  NAN_INLINE bool Worker<T>::IsBusy() const        { return isBusy;   }
  template <class T>
  NAN_INLINE bool Worker<T>::IsOnHold() const      { return isOnHold; }
  template <class T>
  NAN_INLINE bool Worker<T>::IsBatch() const       { return isBatch;  }
  template <class T>
  NAN_INLINE size_t Worker<T>::BatchSize() const   { return batch.size(); }
  template <class T>
  NAN_INLINE size_t Worker<T>::BatchPendingSize() {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);
    return (batch.size() - cursor);
  }
  template <class T>
  NAN_INLINE void Worker<T>::SetupBatch()         { isBatch = true; }
  template <class T>
  NAN_INLINE void Worker<T>::SetupBatch(bool persistent) {
    isPersistentBatch = persistent;
    isBatch = true;
  }
  template <class T>
  NAN_INLINE bool Worker<T>::IsPersistentBatch() const { return isPersistentBatch;  }
  template <class T>
  NAN_INLINE void Worker<T>::IsPersistentBatch(bool persistent)  {
    isPersistentBatch = persistent;
    if ( persistent )
      SetupBatch();
  }
  template <class T>
  NAN_INLINE void Worker<T>::BatchPush(T *job) {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);
    job->SetStorageIndex(storageIndex);
    batch.push_back(job);
  }
  template <class T>
  NAN_INLINE void Worker<T>::BatchUnshift(T *job) {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);
    batch.insert(batch.begin(), job);
    if ( cursorStop > 0 )
      ++cursorStop;
    batch.back()->SetStorageIndex(storageIndex);
  }
  template <class T>
  NAN_INLINE T *Worker<T>::BatchNext() {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);
    return ( cursor < cursorStop ) ? batch.at(cursor++) : NULL;
  }
  template <class T>
  NAN_INLINE void Worker<T>::BatchClearBeforeCursor() {
    typename vector<T*>::iterator curit = batch.begin() + cursor;
    for(typename vector<T*>::iterator it = batch.begin();
                                      it != curit; ++it) {
      delete *it;
      *it = NULL;
    }
  }
  template <class T>
  NAN_INLINE void Worker<T>::BatchReset() {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);
    for(typename vector<T*>::iterator it = batch.begin();
                                      it != batch.end(); ++it)
      delete *it;
    cursor = cursorStop = 0;
    batch.clear();
    if ( ! IsPersistentBatch() ) {
      isBatch = false;
    }
  }
  template <class T>
  NAN_INLINE void Worker<T>::CancelTailJobs() {
    uint32_t disposeIndex;
    {
      NODEMAGICK_WORKER_BATCH_LOCK(_block);
      typename vector<T*>::iterator it = batch.begin() + cursorStop;
      for(; it != batch.end(); ++it )
        delete *it;

      T *job = cursorStop > 0 ? batch.at(cursorStop - 1) : NULL;
      if ( job == NULL ) {
        cursor = cursorStop = 0;
        batch.clear();
        if ( ! IsBusy() && ! IsPersistentBatch() )
          isBatch = false;
        disposeIndex = kStorageMinKey;
      } else {
        batch.erase( batch.begin() + cursorStop, batch.end() );
        disposeIndex = job->GetStorageIndex() + 1;
      }
    }
    DisposeStorageItems(disposeIndex);
  }

  /* this is run on main and there are no async threads */
  template <class T>
  Local<Value> Worker<T>::SyncProcess(T &job, Local<Object> self) {
    NanEscapableScope();
    Local<Value> value;
    try {
      ProcessJob( job );
      value = NanNew(job.HasReturnValue() ? job.ReturnedValue() : ReturnedValue(self) );
    } catch (exception& err) {
      NanThrowError(err.what());
    } catch (...) {
      NanThrowError("unhandled error");
    }
    JobComplete(false);
    return NanEscapeScope(value);
  }

  template <class T>
  NAN_INLINE void Worker<T>::RunAsync() {
    Lock();
    request = uv_work_t();
    request.data = this;
    uv_queue_work(
      uv_default_loop(),
      &request,
      Worker<T>::AsyncExecute,
      (uv_after_work_cb)Worker<T>::AsyncExecuteComplete
    );    
  }

  /* this is run on main but there may be async thread running */
  template <class T>
  void Worker<T>::AsyncWork(const Handle<Object> &self, T *job, bool hasCallback) {
    NanScope();

    {
      NODEMAGICK_WORKER_BATCH_LOCK(_block);
      if ( job == NULL ) {
        if ( batch.size() - cursor == 0 ) {
          job = new T();
          job->Setup();
          batch.push_back(job);
        } else {
          job = batch.back();
        }
      } else {
        batch.push_back(job);
      }

      job->SetStorageIndex(storageIndex, hasCallback);

      cursorStop = batch.size();
    }

    if ( ! IsBusy() ) {
      SetupBatch();
      NanNew(storage)->Set(kSelfKey, self);
      RunAsync();
    }
  }

  /* this is run on main but there may be async thread running */
  template <class T>
  void Worker<T>::AsyncWork(const Handle<Object> &self, const Handle<Function> &fn, T *job) {
    NanScope();

    NanNew(storage)->Set(++storageIndex, fn);

    AsyncWork(self, job, true);

  }

  /* this is run on main but there may be async thread running */
  template <class T>
  bool Worker<T>::Hold() {
    NODEMAGICK_WORKER_BATCH_LOCK(_block);

    if ( IsBusy() || cursor != 0 )
      return false;

    SetupBatch();
    Lock();
    isOnHold = true;
    return true;
  }

  /*
    this is run on main and there are no async threads
    prepends job on the beginning of the batch queue
    and runs async jobs if callbacks were set
    this is a reverse of Hold so only call if IsOnHold() == true
  */
  template <class T>
  void Worker<T>::Release(const Handle<Object> &self, T *job) {
    if ( job != NULL )
      BatchUnshift(job);

    isOnHold = false;
    if ( cursor < cursorStop ) {
      NanScope();
      Local<Object> handle( NanNew(storage) );
      handle->Set(kSelfKey, self);
      RunAsync();
    } else {
      Unlock();
    }
  }

  template <class T>
  void Worker<T>::SetErrorMessage(const char *msg) {
    size_t size = strlen(msg) + 1;
    delete errmsg;
    errmsg = new char[size];
    memcpy(errmsg, msg, size);
  }

  template <class T>
  NAN_INLINE const char* Worker<T>::ErrorMessage() const {
    return errmsg;
  }

  /* run on main or async thread */
  template <class T>
  void Worker<T>::ProcessJob(T &job) {}

  /* this is run on main and there are no async threads */
  template <class T>
  NAN_INLINE Local<Value> Worker<T>::ReturnedValue() {
    NanEscapableScope();
    return NanEscapeScope( NanNew(storage)->Get(kSelfKey) );
  }

  /* this is run on main and there are no async threads */
  template <class T>
  NAN_INLINE Local<Value> Worker<T>::ReturnedValue(Local<Object> self) {
    return self;
  }

  /* this is run on async thread */
  template <class T>
  void Worker<T>::AsyncProcess() {

    T *job = BatchNext();

    delete errmsg;
    errmsg = NULL;
    try {
      for (; job != NULL; job = BatchNext() ) {
        ProcessJob( *job );
        if ( job->IsCallbackSet() ) break;
      }
    } catch (exception& err) {
      SetErrorMessage(err.what());
    } catch (...) {
      SetErrorMessage("unhandled error");
    }
    while ( job != NULL && ! job->IsCallbackSet() ) job = BatchNext();
  }

  /* this is run on main and there are no async threads */
  template <class T>
  void Worker<T>::JobComplete(bool isAsync) {
    NanScope();

    if (isAsync) {

      request.~uv_work_t();

      Local<Object> handle ( NanNew(storage) );
      Handle<Object> global = NanGetCurrentContext()->Global();

      T *job = batch.at(cursor - 1);
      uint32_t topIndex = job->GetStorageIndex();

      /* handle original callback */
      while ( currentIndex <= topIndex ) {
        Local<Value> value = handle->Get(currentIndex);
        handle->Delete(currentIndex++);
        if ( value->IsFunction() ) {
          if ( ErrorMessage() == NULL ) {
            Local<Value> result;
            if ( job->HasReturnValue() ) {
              result = job->ReturnedValue();
            } else
              result = ReturnedValue();
            Local<Value> argv[] = { NanNull(), result };
            NanMakeCallback(global, value.template As<Function>(), 2, argv);
          } else {
            Local<Value> argv[] = {
              Exception::Error( NanNew<String>( ErrorMessage() ) )
            };
            NanMakeCallback(global, value.template As<Function>(), 1, argv);
          }

          break;
        }
      }

      /* scan through storage for additional callbacks */
      while ( currentIndex <= topIndex ) {
        Local<Value> value = handle->Get(currentIndex);
        handle->Delete(currentIndex++);
        if ( value->IsFunction() ) {
          Local<Value> argv[] = { NanNull(), ReturnedValue() };
          NanMakeCallback(global, value.template As<Function>(), 2, argv);
        }
      }

      if ( cursor == batch.size() ) {
        BatchReset();
        DisposeStorageItems(kStorageMinKey);
      } else {
        BatchClearBeforeCursor();
      }

      if ( cursor < cursorStop ) {
        RunAsync();
      } else {
        Unlock();
        handle->Delete(kSelfKey);
        JobAfterComplete(isAsync);
      }

    } else { /* is Sync */

      DisposeStorageItems(kStorageMinKey);
      JobAfterComplete(isAsync);

    }
  }

  /* this is run on main and there are no async threads */
  template <class T>
  NAN_INLINE void Worker<T>::JobAfterComplete(bool isAsync) {}

  template <class T>
  NAN_INLINE void Worker<T>::AsyncExecute(uv_work_t* req) {
    static_cast<Worker<T>*>( req->data )->AsyncProcess();
  }

  template <class T>
  NAN_INLINE void Worker<T>::AsyncExecuteComplete(uv_work_t* req) {
    static_cast<Worker<T>*>( req->data )->JobComplete(true);
  }

}
