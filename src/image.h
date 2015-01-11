#if !defined(NODEMAGICK_IMAGE_HEADER)
#define NODEMAGICK_IMAGE_HEADER

#include "nodemagick.h"
#include "color.h"
#include "imageprocessor.h"
#include "worker.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  class Image
    : public ObjectWrap, public ImageMutualKit, public Worker<ImageProcessJob> {
  /* the order of super classes is important:
     ObjectWrap should be 1st for native wraping to work
     and ImageMutualKit before Worker to be destroyed after Worker clears jobs */
    friend class ImagePropertiesJob;
    friend class ImageCopyJob;
    friend class ImageSynchronizeJob;
    friend class ImageMutualProcessJob;
    public:
      Image(void);
      Image(const Image& image_);
      Image(const char *filepath);
      Image(const Magick::Blob &blob);
      Image(const char *geometry, const char *color);
      Image(const char *geometry, const Magick::Color &color);
      Image(size_t width, size_t height, const char *color);
      Image(size_t width, size_t height, const Magick::Color &color);
      virtual ~Image();
      bool IsEmpty(void);
      void CloneImage(void);
      void RestoreImage(void);
      void CloseImage(void);
      Magick::Image *GetMagickImage(void);
      static void Init(Handle<Object> exports);
      static NAN_GETTER(GetIsBusy);
      static NAN_GETTER(GetIsEmpty);
      static NAN_GETTER(GetBatchSize);
      static NAN_GETTER(GetIsSync);
      static NAN_GETTER(GetBatch);
      static NAN_SETTER(SetBatch);
      static NAN_GETTER(GetAutoClose);
      static NAN_SETTER(SetAutoClose);
      static NAN_GETTER(GetAutoCopy);
      static NAN_SETTER(SetAutoCopy);
      static NAN_GETTER(GetColumns);
      static NAN_SETTER(SetColumns);
      static NAN_GETTER(GetRows);
      static NAN_SETTER(SetRows);
      static NAN_GETTER(GetMagick);
      static NAN_SETTER(SetMagick);
      static NAN_GETTER(GetPage);
      static NAN_SETTER(SetPage);
      static NAN_GETTER(GetBackground);
      static NAN_SETTER(SetBackground);
      static NAN_GETTER(GetFuzz);
      static NAN_SETTER(SetFuzz);
      static NAN_METHOD(Begin);
      static NAN_METHOD(Blur);
      static NAN_METHOD(Close);
      static NAN_METHOD(Color);
      static NAN_METHOD(Comment);
      static NAN_METHOD(Composite);
      static NAN_METHOD(Copy);
      static NAN_METHOD(Crop);
      static NAN_METHOD(End);
      static NAN_METHOD(Extent);
      static NAN_METHOD(Filter);
      static NAN_METHOD(Flip);
      static NAN_METHOD(Flop);
      static NAN_METHOD(Format);
      static NAN_METHOD(Histogram);
      static NAN_METHOD(Negate);
      static NAN_METHOD(Noise);
      static NAN_METHOD(Normalize);
      static NAN_METHOD(Oil);
      static NAN_METHOD(Ping);
      static NAN_METHOD(Properties);
      static NAN_METHOD(Quality);
      static NAN_METHOD(Quantize);
      static NAN_METHOD(Read);
      static NAN_METHOD(Reset);
      static NAN_METHOD(Resize);
      static NAN_METHOD(Restore);
      static NAN_METHOD(Rotate);
      static NAN_METHOD(Sharpen);
      static NAN_METHOD(Size);
      static NAN_METHOD(Strip);
      static NAN_METHOD(Trim);
      static NAN_METHOD(Type);
      static NAN_METHOD(Write);
      static NAN_METHOD(New);
      static NAN_METHOD(UnderscoreHold);
      static NAN_METHOD(UnderscoreWrap);

      static Persistent<FunctionTemplate> constructor;

      static const char * const ResizeTags[];
      static const char ResizeTagValues[];

    protected:
      NAN_INLINE void ProcessJob(ImageProcessJob &job);
      NAN_INLINE virtual Local<Value> ReturnedValue(void);
      NAN_INLINE virtual Local<Value> ReturnedValue(Local<Object> self);
      NAN_INLINE void JobAfterComplete(bool isAsync);

      auto_ptr<Magick::Image> magickimage;
      auto_ptr<Magick::Image> magickcopy;
      bool isCloned;

      /* properties */
      bool autoCopy;
      bool autoClose;

      uv_mutex_t imagemutex;

    private:
      NAN_INLINE Local<Value> NewImageCopyObjectV8(void);
      void operator=(const Image&);
  };

}
#endif
