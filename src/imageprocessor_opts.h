#if !defined(NODEMAGICK_IMAGEPROCESSOR_HEADER)
#  error 'imageprocessor_opts.h' is not supposed to be included directly. Include 'imageprocessor.h' instead.
#endif

#include "util.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  /* ImageBlurJob */

  void ImageBlurJob::Setup(const Handle<Object> &options) {
    NanScope();

    sigma = options->Get(NODEMAGICK_KEY(sigma))->NumberValue();

    if ( isnan(sigma) )
      throw ImageOptionsException("blur() needs sigma number option");

    if ( options->Has(NODEMAGICK_KEY(radius)) )
      radius = options->Get(NODEMAGICK_KEY(radius))->NumberValue();

    if ( options->Has(NODEMAGICK_KEY(channel)) )
      channel.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(channel)) ) );

    if ( options->Has(NODEMAGICK_KEY(gaussian)) )
      gaussian = options->Get(NODEMAGICK_KEY(gaussian))->BooleanValue();

    ImageProcessJob::Setup();
  }

  /* ImageColorJob */

  void ImageColorJob::Setup(const Handle<Object> &options) {
    NanScope();
    if ( options->Has(NODEMAGICK_KEY(points)) ) {
      Setup( options->Get(NODEMAGICK_KEY(points)).As<Array>() );
    } else if ( options->Has(NODEMAGICK_KEY(x)) && options->Has(NODEMAGICK_KEY(y)) ) {

      x = options->Get(NODEMAGICK_KEY(x))->Int32Value();
      y = options->Get(NODEMAGICK_KEY(y))->Int32Value();

      if ( options->Has(NODEMAGICK_KEY(color)) ) {
        const char *errmsg(NULL);
        if ( ! SetColorFromV8Value(color, options->Get(NODEMAGICK_KEY(color)), &errmsg) )
          throw ImageOptionsException(errmsg);

        ImageProcessJob::Setup(false);
      } else
        ImageProcessJob::Setup(true);
    } else
      throw ImageOptionsException("color() needs points or x and y options");
  }

  void ImageColorJob::Setup(const Handle<Array> &pointsArray) {
    NanScope();
    uint32_t numpoints = pointsArray->Length();

    points.reset( new vector<ImagePixel>() );
    points->reserve(numpoints);

    for( uint32_t i = 0; i < numpoints; ++i ) {
      Local<Value> pointValue = pointsArray->Get(i);
      if ( pointValue->IsArray() ) {
        const Handle<Array> &point = pointValue.As<Array>();
        if ( point->Length() >= 2 ) {
          points->push_back( ImagePixel(
            point->Get(0)->Int32Value(),
            point->Get(1)->Int32Value()) );
        } else
          throw ImageOptionsException("color()'s coordinate tuples must contain 2 numbers");
      } else
        throw ImageOptionsException("color()'s Array argument should be an Array of coordinate tuples");
    }
    ImageProcessJob::Setup();
  }

  /* ImageCompositeJob */

  Image *ImageCompositeJob::Setup(const Handle<Object> &options, Handle<Object> &sourceObject) {
    NanScope();

    Image *source(NULL);
    sourceObject = UnwrapCheck<Image>( options->Get(NODEMAGICK_KEY(image)), &source );
    if ( source == NULL )
      throw ImageOptionsException("composite()'s image option is not an Image");

    if ( options->Has(NODEMAGICK_KEY(compose)) )
      compose.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(compose)) ) );

    if ( options->Has(NODEMAGICK_KEY(geometry)) ) {
      Local<Value> size = options->Get(NODEMAGICK_KEY(geometry));
      if ( size->IsArray() ) {
        SetGeometryFromV8Array( geometry, size.As<Array>() );
      } else if ( size->IsString() ) {
        geometry = *NanUtf8String( size );
        geometry.xNegative(false);
        geometry.yNegative(false);
      }
    }
    if ( geometry.isValid() ) {

      type = CompositeGeometry;

    } else if ( options->Has(NODEMAGICK_KEY(x)) && options->Has(NODEMAGICK_KEY(y)) ) {
      x = options->Get(NODEMAGICK_KEY(x))->Int32Value();
      y = options->Get(NODEMAGICK_KEY(y))->Int32Value();

      type = CompositeOffset;

    } else {
      if ( options->Has(NODEMAGICK_KEY(gravity)) ) {
        gravity = GetGravityFromString( *NanUtf8String( options->Get(NODEMAGICK_KEY(gravity)) ) );
      } else {
        gravity = Magick::CenterGravity;
      }

      type = CompositeGravity;
    }
    ImageMutualProcessJob::Setup(source);
    return source;
  }

  /* ImageCopyJob */

  void ImageCopyJob::Setup(const Handle<Object> &options) {
    NanScope();
    if ( options->Has(NODEMAGICK_KEY(autoCopy)) )
      autoCopy = options->Get(NODEMAGICK_KEY(autoCopy))->BooleanValue();
    ImageProcessJob::Setup();
  }

  /* ImageCropJob */

  void ImageCropJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has(NODEMAGICK_KEY(size)) ) {
      Local<Value> size = options->Get(NODEMAGICK_KEY(size));
      if ( size->IsArray() ) {
        SetGeometryFromV8Array( geometry, size.As<Array>() );
      } else if ( size->IsString() ) {
        geometry = *NanUtf8String( size );
      }
    } else if ( options->Has(NODEMAGICK_KEY(width)) && options->Has(NODEMAGICK_KEY(height)) ) {
      geometry.width( options->Get(NODEMAGICK_KEY(width))->Uint32Value() );
      geometry.height( options->Get(NODEMAGICK_KEY(height))->Uint32Value() );
    }
    if ( options->Has(NODEMAGICK_KEY(x)) && options->Has(NODEMAGICK_KEY(y)) ) {
      geometry.xOff( options->Get(NODEMAGICK_KEY(x))->Int32Value() );
      geometry.yOff( options->Get(NODEMAGICK_KEY(y))->Int32Value() );
    }
    if ( ! geometry.isValid() )
      throw ImageOptionsException("crop() needs proper size or width and height options");
    geometry.xNegative(false);
    geometry.yNegative(false);

    ImageProcessJob::Setup();
  }

  /* ImageExtentJob */

  void ImageExtentJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has(NODEMAGICK_KEY(size)) ) {
      Local<Value> size = options->Get(NODEMAGICK_KEY(size));
      if ( size->IsArray() ) {
        SetGeometryFromV8Array( geometry, size.As<Array>() );
      } else if ( size->IsString() ) {
        geometry = *NanUtf8String( size );
      }
    } else if ( options->Has(NODEMAGICK_KEY(width)) && options->Has(NODEMAGICK_KEY(height)) ) {
      geometry.width( options->Get(NODEMAGICK_KEY(width))->Uint32Value() );
      geometry.height( options->Get(NODEMAGICK_KEY(height))->Uint32Value() );
      gravity = Magick::CenterGravity;
    }
    if ( options->Has(NODEMAGICK_KEY(x)) && options->Has(NODEMAGICK_KEY(y)) ) {
      geometry.xOff( options->Get(NODEMAGICK_KEY(x))->Int32Value() );
      geometry.yOff( options->Get(NODEMAGICK_KEY(y))->Int32Value() );
      gravity = Magick::ForgetGravity;
    } else if ( options->Has(NODEMAGICK_KEY(gravity)) ) {
      gravity = GetGravityFromString( *NanUtf8String( options->Get(NODEMAGICK_KEY(gravity)) ) );
    }
    if ( options->Has(NODEMAGICK_KEY(color)) ) {
      const char *errmsg(NULL);
      if ( ! SetColorFromV8Value( color, options->Get(NODEMAGICK_KEY(color)), &errmsg ) )
        throw ImageOptionsException(errmsg);
    }

    if ( ! geometry.isValid() )
      throw ImageOptionsException("extent() needs proper size or width and height options");
    ImageProcessJob::Setup();
  }

  /* ImageNoiseJob */

  void ImageNoiseJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has(NODEMAGICK_KEY(noise)) ) {
      noise.reset(  new NanUtf8String( options->Get(NODEMAGICK_KEY(noise)) ) );
      if ( options->Has(NODEMAGICK_KEY(channel)) )
        channel.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(channel)) ) );
    } else
      throw ImageOptionsException("noise() needs noise string option");

    ImageProcessJob::Setup();
  }


  /* ImagePropertiesJob */

  void ImagePropertiesJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has( NODEMAGICK_KEY(batch) ) )
      batch = options->Get(NODEMAGICK_KEY(batch))->BooleanValue() ? 1 : 0;
    if ( options->Has( NODEMAGICK_KEY(autoClose) ) )
      autoClose = options->Get(NODEMAGICK_KEY(autoClose))->BooleanValue() ? 1 : 0;
    if ( options->Has( NODEMAGICK_KEY(autoCopy) ) )
      autoCopy = options->Get(NODEMAGICK_KEY(autoCopy))->BooleanValue() ? 1 : 0;
    if ( options->Has( NODEMAGICK_KEY(magick) ) )
      magick.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(magick)) ) );
    if ( options->Has( NODEMAGICK_KEY(columns) ) )
      columns = options->Get(NODEMAGICK_KEY(columns))->Uint32Value();
    if ( options->Has( NODEMAGICK_KEY(rows) ) )
      rows = options->Get(NODEMAGICK_KEY(rows))->Uint32Value();
    if ( options->Has( NODEMAGICK_KEY(page) ) )
      page.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(page)) ) );
    if ( options->Has( NODEMAGICK_KEY(fuzz) ) )
      fuzz = options->Get(NODEMAGICK_KEY(fuzz))->NumberValue();
    if ( options->Has( NODEMAGICK_KEY(background) ) ) {
      const char *errmsg(NULL);
      if ( ! SetColorFromV8Value( background, options->Get(NODEMAGICK_KEY(background)), &errmsg ) )
        throw ImageOptionsException(errmsg);
    }

    ImageProcessJob::Setup(false);
  }

  /* ImageQuantizeJob */

  void ImageQuantizeJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has(NODEMAGICK_KEY(colors)) ) {
      ssize_t colors_ = (ssize_t) options->Get(NODEMAGICK_KEY(colors))->IntegerValue();
      if ( colors_ < 0 )
        throw ImageOptionsException("number of colors should be > 0");
      colors = (size_t) colors_;
    }
    if ( options->Has(NODEMAGICK_KEY(colorspace)) )
      colorSpace.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(colorspace)) ) );
    if ( options->Has(NODEMAGICK_KEY(dither)) )
      dither = options->Get(NODEMAGICK_KEY(dither))->BooleanValue() ? 1 : 0;

    ImageProcessJob::Setup();
  }

  /* ImageResizeJob */

  const char * const  ImageResizeJob::ResizeTags[] = {
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
          "filter"    ,
          "sample"    ,
          "scale"     ,
    NULL
  };

  const char ImageResizeJob::ResizeTagValues[] = {
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
    'f',
    'm',
    'c'
  };

  NAN_INLINE void ImageResizeJob::ReadResizeMode(char *mode) {
    int tag = FindTag(ImageResizeJob::ResizeTags, mode);
    while ( tag >= 0) {
      switch(ImageResizeJob::ResizeTagValues[tag]) {
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
        case 'f': resizeType = ResizeFilter;                        break;
        case 'm': resizeType = ResizeSample;                        break;
        case 'c': resizeType = ResizeScale;                         break;
      }
      tag = FindNextTag(ImageResizeJob::ResizeTags);
    }
  }

  void ImageResizeJob::Setup(const Handle<Object> &options) {
    NanScope();

    if ( options->Has(NODEMAGICK_KEY(size)) ) {
      Local<Value> size = options->Get(NODEMAGICK_KEY(size));
      if ( size->IsArray() ) {
        SetGeometryFromV8Array( geometry, size.As<Array>() );
      } else if ( size->IsString() ) {
        geometry = *NanUtf8String( size );
      }
    } else if ( options->Has(NODEMAGICK_KEY(width)) && options->Has(NODEMAGICK_KEY(height)) ) {
      geometry.width( options->Get(NODEMAGICK_KEY(width))->Uint32Value() );
      geometry.height( options->Get(NODEMAGICK_KEY(height))->Uint32Value() );
    }
    if ( options->Has(NODEMAGICK_KEY(mode)) )
      ReadResizeMode( *NanUtf8String( options->Get(NODEMAGICK_KEY(mode)) ) );

    if ( ! geometry.isValid() )
      throw ImageOptionsException("resize() needs proper size or width and height options");

    ImageProcessJob::Setup();
  }

  /* ImageSharpenJob */

  void ImageSharpenJob::Setup(const Handle<Object> &options) {
    NanScope();

    sigma = options->Get(NODEMAGICK_KEY(sigma))->NumberValue();

    if ( isnan(sigma) )
      throw ImageOptionsException("sharpen() needs sigma number option");

    if ( options->Has(NODEMAGICK_KEY(radius)) )
      radius = options->Get(NODEMAGICK_KEY(radius))->NumberValue();

    if ( options->Has(NODEMAGICK_KEY(channel)) )
      channel.reset( new NanUtf8String( options->Get(NODEMAGICK_KEY(channel)) ) );

    ImageProcessJob::Setup();
  }


}