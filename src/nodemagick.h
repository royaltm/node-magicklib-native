#if !defined(NODEMAGICK_HEADER)
#define NODEMAGICK_HEADER

/*
  #define NODEMAGICK_USE_STL_MAP //undefined - use vector instead
*/

#ifdef _MSC_VER
#  define strncasecmp _strnicmp
#  define strcasecmp _stricmp
#  define snprintf _snprintf
#  define NANUTF8STRING_TOULL(var) stoull(*var)
#else
#  define NANUTF8STRING_TOULL(var) strtoull(*var, NULL, 10)
#endif

#include <memory>
#include <vector>
#include <Magick++.h>
#include <node.h>
#include <node_buffer.h>
#include <nan.h>


namespace NodeMagick {
  using namespace node;
  using namespace v8;

  extern Persistent<String> batchSym;
  extern Persistent<String> autoCopySym;
  extern Persistent<String> autoCloseSym;
  extern Persistent<String> magickSym;
  extern Persistent<String> columnsSym;
  extern Persistent<String> rowsSym;
  extern Persistent<String> pageSym;
  extern Persistent<String> colorSym;
  extern Persistent<String> srcSym;
  extern Persistent<String> backgroundSym;
  extern Persistent<String> fuzzSym;
  extern Persistent<String> colorsSym;
  extern Persistent<String> colorspaceSym;
  extern Persistent<String> ditherSym;
  extern Persistent<String> noiseSym;
  extern Persistent<String> channelSym;

  //extern Persistent<Function> bufferFunction;

}

#define NODEMAGICK_TRY_IGNORE_WARNING(expression) \
  try {                                           \
    expression;                                   \
  } catch (Magick::Warning& warning) {            \
    printf("warning: %s\n", warning.what());      \
  } catch (exception& err) {                      \
      return NanThrowError(err.what());           \
  } catch (...) {                                 \
      return NanThrowError("unhandled error");    \
  }

#define NODEMAGICK_TRY_OR_THROW(expression)    \
  try {                                        \
    expression;                                \
  } catch (exception& err) {                   \
      return NanThrowError(err.what());        \
  } catch (...) {                              \
      return NanThrowError("unhandled error"); \
  }

#define NODEMAGICK_SCOPE_ARGC() \
  NanScope();                   \
  uint32_t argc = args.Length()

#define NODEMAGICK_SCOPE_UNWRAP(Klass, var)           \
  NanScope();                                         \
  Klass *var = ObjectWrap::Unwrap<Klass>(args.This())

#define NODEMAGICK_SCOPE_UNWRAP_ARGC(Klass, var) \
  NODEMAGICK_SCOPE_UNWRAP(Klass, var);           \
  uint32_t argc = args.Length()

#endif