#include "nodemagick.h"
#include "image.h"
#include "color.h"

#include <string>

using namespace node;
using namespace v8;

namespace NodeMagick {

  Persistent<String> batchSym;
  Persistent<String> autoCopySym;
  Persistent<String> autoCloseSym;
  Persistent<String> magickSym;
  Persistent<String> columnsSym;
  Persistent<String> rowsSym;
  Persistent<String> pageSym;
  Persistent<String> colorSym;
  Persistent<String> srcSym;
  Persistent<String> backgroundSym;
  Persistent<String> fuzzSym;
  Persistent<String> colorsSym;
  Persistent<String> colorspaceSym;
  Persistent<String> ditherSym;
  Persistent<String> noiseSym;
  Persistent<String> channelSym;
  Persistent<String> sigmaSym;
  Persistent<String> radiusSym;
  Persistent<String> gaussianSym;

  //Persistent<Function> bufferFunction;

  /**
   * Magick resource control
   *
   * limit(type[, limit]) -> current limit
   *
   * type: "memory", "disk" or "thread"
   *
   * limit: for "memory" and "disk" it is number of bytes,
   *        for "threads" no. threads
   **/
  NAN_METHOD(Limit) {
    NODEMAGICK_SCOPE_ARGC();
    MagickCore::ResourceType resourceType = MagickCore::UndefinedResource;
    if ( argc >= 1 && args[0]->IsString() ) {
      NanUtf8String type( args[0] );
      if ( ! strcasecmp(*type, "memory") )
        resourceType = MagickCore::MemoryResource;
      else if ( ! strcasecmp(*type, "disk") )
        resourceType = MagickCore::DiskResource;
      else if ( ! strcasecmp(*type, "thread") )
        resourceType = MagickCore::ThreadResource;
      if ( resourceType != MagickCore::UndefinedResource ) {
        MagickCore::MagickSizeType result = MagickCore::GetMagickResourceLimit( resourceType );
        if ( argc >= 2 ) {
          if ( args[1]->IsNumber() ) {
            MagickCore::MagickSizeType limit = args[1]->IntegerValue();
            MagickCore::SetMagickResourceLimit(resourceType, limit);
          } else if ( args[1]->IsString() ) {
            NanUtf8String limitStr(args[1]);
            MagickCore::MagickSizeType limit =
                (MagickCore::MagickSizeType) NANUTF8STRING_TOULL(limitStr);
            MagickCore::SetMagickResourceLimit(resourceType, limit);
          }
        }
        if ((result & 0x7FFFFFFFL) == result) {
          NanReturnValue( NanNew<Integer>( (int32_t)result ) );
        } else {
          char outstr[22];
          snprintf(outstr, sizeof(outstr), "%llu", (long long unsigned int) result);
          NanReturnValue( NanNew(outstr) );
        }
      }
    }
    return NanThrowError("Unknown resource, argument should be: \"memory\", \"disk\" or \"thread\"");
  }

  static void init(Handle<Object> exports) {
    batchSym      = NODE_PSYMBOL("batch");
    autoCloseSym  = NODE_PSYMBOL("autoClose");
    autoCopySym   = NODE_PSYMBOL("autoCopy");
    magickSym     = NODE_PSYMBOL("magick");
    columnsSym    = NODE_PSYMBOL("columns");
    rowsSym       = NODE_PSYMBOL("rows");
    pageSym       = NODE_PSYMBOL("page");
    colorSym      = NODE_PSYMBOL("color");
    srcSym        = NODE_PSYMBOL("src");
    backgroundSym = NODE_PSYMBOL("background");
    fuzzSym       = NODE_PSYMBOL("fuzz");
    colorsSym     = NODE_PSYMBOL("colors");
    colorspaceSym = NODE_PSYMBOL("colorspace");
    ditherSym     = NODE_PSYMBOL("dither");
    noiseSym      = NODE_PSYMBOL("noise");
    channelSym    = NODE_PSYMBOL("channel");
    sigmaSym      = NODE_PSYMBOL("sigma");
    radiusSym     = NODE_PSYMBOL("radius");
    gaussianSym   = NODE_PSYMBOL("gaussian");
    // NanAssignPersistent(bufferFunction, Local<Function>::Cast(NanGetCurrentContext()->Global()->Get(NanNew<String>("Buffer"))));
    Image::Init(exports);
    Color::Init(exports);
    NODE_SET_METHOD(exports, "limit", Limit);
    exports->Set(NanNew<String>("QUANTUM_DEPTH"),
        NanNew<Integer>(MAGICKCORE_QUANTUM_DEPTH), ReadOnly);
    exports->Set(NanNew<String>("QUANTUM_RANGE"),
        NanNew<Number>((double)((1L << MAGICKCORE_QUANTUM_DEPTH) - 1)), ReadOnly);

    MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);
  }

}

NODE_MODULE(magicklib, NodeMagick::init)
