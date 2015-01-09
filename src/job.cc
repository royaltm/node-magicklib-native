#include "job.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  Job::Job() : storageIndex(0), isCallbackSet(false) {}

  Job::~Job() {
    //printf("freed job!\n");
  }

  bool Job::HasReturnValue() { return false; }

  Local<Value> Job::ReturnedValue() {
    return NanNew<Object>();
  }

  bool Job::IsCallbackSet() {
    return isCallbackSet;
  }

  uint32_t Job::GetStorageIndex() {
    return storageIndex;
  }

  void Job::SetStorageIndex(uint32_t index) {
    storageIndex = index;
  }

  void Job::SetStorageIndex(uint32_t index, bool isCallback) {
    storageIndex = index;
    isCallbackSet = isCallback;
  }
}
