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
  Persistent<String> widthSym;
  Persistent<String> heightSym;
  Persistent<String> xSym;
  Persistent<String> ySym;
  Persistent<String> sizeSym;
  Persistent<String> modeSym;
  Persistent<String> gravitySym;

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
    NanAssignPersistent(batchSym      , NanNew<String>("batch"));
    NanAssignPersistent(autoCloseSym  , NanNew<String>("autoClose"));
    NanAssignPersistent(autoCopySym   , NanNew<String>("autoCopy"));
    NanAssignPersistent(magickSym     , NanNew<String>("magick"));
    NanAssignPersistent(columnsSym    , NanNew<String>("columns"));
    NanAssignPersistent(rowsSym       , NanNew<String>("rows"));
    NanAssignPersistent(pageSym       , NanNew<String>("page"));
    NanAssignPersistent(colorSym      , NanNew<String>("color"));
    NanAssignPersistent(srcSym        , NanNew<String>("src"));
    NanAssignPersistent(backgroundSym , NanNew<String>("background"));
    NanAssignPersistent(fuzzSym       , NanNew<String>("fuzz"));
    NanAssignPersistent(colorsSym     , NanNew<String>("colors"));
    NanAssignPersistent(colorspaceSym , NanNew<String>("colorspace"));
    NanAssignPersistent(ditherSym     , NanNew<String>("dither"));
    NanAssignPersistent(noiseSym      , NanNew<String>("noise"));
    NanAssignPersistent(channelSym    , NanNew<String>("channel"));
    NanAssignPersistent(sigmaSym      , NanNew<String>("sigma"));
    NanAssignPersistent(radiusSym     , NanNew<String>("radius"));
    NanAssignPersistent(gaussianSym   , NanNew<String>("gaussian"));
    NanAssignPersistent(widthSym      , NanNew<String>("width"));
    NanAssignPersistent(heightSym     , NanNew<String>("height"));
    NanAssignPersistent(xSym          , NanNew<String>("x"));
    NanAssignPersistent(ySym          , NanNew<String>("y"));
    NanAssignPersistent(sizeSym       , NanNew<String>("size"));
    NanAssignPersistent(modeSym       , NanNew<String>("mode"));
    NanAssignPersistent(gravitySym    , NanNew<String>("gravity"));

    // NanAssignPersistent(bufferFunction, Local<Function>::Cast(NanGetCurrentContext()->Global()->Get(NanNew<String>("Buffer"))));

    Image::Init(exports);
    Color::Init(exports);
    NODE_SET_METHOD(exports, "limit", Limit);

    size_t value;

    exports->Set(NanNew<String>("QUANTUM_DEPTH_TEXT"),
        NanNew<String>(MagickCore::GetMagickQuantumDepth(&value)),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("QUANTUM_DEPTH"),
        NanNew<Integer>((uint32_t) value),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("QUANTUM_RANGE_TEXT"),
        NanNew<String>(MagickCore::GetMagickQuantumRange(&value)),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("QUANTUM_RANGE"),
        NanNew<Number>((double) value),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_VERSION"),
        NanNew<String>(MagickLibVersionText),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_INFO"),
        NanNew<String>(MagickCore::GetMagickVersion(&value)),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_VERSION_RELEASE"),
        NanNew<String>(MagickCore::GetMagickReleaseDate()),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_VERSION_NUMBER"),
        NanNew<Integer>((uint32_t) value),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_FEATURES"),
        NanNew<String>(MagickCore::GetMagickFeatures()),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_DELEGATES"),
        NanNew<String>(MagickCore::GetMagickDelegates()),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    exports->Set(NanNew<String>("MAGICK_COPYRIGHT"),
        NanNew<String>(MagickCore::GetMagickCopyright()),
                    static_cast<PropertyAttribute>(ReadOnly | DontDelete));

    MagickCore::SetMagickResourceLimit(MagickCore::ThreadResource, 1);
  }

}

NODE_MODULE(magicklib, NodeMagick::init)
