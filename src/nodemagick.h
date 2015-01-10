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
  extern Persistent<String> sigmaSym;
  extern Persistent<String> radiusSym;
  extern Persistent<String> gaussianSym;
  extern Persistent<String> widthSym;
  extern Persistent<String> heightSym;
  extern Persistent<String> xSym;
  extern Persistent<String> ySym;
  extern Persistent<String> sizeSym;
  extern Persistent<String> modeSym;
  extern Persistent<String> gravitySym;
  extern Persistent<String> imageSym;
  extern Persistent<String> composeSym;
  extern Persistent<String> geometrySym;

  //extern Persistent<Function> bufferFunction;

  /* this class is used to scope lock mutex (RAII) */

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

  /* this class is used to scope unlock previously locked mutex (RAII) */

  class unlock {
  private:
    uv_mutex_t *m_;

  public:
    unlock(uv_mutex_t *m) : m_(m) {
      uv_mutex_unlock(m);
    }
    ~unlock() {
      uv_mutex_lock(m_);
    }
  };

  /* this class is used to wait at the barrier after scope (RAII) */

  class wall {
  private:
    uv_barrier_t *b_;

  public:
    wall(uv_barrier_t *b) : b_(b) {}
    ~wall() {
      uv_barrier_wait(b_);
    }
  };


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