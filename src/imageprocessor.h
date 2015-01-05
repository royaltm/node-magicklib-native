#if !defined(NODEMAGICK_IMAGEPROCESSOR_HEADER)
#define NODEMAGICK_IMAGEPROCESSOR_HEADER

#include <exception>
#include "nodemagick.h"
#include "job.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

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

  class ImagePixel {
    public:
      ImagePixel(ssize_t x_, ssize_t y_) : x( x_ ), y( y_ ) {}
      ssize_t x, y;
      Magick::Quantum colorParts[4];
  };

  class Image;

  class ImageProcessJob : public Job {
    public:
      ImageProcessJob(void);
      ImageProcessJob(bool dontCopy_);
      void Setup(void);
      void Setup(bool dontCopy_);
      virtual void ProcessImage(Image *image_);
      virtual bool HasReturnValue(void);
      bool IsValid(void);
      bool DontCopy(void);
    protected:
      bool valid;
      bool dontCopy;
  };


  /* ImageProcessJob subclasses */


  class ImageBlurJob : public ImageProcessJob {
    public:
      ImageBlurJob(void);
      void Setup(double sigma_, double radius_, NanUtf8String *channel_, bool gaussian_);
      void ProcessImage(Image *image_);
    private:
      double sigma, radius;
      bool gaussian;
      auto_ptr<NanUtf8String> channel;
  };

  class ImageCloseJob : public ImageProcessJob {
    public:
      ImageCloseJob(void);
      void ProcessImage(Image *image_);
      bool HasReturnValue();
  };

  class ImageColorJob : public ImageProcessJob {
    public:
      ImageColorJob(void);
      void Setup(ssize_t x_, ssize_t y_);
      void Setup(size_t numpoints);
      void Setup(ssize_t x_, ssize_t y_, Magick::Color& color_);
      void Push(ssize_t x_, ssize_t y_);
      void ProcessImage(Image *image_);
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
      void ProcessImage(Image *image_);
      Local<Value> ReturnedValue(void);
    private:
      string commentStr;
      auto_ptr<NanUtf8String> comment;
  };

  class ImageCopyJob : public ImageProcessJob {
    public:
      ImageCopyJob(void);
      void Setup(void);
      void Setup(bool autoCopy_);
      void ProcessImage(Image *image_);
      bool HasReturnValue(void);
    private:
      char autoCopy;
  };

  class ImageCropJob : public ImageProcessJob {
    public:
      ImageCropJob(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image_);
    private:
      Magick::Geometry geometry;
  };

  class ImageExtentJob : public ImageProcessJob {
    public:
      ImageExtentJob(void);
      void Setup(Magick::Geometry& geometry_);
      void Setup(Magick::Geometry& geometry_, Magick::GravityType gravity_);
      void Setup(Magick::Geometry& geometry_, Magick::Color& color_);
      void Setup(Magick::Geometry& geometry_, Magick::Color& color_, Magick::GravityType gravity_);
      void ProcessImage(Image *image_);
    private:
      Magick::Geometry geometry;
      Magick::Color color;
      Magick::GravityType gravity;
  };

  class ImageFilterJob : public ImageProcessJob {
    public:
      ImageFilterJob(void);
      void Setup(NanUtf8String *filter_);
      void ProcessImage(Image *image_);
    private:
      auto_ptr<NanUtf8String> filter;
  };

  class ImageFlipJob : public ImageProcessJob {
    public:
      ImageFlipJob(void);
      void ProcessImage(Image *image_);
  };

  class ImageFlopJob : public ImageProcessJob {
    public:
      ImageFlopJob(void);
      void ProcessImage(Image *image_);
  };

  class ImageFormatJob : public ImageProcessJob {
    public:
      ImageFormatJob(void);
      void Setup(NanUtf8String *format_);
      void ProcessImage(Image *image_);
    private:
      auto_ptr<NanUtf8String> format;
  };

  class ImageHistogramJob : public ImageProcessJob {
    public:
      ImageHistogramJob(void);
      void ProcessImage(Image *image_);
      Local<Value> ReturnedValue(void);
    private:
#ifdef NODEMAGICK_USE_STL_MAP
      map<Magick::Color,size_t> histogram;
#else
      vector<pair<Magick::Color,size_t> > histogram;
#endif
  };

  class ImageNoiseJob : public ImageProcessJob {
    public:
      ImageNoiseJob(void);
      void Setup(NanUtf8String *noise_);
      void Setup(NanUtf8String *noise_, NanUtf8String *channel_);
      void ProcessImage(Image *image_);
    private:
      auto_ptr<NanUtf8String> noise;
      auto_ptr<NanUtf8String> channel;
  };

  class ImagePropertiesJob : public ImageProcessJob {
    friend class Image;
    public:
      ImagePropertiesJob(void);
      void Setup(bool readProperties_);
      void ProcessImage(Image *image_);
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
      void ProcessImage(Image *image_);
    private:
      size_t quality;
  };

  class ImageQuantizeJob : public ImageProcessJob {
    public:
      ImageQuantizeJob(void);
      void Setup(size_t colors_, NanUtf8String *colorSpace_ = NULL, char dither_ = -1);
      void ProcessImage(Image *image_);
    private:
      size_t colors;
      auto_ptr<NanUtf8String> colorSpace;
      char dither;
  };

  class ImageReadJob : public ImageProcessJob {
    public:
      ImageReadJob(void);
      void Setup(NanUtf8String *file_);
      void Setup(char *data_, ssize_t length_);
      void ProcessImage(Image *image_);
    private:
      auto_ptr<NanUtf8String> file;
      char *data;
      ssize_t length;
  };

  class ImageResetJob : public ImageProcessJob {
    public:
      ImageResetJob(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image_);
    private:
      Magick::Geometry geometry;
  };

  class ImageResizeJob : public ImageProcessJob {
    public:
      ImageResizeJob(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image_);
    private:
      Magick::Geometry geometry;
  };

  class ImageRestoreJob : public ImageProcessJob {
    public:
      ImageRestoreJob(void);
      void ProcessImage(Image *image_);
      bool HasReturnValue();
  };

  class ImageRotateJob : public ImageProcessJob {
    public:
      ImageRotateJob(void);
      void Setup(double degrees);
      void ProcessImage(Image *image_);
    private:
      double degrees;
  };

  class ImageSharpenJob : public ImageProcessJob {
    public:
      ImageSharpenJob(void);
      void Setup(double sigma);
      void Setup(double sigma, double radius);
      void ProcessImage(Image *image_);
    private:
      double sigma, radius;
  };

  class ImageSizeJob : public ImageProcessJob {
    public:
      ImageSizeJob(void);
      void Setup(void);
      void Setup(Magick::Geometry& geometry_);
      void ProcessImage(Image *image_);
      Local<Value> ReturnedValue(void);
    private:
      Magick::Geometry geometry;
  };

  class ImageStripJob : public ImageProcessJob {
    public:
      ImageStripJob(void);
      void ProcessImage(Image *image_);
  };

  class ImageTrimJob : public ImageProcessJob {
    public:
      ImageTrimJob(void);
      void Setup(void);
      void Setup(double fuzz_);
      void ProcessImage(Image *image_);
    private:
      double fuzz;
  };

  class ImageTypeJob : public ImageProcessJob {
    public:
      ImageTypeJob(void);
      void Setup(void);
      void Setup(NanUtf8String *imageType_);
      void ProcessImage(Image *image_);
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
      void ProcessImage(Image *image_);
      bool HasReturnValue(void);
      Local<Value> ReturnedValue(void);
      static NAN_INLINE void FreeBlob(char *data, void *hint);
    private:
      auto_ptr<NanUtf8String> file;
      auto_ptr<Magick::Blob> blob;
  };

}

#endif