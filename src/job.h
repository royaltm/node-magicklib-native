#if !defined(NODEMAGICK_JOB_HEADER)
#define NODEMAGICK_JOB_HEADER

#include "nodemagick.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  /* abstract */ class Job {
    public:
      Job(void);
      virtual ~Job(void);
      virtual bool HasReturnValue(void);
      virtual Local<Value> ReturnedValue(void);
      bool IsCallbackSet();
      uint32_t GetStorageIndex();
      void SetStorageIndex(uint32_t index);
      void SetStorageIndex(uint32_t index, bool isCallback);
    private:
      uint32_t storageIndex;
      bool isCallbackSet;
  };
}
#endif