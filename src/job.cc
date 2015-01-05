#include "job.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  Job::Job() : callbackCount(0) {}
  Job::~Job() {
    //printf("freed job!\n");
  }
  bool Job::HasReturnValue() { return false; }
  Local<Value> Job::ReturnedValue() {
    return NanNew<Object>();
  }
}
