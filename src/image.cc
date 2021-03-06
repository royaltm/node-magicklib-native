#include "image.h"
#include <string.h>
#include <cstring>
#include "util.h"

#define NODEMAGICK_SCOPE_IMAGE_UNWRAP() \
  NODEMAGICK_SCOPE_UNWRAP(Image, image)

#define NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC \
  NODEMAGICK_SCOPE_IMAGE_UNWRAP();         \
  uint32_t argc = args.Length();

#define NODEMAGICK_CHECK_ASYNC_ARGS                 \
  bool callback;                                    \
  int cbargc;                                       \
  if ( argc > 0 && args[argc - 1]->IsFunction() ) { \
    cbargc = --argc;                                \
    callback = true;                                \
  } else {                                          \
    cbargc = 0;                                     \
    callback = false;                               \
  }

#define NODEMAGICK_ASYNC_CALLBACK() \
  args[cbargc].As<Function>()

#define NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC_CALLBACK \
  NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC                \
  NODEMAGICK_CHECK_ASYNC_ARGS

#define NODEMAGICK_BEGIN_IMAGE_WORKER(JobKlass, job) \
  NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC                 \
  JobKlass job;                                      \
  NODEMAGICK_CHECK_ASYNC_ARGS

#define NODEMAGICK_FINISH_IMAGE_WORKER(JobKlass, job, errorstring)   \
  do {                                                               \
    if ( ! job.IsValid() ) {                                         \
      return NanThrowTypeError(errorstring);                         \
    } else {                                                         \
      if ( callback || image->IsBatch() ) {                          \
        if ( callback ) {                                            \
          image->AsyncWork( args.This(),NODEMAGICK_ASYNC_CALLBACK(), \
                            new JobKlass(job) );                     \
        } else {                                                     \
          image->BatchPush( new JobKlass(job) );                     \
        }                                                            \
      } else {                                                       \
        NanReturnValue( image->SyncProcess( job, args.This() ) );    \
      }                                                              \
      NanReturnValue(args.This());                                   \
    }                                                                \
  } while(0)

#define NODEMAGICK_BEGIN_MUTUAL_WORKER(JobKlass, job) \
  NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC                  \
  JobKlass job(*image);                               \
  Local<Object> sourceObject;                         \
  Image *source(NULL);                                \
  NODEMAGICK_CHECK_ASYNC_ARGS

#define NODEMAGICK_UNWRAP_IMAGE_SOURCE(value) \
  do { sourceObject = UnwrapCheck<Image>(value, &source); } while(0)

#define NODEMAGICK_IMAGE_SOURCE_UNWRAPPED() \
  ( ! sourceObject.IsEmpty() )

