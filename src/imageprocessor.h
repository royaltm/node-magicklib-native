#if !defined(NODEMAGICK_IMAGEPROCESSOR_HEADER)
#define NODEMAGICK_IMAGEPROCESSOR_HEADER

#include <exception>
#include "nodemagick.h"
#include "job.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  class ImageOptionsException: public exception {
    public:
      ImageOptionsException(const char *message_) {
        message = message_;
      }
      virtual const char* what(void) const throw() {
        return message;
      }
    private:
      const char *message;
      ImageOptionsException(void);
  };

  class ImageSynchronizeException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Could not synchronize images";
      }
  };

  class ImageFilterException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized filter";
      }
  };

  class ImageColorspaceException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized colorspace";
      }
  };

  class ImageTypeException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized image type";
      }
  };

  class ImageChannelException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized channel type";
      }
  };

  class ImageNoiseException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized noise type";
      }
  };

  class ImageCompositeOperatorException: public exception {
    public:
      virtual const char* what(void) const throw() {
        return "Unrecognized composite operator";
      }
  };

  class ImagePixel {
    public:
      ImagePixel(ssize_t x_, ssize_t y_) : x( x_ ), y( y_ ) {}
      ssize_t x, y;
      Magick::Quantum colorParts[4];
  };

  class ImageMutualKit {
    public:
      ImageMutualKit(void) {
        uv_mutex_init(&guard);
        uv_barrier_init(&barrier, 2);
      }

      virtual ~ImageMutualKit(void) {
        uv_barrier_destroy(&barrier);
        uv_mutex_destroy(&guard);
      }
      NAN_INLINE uv_mutex_t *GuardMutex(void) { return &guard;   }
      NAN_INLINE uv_barrier_t *Barrier(void)  { return &barrier; }
    private:
      uv_mutex_t guard;
      uv_barrier_t barrier;
  };

  class Image;

  class ImageProcessJob : public Job {
    public:
      ImageProcessJob(void);
      ImageProcessJob(bool dontCopy_);
      void Setup(void);
      void Setup(bool dontCopy_);
      virtual void ProcessImage(Image *image);
      virtual bool HasReturnValue(void);
      bool IsValid(void);
      bool DontCopy(void);
    protected:
      bool valid;
      bool dontCopy;
  };


  /* ImageProcessJob subclasses */

  class ImageSynchronizeJob;
  class ImageMutualProcessJob : public ImageProcessJob {
    friend class ImageSynchronizeJob;
    public:
      ImageMutualProcessJob(ImageMutualKit &kit);
      virtual ~ImageMutualProcessJob(void);
      void Setup(Image *source_);
      ImageSynchronizeJob *SetupSynchronizeJob(ImageSynchronizeJob *job_=NULL);
      void ProcessImage(Image *image);
      virtual void ProcessImagesSynchronized(Image *image, Image *source);
    private:
      ImageSynchronizeJob *job;
      Image *source;
      uv_mutex_t *guard;
      uv_barrier_t *barrier;
      bool haveWait;
      ImageMutualProcessJob(void);
  };

  class ImageSynchronizeJob : public ImageProcessJob {
    friend class ImageMutualProcessJob;
    public:
      ImageSynchronizeJob(ImageMutualKit &kit);
      virtual ~ImageSynchronizeJob(void);
      void Setup(ImageMutualProcessJob &job_);
      void ProcessImage(Image *image);
      bool HasReturnValue(void);
    private:
      ImageMutualProcessJob *job;
      uv_mutex_t *guard;
      bool haveWait;
      ImageSynchronizeJob(void);
  };

  class ImageBlurJob : public ImageProcessJob {
    public:
      ImageBlurJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(double sigma_, double radius_, NanUtf8String *channel_, bool gaussian_);
      void ProcessImage(Image *image);
    private:
      double sigma, radius;
      bool gaussian;
      auto_ptr<NanUtf8String> channel;
  };

  class ImageCloseJob : public ImageProcessJob {
    public:
      ImageCloseJob(void);
      void ProcessImage(Image *image);
      bool HasReturnValue();
  };

  class ImageColorJob : public ImageProcessJob {
    public:
      ImageColorJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(const Handle<Array> &points);
      void Setup(ssize_t x_, ssize_t y_);
      void Setup(ssize_t x_, ssize_t y_, Magick::Color& color_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      bool readPixel;
      ssize_t x, y;
      Magick::Color color;
      auto_ptr<vector<ImagePixel> > points;
  };

  class ImageCommentJob : public ImageProcessJob {
    public:
      ImageCommentJob(void);
      void Setup(void);
      void Setup(NanUtf8String *comment_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      string commentStr;
      auto_ptr<NanUtf8String> comment;
  };

  typedef enum {
    CompositeGravity, CompositeGeometry, CompositeOffset
  } CompositeType;

  class ImageCompositeJob : public ImageMutualProcessJob {
    public:
      ImageCompositeJob(ImageMutualKit &kit);
      Image *Setup(const Handle<Object> &options, Handle<Object> &sourceObject);
      void Setup(Image *source_, NanUtf8String *compose_, Magick::Geometry &geometry_);
      void Setup(Image *source_, NanUtf8String *compose_, Magick::GravityType gravity_);
      void Setup(Image *source_, NanUtf8String *compose_, ssize_t x_, ssize_t y_);
      void ProcessImagesSynchronized(Image *image, Image *source);
    private:
      CompositeType type;
      ssize_t x, y;
      Magick::Geometry geometry;
      Magick::GravityType gravity;
      auto_ptr<NanUtf8String> compose;
  };

  class ImageCopyJob : public ImageProcessJob {
    public:
      ImageCopyJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(void);
      void Setup(bool autoCopy_);
      void ProcessImage(Image *image);
      bool HasReturnValue(void);
    private:
      char autoCopy;
  };

  class ImageCropJob : public ImageProcessJob {
    public:
      ImageCropJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image);
    private:
      Magick::Geometry geometry;
  };

  class ImageExtentJob : public ImageProcessJob {
    public:
      ImageExtentJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(Magick::Geometry& geometry_);
      void Setup(Magick::Geometry& geometry_, Magick::GravityType gravity_);
      void Setup(Magick::Geometry& geometry_, Magick::Color& color_);
      void Setup(Magick::Geometry& geometry_, Magick::Color& color_, Magick::GravityType gravity_);
      void ProcessImage(Image *image);
    private:
      Magick::Geometry geometry;
      Magick::Color color;
      Magick::GravityType gravity;
  };

  class ImageFilterJob : public ImageProcessJob {
    public:
      ImageFilterJob(void);
      void Setup(NanUtf8String *filter_);
      void ProcessImage(Image *image);
    private:
      auto_ptr<NanUtf8String> filter;
  };

  class ImageFlipJob : public ImageProcessJob {
    public:
      ImageFlipJob(void);
      void ProcessImage(Image *image);
  };

  class ImageFlopJob : public ImageProcessJob {
    public:
      ImageFlopJob(void);
      void ProcessImage(Image *image);
  };

  class ImageFormatJob : public ImageProcessJob {
    public:
      ImageFormatJob(void);
      void Setup(NanUtf8String *format_);
      void ProcessImage(Image *image);
    private:
      auto_ptr<NanUtf8String> format;
  };

  class ImageHistogramJob : public ImageProcessJob {
    public:
      ImageHistogramJob(void);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
#ifdef NODEMAGICK_USE_STL_MAP
      map<Magick::Color,size_t> histogram;
#else
      vector<pair<Magick::Color,size_t> > histogram;
#endif
  };

  class ImageNegateJob : public ImageProcessJob {
    public:
      ImageNegateJob(void);
      void Setup(bool grayscale_ = false);
      void ProcessImage(Image *image);
    private:
      bool grayscale;
  };

  class ImageNoiseJob : public ImageProcessJob {
    public:
      ImageNoiseJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(NanUtf8String *noise_);
      void Setup(NanUtf8String *noise_, NanUtf8String *channel_);
      void ProcessImage(Image *image);
    private:
      auto_ptr<NanUtf8String> noise;
      auto_ptr<NanUtf8String> channel;
  };

  class ImageNormalizeJob : public ImageProcessJob {
    public:
      ImageNormalizeJob(void);
      void ProcessImage(Image *image);
  };

  class ImageOilJob : public ImageProcessJob {
    public:
      ImageOilJob(void);
      void Setup(double radius_);
      void ProcessImage(Image *image);
    private:
      double radius;
  };

  class ImagePingJob : public ImageProcessJob {
    public:
      ImagePingJob(void);
      void Setup(NanUtf8String *file_);
      void Setup(char *data_, size_t length_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      auto_ptr<NanUtf8String> file;
      char *data;
      size_t length, columns, rows;
  };

  class ImagePropertiesJob : public ImageProcessJob {
    public:
      ImagePropertiesJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(bool readProperties_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      char batch;
      char autoClose;
      char autoCopy;
      auto_ptr<NanUtf8String> magick;
      string magickStr;
      ssize_t columns;
      ssize_t rows;
      auto_ptr<NanUtf8String> page;
      Magick::Geometry pageGeometry;
      Magick::Color background;
      double fuzz;
  };

  class ImageQualityJob : public ImageProcessJob {
    public:
      ImageQualityJob(void);
      void Setup(size_t quality_);
      void ProcessImage(Image *image);
    private:
      size_t quality;
  };

  class ImageQuantizeJob : public ImageProcessJob {
    public:
      ImageQuantizeJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(size_t colors_, NanUtf8String *colorSpace_ = NULL, char dither_ = -1);
      void ProcessImage(Image *image);
    private:
      size_t colors;
      auto_ptr<NanUtf8String> colorSpace;
      char dither;
  };

  class ImageReadJob : public ImageProcessJob {
    public:
      ImageReadJob(void);
      void Setup(NanUtf8String *file_);
      void Setup(char *data_, size_t length_);
      void ProcessImage(Image *image);
    private:
      auto_ptr<NanUtf8String> file;
      char *data;
      size_t length;
  };

  class ImageResetJob : public ImageProcessJob {
    public:
      ImageResetJob(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image);
    private:
      Magick::Geometry geometry;
  };

  typedef enum {
    ResizeFilter, ResizeSample, ResizeScale
  } ResizeType;

  class ImageResizeJob : public ImageProcessJob {
    public:
      ImageResizeJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(Magick::Geometry& geometry_, char *mode = NULL);
      void ProcessImage(Image *image);
    private:
      static const char * const ResizeTags[];
      static const char ResizeTagValues[];
      NAN_INLINE void ReadResizeMode(char *mode);
      Magick::Geometry geometry;
      ResizeType resizeType;
  };

  class ImageRestoreJob : public ImageProcessJob {
    public:
      ImageRestoreJob(void);
      void ProcessImage(Image *image);
      bool HasReturnValue();
  };

  class ImageRotateJob : public ImageProcessJob {
    public:
      ImageRotateJob(void);
      void Setup(double degrees);
      void ProcessImage(Image *image);
    private:
      double degrees;
  };

  class ImageSharpenJob : public ImageProcessJob {
    public:
      ImageSharpenJob(void);
      void Setup(const Handle<Object> &options);
      void Setup(double sigma_, double radius_, NanUtf8String *channel_);
      void ProcessImage(Image *image);
    private:
      double sigma, radius;
      auto_ptr<NanUtf8String> channel;
  };

  class ImageSizeJob : public ImageProcessJob {
    public:
      ImageSizeJob(void);
      void Setup(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      Magick::Geometry geometry;
  };

  class ImageStripJob : public ImageProcessJob {
    public:
      ImageStripJob(void);
      void ProcessImage(Image *image);
  };

  class ImageTrimJob : public ImageProcessJob {
    public:
      ImageTrimJob(void);
      void Setup(void);
      void Setup(double fuzz_);
      void ProcessImage(Image *image);
    private:
      double fuzz;
  };

  class ImageTypeJob : public ImageProcessJob {
    public:
      ImageTypeJob(void);
      void Setup(void);
      void Setup(NanUtf8String *imageType_);
      void ProcessImage(Image *image);
      Local<Value> ReturnedValue(void);
    private:
      const char *typeCstr;
      auto_ptr<NanUtf8String> imageType;
  };

  class ImageWriteJob : public ImageProcessJob {
    public:
      ImageWriteJob(void);
      void Setup(void);
      void Setup(NanUtf8String *file_);
      void ProcessImage(Image *image);
      bool HasReturnValue(void);
      Local<Value> ReturnedValue(void);
      static NAN_INLINE void FreeBlob(char *data, void *hint);
    private:
      auto_ptr<NanUtf8String> file;
      auto_ptr<Magick::Blob> blob;
  };

}

#endif