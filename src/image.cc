#include "image.h"
#include <string.h>
#include <cstring>


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

#define NODEMAGICK_FINISH_IMAGE_WORKER_NO_ARGS(JobKlass, job)         \
  do {                                                                \
    if ( callback || image->IsBatch() ) {                             \
      image->BatchPush( new JobKlass(job) );                          \
      if ( callback ) {                                               \
        image->AsyncWork( args.This(), NODEMAGICK_ASYNC_CALLBACK() ); \
      }                                                               \
    } else {                                                          \
      NanReturnValue( image->SyncProcess( job, args.This() ) );       \
    }                                                                 \
    NanReturnValue(args.This());                                      \
  } while(0)

#define NODEMAGICK_FINISH_IMAGE_WORKER(JobKlass, job, errorstring) \
  do {                                                             \
    if ( ! job.IsValid() ) {                                       \
      return NanThrowTypeError(errorstring);                       \
    } else {                                                       \
      NODEMAGICK_FINISH_IMAGE_WORKER_NO_ARGS(JobKlass, job);       \
    }                                                              \
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
    NODE_SET_PROTOTYPE_METHOD(tpl, "copy"       , Copy);
    NODE_SET_PROTOTYPE_METHOD(tpl, "crop"       , Crop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "end"        , End);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extent"     , Extent);
    NODE_SET_PROTOTYPE_METHOD(tpl, "filter"     , Filter);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flip"       , Flip);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flop"       , Flop);
    NODE_SET_PROTOTYPE_METHOD(tpl, "format"     , Format);
    NODE_SET_PROTOTYPE_METHOD(tpl, "histogram"  , Histogram);
    NODE_SET_PROTOTYPE_METHOD(tpl, "noise"      , Noise);
    NODE_SET_PROTOTYPE_METHOD(tpl, "properties" , Properties);
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
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }
  Image::Image(const char *geometry, const Magick::Color &color)
    : magickimage( new Magick::Image(geometry, color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }
  Image::Image(size_t width, size_t height, const char *color)
    : magickimage( new Magick::Image(Magick::Geometry(width, height), color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }
  Image::Image(size_t width, size_t height, const Magick::Color &color)
    : magickimage( new Magick::Image(Magick::Geometry(width, height), color) ), magickcopy(NULL)
    , isCloned(false), autoCopy(false), autoClose(true) { uv_mutex_init(&imagemutex); }

  Image::~Image(void) {
    uv_mutex_destroy(&imagemutex);
    // printf("Image freed\n");
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
    if ( mimage != NULL )
      magickimage.reset( new Magick::Image( *mimage ) );
    else
      magickimage.reset( new Magick::Image() );

    isCloned = true;
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
   *
   * options:
   *
   *   - src:       image data Buffer
   *   - columns:   number of columns (width)
   *   - rows:      number of rows (height)
   *   - color:     new image color string or Color object, default: transparent
   *   - magick:    set format, default: Magick++ default
   *   - page:      page description string
   *   - batch:     set persistent batch mode, default: false
   *   - autoClose: set auto close mode, default: true
   *   - autoCopy:  set auto copy mode, default: false
   *   - background: set background color, default: unknown
   *   - fuzz:       set color fuzz, default: 0
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
              if ( properties->Has(srcSym) ) {
                Local<Value> src = properties->Get(srcSym);
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
                size_t width  = properties->Get(columnsSym)->Uint32Value();
                size_t height = properties->Get(rowsSym)->Uint32Value();
                if ( width == 0 || height == 0 ) {
                  image = new Image();
                } else {
                  Local<Value> colorValue = properties->Get(colorSym);
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
              if ( properties->Has(batchSym) )
                image->IsPersistentBatch( properties->Get(batchSym)->BooleanValue() );
              if ( properties->Has(autoCopySym) )
                image->autoCopy = properties->Get(autoCopySym)->BooleanValue();
              if ( properties->Has(autoCloseSym) )
                image->autoClose = properties->Get(autoCloseSym)->BooleanValue();
              if ( properties->Has(magickSym) )
                image->GetMagickImage()->magick( *NanUtf8String( properties->Get(magickSym) ) );
              if ( properties->Has(pageSym) )
                image->GetMagickImage()->page( *NanUtf8String( properties->Get(pageSym) ) );
              if ( properties->Has(backgroundSym) ) {
                NODEMAGICK_COLOR_FROM_VALUE( color, properties->Get(backgroundSym) );
                image->GetMagickImage()->backgroundColor( color );
              }
              if ( properties->Has(fuzzSym) )
                image->GetMagickImage()->colorFuzz( properties->Get(fuzzSym)->NumberValue() );
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
   * if callback is provided executes batch otherwise clears batch tail from jobs
   *
   * if batch or options argument is provided this function
   * immediately changes batch image property.
   **/
  NAN_METHOD(Image::Begin) {
    NODEMAGICK_SCOPE_IMAGE_UNWRAP_ARGC
    if ( argc >= 1 ) {
      bool batch;
      if ( args[0]->IsObject() ) {
        batch = NanBooleanOptionValue(args[0].As<Object>(), batchSym);
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
   * image.blur(sigma[, callback(err, image)])
   * image.blur(sigma, radius[, callback(err, image)])
   **/
  NAN_METHOD(Image::Blur) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageBlurJob, blur)

    if ( argc == 2 ) {
      if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
        blur.Setup(args[0]->NumberValue(), args[1]->NumberValue());
      }
    } else if ( argc == 1 ) {
      if ( args[0]->IsNumber() ) {
        blur.Setup(args[0]->NumberValue());
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageBlurJob, blur, "blur()'s arguments should be 1 or 2 number(s)");
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
        copyjob.Setup( NanBooleanOptionValue(args[0].As<Object>(), autoCopySym) );
      } else if ( args[0]->IsBoolean() ) {
        copyjob.Setup( args[0]->BooleanValue() );
      }
    } else if (argc == 0) {
      copyjob.Setup();
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageCopyJob, copyjob, "copy()'s 1st argument should be boolean or Object");
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
        image->autoClose = NanBooleanOptionValue(args[0].As<Object>(), autoCloseSym, true);
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
   * Comment image
   *
   * image.comment(comment[,callback(err, comment)])
   * image.comment([callback(err, image)])
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
   * Peek or plot color pixel
   *
   * in synchronous context:
   * image.color(x, y) -> color
   * image.color(points) -> colors
   * image.color(x, y, newcolor) -> image
   *
   * in asynchronous context:
   * image.color(x, y, callback(err, color))
   * image.color(points, callback(err, colors))
   * image.color(x, y, newcolor, callback(err, image))
   *
   * points: an Array of [x, y] tuples
   * newcolor: string or Color object
   * color: Color object
   * colors: an Array of Color objects
   **/
  NAN_METHOD(Image::Color) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageColorJob, colorjob)

    if ( argc == 1 && args[0]->IsArray() ) {
      Local<Array> points = args[0].As<Array>();
      uint32_t numpixels = points->Length();

      colorjob.Setup(numpixels);

      for(uint32_t i = 0; i < numpixels; ++i) {
        Local<Value> pointValue = points->Get(i);
        if ( pointValue->IsArray() ) {
          const Handle<Array> &point = pointValue.As<Array>();
          if ( point->Length() >= 2 ) {
            colorjob.Push(point->Get(0)->Int32Value(), point->Get(1)->Int32Value());
          } else {
            return NanThrowError("color()'s coordinate tuples must contain 2 numbers");
          }
        } else {
          return NanThrowError("color()'s Array argument should be an Array of coordinate tuples");
        }
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

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageColorJob, colorjob, "color()'s arguments should be Array or Numbers and Color");
  }

  /**
   * Crop image
   *
   * image.crop(width, height, x, y[, callback(err, image)])
   * image.crop(size[, callback(err, image)])
   * image.crop(geometry[, callback(err, image)])
   *
   * size: an Array of [width, height, x, y]
   * geometry: geometry string
   **/
  NAN_METHOD(Image::Crop) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageCropJob, cropper)

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
      }
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
        batch = NanBooleanOptionValue(args[0].As<Object>(), batchSym);
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
      Local<Value> argv[] = { args.This() };
      NanMakeCallback( NanGetCurrentContext()->Global(), NODEMAGICK_ASYNC_CALLBACK(), 1, argv );
    } else {
      if ( setpersist )
        image->IsPersistentBatch(batch);
    }
    NanReturnValue(args.This());
  }

  /**
   * Extent image
   *
   * image.extent(width, height[, gravity="center"][, color="transparent"][, callback(err, image)])
   * image.extent(width, height, x, y[, color="transparent"][, callback(err, image)])
   * image.extent(size[, gravity="ignore"][, color="transparent"][, callback(err, image)])
   * image.extent(geometry[, gravity="ignore"][, color="transparent"][, callback(err, image)])
   *
   * size: an Array of [width, height] or [width, height, x, y]
   * geometry: geometry string
   * color: string or Color object
   **/
  NAN_METHOD(Image::Extent) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageExtentJob, extender)

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
      if ( args[0]->IsString() ) {
        if ( argc == 1 ) {
          geometry = *NanUtf8String(args[0]);
          if ( ! geometry.isValid() )
            return NanThrowError("bad geometry string");
        }
      } else if ( args[0]->IsArray() ) {
        if ( argc == 1 ) {
          Local<Array> size( args[0].As<Array>() );
          SetGeometryFromV8Array( geometry, size );
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
    NODEMAGICK_FINISH_IMAGE_WORKER(ImageExtentJob, extender, "extent()'s arguments should be string or number(s)");
  }

  /**
   * Filter change for resizing
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

    if ( argc == 1 ) {
      if ( args[0]->IsString() ) {
        noisejob.Setup( new NanUtf8String(args[0]) );
      } else if ( args[0]->IsObject() ) {
        Local<Object> options = args[0].As<Object>();
        if ( options->Has(noiseSym) ) {
          if ( options->Has(channelSym) ) {
            noisejob.Setup( new NanUtf8String(options->Get(noiseSym)),
                            new NanUtf8String(options->Get(channelSym)) );
          } else {
            noisejob.Setup( new NanUtf8String(options->Get(noiseSym)) );
          }
        }
      }
    } else if ( argc == 2 ) {
      if ( args[0]->IsString() && args[1]->IsString() )
        noisejob.Setup( new NanUtf8String(args[0]), new NanUtf8String(args[1]) );
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageNoiseJob, noisejob, "noise()'s arguments should be strings");
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

    if ( argc == 0 ) {
      propjob.Setup(true);
    } else if ( argc == 1 ) {
      if ( ! args[0]->IsUndefined() && args[0]->IsObject() ) {
        Local<Object> properties( args[0].As<Object>() );
        propjob.Setup(false);
        if ( properties->Has( batchSym ) )
          propjob.batch = properties->Get(batchSym)->BooleanValue() ? 1 : 0;
        if ( properties->Has( autoCloseSym ) )
          propjob.autoClose = properties->Get(autoCloseSym)->BooleanValue() ? 1 : 0;
        if ( properties->Has( autoCopySym ) )
          propjob.autoCopy = properties->Get(autoCopySym)->BooleanValue() ? 1 : 0;
        if ( properties->Has( magickSym ) )
          propjob.magick.reset( new NanUtf8String( properties->Get(magickSym) ) );
        if ( properties->Has( columnsSym ) )
          propjob.columns = properties->Get(columnsSym)->Uint32Value();
        if ( properties->Has( rowsSym ) )
          propjob.rows = properties->Get(rowsSym)->Uint32Value();
        if ( properties->Has( pageSym ) )
          propjob.page.reset( new NanUtf8String( properties->Get(pageSym) ) );
        if ( properties->Has( fuzzSym ) )
          propjob.fuzz = properties->Get(fuzzSym)->NumberValue();
        if ( properties->Has( backgroundSym ) ) {
          NODEMAGICK_COLOR_FROM_VALUE( color, properties->Get(backgroundSym) );
          propjob.background = color;
        }
      }
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

    ssize_t colors(0);
    char dither (-1);
    NanUtf8String *colorSpace(NULL);
    if ( argc == 1 && args[0]->IsObject() ) {
      Local<Object> options = args[0].As<Object>();
      if ( options->Has(colorsSym) )
        colors = (ssize_t) options->Get(colorsSym)->IntegerValue();
      if ( options->Has(colorspaceSym) )
        colorSpace = new NanUtf8String(options->Get(colorspaceSym));
      if ( options->Has(ditherSym) )
        dither = options->Get(ditherSym)->BooleanValue() ? 1 : 0;
    } else if ( argc <= 3 ) {
      for( uint32_t i = 0; i < argc; ++i ) {
        if ( args[i]->IsString() ) {
          colorSpace = new NanUtf8String(args[i]);
        } else if (args[i]->IsBoolean() ) {
          dither = args[i]->BooleanValue() ? 1 : 0;
        } else {
          colors = (ssize_t) args[i]->IntegerValue();
        }
      }
    }
    if ( colors < 0 )
      return NanThrowError("number of colors should be > 0");
    quantizer.Setup( (size_t)colors, colorSpace, dither );

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageQuantizeJob, quantizer, "quantize()'s arguments should be number, string or boolean");
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
   * image.sharpen(sigma[, radius=0][, callback(err, image)])
   *
   * sigma: a standard deviation
   * radius: pixel radius
   **/
  NAN_METHOD(Image::Sharpen) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageSharpenJob, sharpener)

    if ( argc == 2 ) {
      if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
        sharpener.Setup(args[0]->NumberValue(), args[1]->NumberValue());
      }
    } else if ( argc == 1 ) {
      if ( args[0]->IsNumber() ) {
        sharpener.Setup(args[0]->NumberValue());
      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageSharpenJob, sharpener, "sharpen()'s arguments should be 1 or 2 Numbers");
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
   * size: an Array or [width, height]
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
   * Read image
   *
   * image.read(file|buffer[, callback(err, image)])
   *
   * buffer: a Buffer object to read image from (recommended)
   * file: a file to read from (not recommended, uses Magick++ threaded I/O)
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

  const char * const  Image::ResizeTags[] = {
    "#" , "aspectfit" ,
    "^" , "aspectfill",
    "!" , "noaspect"  , "fill"         ,
    ">" , "larger"    , "greater"      ,
    "!>", "nolarger"  , "nogreater"    ,
    "<" , "smaller"   , "less"         ,
    "!<", "nosmaller" , "noless"       ,
    "%" , "percent"   ,
    "!%", "nopercent" ,
    "@" , "limit"     , "limitpixels"  ,
    "!@", "nolimit"   , "nolimitpixels",
    NULL
  };

  const char Image::ResizeTagValues[] = {
    '#', '#',
    '^', '^',
    '!', '!', '!',
    '>', '>', '>',
    '-', '-', '-',
    '<', '<', '<',
    '+', '+', '+',
    '%', '%',
    '=', '=',
    '@', '@', '@',
    '$', '$', '$',
  };

  static NAN_INLINE void AlterGeometryMode(Magick::Geometry &geometry, char *mode) {
    int tag = FindTag(Image::ResizeTags, mode);
    while ( tag >= 0) {
      switch(Image::ResizeTagValues[tag]) {
        case '#': geometry.fillArea(false); geometry.aspect(false); break;
        case '^': geometry.fillArea(true); geometry.aspect(false);  break;
        case '!': geometry.aspect(true);                            break;
        case '>': geometry.greater(true); geometry.less(false);     break;
        case '-': geometry.greater(false);                          break;
        case '<': geometry.less(true); geometry.greater(false);     break;
        case '+': geometry.less(false);                             break;
        case '%': geometry.percent(true);                           break;
        case '=': geometry.percent(false);                          break;
        case '@': geometry.limitPixels(true);                       break;
        case '$': geometry.limitPixels(false);                      break;
      }
      tag = FindNextTag(Image::ResizeTags);
    }
  }

  /**
   * Resize image applying filter
   *
   * image.resize(width, height[, mode="aspectfit"][, callback(err, image)])
   * image.resize(size[, mode="aspectfit"][, callback(err, image)])
   * image.resize(geometry[, callback(err, image)])
   * image.resize(geometry[, mode][, callback(err, image)])
   *
   * size: an Array of [width, height] or [width, height, x, y ]
   * geometry: geometry string
   *
   * mode:
   *   - "#" or "aspectfit"          Resize the image based on the largest fitting dimenstion (default)
   *   - "^" or "aspectfill"         Resize the image based on the smallest fitting dimension
   *   - "!" or "noaspect" or "fill" Force exact size
   *   - ">" or "larger"             Resize only if larger than geometry
   *   - "!>" or "nolarger"          Don't resize only if smaller than geometry
   *   - "<" or "smaller"            Resize only if smaller than geometry
   *   - "!<" or "nosmaller"         Don't resize only if larger than geometry
   *   - "%" or "percent"            Interpret width & height as percentages
   *   - "!%" or "nopercent"         Don't interpret width & height as percentages
   *   - "@" or "limit"              Resize using a pixel area count limit
   *   - "!@" or "nolimit"           Don't resize using a pixel area count limit
   *
   * mode tags can be combined (separated by space, comma or (semi)colon)
   **/
  NAN_METHOD(Image::Resize) {
    NODEMAGICK_BEGIN_IMAGE_WORKER(ImageResizeJob, resizer)

    if ( argc >= 1 && argc <= 3 ) {
      int modeargindex = -1;
      if ( argc >= 2 ) {
        if ( args[argc - 1]->IsString() )
          modeargindex = --argc;
      }
      if ( argc == 2 ) {
        if ( args[0]->IsNumber() && args[1]->IsNumber() ) {
          Magick::Geometry geometry( args[0]->Int32Value(), args[1]->Int32Value() );
          if ( modeargindex == 2 ) {
            AlterGeometryMode( geometry, *NanUtf8String( args[2] ) );
          }
          resizer.Setup(geometry);
        }
      } else if ( argc == 1 ) {
        if ( args[0]->IsString() ) {
          Magick::Geometry geometry(*NanUtf8String(args[0]));
          if ( modeargindex == 1 ) {
            AlterGeometryMode( geometry, *NanUtf8String( args[1] ) );
          }
          resizer.Setup(geometry);
        } else if ( args[0]->IsArray() ) {
          Magick::Geometry geometry;
          Local<Array> size( args[0].As<Array>() );
          if ( SetGeometryFromV8Array( geometry, size ) ) {
            if ( modeargindex == 1 ) {
              AlterGeometryMode( geometry, *NanUtf8String( args[1] ) );
            }
            resizer.Setup( geometry );
          }
        }

      }
    }

    NODEMAGICK_FINISH_IMAGE_WORKER(ImageResizeJob, resizer, "resize()'s arguments should be string, 2 Numbers or callback");
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
   * file: a file to write to (not recommended, uses Magick++ threaded I/O)
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
   */
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