#define NODEMAGICK_FINISH_MUTUAL_WORKER(JobKlass, job, errorstring)  \
  do {                                                               \
    if ( ! job.IsValid() ) {                                         \
      return NanThrowTypeError(errorstring);                         \
    } else if ( sourceObject.IsEmpty() ) {                           \
      return NanThrowError("required Image argument is missing");    \
    } else {                                                         \
      if ( callback || image->IsBatch() ) {                          \
        JobKlass *_ ## job = new JobKlass(job);                      \
        ImageSynchronizeJob *_sync ## job =                          \
                                 _ ## job->SetupSynchronizeJob();    \
        if ( callback ) {                                            \
          image->AsyncWork( args.This(),NODEMAGICK_ASYNC_CALLBACK(), \
                            _ ## job );                              \
        } else {                                                     \
          image->BatchPush( _ ## job );                              \
        }                                                            \
        source->AsyncWork( sourceObject, _sync ## job );             \
      } else {                                                       \
        NanReturnValue( image->SyncProcess( job, args.This() ) );    \
      }                                                              \
      NanReturnValue(args.This());                                   \
    }                                                                \
  } while(0)

#define NODEMAGICK_COLOR_FROM_VALUE(color, value)         \
  Magick::Color color;                                    \
  do {                                                    \
    const char *errmsg(NULL);                             \
    if ( ! SetColorFromV8Value(color, (value), &errmsg) ) \
      return NanThrowError(errmsg);                       \
  } while(0)


namespace NodeMagick {

  using namespace std;

  Persistent<FunctionTemplate> Image::constructor;

  void Image::Init(Handle<Object> exports) {
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
    NanAssignPersistent(constructor, tpl);
    Local<String> imageSym = NanNew<String>("Image");
    tpl->SetClassName(imageSym);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "begin"      , Begin);
    NODE_SET_PROTOTYPE_METHOD(tpl, "blur"       , Blur);
    NODE_SET_PROTOTYPE_METHOD(tpl, "close"      , Close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "color"      , Color);
    NODE_SET_PROTOTYPE_METHOD(tpl, "comment"    , Comment);
    NODE_SET_PROTOTYPE_METHOD(tpl, "composite"  , Composite);
    NODE_SET_PROTOTYPE_METHOD(tpl, "copy"       , Copy);
    NODE_SET_PROTOTYPE_METHOD(tpl, "crop"       , Crop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "end"        , End);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extent"     , Extent);
    NODE_SET_PROTOTYPE_METHOD(tpl, "filter"     , Filter);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flip"       , Flip);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flop"       , Flop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "format"     , Format);
    NODE_SET_PROTOTYPE_METHOD(tpl, "histogram"  , Histogram);
    NODE_SET_PROTOTYPE_METHOD(tpl, "negate"     , Negate);
    NODE_SET_PROTOTYPE_METHOD(tpl, "noise"      , Noise);
    NODE_SET_PROTOTYPE_METHOD(tpl, "normalize"  , Normalize);
    NODE_SET_PROTOTYPE_METHOD(tpl, "oil"        , Oil);
    NODE_SET_PROTOTYPE_METHOD(tpl, "properties" , Properties);
    NODE_SET_PROTOTYPE_METHOD(tpl, "ping"       , Ping);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quality"    , Quality);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quantize"   , Quantize);
    NODE_SET_PROTOTYPE_METHOD(tpl, "read"       , Read);
    NODE_SET_PROTOTYPE_METHOD(tpl, "reset"      , Reset);
    NODE_SET_PROTOTYPE_METHOD(tpl, "resize"     , Resize);
    NODE_SET_PROTOTYPE_METHOD(tpl, "restore"    , Restore);
    NODE_SET_PROTOTYPE_METHOD(tpl, "rotate"     , Rotate);
    NODE_SET_PROTOTYPE_METHOD(tpl, "sharpen"    , Sharpen);
    NODE_SET_PROTOTYPE_METHOD(tpl, "size"       , Size);
    NODE_SET_PROTOTYPE_METHOD(tpl, "strip"      , Strip);
    NODE_SET_PROTOTYPE_METHOD(tpl, "trim"       , Trim);
    NODE_SET_PROTOTYPE_METHOD(tpl, "type"       , Type);
    NODE_SET_PROTOTYPE_METHOD(tpl, "write"      , Write);
    NODE_SET_PROTOTYPE_METHOD(tpl, "_hold"      , UnderscoreHold);
    NODE_SET_PROTOTYPE_METHOD(tpl, "_wrap"      , UnderscoreWrap);

    Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
    proto->SetAccessor(NanNew<String>("busy")       , GetIsBusy);
    proto->SetAccessor(NanNew<String>("empty")      , GetIsEmpty);
    proto->SetAccessor(NanNew<String>("batchSize")  , GetBatchSize);
    proto->SetAccessor(NanNew<String>("isSync")     , GetIsSync);

    Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
    i_t->SetAccessor(NanNew<String>("batch")     , GetBatch, SetBatch);
    i_t->SetAccessor(NanNew<String>("autoClose") , GetAutoClose, SetAutoClose);
    i_t->SetAccessor(NanNew<String>("autoCopy")  , GetAutoCopy, SetAutoCopy);
    i_t->SetAccessor(NanNew<String>("columns")   , GetColumns, SetColumns);
    i_t->SetAccessor(NanNew<String>("rows")      , GetRows, SetRows);
    i_t->SetAccessor(NanNew<String>("magick")    , GetMagick, SetMagick);
    i_t->SetAccessor(NanNew<String>("page")      , GetPage, SetPage);
    i_t->SetAccessor(NanNew<String>("background"), GetBackground, SetBackground);
    i_t->SetAccessor(NanNew<String>("fuzz")      , GetFuzz, SetFuzz);
    exports->Set(imageSym, NanNew<FunctionTemplate>(constructor)->GetFunction());
  }

  Image::Image(void)
    : magickimage(NULL), magickcopy(NULL), isCloned(false), autoCopy(false), autoClose(true) {
      uv_mutex_init(&imagemutex);
    }
  Image::Image(const Image& image_)
    : magickcopy(NULL), isCloned(false), autoCopy(false), autoClose(true) {
    uv_mutex_init(&imagemutex);
    const Magick::Image * mimage( image_.magickimage.get() );
    if ( mimage != NULL ) {
      magickimage.reset( new Magick::Image( *mimage ) );
    } else {
      magickimage.reset( new Magick::Image() );
    }
    autoCopy  = image_.autoCopy;
    autoClose = image_.autoClose;
    IsPersistentBatch( image_.IsPersistentBatch() );
  }
  Image::Image(const char *filepath)
    : magickimage( new Magick::Image(filepath) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }
  Image::Image(const Magick::Blob &blob)
    : magickimage( new Magick::Image(blob) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }
  Image::Image(const char *geometry, const char *color)
    : magickimage( new Magick::Image(geometry, color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) {
    magickimage->backgroundColor( color );
    uv_mutex_init(&imagemutex);
  }
  Image::Image(const char *geometry, const Magick::Color &color)
    : magickimage( new Magick::Image(geometry, color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) {
    magickimage->backgroundColor( color );
    uv_mutex_init(&imagemutex);
  }
  Image::Image(size_t width, size_t height, const char *color)
    : magickimage( new Magick::Image(Magick::Geometry(width, height), color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) {
    magickimage->backgroundColor( color );
    uv_mutex_init(&imagemutex);
  }
  Image::Image(size_t width, size_t height, const Magick::Color &color)
    : magickimage( new Magick::Image(Magick::Geometry(width, height), color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) {
    magickimage->backgroundColor( color );
    uv_mutex_init(&imagemutex);
  }

  Image::~Image(void) {
    // printf("image dtor\n");
    uv_mutex_destroy(&imagemutex);
  }


  /* check if image is empty */
  bool Image::IsEmpty(void) {
    lock _milock(&imagemutex);
    return magickimage.get() == NULL;
  }

  /* make internal copy of magickimage */
  void Image::CloneImage(void) {
    if ( isCloned ) return;
    const Magick::Image * mimage( magickimage.get() );
    magickcopy = magickimage; /* image transfered not copied (auto_ptr) */

    isCloned = true;

    if ( mimage != NULL )
      magickimage.reset( new Magick::Image( *mimage ) );
    else
      magickimage.reset( new Magick::Image() );
  }

  /* restore internal magickimage */
  void Image::RestoreImage(void) {
    if ( ! isCloned ) return;
    magickimage = magickcopy; /* image transfered not copied (auto_ptr) */

    isCloned = false;
  }

  /* dispose any images  */
  void Image::CloseImage(void) {
    magickimage.reset();
    if ( isCloned ) {
      magickcopy.reset();
      isCloned = false;
    }
  }

  /* get or (lazy) create magickimage */
  Magick::Image *Image::GetMagickImage(void) {
    Magick::Image *mi( magickimage.get() );
    if ( mi != NULL )
      return mi;
    magickimage.reset( mi = new Magick::Image() );
    return mi;
  }

  /* run on main or async thread */
  NAN_INLINE void Image::ProcessJob(ImageProcessJob &job) {
    lock _milock(&imagemutex);
    if ( autoCopy && ! isCloned && ! job.DontCopy() )
      CloneImage();
    job.ProcessImage( this );
  }

  NAN_INLINE Local<Value> Image::NewImageCopyObjectV8() {
    NanEscapableScope();
    Local<Object> newImage = NanNew<FunctionTemplate>(Image::constructor)->GetFunction()->NewInstance();
    Image *image = ObjectWrap::Unwrap<Image>( newImage );
    image->magickimage = magickimage; /* image transfered not copied (auto_ptr) */
    image->autoCopy = autoCopy;
    image->autoClose = autoClose;
    image->IsPersistentBatch( IsPersistentBatch() );
    RestoreImage();
    return NanEscapeScope( newImage );
  }

  /* this is run on main and there are no async threads */
  NAN_INLINE Local<Value> Image::ReturnedValue(void) {
    if ( isCloned ) {
      return NewImageCopyObjectV8();
    }
    return Worker<ImageProcessJob>::ReturnedValue();
  }

  /* this is run on main and there are no async threads */
  NAN_INLINE Local<Value> Image::ReturnedValue(Local<Object> self) {
    if ( isCloned ) {
      return NewImageCopyObjectV8();
    }
    return Worker<ImageProcessJob>::ReturnedValue(self);
  }

  /* this is run on main and there are no async threads */
  NAN_INLINE void Image::JobAfterComplete(bool isAsync) {
    if ( autoClose && isAsync )
      CloseImage();
    else if ( isCloned )
      RestoreImage();
  }

  /**
   * Image object factory
   *
   * new Image([options])
   * new Image(buffer)
   * new Image(geometry, color)
   * new Image(width, height[, color="transparent"])
   * new Image(file)
   * new Image(image)
   *
   * buffer:      data Buffer to synchronously load from
   * geometry:    geometry string
   * color:       color string or Color object instance
   * image:       Image object to clone
   * file:        file name to synchronously load from
   *              (not recommended, uses Magick++ blocking I/O)
   *
   * options:
   *
   *   - src:        image data Buffer
   *   - columns:    number of columns (width)
   *   - rows:       number of rows (height)
   *   - color:      new image color string or Color object, default: transparent
   *   - magick:     set format, default: Magick++ default
   *   - page:       page description string
   *   - batch:      set persistent batch mode, default: false
   *   - autoClose:  set auto close mode, default: true
   *   - autoCopy:   set auto copy mode, default: false
   *   - background: set background color, default: color
   *   - fuzz:       set color fuzz, default: 0
   *   - type:       set image type, e.g. "truecolor"
   **/
  NAN_METHOD(Image::New) {
    NODEMAGICK_SCOPE_ARGC();

    if (args.IsConstructCall()) {

      Image* image;

      try {
        if (argc >= 2) {
          if ( args[0]->IsString() ) {
            NanUtf8String size(args[0]);
            if ( args[1]->IsString() ) {
              NanUtf8String color(args[1]);
              image = new Image(*size, *color);
            } else if ( NODEMAGICK_VALUE_IS_COLOR(args[1]) ) {
              const Magick::Color &color = GetColorFromV8ColorObject( args[1].As<Object>() );
              image = new Image(*size, color);
            } else {
              image = new Image(*size, "transparent");
            }

          } else {
            size_t width = args[0]->Uint32Value();
            size_t height = args[1]->Uint32Value();
            if ( width == 0 || height == 0 ) {
              image = new Image();
            } else if ( args[2]->IsString() ) {
              NanUtf8String color(args[2]);
              image = new Image(width, height, *color);
            } else if ( NODEMAGICK_VALUE_IS_COLOR(args[2]) ) {
              const Magick::Color &color = GetColorFromV8ColorObject( args[2].As<Object>() );
              image = new Image(width, height, color);
            } else {
              image = new Image(width, height, "transparent");
            }
          }
        } else if (argc == 1) {
          if (args[0]->IsString()) {
            NanUtf8String file(args[0]);

            image = new Image(*file);

          } else if (Buffer::HasInstance(args[0])) {
            Local<Object> buffer( args[0].As<Object>() );
            Magick::Blob blob(Buffer::Data(buffer), Buffer::Length(buffer));

            image = new Image(blob);

          } else if ( args[0]->IsObject() ) {
            if ( NanHasInstance(constructor, args[0]) ) {

              image = ObjectWrap::Unwrap<Image>( args[0].As<Object>() );

              if ( image->IsBusy() ) {
                return NanThrowTypeError("image is busy and could not be cloned synchronously");
              }
 
              image = new Image( *image );
 
            } else {
              const Handle<Object> &properties = args[0].As<Object>();
              if ( properties->Has(NanNew(srcSym)) ) {
                Local<Value> src = properties->Get(NanNew(srcSym));
                if ( Buffer::HasInstance(src) ) {
                  Local<Object> buffer( src.As<Object>() );
                  Magick::Blob blob(Buffer::Data(buffer), Buffer::Length(buffer));

                  image = new Image(blob);

                } else if ( src->IsString() ) {
                  NanUtf8String file(src);

                  image = new Image(*file);

                } else {
                  return NanThrowError("src option should be a Buffer or string");
                }
              } else {
                size_t width  = properties->Get(NanNew(columnsSym))->Uint32Value();
                size_t height = properties->Get(NanNew(rowsSym))->Uint32Value();
                if ( width == 0 || height == 0 ) {
                  image = new Image();
                } else {
                  Local<Value> colorValue = properties->Get(NanNew(colorSym));
                  if ( colorValue->IsString() ) {
                    NanUtf8String color(colorValue);
                    image = new Image(width, height, *color);
                  } else if ( NODEMAGICK_VALUE_IS_COLOR(colorValue) ) {
                    const Magick::Color &color = GetColorFromV8ColorObject( colorValue.As<Object>() );
                    image = new Image(width, height, color);
                  } else {
                    image = new Image(width, height, "transparent");
                  }
                }
              }
              if ( properties->Has(NanNew(batchSym)) )
                image->IsPersistentBatch( properties->Get(NanNew(batchSym))->BooleanValue() );
              if ( properties->Has(NanNew(autoCopySym)) )
                image->autoCopy = properties->Get(NanNew(autoCopySym))->BooleanValue();
              if ( properties->Has(NanNew(autoCloseSym)) )
                image->autoClose = properties->Get(NanNew(autoCloseSym))->BooleanValue();
              if ( properties->Has(NanNew(magickSym)) )
                image->GetMagickImage()->magick( *NanUtf8String( properties->Get(NanNew(magickSym)) ) );
              if ( properties->Has(NanNew(pageSym)) )
                image->GetMagickImage()->page( *NanUtf8String( properties->Get(NanNew(pageSym)) ) );
              if ( properties->Has(NanNew(backgroundSym)) ) {
                NODEMAGICK_COLOR_FROM_VALUE( bgcolor, properties->Get(NanNew(backgroundSym)) );
                image->GetMagickImage()->backgroundColor( bgcolor );
              }
              if ( properties->Has(NanNew(fuzzSym)) )
                image->GetMagickImage()->colorFuzz( properties->Get(NanNew(fuzzSym))->NumberValue() );
              if ( properties->Has(NanNew(typeSym)) ) {
                ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickTypeOptions, Magick::MagickFalse,
                                *NanUtf8String(properties->Get(NanNew(typeSym))));
                if ( type != -1 ) {
                  image->GetMagickImage()->type( (Magick::ImageType) type );
                } else {
                  throw ImageTypeException();
                }
              }
            }
          } else {
            image = new Image();
          }
        } else {
          image = new Image();
        }
      } catch (exception& err) {
          return NanThrowError(err.what());
      } catch (...) {
          return NanThrowError("unhandled error");
      }

      Local<Object> self = args.This();

      image->Wrap(self);

      NanReturnValue(self);
    } else {
      vector<Local<Value> > argv;
      for (uint32_t i = 0; i < argc; ++i) {
        argv.push_back(args[i]);
      }
      NanReturnValue(NanNew<FunctionTemplate>(constructor)->GetFunction()->NewInstance(argc, &argv[0]));
    }
  }

  /*

    Image Worker Methods 

  */

  /**
   * Begin batch mode
   *
   * image.begin(batch)
   * image.begin(options)
   *
   * options:
   *
   *   - batch: set persistent batch mode, default: false
   *
   * if batch or options argument is provided this function
   * immediately changes batch image property.
   **/
  NAN_METHOD(Image::Begin) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC
    if ( argc >= 1 ) {
      bool batch;
      if ( args[0]->IsObject() ) {
        batch = NanBooleanOptionValue(args[0].As<Object>(), NanNew(batchSym));
      } else
        batch = args[0]->BooleanValue();
      image->SetupBatch(batch);
    } else
      image->SetupBatch();
    NanReturnValue(args.This());
  }

  /**
   * Blur image
   *
   * image.blur(sigma[, radius][, channel][, gaussian][, callback(err, image)])
   * image.blur(options[, callback(err, image)])
   *
   * options:
   *
   *   - sigma: standard deviation of the Laplacian or the Gaussian bell curve
   *   - radius: the radius of the Gaussian, in pixels or number of neighbor
   *             pixels to be included in the convolution mask
   *   - channel: channel type string, e.g.: "yellow"
   *   - gaussian: true for gaussian blur
   *
   **/
  NAN_METHOD(Image::Blur) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageBlurJob, blur)

    try {
      if ( argc == 1 && args[0]->IsObject() ) {

        blur.Setup( args[0].As<Object>() );

      } else if ( argc >= 1 && argc <= 4 ) {

        double sigma( args[0]->NumberValue() );
        double radius(0.0);
        NanUtf8String *channel(NULL);
        bool gaussian(false);
        for( uint32_t i = 1; i < argc; ++i ) {
          if ( args[i]->IsString() ) {
            channel = new NanUtf8String(args[i]);
          } else if (args[i]->IsBoolean() ) {
            gaussian = args[i]->BooleanValue() ? 1 : 0;
          } else {
            radius = args[i]->NumberValue();
          }
        }

        if ( isnan(sigma) )
          throw ImageOptionsException("blur()'s sigma is not a number");
        blur.Setup(sigma, radius, channel, gaussian);

      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageBlurJob, blur, "blur()'s arguments should be number(s)[, string][, boolean]");
  }

  /**
   * Close image
   *
   * image.close([callback(err, image)])
   * image.close(autoClose) -> image
   * image.close(options) -> image
   * 
   * options:
   *
   *   - autoClose: set auto close mode, default: true
   *
   * free magick image resource (even if image.autoCopy==true)
   *
   * if autoClose or options argument is provided this function
   * immediately changes autoClose image property.
   **/
  NAN_METHOD(Image::Close) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageCloseJob, closejob)

    if ( argc >= 1 ) {
      if ( args[0]->IsObject() ) {
        image->autoClose = NanBooleanOptionValue(args[0].As<Object>(), NanNew(autoCloseSym), true);
        NanReturnValue( args.This() );
      } else if ( args[0]->IsBoolean() ) {
        image->autoClose = args[0]->BooleanValue();
        NanReturnValue( args.This() );
      }
    } else if (argc == 0) {
      closejob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageCloseJob, closejob, "close()'s 1st argument should be boolean or Object");
  }

  /**
   * Color pixel plot or peek
   *
   * in synchronous context:
   * image.color(x, y) -> color
   * image.color(points) -> colors
   * image.color(x, y, newcolor) -> image
   * image.color(options) -> image
   *
   * in asynchronous context:
   * image.color(x, y, callback(err, color))
   * image.color(points, callback(err, colors))
   * image.color(x, y, newcolor, callback(err, image))
   * image.color(options, callback(err, image))
   *
   * points: an Array of [x, y] tuples
   * newcolor: string or Color object
   * color: Color object
   * colors: an Array of Color objects
   *
   * options:
   *   - points: an Array of [x, y] tuples
   *   or
   *   - x: x offset number
   *   - y: y offset number
   *   - color:  string or Color object
   **/
  NAN_METHOD(Image::Color) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageColorJob, colorjob)

    try {
      if ( argc == 1 ) {
        if ( args[0]->IsArray() ) {

          colorjob.Setup( args[0].As<Array>() );

        } else if ( args[0]->IsObject() ) {

          colorjob.Setup( args[0].As<Object>() );

        }
      } else if ( argc >= 2 && argc <= 3 ) {

        ssize_t x = args[0]->Int32Value();
        ssize_t y = args[1]->Int32Value();

        if ( argc == 2 ) {
          colorjob.Setup(x, y);
        } else {
          NODEMAGICK_COLOR_FROM_VALUE(color, args[2]);
          colorjob.Setup(x, y, color);
        }
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageColorJob, colorjob, "color()'s arguments should be Array or Numbers and Color");
  }

  /**
   * Comment image
   *
   * image.comment(comment[,callback(err, image)])
   * image.comment([callback(err, comment)])
   **/
  NAN_METHOD(Image::Comment) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageCommentJob, commentjob)

    if ( argc >= 1 ) {
      if ( args[0]->IsString() )
        commentjob.Setup( new NanUtf8String(args[0]) );
    } else if (argc == 0) {
      commentjob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageCommentJob, commentjob, "comment()'s 1st argument should be a string");
  }

  /**
   * Composite images
   *
   * image.composite(composeImage[, gravity="center"][, compose="over"][, callback(err, image)])
   * image.composite(composeImage[, geometry][, compose="over"][, callback(err, image)])
   * image.composite(composeImage[, x, y][, compose="over"][, callback(err, image)])
   * image.composite(options[, callback(err, image)])
   *
   * options:
   *
   *   - image: an image to compose onto image
   *   - gravity: gravity string, e.g: "southwest"
   *   - geometry: geometry [_, _, x, y] or string, e.g: "+100-50"
   *   - x: x offset
   *   - y: y offset
   *   - compose: compose string, "multiply"
   **/
  NAN_METHOD(Image::Composite) {
    NODEMAGICK_BEGIN_MUTUAL_WORKER(ImageCompositeJob, compositejob)

    try {
      if ( argc >= 1 && args[0]->IsObject() ) {
        NODEMAGICK_UNWRAP_IMAGE_SOURCE(args[0]);
        if ( NODEMAGICK_IMAGE_SOURCE_UNWRAPPED() ) {
          if ( argc == 1) {
            compositejob.Setup(source, NULL, Magick::CenterGravity);
          } if ( argc >= 2 && args[1]->IsArray() ) {
            Magick::Geometry geometry;
            SetGeometryFromV8Array( geometry, args[1].As<Array>() );
            if ( argc == 3 && args[2]->IsString() ) {
              compositejob.Setup(source, new NanUtf8String( args[2] ), geometry);
            } else if ( argc == 2 ) {
              compositejob.Setup(source, NULL, geometry);
            }
          } else if ( argc == 2 && args[1]->IsString() ) {
            auto_ptr<NanUtf8String> compose( new NanUtf8String( args[1] ) );
            Magick::GravityType gravity( GetGravityFromString(**compose) );
            if ( gravity != Magick::ForgetGravity ) {
              compositejob.Setup(source, NULL, gravity);
            } else {
              Magick::Geometry geometry( **compose );
              if ( geometry.isValid() ) {
                geometry.xNegative(false);
                geometry.yNegative(false);
                compositejob.Setup(source, NULL, geometry);
              } else {
                compositejob.Setup(source, compose.release(), Magick::CenterGravity);
              }
            }
          } else if ( argc >= 3 ) {
            if ( args[1]->IsNumber() && args[2]->IsNumber() ) {
              ssize_t x = args[1]->Int32Value(),
                      y = args[2]->Int32Value();
              if ( argc == 4 && args[3]->IsString() ) {
                compositejob.Setup(source, new NanUtf8String( args[3] ), x, y);
              } else if ( argc == 3 ) {
                compositejob.Setup(source, NULL, x, y);
              }
            } else if ( argc == 3 && args[1]->IsString() ) {
              NanUtf8String string( args[1] );
              Magick::GravityType gravity( GetGravityFromString(*string) );
              if ( gravity == Magick::ForgetGravity ) {
                Magick::Geometry geometry( *string );
                if ( geometry.isValid() ) {
                  geometry.xNegative(false);
                  geometry.yNegative(false);
                  compositejob.Setup(source, new NanUtf8String( args[2] ), geometry);
                }
              } else {
                compositejob.Setup(source, new NanUtf8String( args[2] ), gravity);
              }
            }
          }
        } else if ( argc == 1 ) {

          source = compositejob.Setup( args[0].As<Object>(), sourceObject );

        }
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_MUTUAL_WORKER(ImageCompositeJob, compositejob, "composite()'s arguments should be Image, strings and numbers");
  }

  /**
   * Copy image
   *
   * image.copy([callback(err, newimage)])
   * image.copy(autoCopy[, callback(err, newimage)]) -> image
   * image.copy(options[, callback(err, newimage)]) -> image
   *
   * options:
   *
   *   - autoCopy: set auto copy mode, default: false
   **/
  NAN_METHOD(Image::Copy) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageCopyJob, copyjob)

    if ( argc >= 1 ) {
      if ( args[0]->IsObject() ) {
        copyjob.Setup( args[0].As<Object>() );
      } else if ( args[0]->IsBoolean() ) {
        copyjob.Setup( args[0]->BooleanValue() );
      }
    } else if (argc == 0) {
      copyjob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageCopyJob, copyjob, "copy()'s 1st argument should be boolean or Object");
  }

  /**
   * Crop image
   *
   * image.crop(width, height, x, y[, callback(err, image)])
   * image.crop(size[, callback(err, image)])
   * image.crop(options[, callback(err, image)])
   *
   * size: an Array of [width, height, x, y] or geometry string
   *
   * options:
   *
   *   - size: crop size [witdth, height, x, y] or geometry string
   *   - width: crop width
   *   - height: crop height
   *   - x: crop column offset
   *   - y: crop row offset
   **/
  NAN_METHOD(Image::Crop) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageCropJob, cropper)

    try {
      if ( argc == 4 ) {
        if ( args[0]->IsNumber() && args[1]->IsNumber() &&
             args[2]->IsNumber() && args[3]->IsNumber() ) {
          Magick::Geometry geometry(
            args[0]->Uint32Value(),  args[1]->Uint32Value(),
            args[2]->Int32Value(), args[3]->Int32Value(),
            false, false);
          cropper.Setup(geometry);
        }
      } else if ( argc == 1 ) {
        if ( args[0]->IsString() ) {
          Magick::Geometry geometry( *NanUtf8String(args[0]) );
          geometry.xNegative(false);
          geometry.yNegative(false);
          cropper.Setup(geometry);
        } else if ( args[0]->IsArray() ) {
          Magick::Geometry geometry;
          Local<Array> size( args[0].As<Array>() );
          if ( SetGeometryFromV8Array( geometry, size ) ) {
            cropper.Setup(geometry);
          }
        } else if ( args[0]->IsObject() ) {

          cropper.Setup( args[0].As<Object>() );

        }
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageCropJob, cropper, "crop()'s arguments should be string or 4 numbers");
  }

  /**
   * End batch
   * 
   * image.end([callback(err, image)])
   * image.end(batch[, callback(err, image)])
   * image.end(options[, callback(err, image)])
   *
   * options:
   *
   *   - batch: set persistent batch mode, default: false
   *
   * if callback is provided executes batch otherwise clears batch tail from jobs
   *
   * if batch or options argument is provided this function
   * immediately changes batch image property.
   **/
  NAN_METHOD(Image::End) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC_CALLBACK

    bool setpersist(false);
    bool batch(false);

    if ( argc >= 1 ) {
      if ( args[0]->IsObject() ) {
        setpersist = true;
        batch = NanBooleanOptionValue(args[0].As<Object>(), NanNew(batchSym));
      } else if ( args[0]->IsBoolean() ) {
        setpersist = true;
        batch = args[0]->BooleanValue();
      }
    }

    if ( image->IsBatch() ) {

      if (setpersist)
        image->IsPersistentBatch(batch);

      if ( ! callback ) {

        image->CancelTailJobs();

      } else {

        image->AsyncWork( args.This(), NODEMAGICK_ASYNC_CALLBACK() );

      }
      NanReturnValue(args.This());

    }

    if ( callback ) {

      Local<Value> argv[] = { NanNull(), args.This() };
      NanMakeCallback( NanGetCurrentContext()->Global(), NODEMAGICK_ASYNC_CALLBACK(), 2, argv );

    } else {
      if ( setpersist )
        image->IsPersistentBatch(batch);
    }
    NanReturnValue(args.This());
  }

  /**
   * Extent image
   *
   * image.extent(width, height[, gravity="center"][, color=background][, callback(err, image)])
   * image.extent(width, height, x, y[, color=background][, callback(err, image)])
   * image.extent(size[, gravity="ignore"][, color=background][, callback(err, image)])
   * image.extent(options[, callback(err, image)])
   *
   * size: an Array of [width, height] or [width, height, x, y]
   *       or geometry string
   * color: string or Color object
   * gravity: gravity string, e.g.: "northwest"
   *
   * options:
   *
   *   - size: extent size [witdth, height] or geometry string
   *   - width: extent width
   *   - height: extent height
   *   - x: extent column offset
   *   - y: extent row offset
   *   - gravity: gravity string, e.g.: "east"
   *   - color: string or Color object
   **/
  NAN_METHOD(Image::Extent) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageExtentJob, extender)

    try {
      if ( argc >= 1 ) {
        Magick::GravityType gravity( Magick::ForgetGravity );
        bool ignoreGravity = false;
        Magick::Color color;
        const char *errmsg(NULL);
        if ( argc == 5 ) {
          if ( ! SetColorFromV8Value( color, args[4], &errmsg ) )
            return NanThrowError(errmsg);
          argc = 4;
        } else if ( ( argc == 3 || argc == 4 ) && args[argc - 2]->IsString() ) {
          gravity = GetGravityFromString( *NanUtf8String(args[argc - 2]) );
          if ( ! SetColorFromV8Value( color, args[argc - 1], &errmsg ) )
            return NanThrowError(errmsg);
          argc -= 2;
        } else if ( ( argc == 2 || argc == 3 ) ) {
          if ( args[argc - 1]->IsString() ) {
            NanUtf8String gravityOrColorStr( args[argc - 1] );
            gravity = GetGravityFromString( *gravityOrColorStr );
            if ( gravity == Magick::ForgetGravity ) {
              if ( strcasecmp( *gravityOrColorStr, "ignore" ) ) {
                if ( ! SetColorFromString( color, *gravityOrColorStr, &errmsg ) )
                  return NanThrowError(errmsg);
              } else {
                ignoreGravity = true;
              }
            }
            argc -= 1;
          } else if ( NODEMAGICK_VALUE_IS_COLOR(args[argc - 1]) ) {
            color = GetColorFromV8ColorObject( args[argc - 1].As<Object>() );
            argc -= 1;
          }
        }
        Magick::Geometry geometry;
        if ( argc == 1 ) {
          if ( args[0]->IsString() ) {
            geometry = *NanUtf8String(args[0]);
          } else if ( args[0]->IsArray() ) {
            SetGeometryFromV8Array( geometry, args[0].As<Array>() );
          } else if ( args[0]->IsObject() ) {
            extender.Setup( args[0].As<Object>() );
          }
        } else if ( argc == 2 || argc == 4 ) {
          if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
            geometry = Magick::Geometry(
              args[0]->Uint32Value(), args[1]->Uint32Value(), 0, 0, false, false );
            if ( argc == 4 && args[2]->IsNumber() && args[3]->IsNumber() ) {
              geometry.xOff( args[2]->Int32Value() );
              geometry.yOff( args[3]->Int32Value() );
            } else if ( !ignoreGravity && gravity == Magick::ForgetGravity ) {
              gravity = Magick::CenterGravity;
            }
          }
        }
        if ( geometry.isValid() ) {
          if ( color.isValid() ) {
            if ( gravity != Magick::ForgetGravity ) {
              extender.Setup( geometry, color, gravity );
            } else {
              extender.Setup( geometry, color );
            }
          } else {
            if ( gravity != Magick::ForgetGravity ) {
              extender.Setup( geometry, gravity );
            } else {
              extender.Setup( geometry );
            }
          }
        }
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageExtentJob, extender, "extent()'s arguments should be string or number(s)");
  }

  /**
   * Fill image
   *
   * image.fill(texture, x, y[, border][, callback(err, image)])
   * image.fill(texture, point[, border][, callback(err, image)])
   *
   * options:
   *
   *   - texture: color string, Color object instance or texture Image
   *   - point: [x, y] or string, e.g: "+100-50"
   *   - x: x offset
   *   - y: y offset
   *   - border: border color string or Color object instance
   **/
  // NAN_METHOD(Image::Fill) {

  // }

  /**
   * Filter for resize
   * 
   * image.filter(filter[, callback(err, image)])
   *
   * filter: filter string, default: "Lanczos"
   **/
  NAN_METHOD(Image::Filter) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageFilterJob, filter)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        filter.Setup( new NanUtf8String(args[0]) );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageFilterJob, filter, "filter()'s 1st argument should be string");
  }

  /**
   * Flip image
   * 
   * image.flip([callback(err, image)])
   **/
  NAN_METHOD(Image::Flip) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageFlipJob, flipper)

    if (argc == 0) {
      flipper.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageFlipJob, flipper, "flip()'s 1st argument should be callback");
  }

  /**
   * Flop image
   * 
   * image.flop([callback(err, image)])
   **/
  NAN_METHOD(Image::Flop) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageFlopJob, flopper)

    if (argc == 0) {
      flopper.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageFlopJob, flopper, "flop()'s 1st argument should be callback");
  }

  /**
   * Format change
   *
   * image.format(magick[, callback(err, image)])
   *
   * magick: format string, e.g. "JPEG"
   **/
  NAN_METHOD(Image::Format) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageFormatJob, formatter)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        formatter.Setup( new NanUtf8String(args[0]) );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageFormatJob, formatter, "format()'s 1st argument should be string");
  }

  /**
   * Histogram
   *
   * image.histogram([callback(err, histogram)])
   **/
  NAN_METHOD(Image::Histogram) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageHistogramJob, histogramjob)

    if (argc == 0) {
      histogramjob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageHistogramJob, histogramjob, "histogram()'s 1st argument should be callback");
  }

  /**
   * Negate image
   *
   * image.negate([grayscale=false][, callback(err, image)])
   **/
  NAN_METHOD(Image::Negate) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageNegateJob, negator)

    if ( argc == 0 ) {
      negator.Setup();
    } else if ( argc == 1 ) {
      negator.Setup( args[0]->BooleanValue() );
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageNegateJob, negator, "negate()'s argument should be boolean");
  }

  /**
   * Noise image
   *
   * image.noise(noise[, channel][, callback(err, image)])
   * image.noise(options[, callback(err, image)])
   *
   * options:
   *
   *   - noise: noise type string, e.g.: "multiplicative"
   *   - channel: channel type string, e.g.: "matte"
   *
   * see: http://www.imagemagick.org/Magick++/Enumerations.html#NoiseType
   *      http://www.imagemagick.org/Magick++/Enumerations.html#ChannelType
   **/
  NAN_METHOD(Image::Noise) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageNoiseJob, noisejob)

    try {
      if ( argc == 1 ) {
        if ( args[0]->IsString() ) {
          noisejob.Setup( new NanUtf8String(args[0]) );
        } else if ( args[0]->IsObject() ) {
          noisejob.Setup( args[0].As<Object>() );
        }
      } else if ( argc == 2 ) {
        if ( args[0]->IsString() && args[1]->IsString() )
          noisejob.Setup( new NanUtf8String(args[0]), new NanUtf8String(args[1]) );
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageNoiseJob, noisejob, "noise()'s arguments should be strings");
  }

  /**
   * Normalize image
   *
   * image.normalize([callback(err, image)])
   **/
  NAN_METHOD(Image::Normalize) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageNormalizeJob, normalizer)

    if (argc == 0) {
      normalizer.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageNormalizeJob, normalizer, "normalize()'s 1st argument should be callback");
  }

  /**
   * Oilpaint image
   *
   * image.oil([radius=3][, callback(err, image)])
   **/
  NAN_METHOD(Image::Oil) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageOilJob, oiljob)

    if ( argc == 0 ) {
      oiljob.Setup(3);
    } else if ( argc == 1 ) {
      oiljob.Setup( args[0]->NumberValue() );
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageOilJob, oiljob, "oil()'s 1st argument should be number");
  }

  /**
   * Ping image
   *
   * image.ping(file|buffer[, callback(err, size)])
   *
   * buffer: a Buffer object to read image from (recommended)
   * file: a file to read from (not recommended, uses Magick++ blocking I/O)
   * size: an Array of [width, height]
   *
   * todo: implement async read for file
   **/
  NAN_METHOD(Image::Ping) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImagePingJob, pinger)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        pinger.Setup( new NanUtf8String(args[0]) );
      } else if ( Buffer::HasInstance(args[0]) ) {
        Local<Object> buffer( args[0]->ToObject() );
        image->SaveObject( buffer );
        pinger.Setup( Buffer::Data(buffer), Buffer::Length(buffer) );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImagePingJob, pinger, "ping()'s 1st argument should be string or Buffer");
  }

  /**
   * Properties set or get
   *
   * in synchronous context:
   * image.properties() -> properties
   * image.properties(options)
   *
   * in asynchronous context:
   * image.properties(callback(err, properties))
   * image.properties(options, callback(err, image))
   *
   * properties: an Object with image properties
   *
   * options:
   *
   *   - columns:    number of columns (width)
   *   - rows:       number of rows (height)
   *   - magick:     set format
   *   - page:       page description string
   *   - batch:      set persistent batch mode
   *   - autoClose:  set auto close mode
   *   - autoCopy:   set auto copy mode
   *   - background: set background color
   *   - fuzz:       set color fuzz
   **/
  NAN_METHOD(Image::Properties) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImagePropertiesJob, propjob)

    try {
      if ( argc == 0 ) {
        propjob.Setup(true);
      } else if ( argc == 1 && args[0]->IsObject() ) {
        propjob.Setup( args[0].As<Object>() );
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImagePropertiesJob, propjob, "properties()'s 1st argument should be an Object");
  }

  /**
   * Quality change
   *
   * image.quality(quality[, callback(err, image)])
   *
   * quality: a number 0 - 100
   **/
  NAN_METHOD(Image::Quality) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageQualityJob, quality)

    if ( argc == 1 ) {
      if ( args[0]->IsUint32() ) {
        quality.Setup( args[0]->Uint32Value() );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageQualityJob, quality, "quality()'s arguments should be a number");
  }

  /**
   * Quantize image
   *
   * image.quantize([colors][, colorspace][, dither][, callback(err, image)])
   * image.quantize(options, callback(err, image)])
   *
   * options:
   *
   *   - colors: a number of quantized colors
   *   - colorspace: a colorspace string
   *   - dither: true or false
   **/
  NAN_METHOD(Image::Quantize) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageQuantizeJob, quantizer)

    try {
      if ( argc == 1 && args[0]->IsObject() ) {
        quantizer.Setup( args[0].As<Object>() );
      } else if ( argc <= 3 ) {
        ssize_t colors(0);
        char dither (-1);
        NanUtf8String *colorSpace(NULL);
        for( uint32_t i = 0; i < argc; ++i ) {
          if ( args[i]->IsString() ) {
            colorSpace = new NanUtf8String(args[i]);
          } else if (args[i]->IsBoolean() ) {
            dither = args[i]->BooleanValue() ? 1 : 0;
          } else {
            colors = (ssize_t) args[i]->IntegerValue();
          }
        }
        if ( colors < 0 )
          return NanThrowTypeError("number of colors should be > 0");
        quantizer.Setup( (size_t)colors, colorSpace, dither );
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageQuantizeJob, quantizer, "quantize()'s arguments should be number, string or boolean");
  }

  /**
   * Read image
   *
   * image.read(file|buffer[, callback(err, image)])
   *
   * buffer: a Buffer object to read image from (recommended)
   * file: a file to read from (not recommended, uses Magick++ blocking I/O)
   *
   * todo: implement async read for file
   **/
  NAN_METHOD(Image::Read) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageReadJob, reader)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        reader.Setup( new NanUtf8String(args[0]) );
      } else if ( Buffer::HasInstance(args[0]) ) {
        Local<Object> buffer( args[0]->ToObject() );
        image->SaveObject( buffer );
        reader.Setup( Buffer::Data(buffer), Buffer::Length(buffer) );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageReadJob, reader, "read()'s 1st argument should be string or Buffer");
  }

  /**
   * Reset image page
   *
   * image.reset([callback])
   * image.reset(page[, callback])
   * image.reset(width, height[, x, y][, callback(err, image)])
   *
   * page: geometry string or page description, e.g. "A4"
   **/
  NAN_METHOD(Image::Reset) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageResetJob, resetter)

    if ( argc >= 2 ) {
      if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
        Magick::Geometry geometry( args[0]->Int32Value(), args[1]->Int32Value(), 0, 0, false, false );
        if ( argc == 4 ) {
          if ( args[2]->IsNumber() && args[3]->IsNumber() ) {
            geometry.xOff( args[2]->Int32Value() );
            geometry.yOff( args[3]->Int32Value() );
            resetter.Setup(geometry);
          }
        } else if ( argc == 2 ) {
          resetter.Setup(geometry);
        }
      }
    } else if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        Magick::Geometry geometry( *NanUtf8String(args[0]) );
        resetter.Setup(geometry);
      }
    } else if ( argc == 0 ) {
      Magick::Geometry geometry;
      resetter.Setup(geometry);
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageResetJob, resetter, "reset()'s arguments should be string, number(s) or callback");
  }

  /**
   * Resize image
   *
   * image.resize(width, height[, mode="aspectfit"][, callback(err, image)])
   * image.resize(size[, mode="aspectfit"][, callback(err, image)])
   * image.resize(options[, callback(err, image)])
   *
   * size: an Array of [width, height] or geometry string, e.g.: "100x100^"
   *
   * mode:
   *   - "#" or "aspectfit"          Resize the image based on the largest fitting dimenstion (default)
   *   - "^" or "aspectfill"         Resize the image based on the smallest fitting dimension
   *   - "!" or "noaspect" or "fill" Force exact size
   *   - ">" or "larger"             Resize only if larger than geometry
   *   - "!>" or "nolarger"          Do not resize only if smaller than geometry
   *   - "<" or "smaller"            Resize only if smaller than geometry
   *   - "!<" or "nosmaller"         Do not resize only if larger than geometry
   *   - "%" or "percent"            Interpret width & height as percentages
   *   - "!%" or "nopercent"         Do not interpret width & height as percentages
   *   - "@" or "limit"              Resize using a pixel area count limit
   *   - "!@" or "nolimit"           Do not resize using a pixel area count limit
   *   - "filter"                    Use resize filter (default)
   *   - "sample"                    Use pixel sampling algorithm (ignore filters)
   *   - "scale"                     Use simple ratio algorithm (ignore filters)
   *
   * mode tags can be combined (separated by space, comma or (semi)colon),
   *      e.g.: "scale aspectfill smaller"
   *
   * options:
   *
   *   - size: target size [witdth, height] or geometry string
   *   - width: target width
   *   - height: target height
   *   - mode: resize mode
   **/
  NAN_METHOD(Image::Resize) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageResizeJob, resizer)

    try {
      if ( argc >= 1 && argc <= 3 ) {
        int modeargindex = -1;
        if ( argc >= 2 ) {
          if ( args[argc - 1]->IsString() )
            modeargindex = --argc;
        }
        if ( argc == 2 ) {
          if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
            Magick::Geometry geometry( args[0]->Uint32Value(), args[1]->Uint32Value() );
            if ( modeargindex == 2 ) {
              resizer.Setup( geometry, *NanUtf8String( args[2] ) );
            } else
              resizer.Setup( geometry );
          }
        } else if ( argc == 1 ) {
          if ( args[0]->IsString() ) {
            Magick::Geometry geometry(*NanUtf8String(args[0]));
            if ( geometry.isValid() ) {
              if ( modeargindex == 1 ) {
                resizer.Setup( geometry, *NanUtf8String( args[1] ) );
              } else
                resizer.Setup( geometry );
            }
          } else if ( args[0]->IsArray() ) {
            Magick::Geometry geometry;
            Local<Array> size( args[0].As<Array>() );
            if ( SetGeometryFromV8Array( geometry, size ) ) {
              if ( modeargindex == 1 ) {
                resizer.Setup( geometry, *NanUtf8String( args[1] ) );
              } else
                resizer.Setup( geometry );
            }
          } else if ( args[0]->IsObject() ) {
            resizer.Setup( args[0].As<Object>() );
          }
        }
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageResizeJob, resizer, "resize()'s arguments should be string or numbers");
  }

  /**
   * Restore image
   *
   * image.restore([callback(err, image)])
   **/
  NAN_METHOD(Image::Restore) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageRestoreJob, restore)

    if (argc == 0) {
      restore.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageRestoreJob, restore, "restore()'s 1st argument should be callback");
  }

  /**
   * Rotate image
   *
   * image.rotate(degrees[, callback(err, image)])
   *
   * degrees: a number 0 - 360
   **/
  NAN_METHOD(Image::Rotate) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageRotateJob, rotator)

    if ( argc == 1 ) {
      if ( args[0]->IsNumber() ) {
        rotator.Setup( args[0]->NumberValue() );
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageRotateJob, rotator, "rotate()'s arguments should be a number");
  }

  /**
   * Sharpen image
   *
   * image.sharpen(sigma[, radius][, channel][, callback(err, image)])
   * image.sharpen(options[, callback(err, image)])
   *
   * options:
   *
   *   - sigma: standard deviation of the Laplacian
   *   - radius: the radius of the Gaussian, in pixels
   *   - channel: channel type string, e.g.: "blue"
   *
   **/
  NAN_METHOD(Image::Sharpen) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageSharpenJob, sharpener)

    try {
      if ( argc == 1 && args[0]->IsObject() ) {
        sharpener.Setup( args[0].As<Object>() );
      } else if ( argc >= 1 && argc <= 3 ) {
        double sigma( args[0]->NumberValue() );
        double radius(0.0);
        NanUtf8String *channel(NULL);
        for( uint32_t i = 1; i < argc; ++i ) {
          if ( args[i]->IsString() ) {
            channel = new NanUtf8String(args[i]);
          } else {
            radius = args[i]->NumberValue();
          }
        }
        if ( isnan(sigma) )
          throw ImageOptionsException("sharpen()'s sigma is not a number");
        sharpener.Setup(sigma, radius, channel);
      }
    } catch (ImageOptionsException& err) {
        return NanThrowTypeError(err.what());
    } catch (exception& err) {
        return NanThrowError(err.what());
    } catch (...) {
        return NanThrowError("unhandled error");
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageSharpenJob, sharpener, "sharpen()'s arguments should be number(s)[, string]");
  }

  /**
   * Size get or set
   *
   * in synchronous context:
   * image.size() -> size
   * image.size(size)
   * image.size(width, height)
   *
   * in asynchronous context:
   * image.size(callback(err, size))
   * image.size(size, callback(err, image))
   * image.size(width, height, callback(err, image))
   *
   * size: an Array of [width, height]
   **/
  NAN_METHOD(Image::Size) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageSizeJob, sizer)

    if ( argc == 0 ) {

      sizer.Setup();

    } else if ( argc == 2 ) {
      if ( args[0]->IsUint32() && args[1]->IsUint32() ) {
        Magick::Geometry geometry( args[0]->Uint32Value(), args[1]->Uint32Value() );
        sizer.Setup(geometry);
      }
    } else if ( argc == 1 ) {
      if ( args[0]->IsArray() ) {
        Magick::Geometry geometry;
        Local<Array> size( args[0].As<Array>() );
        if ( SetGeometryFromV8Array( geometry, size) ) {
          sizer.Setup(geometry);
        }
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageSizeJob, sizer, "size()'s arguments should be an Array(2) or 2 Numbers");
  }

  /**
   * Strip image of profiles and comments
   *
   * image.strip([callback(err, image)])
   **/
  NAN_METHOD(Image::Strip) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageStripJob, stripper)

    if (argc == 0) {
      stripper.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageStripJob, stripper, "strip()'s 1st argument should be callback");
  }

  /**
   * Trim image
   *
   * image.trim([fuzz][, callback(err, image)])
   *
   * fuzz: a percent string "50%" or a number 0.0 - 1.0
   *       colors within fuzz distance are considered equal
   **/
  NAN_METHOD(Image::Trim) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageTrimJob, trimmer)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        NanUtf8String percent(args[0]);
        char *endp;
        double fuzz = strtod(*percent, &endp);
        if (endp[0] == '%')
          fuzz /= 100.0;
        trimmer.Setup( fuzz * (double) (1L << MAGICKCORE_QUANTUM_DEPTH) );
      } else if ( args[0]->IsNumber() ) {
        trimmer.Setup( args[0]->NumberValue() * (double) (1L << MAGICKCORE_QUANTUM_DEPTH) );
      }
    } else if ( argc == 0 ){
      trimmer.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageTrimJob, trimmer, "trim()'s 1st argument should be string, number or Function");
  }

  /**
   * Type image
   *
   * image.type([callback(err, imageType)])
   * image.type(imageType[, callback(err, image)])
   *
   * imageType: an image type string, e.g. "greyscale", "palette", "truecolor"
   * see: http://www.imagemagick.org/Magick++/Enumerations.html#ImageType
   **/
  NAN_METHOD(Image::Type) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageTypeJob, typejob)

    if ( argc >= 1 ) {
      if ( args[0]->IsString() )
        typejob.Setup( new NanUtf8String(args[0]) );
    } else if (argc == 0) {
      typejob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageTypeJob, typejob, "type()'s 1st argument should be string");
  }

  /**
   * Write image
   *
   * image.write(file[, callback(err, image)])
   * image.write([callback(err, buffer)])
   *
   * file: a file to write to (not recommended, uses Magick++ blocking I/O)
   *
   * without file or if file == null the result is buffer
   *
   * todo: implement async write to file
   **/
  NAN_METHOD(Image::Write) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageWriteJob, writer)

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        writer.Setup( new NanUtf8String(args[0]) );
      } else if ( args[0]->IsNull() || args[0]->IsUndefined() ) {
        writer.Setup();
      }
    } else if ( argc == 0 ) {
      writer.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageWriteJob, writer, "write()'s 1st argument should be string, Null or Function");
  }

  /**
   * Image properties
   *
   * image.busy -> true if asynchronous operation in progress
   */
  NAN_GETTER(Image::GetIsBusy) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    NanReturnValue( NanNew<Boolean>( image->IsBusy() ) );
  }

  /**
   * image.empty -> true if image is initialized as empty or closed
   */
  NAN_GETTER(Image::GetIsEmpty) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    NanReturnValue( NanNew<Boolean>( image->IsEmpty() ) );
  }

  /**
   * image.batchSize -> number of jobs pushed to batch
   */
  NAN_GETTER(Image::GetBatchSize) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( image->IsBatch() ) {
      NanReturnValue( NanNew<Integer>( (uint32_t) image->BatchPendingSize() ) );
    } else
      NanReturnNull();
  }

  /**
   * image.isSync -> true if image is ready for synchronous operation
   *                 (not busy and not in batch mode)
   */
  NAN_GETTER(Image::GetIsSync) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    NanReturnValue( NanNew<Boolean>( ! image->IsBatch() ) );
  }

  /**
   * image.batch -> true if image in batch-only (persistent) mode
   * image.batch = boolean changes batch-only (persistent) mode state
   */
  NAN_GETTER(Image::GetBatch) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    NanReturnValue( NanNew<Boolean>( image->IsPersistentBatch() ) );
  }

  NAN_SETTER(Image::SetBatch) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    image->IsPersistentBatch( value->BooleanValue() );
  }

  /**
   * image.autoClose -> true if image in close mode
   * image.autoClose = boolean changes close mode state
   */
  NAN_GETTER(Image::GetAutoClose) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    NanReturnValue( NanNew<Boolean>( image->autoClose ) );
  }

  NAN_SETTER(Image::SetAutoClose) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    image->autoClose = value->BooleanValue();
  }

  /**
   * image.autoCopy -> true if image in copy mode
   * image.autoCopy = boolean changes copy mode state
   */
  NAN_GETTER(Image::GetAutoCopy) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<Boolean>( image->autoCopy ) );
  }

  NAN_SETTER(Image::SetAutoCopy) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    image->autoCopy = value->BooleanValue();
  }

  /**
   * image.columns -> number of columns in image
   * image.columns = number changes number of columns
   */
  NAN_GETTER(Image::GetColumns) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<Integer>( (uint32_t) image->GetMagickImage()->columns() ) );
  }

  NAN_SETTER(Image::SetColumns) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( value->IsUint32() ) {
      lock _milock(&image->imagemutex);
      Magick::Image * mi( image->GetMagickImage() );
      Magick::Geometry geometry( value->Uint32Value(), mi->rows() );
      mi->size( geometry );
    } else {
      NanThrowError("columns must be a number >= 0");
    }
  }

  /**
   * image.rows -> number of rows in image
   * image.rows = number changes number of rows
   */
  NAN_GETTER(Image::GetRows) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<Integer>( (uint32_t) image->GetMagickImage()->rows() ) );
  }

  NAN_SETTER(Image::SetRows) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( value->IsUint32() ) {
      lock _milock(&image->imagemutex);
      Magick::Image * mi( image->GetMagickImage() );
      Magick::Geometry geometry( mi->columns(), value->Uint32Value() );
      mi->size( geometry );
    } else {
      NanThrowError("rows must be a number >= 0");
    }
  }

  /**
   * image.magick -> image format string
   * image.magick = string changes format
   */
  NAN_GETTER(Image::GetMagick) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<String>( image->GetMagickImage()->magick() ) );
  }

  NAN_SETTER(Image::SetMagick) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( ! value->IsString() ) {
      NanThrowError("magick should be a string");
    } else {
      lock _milock(&image->imagemutex);
      image->GetMagickImage()->magick( *NanUtf8String(value) );
    }
  }

  /**
   * image.page -> image page string
   * image.page = string changes page
   */
  NAN_GETTER(Image::GetPage) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<String>( string(image->GetMagickImage()->page()) ) );
  }

  NAN_SETTER(Image::SetPage) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( ! value->IsString() ) {
      NanThrowError("page should be a string");
    } else {
      lock _milock(&image->imagemutex);
      image->GetMagickImage()->page( *NanUtf8String(value) );
    }
  }

  /**
   * image.background -> image background color
   * image.background = changes background color
   */
  NAN_GETTER(Image::GetBackground) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( Color::NewColorObjectV8( image->GetMagickImage()->backgroundColor() ) );
  }

  NAN_SETTER(Image::SetBackground) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    Magick::Color color;
    const char *errmsg(NULL);
    if ( ! SetColorFromV8Value(color, value, &errmsg) )
      NanThrowError(errmsg);
    else {
      lock _milock(&image->imagemutex);
      image->GetMagickImage()->backgroundColor( color );
    }
  }

  /**
   * image.fuzz -> image color fuzz number
   * image.fuzz = changes fuzz
   **/
  NAN_GETTER(Image::GetFuzz) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    NanReturnValue( NanNew<Number>( image->GetMagickImage()->colorFuzz() ) );
  }

  NAN_SETTER(Image::SetFuzz) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    lock _milock(&image->imagemutex);
    image->GetMagickImage()->colorFuzz( value->NumberValue() );
  }

  /* private api for streams */

  NAN_METHOD(Image::UnderscoreHold) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP();
    if ( ! image->Hold() ) {
      return NanThrowError("Can't hold, already running");
    }
    NanReturnValue(args.This());
  }

  NAN_METHOD(Image::UnderscoreWrap) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC
    if ( image->IsOnHold() ) {
      if ( argc == 1 && Buffer::HasInstance(args[0]) ) {
        ImageReadJob *reader = new ImageReadJob();
        Local<Object> buffer( args[0]->ToObject() );
        image->SaveObject( buffer );
        reader->Setup( Buffer::Data(buffer), Buffer::Length(buffer) );
        image->Release( args.This(), reader );
      } else {
        image->Release( args.This(), NULL );
      }
    }
    NanReturnUndefined();
  }
}
