#include "imageprocessor.h"
#include "imageprocessor_opts.h"
#include "image.h"
#include <limits>
#include <cmath>
#ifdef NODEMAGICK_USE_STL_MAP
#  include <map>
#endif

using namespace node;
using namespace v8;

namespace NodeMagick {
  using namespace std;

  /* ImageProcessJob */

  ImageProcessJob::ImageProcessJob() : Job(), valid(false), dontCopy(false) {}
  ImageProcessJob::ImageProcessJob(bool dontCopy_) : Job(), valid(false), dontCopy(dontCopy_) {}
  void ImageProcessJob::Setup() {
    valid = true;
  }

  void ImageProcessJob::Setup(bool dontCopy_) {
    dontCopy = dontCopy_;
    valid = true;
  }

  void ImageProcessJob::ProcessImage(Image *image) {
    //printf("ImageProcessJob::ProcessImage: %p\n", image);
  }

  bool ImageProcessJob::HasReturnValue() { return dontCopy; }
  bool ImageProcessJob::IsValid()        { return valid; }
  bool ImageProcessJob::DontCopy()       { return dontCopy; }

  /* ImageMutualProcessJob */

  ImageMutualProcessJob::ImageMutualProcessJob(ImageMutualKit &kit)
    : ImageProcessJob(), job(NULL), source(NULL), guard(NULL), barrier(NULL), haveWait(false) {
    guard = kit.GuardMutex();
    barrier = kit.Barrier();
  }

  ImageSynchronizeJob *ImageMutualProcessJob::SetupSynchronizeJob(ImageSynchronizeJob *job_) {
    job = job_ == NULL ? new ImageSynchronizeJob(*source) : job_;
    job->Setup(*this);
    return job;
  }

  void ImageMutualProcessJob::Setup(Image *source_) {
    source = source_;
    ImageProcessJob::Setup();
  }

  /* the destruction won't happen during own ProcessImage */
  ImageMutualProcessJob::~ImageMutualProcessJob() {
    if ( ! haveWait ) { /* if we have waited, then skip */
      { /* try to cancel synchronize job */
        lock guardscope(guard);
        if ( job != NULL ) { /* the synchronize job have not cancelled us out already */
          {
            lock synchroscope(job->guard);
            job->job = NULL; /* cancel synchronize job */
            haveWait = job->haveWait; /* wait if synchronize job is waiting */
          }
        }
      }

      if (haveWait)
        uv_barrier_wait(barrier);
    }
  }

  void ImageMutualProcessJob::ProcessImagesSynchronized(Image *image, Image *source) {}

  void ImageMutualProcessJob::ProcessImage(Image *image) {
    if ( ! image->IsBatch() ) { /* processing synchronously */
      if ( source->IsBatch() ) {
        throw ImageSynchronizeException();
      } else {
        lock sourcelock(&source->imagemutex);
        ProcessImagesSynchronized(image, source);
        return;
      }
    }

    {
      lock guardscope(guard);

      if ( job == NULL )
        throw ImageSynchronizeException();
      haveWait = true;
    }

    {
      unlock imagescope(&image->imagemutex);
      uv_barrier_wait(barrier);
    }

    if ( job == NULL )
      throw ImageSynchronizeException();

    {
      wall scopewait(barrier); /* release synchronize job after scope */
      lock sourcelock(&source->imagemutex);
      ProcessImagesSynchronized(image, source);
    }
  }

  /* ImageSynchronizeJob */

  ImageSynchronizeJob::ImageSynchronizeJob(ImageMutualKit &kit)
    : ImageProcessJob(true), job(NULL), guard(NULL), haveWait(false) {
    guard = kit.GuardMutex();
  }

  void ImageSynchronizeJob::Setup(ImageMutualProcessJob &job_) {
    job = &job_;
    ImageProcessJob::Setup();
  }

  /* the destruction won't happen during own ProcessImage */
  ImageSynchronizeJob::~ImageSynchronizeJob() {
    if ( ! haveWait ) { /* if we have waited, then skip */
      /* try to cancel mutual job */
      ImageMutualProcessJob *job_;
      { /* avoid mutual deadlock */
        lock guardscope(guard);
        job_ = job;
        haveWait = true; /* tell mutual job we will wait eventually */
      }
      if ( job_ != NULL ) { /* the mutual job have not cancelled us out already */
        bool wait;
        {
          lock mutualscope(job->guard);
          job_->job = NULL; /* cancel mutual job */
          wait = job_->haveWait; /* wait if mutual job is waiting */
        }

        if (wait)
          uv_barrier_wait(job_->barrier);
      }
    }
  }

