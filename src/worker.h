#if !defined(NODEMAGICK_WORKER_HEADER)
#define NODEMAGICK_WORKER_HEADER

#include "nodemagick.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  class lock {
  private:
    uv_mutex_t *m_;

  public:
    lock(uv_mutex_t *m) : m_(m) {
      uv_mutex_lock(m);
    }
    ~lock() {
      uv_mutex_unlock(m_);
    }
  };

  template <class T>
  class Worker {
    public:
      virtual ~Worker(void);
    protected:
      Worker(void);
      Local<Value> SyncProcess(T &job, Local<Object> self);
      void AsyncWork(const Handle<Object>&self, T *job, bool hasCallback = false);
      void AsyncWork(const Handle<Object>&self, const Handle<Function>&fn, T *job = NULL);
      bool Hold(void);
      void Release(const Handle<Object> &self, T *job);
      virtual void ProcessJob(T &job);
      NAN_INLINE virtual Local<Value> ReturnedValue(void);
      NAN_INLINE virtual Local<Value> ReturnedValue(Local<Object> self);
      NAN_INLINE virtual void JobAfterComplete(bool isAsync);
      uint32_t SaveObject(const Handle<Object> &object);
      NAN_INLINE bool IsBusy(void) const;
      NAN_INLINE bool IsOnHold(void) const;
      NAN_INLINE void SetupBatch();
      NAN_INLINE void SetupBatch(bool persistent);
      NAN_INLINE bool IsBatch(void) const;
      NAN_INLINE bool IsPersistentBatch(void) const;
      NAN_INLINE void IsPersistentBatch(bool persistent);
      NAN_INLINE void BatchPush(T *job);
      NAN_INLINE void CancelTailJobs(void);
      NAN_INLINE size_t BatchSize(void) const;
      NAN_INLINE size_t BatchPendingSize(void);

    private:
      void AsyncProcess(void);
      void JobComplete(bool isAsync);
      NAN_INLINE void RunAsync(void);
      NAN_INLINE void Lock(void);
      NAN_INLINE void Unlock(void);
      NAN_INLINE void BatchUnshift(T *job);
      NAN_INLINE T *BatchNext(void);
      NAN_INLINE void BatchReset(void);
      NAN_INLINE void BatchClearBeforeCursor(void);
      void SetErrorMessage(const char *msg);
      NAN_INLINE const char* ErrorMessage() const;
      NAN_INLINE void DisposeStorageItems(uint32_t minIndex);

      static void AsyncExecute (uv_work_t*);
      static void AsyncExecuteComplete (uv_work_t*);

      uv_mutex_t mutex;
      uv_work_t request;


      /* the async operation is running on thread queue */
      bool isBusy;

      /* the async operation is on hold */
      bool isOnHold;

      /* the batch operation has begun */
      bool isBatch;

      /* do not get back to sync mode after batch completes */
      bool isPersistentBatch;

      /* here be jobs */
      vector<T*> batch;
      size_t cursor;
      size_t cursorStop;
      /* storage for callbacks and persistent data (mainly blobs) */
      Persistent<Object> storage;
      /*
        [ kSelfKey: self,
          kCallbackKey: callback, ...,
          storageIndex: lastItem]
      */
      /* storage cursor */
      uint32_t currentIndex;
      /* storage top */
      uint32_t storageIndex;
      char *errmsg;
      static const uint32_t kSelfKey = 0, kStorageMinKey = 1;
  };

}

#include "worker_impl.h"

#endif
