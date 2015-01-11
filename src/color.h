#if !defined(NODEMAGICK_COLOR_HEADER)
#define NODEMAGICK_COLOR_HEADER

#include "nodemagick.h"

using namespace node;
using namespace v8;

#define NODEMAGICK_VALUE_IS_COLOR(value) \
  ( NanHasInstance(Color::constructor, (value)) )

namespace NodeMagick {

  using namespace std;
  
  class Color : public ObjectWrap {
    public:
      Color(void);
      Color(const Color& color_);
      Color(const char *name);
      Color(Magick::Quantum red, Magick::Quantum green, Magick::Quantum blue);
      Color(Magick::Quantum red, Magick::Quantum green, Magick::Quantum blue, Magick::Quantum alpha);
      virtual ~Color();
      static void Init(Handle<Object> exports);
/*
      Disabled C++ implementation in favour of JS.
      The overhead of calling C++ is too large compared to function simplicity. (~x10)

      static NAN_GETTER(GetRed);
      static NAN_GETTER(GetGreen);
      static NAN_GETTER(GetBlue);
      static NAN_GETTER(GetAlpha);
      static NAN_METHOD(ScaleRed);
      static NAN_METHOD(ScaleGreen);
      static NAN_METHOD(ScaleBlue);
*/
      static NAN_METHOD(ScaleAlpha);
      static NAN_METHOD(ScaleHue);
      static NAN_METHOD(ScaleLightness);
      static NAN_METHOD(ScaleValue);
      static NAN_METHOD(ScaleBrightness);
      static NAN_METHOD(ScaleIntensity);
      static NAN_METHOD(ScaleY);
      static NAN_METHOD(ScaleU);
      static NAN_METHOD(ScaleV);
      static NAN_METHOD(ScaleHsl);
      static NAN_METHOD(ScaleHsla);
      static NAN_METHOD(ScaleRgb);
      static NAN_METHOD(ScaleRgba);
      static NAN_METHOD(ScaleYuv);
      static NAN_METHOD(ScaleYuva);
      static NAN_METHOD(ScaleHsv);
      static NAN_METHOD(ScaleHsva);
      static NAN_METHOD(ScaleHsb);
      static NAN_METHOD(ScaleHsba);
      static NAN_METHOD(ScaleHsi);
      static NAN_METHOD(ScaleHsia);
      static NAN_METHOD(ScaleHwb);
      static NAN_METHOD(ScaleHwba);
      static NAN_METHOD(ValueOf);
      static NAN_METHOD(ToString);
      static NAN_METHOD(New);
      // static NAN_METHOD(NewRgb); // implemented in JS
      static NAN_METHOD(NewYuv);
      static NAN_METHOD(NewHsl);
      static NAN_METHOD(NewHsv);
      static NAN_METHOD(NewHsb);
      static NAN_METHOD(NewHsi);
      static NAN_METHOD(NewHwb);
      // static NAN_METHOD(ToQuantum); // implemented in JS
      // static NAN_METHOD(FromQuantum); // implemented in JS
      static Persistent<FunctionTemplate> constructor;
      static Persistent<String> redSym;
      static Persistent<String> greenSym;
      static Persistent<String> blueSym;
      static Persistent<String> alphaSym;

      static Local<Value> NewColorObjectV8(const Magick::Color &magickcolor_);
      static Local<Value> NewColorObjectV8(Magick::Quantum colorParts[]);

      Magick::Color magickcolor;
    private:
  };

}
#endif