  void ImageSynchronizeJob::ProcessImage(Image *image) {
    uv_barrier_t *barrier_;
    {
      lock guardscope(guard);

      if ( job == NULL )
        return;

      barrier_ = job->barrier;
      haveWait = true;
    }

    {
      unlock imagescope(&image->imagemutex);
      uv_barrier_wait(barrier_);

      {
        lock guardscope(guard);

        if ( job != NULL ) {
          /* wait for mutual job to complete */
          uv_barrier_wait(barrier_);
        }
      }
    }
  }

  bool ImageSynchronizeJob::HasReturnValue() {
    return false;
  }

  /* ImageBlurJob */

  ImageBlurJob::ImageBlurJob()
    : ImageProcessJob(), sigma(0.0), radius(0.0), gaussian(false), channel(NULL) {}

  void ImageBlurJob::Setup(double sigma_, double radius_, NanUtf8String *channel_, bool gaussian_) {
    sigma = sigma_;
    radius = radius_;
    channel.reset(channel_);
    gaussian = gaussian_;
    ImageProcessJob::Setup();
  }

  void ImageBlurJob::ProcessImage(Image *image) {
    NanUtf8String *channel_ = channel.get();
    if ( channel_ != NULL ) {
      Magick::ChannelType channelType( Magick::UndefinedChannel );
      if ( channel_ != NULL ) {
        ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickChannelOptions, Magick::MagickFalse, **channel_);
        if ( type == -1 ) {
          throw ImageChannelException();
        }
        channelType = (Magick::ChannelType) type;
      }
      if ( gaussian ) {
        image->GetMagickImage()->gaussianBlurChannel(channelType, radius, sigma);
      } else {
        image->GetMagickImage()->blurChannel(channelType, radius, sigma);
      }
    } else if ( gaussian ) {
      image->GetMagickImage()->gaussianBlur(radius, sigma);
    } else {
      image->GetMagickImage()->blur(radius, sigma);
    }
  }

  /* ImageCloseJob */

  ImageCloseJob::ImageCloseJob() : ImageProcessJob(true) {}

  void ImageCloseJob::ProcessImage(Image *image) {
    image->CloseImage();
  }

  bool ImageCloseJob::HasReturnValue() {
    return false;
  }

  /* ImageColorJob */

  ImageColorJob::ImageColorJob() : ImageProcessJob(true), readPixel(false), points(NULL) {}

  void ImageColorJob::Setup(ssize_t x_, ssize_t y_) {
    x = x_;
    y = y_;
    ImageProcessJob::Setup();
  }

  void ImageColorJob::Setup(ssize_t x_, ssize_t y_, Magick::Color& color_) {
    x = x_;
    y = y_;
    color = color_;
    ImageProcessJob::Setup(false);
  }

  void ImageColorJob::ProcessImage(Image *image) {
    Magick::Image *mi = image->GetMagickImage();
    size_t width = mi->columns(), height = mi->rows();
    if ( dontCopy ) {
      vector<ImagePixel> *pixels = points.get();
      if ( pixels == NULL ) {
        color = mi->pixelColor(x >= 0 ? x : width + x, y >= 0 ? y : height + y);
      } else {
        Magick::Image *mi = image->GetMagickImage();
        for(vector<ImagePixel>::iterator it = pixels->begin();
                                         it != pixels->end(); ++it) {
          ssize_t x = it->x, y = it->y;
          Magick::Color color( mi->pixelColor(x >= 0 ? x : width + x, y >= 0 ? y : height + y) );
          Magick::Quantum *colorParts = it->colorParts;

          colorParts[0] = color.redQuantum();
          colorParts[1] = color.greenQuantum();
          colorParts[2] = color.blueQuantum();
          colorParts[3] = color.alphaQuantum();
        }
      }
    } else {
      mi->pixelColor(x >= 0 ? x : width + x, y >= 0 ? y : height + y, color);
    }
  }

  Local<Value> ImageColorJob::ReturnedValue() {
    vector<ImagePixel> *pixels = points.get();
    if ( pixels == NULL ) {
      return Color::NewColorObjectV8(color);
    } else {
      NanEscapableScope();
      Local<Array> result( NanNew<Array>( (int)pixels->size() ) );
      uint32_t index = 0;
      for(vector<ImagePixel>::iterator it = pixels->begin();
                                       it != pixels->end(); ++it) {
        result->Set(index++, Color::NewColorObjectV8( it->colorParts ));
      }
      return NanEscapeScope(result);
    }
  }

  /* ImageCommentJob */

  ImageCommentJob::ImageCommentJob() : ImageProcessJob() {}

  void ImageCommentJob::Setup() {
    ImageProcessJob::Setup(true);
  }

  void ImageCommentJob::Setup(NanUtf8String *comment_) {
    comment.reset(comment_);
    ImageProcessJob::Setup();
  }

  void ImageCommentJob::ProcessImage(Image *image) {
    if ( dontCopy )
      commentStr = image->GetMagickImage()->comment();
    else
      image->GetMagickImage()->comment(**comment);
  }

  Local<Value> ImageCommentJob::ReturnedValue() {
    return NanNew<String>(commentStr);
  }

  /* ImageCompositeJob */

  ImageCompositeJob::ImageCompositeJob(ImageMutualKit &kit_)
    : ImageMutualProcessJob(kit_)
    , type(CompositeGravity), x(0), y(0), gravity(Magick::CenterGravity), compose(NULL) {}

  void ImageCompositeJob::Setup(Image *source_, NanUtf8String *compose_, Magick::GravityType gravity_) {
    ImageMutualProcessJob::Setup(source_);
    compose.reset(compose_);
    gravity = gravity_;
    type = CompositeGravity;
  }

  void ImageCompositeJob::Setup(Image *source_, NanUtf8String *compose_, Magick::Geometry &geometry_) {
    ImageMutualProcessJob::Setup(source_);
    compose.reset(compose_);
    geometry = geometry_;
    type = CompositeGeometry;
  }

  void ImageCompositeJob::Setup(Image *source_, NanUtf8String *compose_, ssize_t x_, ssize_t y_) {
    ImageMutualProcessJob::Setup(source_);
    compose.reset(compose_);
    x = x_;
    y = y_;
    type = CompositeOffset;
  }

  void ImageCompositeJob::ProcessImagesSynchronized(Image *image, Image *source) {
    NanUtf8String *compose_ = compose.get();
    ssize_t composeType = (ssize_t) Magick::OverCompositeOp;
    if ( compose_ != NULL ) {
      composeType = MagickCore::ParseCommandOption(MagickCore::MagickComposeOptions, Magick::MagickFalse, **compose_);
      if ( composeType == -1 )
        throw ImageCompositeOperatorException();
    }

    switch(type) {
      case CompositeGravity:
        image->GetMagickImage()->composite(*source->GetMagickImage(), gravity,
          (Magick::CompositeOperator) composeType);
        break;
      case CompositeGeometry:
        image->GetMagickImage()->composite(*source->GetMagickImage(), geometry,
          (Magick::CompositeOperator) composeType);
        break;
      case CompositeOffset:
        image->GetMagickImage()->composite(*source->GetMagickImage(), x, y,
          (Magick::CompositeOperator) composeType);
        break;
    }
  }

  /* ImageCopyJob */

  ImageCopyJob::ImageCopyJob() : ImageProcessJob(true), autoCopy(-1) {}

  void ImageCopyJob::Setup() {
    ImageProcessJob::Setup();
  }

  void ImageCopyJob::Setup(bool autoCopy_) {
    autoCopy = autoCopy_ ? 1 : 0;
    ImageProcessJob::Setup();
  }

  bool ImageCopyJob::HasReturnValue() {
    return false;
  }

  void ImageCopyJob::ProcessImage(Image *image) {
    switch( autoCopy ) {
      case 0:
        image->autoCopy = false;
        break;
      case 1:
        image->autoCopy = true;
      default:
        image->CloneImage();
    }
  }

  /* ImageCropJob */

  ImageCropJob::ImageCropJob() : ImageProcessJob() {}

  void ImageCropJob::Setup(Magick::Geometry& geometry_) {
    geometry = geometry_;
    ImageProcessJob::Setup();
  }

  void ImageCropJob::ProcessImage(Image *image) {
    image->GetMagickImage()->crop(geometry);
  }

  /* ImageExtentJob */

  ImageExtentJob::ImageExtentJob()
    : ImageProcessJob(), gravity(Magick::ForgetGravity) {}

  void ImageExtentJob::Setup(Magick::Geometry& geometry_) {
    geometry = geometry_;
    ImageProcessJob::Setup();
  }
  void ImageExtentJob::Setup(Magick::Geometry& geometry_, Magick::GravityType gravity_) {
    geometry = geometry_;
    gravity = gravity_;
    ImageProcessJob::Setup();
  }
  void ImageExtentJob::Setup(Magick::Geometry& geometry_, Magick::Color& color_) {
    geometry = geometry_;
    color = color_;
    ImageProcessJob::Setup();
  }
  void ImageExtentJob::Setup(Magick::Geometry& geometry_, Magick::Color& color_, Magick::GravityType gravity_) {
    geometry = geometry_;
    color = color_;
    gravity = gravity_;
    ImageProcessJob::Setup();
  }

  void ImageExtentJob::ProcessImage(Image *image) {
    if ( color.isValid() ) {
      if ( gravity == Magick::ForgetGravity ) {
        image->GetMagickImage()->extent(geometry, color);
      } else {
        image->GetMagickImage()->extent(geometry, color, gravity);
      }
    } else if ( gravity == Magick::ForgetGravity ) {
      image->GetMagickImage()->extent(geometry);
    } else {
      image->GetMagickImage()->extent(geometry, gravity);
    }
  }

  /* ImageFilterJob */

  ImageFilterJob::ImageFilterJob() : ImageProcessJob() {}

  void ImageFilterJob::Setup(NanUtf8String *filter_) {
    filter.reset(filter_);
    ImageProcessJob::Setup();
  }

  void ImageFilterJob::ProcessImage(Image *image) {
    ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickFilterOptions, Magick::MagickFalse, **filter);
    if ( type != -1 ) {
      image->GetMagickImage()->filterType( (Magick::FilterTypes) type );
    } else {
      throw ImageFilterException();
    }
  }

  /* ImageFlipJob */

  ImageFlipJob::ImageFlipJob() : ImageProcessJob() {}

  void ImageFlipJob::ProcessImage(Image *image) {
    image->GetMagickImage()->flip();
  }

  /* ImageFlopJob */

  ImageFlopJob::ImageFlopJob() : ImageProcessJob() {}

  void ImageFlopJob::ProcessImage(Image *image) {
    image->GetMagickImage()->flop();
  }

  /* ImageFormatJob */

  ImageFormatJob::ImageFormatJob() : ImageProcessJob() {}

  void ImageFormatJob::Setup(NanUtf8String *format_) {
    format.reset(format_);
    ImageProcessJob::Setup();
  }

  void ImageFormatJob::ProcessImage(Image *image) {
    image->GetMagickImage()->magick( **format );
  }

  /* ImageHistogramJob */

  ImageHistogramJob::ImageHistogramJob() : ImageProcessJob(true) {}

  void ImageHistogramJob::ProcessImage(Image *image) {
    Magick::colorHistogram( &histogram, *image->GetMagickImage() );
  }

  Local<Value> ImageHistogramJob::ReturnedValue() {
    NanEscapableScope();
    Local<Object> result( NanNew<Array>( (int) histogram.size() ) );
    uint32_t index = 0;
#ifdef NODEMAGICK_USE_STL_MAP
    for( map<Magick::Color,size_t>::const_iterator
#else
    for( vector<pair<Magick::Color,size_t> >::const_iterator
#endif
                                                   it = histogram.begin();
                                                   it != histogram.end();
                                                 ++it) {
      Local<Object> tuple( NanNew<Array>(2) );
      tuple->Set( 0, Color::NewColorObjectV8( it->first ) );
      tuple->Set( 1, NanNew<Integer>( (uint32_t) it->second ) );
      result->Set( index++, tuple );
    }
    return NanEscapeScope(result);
  }

  /* ImageNegateJob */

  ImageNegateJob::ImageNegateJob() : ImageProcessJob() {}

  void ImageNegateJob::Setup(bool grayscale_) {
    grayscale = grayscale_;
    ImageProcessJob::Setup();
  }

  void ImageNegateJob::ProcessImage(Image *image) {
    image->GetMagickImage()->negate(grayscale);
  }

  /* ImageNoiseJob */

  ImageNoiseJob::ImageNoiseJob()
    : ImageProcessJob(), noise(NULL), channel(NULL) {}

  void ImageNoiseJob::Setup(NanUtf8String *noise_) {
    noise.reset(noise_);
    ImageProcessJob::Setup();
  }

  void ImageNoiseJob::Setup(NanUtf8String *noise_, NanUtf8String *channel_) {
    noise.reset(noise_);
    channel.reset(channel_);
    ImageProcessJob::Setup();
  }

  void ImageNoiseJob::ProcessImage(Image *image) {
    NanUtf8String *channel_ = channel.get();
    Magick::ChannelType channelType( Magick::UndefinedChannel );
    if ( channel_ != NULL ) {
      ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickChannelOptions, Magick::MagickFalse, **channel_);
      if ( type == -1 ) {
        throw ImageChannelException();
      }
      channelType = (Magick::ChannelType) type;
    }
    ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickNoiseOptions, Magick::MagickFalse, **noise);
    if ( type == -1 ) {
      throw ImageNoiseException();
    }
    if ( channelType != Magick::UndefinedChannel ) {
      image->GetMagickImage()->addNoiseChannel( channelType, (Magick::NoiseType) type );
    } else {
      image->GetMagickImage()->addNoise( (Magick::NoiseType) type );
    }
  }

  /* ImageNormalizeJob */

  ImageNormalizeJob::ImageNormalizeJob() : ImageProcessJob() {}

  void ImageNormalizeJob::ProcessImage(Image *image) {
    image->GetMagickImage()->normalize();
  }

  /* ImageOilJob */

  ImageOilJob::ImageOilJob() : ImageProcessJob() {}

  void ImageOilJob::Setup(double radius_) {
    radius = radius_;
    ImageProcessJob::Setup();
  }

  void ImageOilJob::ProcessImage(Image *image) {
    image->GetMagickImage()->oilPaint(radius);
  }

  /* ImagePingJob */

  ImagePingJob::ImagePingJob() : ImageProcessJob(true) {}

  void ImagePingJob::Setup(NanUtf8String *file_) {
    file.reset(file_);
    ImageProcessJob::Setup();
  }

  void ImagePingJob::Setup(char *data_, size_t length_) {
    data = data_;
    length = length_;
    ImageProcessJob::Setup();
  }

  void ImagePingJob::ProcessImage(Image *image) {
    Magick::Image mi;
    NanUtf8String *file_ = file.get();
    if (file_ == NULL) {
      Magick::Blob blob(data, length);
      mi.ping(blob);
    } else {
      mi.ping(**file_);
    }
    columns = mi.columns();
    rows = mi.rows();
  }

  Local<Value> ImagePingJob::ReturnedValue() {
    NanEscapableScope();
    Local<Object> result( NanNew<Array>(2) );
    result->Set( 0, NanNew<Integer>( (uint32_t) columns) );
    result->Set( 1, NanNew<Integer>( (uint32_t) rows) );
    return NanEscapeScope(result);
  }

  /* ImagePropertiesJob */

  ImagePropertiesJob::ImagePropertiesJob() : ImageProcessJob()
      , batch(-1), autoClose(-1), autoCopy(-1), magick(NULL), columns(-1), rows(-1), page(NULL)
      , fuzz(numeric_limits<double>::quiet_NaN()) {}

  void ImagePropertiesJob::Setup(bool readProperties_) {
    ImageProcessJob::Setup(readProperties_);
  }

  void ImagePropertiesJob::ProcessImage(Image *image) {
    Magick::Image * mi( image->GetMagickImage() );
    if ( dontCopy ) {
      batch        = image->IsPersistentBatch();
      autoClose    = image->autoClose;
      autoCopy     = image->autoCopy;
      magickStr    = mi->magick();
      pageGeometry = mi->page();
      columns      = mi->columns();
      rows         = mi->rows();
      background   = mi->backgroundColor();
      fuzz         = mi->colorFuzz();
    } else {
      if ( batch >= 0 )
        image->IsPersistentBatch( batch != 0 );
      if ( autoClose >= 0 )
        image->autoClose = autoClose != 0;
      if ( autoCopy >= 0 )
        image->autoCopy = autoCopy != 0;
      NanUtf8String *s = magick.get();
      if ( s != NULL )
        mi->magick( **s );
      if ( columns >= 0 ) {
        Magick::Geometry geometry( columns, rows >= 0 ? rows : mi->rows() );
        mi->size( geometry );
      } else if ( rows >= 0 ) {
        Magick::Geometry geometry( mi->columns(), rows );
        mi->size( geometry );
      }
      s = page.get();
      if ( s != NULL )
        mi->page( **s );
      if ( background.isValid() )
        mi->backgroundColor(background);
      if ( ! isnan(fuzz) )
        mi->colorFuzz(fuzz);
    }
  }

  Local<Value> ImagePropertiesJob::ReturnedValue() {
    NanEscapableScope();
    Local<Object> properties( NanNew<Object>() );
    properties->Set(NanNew(batchSym), NanNew<Boolean>( batch != 0 ) );
    properties->Set(NanNew(autoCloseSym), NanNew<Boolean>( autoClose != 0 ) );
    properties->Set(NanNew(autoCopySym), NanNew<Boolean>( autoCopy != 0 ) );
    properties->Set(NanNew(magickSym), NanNew<String>( magickStr ) );
    properties->Set(NanNew(columnsSym), NanNew<Integer>( (int32_t) columns ) );
    properties->Set(NanNew(rowsSym), NanNew<Integer>( (int32_t) rows ) );
    properties->Set(NanNew(pageSym), NanNew<String>( string(pageGeometry) ) );
    properties->Set(NanNew(backgroundSym), Color::NewColorObjectV8( background ) );
    properties->Set(NanNew(fuzzSym), NanNew<Number>( fuzz ) );
    return NanEscapeScope(properties);
  }

  /* ImageQualityJob */

  ImageQualityJob::ImageQualityJob() : ImageProcessJob() {}

  void ImageQualityJob::Setup(size_t quality_) {
    quality = quality_;
    ImageProcessJob::Setup();
  }

  void ImageQualityJob::ProcessImage(Image *image) {
    image->GetMagickImage()->quality(quality);
  }

  /* ImageQuantizeJob */

  ImageQuantizeJob::ImageQuantizeJob()
    : ImageProcessJob(), colors(0), colorSpace(NULL), dither(-1) {}

  void ImageQuantizeJob::Setup(size_t colors_, NanUtf8String *colorSpace_, char dither_) {
    dither = dither_;
    colorSpace.reset(colorSpace_);
    colors = colors_;
    ImageProcessJob::Setup();
  }

  void ImageQuantizeJob::ProcessImage(Image *image) {
    NanUtf8String *colorSpace_ = colorSpace.get();
    Magick::Image *mi = image->GetMagickImage();
    Magick::ColorspaceType savedColorSpace( Magick::UndefinedColorspace );
    size_t savedColors(0);
    char savedDither = -1;
    if ( colorSpace_ != NULL ) {
      ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickColorspaceOptions, Magick::MagickFalse, **colorSpace_);
      if ( type != -1 ) {
        savedColorSpace = mi->quantizeColorSpace();
        mi->quantizeColorSpace( (Magick::ColorspaceType) type );
      } else {
        throw ImageColorspaceException();
      }
    }
    if ( colors != 0 ) {
      savedColors = mi->quantizeColors();
      mi->quantizeColors(colors);
    }
    if ( dither >= 0 ) {
      savedDither = mi->quantizeDither() ? 1 : 0;
      mi->quantizeDither( dither != 0 );
    }
    mi->quantize();
    if ( savedColorSpace != Magick::UndefinedColorspace )
      mi->quantizeColorSpace(savedColorSpace);
    if ( savedColors != 0 )
      mi->quantizeColors(savedColors);
    if ( savedDither >= 0 )
      mi->quantizeDither( savedDither != 0 );
  }

  /* ImageReadJob */

  ImageReadJob::ImageReadJob() : ImageProcessJob(), file( NULL ), data( NULL ) {}

  void ImageReadJob::Setup(NanUtf8String *file_) {
    file.reset(file_);
    ImageProcessJob::Setup();
  }
  void ImageReadJob::Setup(char *data_, size_t length_) {
    data = data_;
    length = length_;
    ImageProcessJob::Setup();
  }

  void ImageReadJob::ProcessImage(Image *image) {
    NanUtf8String *file_ = file.get();
    if (file_ == NULL) {
      Magick::Blob blob(data, length);
      image->GetMagickImage()->read(blob);
    } else {
      image->GetMagickImage()->read(**file_);
    }
  }

  /* ImageResetJob */

  ImageResetJob::ImageResetJob() : ImageProcessJob() {}

  void ImageResetJob::Setup(Magick::Geometry& geometry_) {
    geometry = geometry_;
    ImageProcessJob::Setup();
  }

  void ImageResetJob::ProcessImage(Image *image) {
    if ( geometry.isValid() ) {
      image->GetMagickImage()->page(geometry);
    } else {
      Magick::Image *mi = image->GetMagickImage();
      mi->page( Magick::Geometry(mi->columns(), mi->rows(), 0, 0) );
    }
  }

  /* ImageResizeJob */

  ImageResizeJob::ImageResizeJob()
    : ImageProcessJob(), resizeType(ResizeFilter) {}

  void ImageResizeJob::Setup(Magick::Geometry& geometry_, char *mode) {
    geometry = geometry_;
    if ( mode != NULL ) ReadResizeMode(mode);
    ImageProcessJob::Setup();
  }

  void ImageResizeJob::ProcessImage(Image *image) {
    switch(resizeType) {
      case ResizeScale:
        image->GetMagickImage()->scale(geometry);
        break;
      case ResizeSample:
        image->GetMagickImage()->sample(geometry);
        break;
      default:
        image->GetMagickImage()->resize(geometry);
    }
  }

  /* ImageRestoreJob */

  ImageRestoreJob::ImageRestoreJob() : ImageProcessJob(true) {}

  void ImageRestoreJob::ProcessImage(Image *image) {
    image->RestoreImage();
  }

  bool ImageRestoreJob::HasReturnValue() {
    return false;
  }

  /* ImageRotateJob */

  ImageRotateJob::ImageRotateJob() : ImageProcessJob() {}

  void ImageRotateJob::Setup(double degrees_) {
    degrees = degrees_;
    ImageProcessJob::Setup();
  }

  void ImageRotateJob::ProcessImage(Image *image) {
    image->GetMagickImage()->rotate(degrees);
  }

  /* ImageSharpenJob */

  ImageSharpenJob::ImageSharpenJob()
    : ImageProcessJob(), sigma(0.0), radius(0.0), channel(NULL) {}

  void ImageSharpenJob::Setup(double sigma_, double radius_, NanUtf8String *channel_) {
    sigma = sigma_;
    radius = radius_;
    channel.reset(channel_);
    ImageProcessJob::Setup();
  }

  void ImageSharpenJob::ProcessImage(Image *image) {
    NanUtf8String *channel_ = channel.get();
    if ( channel_ != NULL ) {
      Magick::ChannelType channelType( Magick::UndefinedChannel );
      if ( channel_ != NULL ) {
        ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickChannelOptions, Magick::MagickFalse, **channel_);
        if ( type == -1 ) {
          throw ImageChannelException();
        }
        channelType = (Magick::ChannelType) type;
      }
      image->GetMagickImage()->sharpenChannel(channelType, radius, sigma);
    } else {
      image->GetMagickImage()->sharpen(radius, sigma);
    }
  }

  /* ImageSizeJob */

  ImageSizeJob::ImageSizeJob() : ImageProcessJob() {}

  void ImageSizeJob::Setup() {
    ImageProcessJob::Setup(true);
  }

  void ImageSizeJob::Setup(Magick::Geometry& geometry_) {
    geometry = geometry_;
    ImageProcessJob::Setup();
  }

  void ImageSizeJob::ProcessImage(Image *image) {
    if ( dontCopy ) {
      geometry = image->GetMagickImage()->size();
    } else {
      image->GetMagickImage()->size(geometry);
    }
  }

  Local<Value> ImageSizeJob::ReturnedValue() {
    NanEscapableScope();
    Local<Object> size( NanNew<Array>(2) );
    size->Set( 0, NanNew<Integer>( (int32_t) geometry.width() ) );
    size->Set( 1, NanNew<Integer>( (int32_t) geometry.height() ));
    return NanEscapeScope(size);
  }

  /* ImageStripJob */

  ImageStripJob::ImageStripJob() : ImageProcessJob() {}

  void ImageStripJob::ProcessImage(Image *image) {
    image->GetMagickImage()->strip();
  }

  /* ImageTrimJob */

  ImageTrimJob::ImageTrimJob() : ImageProcessJob() {}

  void ImageTrimJob::Setup() {
    fuzz = -1.0;
    ImageProcessJob::Setup();
  }

  void ImageTrimJob::Setup(double fuzz_) {
    fuzz = fuzz_;
    ImageProcessJob::Setup();
  }

  void ImageTrimJob::ProcessImage(Image *image) {
    if (fuzz < 0.0) {
      image->GetMagickImage()->trim();
    } else {
      Magick::Image *mi = image->GetMagickImage();
      double fuzz_ = mi->colorFuzz();
      mi->colorFuzz(fuzz);
      mi->trim();
      mi->colorFuzz(fuzz_);
    }
  }

  /* ImageTypeJob */

  ImageTypeJob::ImageTypeJob() : ImageProcessJob() {}

  void ImageTypeJob::Setup() {
    ImageProcessJob::Setup(true);
  }

  void ImageTypeJob::Setup(NanUtf8String *imageType_) {
    imageType.reset(imageType_);
    ImageProcessJob::Setup();
  }

  void ImageTypeJob::ProcessImage(Image *image) {
    if ( dontCopy ) {
      Magick::ImageType type( image->GetMagickImage()->type() );
      typeCstr = MagickCore::CommandOptionToMnemonic(MagickCore::MagickTypeOptions, (ssize_t) type);
    } else {
      ssize_t type = MagickCore::ParseCommandOption(MagickCore::MagickTypeOptions, Magick::MagickFalse, **imageType);
      if ( type != -1 ) {
        image->GetMagickImage()->type( (Magick::ImageType) type );
      } else {
        throw ImageTypeException();
      }
    }
  }

  Local<Value> ImageTypeJob::ReturnedValue() {
    if ( typeCstr != NULL )
      return NanNew<String>(typeCstr);
    else
      return NanNew<String>("unknown");
  }

  /* ImageWriteJob */

  ImageWriteJob::ImageWriteJob() : ImageProcessJob(), file(NULL), blob(NULL) {}

  void ImageWriteJob::Setup() {
    blob.reset( new Magick::Blob() );
    ImageProcessJob::Setup();
  }
  void ImageWriteJob::Setup(NanUtf8String *file_) {
    file.reset(file_);
    ImageProcessJob::Setup();
  }

  void ImageWriteJob::ProcessImage(Image *image) {
    if (file.get() != NULL) {
      image->GetMagickImage()->write(**file);
    } else {
      image->GetMagickImage()->write(blob.get());
    }
  }

  bool ImageWriteJob::HasReturnValue() {
    return blob.get() != NULL;
  }

  /* Buffer free memory callback */
  NAN_INLINE void ImageWriteJob::FreeBlob(char *, void *blob) {
    delete (Magick::Blob *)blob;
    // printf("freed buffer: %p\n", blob);
  }

  Local<Value> ImageWriteJob::ReturnedValue() {
    // Local<Value> bufargv[] = { NanNew<Integer>(blob->length()) };
    // const Local<Object> buffer(bufferFunction->NewInstance(1, bufargv));
    // void* out = buffer->GetIndexedPropertiesExternalArrayData();
    // memcpy(out, blob->data(), blob->length());
    // return buffer;

    /* Bind blob and use NO COPY Buffer  */
    Magick::Blob *blob_ = blob.release();
    return NanNewBufferHandle( (char *)blob_->data(), blob_->length(),
                          (NanFreeCallback) ImageWriteJob::FreeBlob, (void *)blob_ );
  }

}
