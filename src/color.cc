#include "color.h"

#if MAGICKCORE_QUANTUM_DEPTH > 8
  #define NODEMAGICK_NORMALIZE_8_QUANTUM(value) \
    do { value >>= MAGICKCORE_QUANTUM_DEPTH - 8; } while(0)
#else
#if MAGICKCORE_QUANTUM_DEPTH > 8
  #define NODEMAGICK_NORMALIZE_8_QUANTUM(value) \
    do { value <<= 8 - MAGICKCORE_QUANTUM_DEPTH; } while(0)
#else
  #define NODEMAGICK_NORMALIZE_8_QUANTUM(value)
#endif
#endif

#define NODEMAGICK_SCOPE_COLOR_UNWRAP() \
  NODEMAGICK_SCOPE_UNWRAP(Color, color)

#define NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC() \
  NODEMAGICK_SCOPE_COLOR_UNWRAP();           \
  uint32_t argc = args.Length()

#define NODEMAGICK_GETTER_COLORPART(Name, method)                    \
  NAN_GETTER(Color::Name) {                                          \
    NODEMAGICK_SCOPE_COLOR_UNWRAP();                                 \
    NanReturnValue( NanNew<Number>( color->magickcolor.method() ) ); \
  }

#define NODEMAGICK_METHOD_COLORPART(MethodName, name, ColorKlass)                           \
  NAN_METHOD(Color::MethodName) {                                                           \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                                   \
    if ( argc == 1 ) {                                                                      \
      if ( ! args[0]->IsUndefined() ) {                                                     \
        double scale = args[0]->NumberValue();                                              \
        NanReturnValue( NanNew<Number>( ColorKlass(color->magickcolor).name() * scale ) );  \
      }                                                                                     \
    } else if ( argc == 0 ) {                                                               \
      NanReturnValue( NanNew<Number>( ColorKlass(color->magickcolor).name() ) );            \
    }                                                                                       \
    return NanThrowTypeError("#name() argument should be a number");                        \
  }

#define NODEMAGICK_METHOD_COLORPART_CONVERT(MethodName, name, Conversion, argpos) \
  NAN_METHOD(Color::MethodName) {                                                 \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                         \
    Magick::Color &cmodel = color->magickcolor;                                   \
    double arg1, arg2, arg3;                                                      \
    Conversion(                                                                   \
        cmodel.redQuantum(),                                                      \
        cmodel.greenQuantum(),                                                    \
        cmodel.blueQuantum(),                                                     \
        &arg1, &arg2, &arg3 );                                                    \
    if ( argc == 1 ) {                                                            \
      if ( ! args[0]->IsUndefined() ) {                                           \
        double scale = args[0]->NumberValue();                                    \
        NanReturnValue( NanNew<Number>( argpos * scale ) );                       \
      }                                                                           \
    } else if ( argc == 0 ) {                                                     \
      NanReturnValue( NanNew<Number>( argpos ) );                                 \
    }                                                                             \
    return NanThrowTypeError("#name() argument should be a number");              \
  }

#define NODEMAGICK_METHOD_COLORARRAY_3(MethodName, name, ColorKlass, arg1, arg2, arg3) \
  NAN_METHOD(Color::MethodName) {                                             \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                     \
    ColorKlass cmodel( color->magickcolor );                                  \
    double arg1 = cmodel.arg1(), arg2 = cmodel.arg2(), arg3 = cmodel.arg3();  \
    Local<Array> array( NanNew<Array>(3) );                                   \
    if ( argc == 1 ) {                                                        \
      if ( ! args[0]->IsUndefined() ) {                                       \
        double scale = args[0]->NumberValue();                                \
        array->Set( 0, NanNew<Number>( arg1 * scale ) );                      \
        array->Set( 1, NanNew<Number>( arg2 * scale ) );                      \
        array->Set( 2, NanNew<Number>( arg3 * scale ) );                      \
        NanReturnValue( array );                                              \
      }                                                                       \
    } else if ( argc == 0 ) {                                                 \
      array->Set( 0, NanNew<Number>( arg1 ) );                                \
      array->Set( 1, NanNew<Number>( arg2 ) );                                \
      array->Set( 2, NanNew<Number>( arg3 ) );                                \
      NanReturnValue( array );                                                \
    }                                                                         \
    return NanThrowTypeError("#name() argument should be a number");          \
  }

#define NODEMAGICK_METHOD_COLORARRAY_ALPHA(MethodName, name, ColorKlass, arg1, arg2, arg3) \
  NAN_METHOD(Color::MethodName) {                                             \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                     \
    ColorKlass cmodel( color->magickcolor );                                  \
    double arg1 = cmodel.arg1(), arg2 = cmodel.arg2(), arg3 = cmodel.arg3(),  \
           alpha = cmodel.alpha();                                            \
    Local<Array> array( NanNew<Array>(4) );                                   \
    if ( argc == 1 ) {                                                        \
      if ( ! args[0]->IsUndefined() ) {                                       \
        double scale = args[0]->NumberValue();                                \
        array->Set( 0, NanNew<Number>( arg1 * scale ) );                      \
        array->Set( 1, NanNew<Number>( arg2 * scale ) );                      \
        array->Set( 2, NanNew<Number>( arg3 * scale ) );                      \
        array->Set( 3, NanNew<Number>( alpha * scale ) );                     \
        NanReturnValue( array );                                              \
      }                                                                       \
    } else if ( argc == 0 ) {                                                 \
      array->Set( 0, NanNew<Number>( arg1 ) );                                \
      array->Set( 1, NanNew<Number>( arg2 ) );                                \
      array->Set( 2, NanNew<Number>( arg3 ) );                                \
      array->Set( 3, NanNew<Number>( alpha ) );                               \
      NanReturnValue( array );                                                \
    }                                                                         \
    return NanThrowTypeError("#name() argument should be a number");          \
  }

#define NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(MethodName, name, Conversion) \
  NAN_METHOD(Color::MethodName) {                                            \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                    \
    Magick::Color &cmodel = color->magickcolor;                              \
    double arg1, arg2, arg3;                                                 \
    Conversion(                                                              \
        cmodel.redQuantum(),                                                 \
        cmodel.greenQuantum(),                                               \
        cmodel.blueQuantum(),                                                \
        &arg1, &arg2, &arg3 );                                               \
    Local<Array> array( NanNew<Array>(3) );                                  \
    if ( argc == 1 ) {                                                       \
      if ( ! args[0]->IsUndefined() ) {                                      \
        double scale = args[0]->NumberValue();                               \
        array->Set( 0, NanNew<Number>( arg1 * scale ) );                     \
        array->Set( 1, NanNew<Number>( arg2 * scale ) );                     \
        array->Set( 2, NanNew<Number>( arg3 * scale ) );                     \
        NanReturnValue( array );                                             \
      }                                                                      \
    } else if ( argc == 0 ) {                                                \
      array->Set( 0, NanNew<Number>( arg1 ) );                               \
      array->Set( 1, NanNew<Number>( arg2 ) );                               \
      array->Set( 2, NanNew<Number>( arg3 ) );                               \
      NanReturnValue( array );                                               \
    }                                                                        \
    return NanThrowTypeError("#name() argument should be a number");         \
  }

#define NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(MethodName, name, Conversion) \
  NAN_METHOD(Color::MethodName) {                                                \
    NODEMAGICK_SCOPE_COLOR_UNWRAP_ARGC();                                        \
    Magick::Color &cmodel = color->magickcolor;                                  \
    double arg1, arg2, arg3;                                                     \
    Conversion(                                                                  \
        cmodel.redQuantum(),                                                     \
        cmodel.greenQuantum(),                                                   \
        cmodel.blueQuantum(),                                                    \
        &arg1, &arg2, &arg3 );                                                   \
    Local<Array> array( NanNew<Array>(4) );                                      \
    if ( argc == 1 ) {                                                           \
      if ( ! args[0]->IsUndefined() ) {                                          \
        double scale = args[0]->NumberValue();                                   \
        array->Set( 0, NanNew<Number>( arg1 * scale ) );                         \
        array->Set( 1, NanNew<Number>( arg2 * scale ) );                         \
        array->Set( 2, NanNew<Number>( arg3 * scale ) );                         \
        array->Set( 3, NanNew<Number>( cmodel.alpha() * scale ) );               \
        NanReturnValue( array );                                                 \
      }                                                                          \
    } else if ( argc == 0 ) {                                                    \
      array->Set( 0, NanNew<Number>( arg1 ) );                                   \
      array->Set( 1, NanNew<Number>( arg2 ) );                                   \
      array->Set( 2, NanNew<Number>( arg3 ) );                                   \
      array->Set( 3, NanNew<Number>( cmodel.alpha() ) );                         \
      NanReturnValue( array );                                                   \
    }                                                                            \
    return NanThrowTypeError("#name() argument should be a number");             \
  }


#define NODEMAGICK_METHOD_COLORCONSTR(MethodName, name, ColorKlass)             \
  NAN_METHOD(Color::MethodName) {                                               \
    NODEMAGICK_SCOPE_ARGC();                                                     \
    if ( argc >= 3 && argc <= 4 ) {                                             \
      ColorKlass color(                                                         \
        args[0]->NumberValue(),                                                 \
        args[1]->NumberValue(),                                                 \
        args[2]->NumberValue() );                                               \
      if ( argc == 4 ) {                                                        \
        Magick::Color acolor(                                                   \
          color.redQuantum(),                                                   \
          color.greenQuantum(),                                                 \
          color.blueQuantum(),                                                  \
          Magick::Color::scaleDoubleToQuantum(args[3]->NumberValue()) );        \
        NanReturnValue( NewColorObjectV8(acolor) );                             \
      }                                                                         \
      NanReturnValue( NewColorObjectV8(color) );                                \
    }                                                                           \
    return NanThrowTypeError("Color.#name() arguments must be 3 or 4 numbers"); \
  }

#define NODEMAGICK_METHOD_COLORCONSTR_CONVERT(MethodName, name, Conversion)     \
  NAN_METHOD(Color::MethodName) {                                               \
    NODEMAGICK_SCOPE_ARGC();                                                    \
    if ( argc >= 3 && argc <= 4 ) {                                             \
      Magick::Quantum carg1, carg2, carg3;                                      \
      Conversion(                                                               \
          args[0]->NumberValue(),                                               \
          args[1]->NumberValue(),                                               \
          args[2]->NumberValue(),                                               \
          &carg1, &carg2, &carg3 );                                             \
      if ( argc == 4 ) {                                                        \
        Magick::Color color( carg1, carg2, carg3,                               \
          Magick::Color::scaleDoubleToQuantum(args[3]->NumberValue()) );        \
        NanReturnValue( NewColorObjectV8(color) );                              \
      }                                                                         \
      Magick::Color color( carg1, carg2, carg3 );                               \
      NanReturnValue( NewColorObjectV8(color) );                                \
    }                                                                           \
    return NanThrowTypeError("Color.#name() arguments must be 3 or 4 numbers"); \
  }


namespace NodeMagick {

  using namespace std;

  Persistent<FunctionTemplate> Color::constructor;
  Persistent<String> Color::redSym;
  Persistent<String> Color::greenSym;
  Persistent<String> Color::blueSym;
  Persistent<String> Color::alphaSym;

  void Color::Init(Handle<Object> exports) {
    NanAssignPersistent(redSym  , NanNew<String>("r"));
    NanAssignPersistent(greenSym, NanNew<String>("g"));
    NanAssignPersistent(blueSym , NanNew<String>("b"));
    NanAssignPersistent(alphaSym, NanNew<String>("a"));
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
    NanAssignPersistent(constructor, tpl);
    Local<String> colorSym = NanNew<String>("Color");
    tpl->SetClassName(colorSym);
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // NODE_SET_METHOD(tpl, "RGB", NewRgb);
    NODE_SET_METHOD(tpl, "YUV", NewYuv);
    NODE_SET_METHOD(tpl, "HSL", NewHsl);
    NODE_SET_METHOD(tpl, "HSV", NewHsv);
    NODE_SET_METHOD(tpl, "HSB", NewHsb);
    NODE_SET_METHOD(tpl, "HSI", NewHsi);
    NODE_SET_METHOD(tpl, "HWB", NewHwb);
/*
    NODE_SET_METHOD(tpl, "toQuantum", ToQuantum);
    NODE_SET_METHOD(tpl, "fromQuantum", FromQuantum);

    NODE_SET_PROTOTYPE_METHOD(tpl, "red",        ScaleRed);
    NODE_SET_PROTOTYPE_METHOD(tpl, "green",      ScaleGreen);
    NODE_SET_PROTOTYPE_METHOD(tpl, "blue",       ScaleBlue);
    NODE_SET_PROTOTYPE_METHOD(tpl, "alpha",      ScaleAlpha);
*/
    NODE_SET_PROTOTYPE_METHOD(tpl, "hue",        ScaleHue);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hue",        ScaleHue);
    NODE_SET_PROTOTYPE_METHOD(tpl, "lightness",  ScaleLightness);
    NODE_SET_PROTOTYPE_METHOD(tpl, "value",      ScaleValue);
    NODE_SET_PROTOTYPE_METHOD(tpl, "brightness", ScaleBrightness);
    NODE_SET_PROTOTYPE_METHOD(tpl, "intensity",  ScaleIntensity);
    NODE_SET_PROTOTYPE_METHOD(tpl, "y",          ScaleY);
    NODE_SET_PROTOTYPE_METHOD(tpl, "u",          ScaleU);
    NODE_SET_PROTOTYPE_METHOD(tpl, "v",          ScaleV);
    NODE_SET_PROTOTYPE_METHOD(tpl, "rgb",        ScaleRgb);
    NODE_SET_PROTOTYPE_METHOD(tpl, "rgba",       ScaleRgba);
    NODE_SET_PROTOTYPE_METHOD(tpl, "yuv",        ScaleYuv);
    NODE_SET_PROTOTYPE_METHOD(tpl, "yuva",       ScaleYuva);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsl",        ScaleHsl);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsla",       ScaleHsla);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsv",        ScaleHsv);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsva",       ScaleHsva);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsb",        ScaleHsb);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsba",       ScaleHsba);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsi",        ScaleHsi);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hsia",       ScaleHsia);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hwb",        ScaleHwb);
    NODE_SET_PROTOTYPE_METHOD(tpl, "hwba",       ScaleHwba);
    NODE_SET_PROTOTYPE_METHOD(tpl, "valueOf",    ValueOf);
    NODE_SET_PROTOTYPE_METHOD(tpl, "toString",   ToString);

/*
    // Local<ObjectTemplate> proto = tpl->PrototypeTemplate();

    Local<ObjectTemplate> i_t = tpl->InstanceTemplate();
    i_t->SetAccessor(NanNew<String>("r"), GetRed);
    i_t->SetAccessor(NanNew<String>("g"), GetGreen);
    i_t->SetAccessor(NanNew<String>("b"), GetBlue);
    i_t->SetAccessor(NanNew<String>("a"), GetAlpha);
*/
    exports->Set(colorSym, NanNew<FunctionTemplate>(constructor)->GetFunction());
  }

  Color::Color(void)
    : magickcolor(0,0,0,(Magick::Quantum)-1) {}
  Color::Color(const Color& color_)
    : magickcolor(color_.magickcolor) {}
  Color::Color(const char *name)
    : magickcolor(name) {}
  Color::Color(Magick::Quantum red, Magick::Quantum green, Magick::Quantum blue)
    : magickcolor(red, green, blue) {}
  Color::Color(Magick::Quantum red, Magick::Quantum green, Magick::Quantum blue, Magick::Quantum alpha)
    : magickcolor(red, green, blue, alpha) {}
  Color::~Color(void) {
    //printf("Color freed\n");
  }

  /**
   * Color object factory
   *
   * new Color(redQuantum, greenQuantum, blueQuantum[, alphaQuantum=opaque])
   * new Color(color)
   * new Color(value)
   *
   * colorQuantum: an integer from 0 - magick.QUANTUM_RANGE
   * color:        color string or Color object instance
   * value:        a 32-bit integer color value
   **/
  NAN_METHOD(Color::New) {
    NanScope();
    if (args.IsConstructCall()) {

      Color *color(NULL);
      uint32_t argc = args.Length();

      try {
        if ( argc == 1 ) {
          if ( args[0]->IsString() ) {
            NanUtf8String name( args[0] );
            color = new Color(*name);
          } else if ( args[0]->IsUint32() || args[0]->IsInt32() ) {
            uint32_t value = args[0]->Uint32Value();
            unsigned char alpha, red, green, blue;
            blue = value & 0xFF;
            green = (value >>= 8) & 0xFF;
            red = (value >>= 8) & 0xFF;
            alpha = (value >>= 8) & 0xFF;
            color = new Color(red, green, blue, alpha);
          } else if ( NODEMAGICK_VALUE_IS_COLOR(args[0]) ) {
            color = ObjectWrap::Unwrap<Color>( args[0].As<Object>() );
            color = new Color( *color );
          }
        } else if ( argc >= 3 && argc <= 4 ) {
          Magick::Quantum red, green, blue, alpha;
          red = args[0]->Uint32Value();
          green = args[1]->Uint32Value();
          blue = args[2]->Uint32Value();
          if ( argc == 4 ) {
            alpha = args[3]->Uint32Value();
            color = new Color(red, green, blue, alpha);
          } else {
            color = new Color(red, green, blue);
          }
        } else if ( argc == 0 ) {
          color = new Color();
        }
      } catch (exception& err) {
          return NanThrowError(err.what());
      } catch (...) {
          return NanThrowError("unhandled error");
      }

      if ( color == NULL )
        return NanThrowTypeError("Color() constructor needs String, or (1, 3 or 4) Integers");

      Local<Object> self = args.This();

      color->Wrap(self);

      Magick::Color *mc = &color->magickcolor;

      self->Set(NanNew(redSym),   NanNew<Integer>(mc->redQuantum()),   ReadOnly);
      self->Set(NanNew(greenSym), NanNew<Integer>(mc->greenQuantum()), ReadOnly);
      self->Set(NanNew(blueSym),  NanNew<Integer>(mc->blueQuantum()),  ReadOnly);
      self->Set(NanNew(alphaSym), NanNew<Integer>(mc->alphaQuantum()), ReadOnly);

      NanReturnValue(self);
    } else {
      const uint32_t argc = args.Length();
      vector<Local<Value> > argv;
      for (uint32_t i = 0; i < argc; ++i) {
        argv.push_back(args[i]);
      }
      NanReturnValue(NanNew<FunctionTemplate>(constructor)->GetFunction()->NewInstance(argc, &argv[0]));
    }
  }

/*
  NODEMAGICK_GETTER_COLORPART(GetRed,   redQuantum)
  NODEMAGICK_GETTER_COLORPART(GetGreen, greenQuantum)
  NODEMAGICK_GETTER_COLORPART(GetBlue,  blueQuantum)
  NODEMAGICK_GETTER_COLORPART(GetAlpha, alphaQuantum)
*/
  /**
   * Red color
   *
   * color.red([scale=1]) -> number
   **/
  // NODEMAGICK_METHOD_COLORPART(ScaleRed,   red,   Magick::ColorRGB)
  /**
   * Green color
   *
   * color.red([scale=1]) -> number
   **/
  // NODEMAGICK_METHOD_COLORPART(ScaleGreen, green, Magick::ColorRGB)
  /**
   * Blue color
   *
   * color.red([scale=1]) -> number
   **/
  // NODEMAGICK_METHOD_COLORPART(ScaleBlue,  blue,  Magick::ColorRGB)
  /**
   * Alpha color
   *
   * color.alpha([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART(ScaleAlpha, alpha, Magick::ColorRGB)

  /**
   * Y color (from YUV model)
   *
   * color.y([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART(ScaleY, y, Magick::ColorYUV)
  /**
   * U color (from YUV model)
   *
   * color.u([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART(ScaleU, u, Magick::ColorYUV)
  /**
   * V color (from YUV model)
   *
   * color.v([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART(ScaleV, v, Magick::ColorYUV)

  /**
   * Hue color (from HSB model)
   *
   * color.hue([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART_CONVERT(ScaleHue,        hue,         MagickCore::ConvertRGBToHSB, arg1)
  /**
   * Lightness color (from HSL model)
   *
   * color.lightness([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART_CONVERT(ScaleLightness,  lightness,   MagickCore::ConvertRGBToHSL, arg3)
  /**
   * Value color (from HSV model)
   *
   * color.value([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART_CONVERT(ScaleValue,      value,       MagickCore::ConvertRGBToHSV, arg3)
  /**
   * Brightness color (from HSB model)
   *
   * color.brightness([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART_CONVERT(ScaleBrightness, brightness,  MagickCore::ConvertRGBToHSB, arg3)
  /**
   * Intensity color (from HSI model)
   *
   * color.intensity([scale=1]) -> number
   **/
  NODEMAGICK_METHOD_COLORPART_CONVERT(ScaleIntensity,  intensity,   MagickCore::ConvertRGBToHSI, arg3)

  /**
   * RGB scaled color (from RGB model)
   *
   * color.rgb([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_3(ScaleRgb,         rgb,  Magick::ColorRGB, red, green, blue)
  /**
   * RGBA scaled color (from RGB model)
   *
   * color.rgba([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_ALPHA(ScaleRgba,    rgba, Magick::ColorRGB, red, green, blue)
  /**
   * YUV scaled color (from YUV model)
   *
   * color.yuv([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_3(ScaleYuv,         yuv,  Magick::ColorYUV, y, u, v)
  /**
   * YUVA scaled color (from YUV model)
   *
   * color.yuva([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_ALPHA(ScaleYuva,    yuva, Magick::ColorYUV, y, u, v)
  /**
   * HSL scaled color (from HSL model)
   *
   * color.hsl([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(ScaleHsl,  hsl,  MagickCore::ConvertRGBToHSL)
  /**
   * HSLA scaled color (from HSL model)
   *
   * color.hsla([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(ScaleHsla,    hsla, MagickCore::ConvertRGBToHSL)
  /**
   * HSV scaled color (from HSV model)
   *
   * color.hsv([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(ScaleHsv,  hsv,  MagickCore::ConvertRGBToHSV)
  /**
   * HSVA scaled color (from HSV model)
   *
   * color.hsva([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(ScaleHsva,    hsva, MagickCore::ConvertRGBToHSV)
  /**
   * HSB scaled color (from HSB model)
   *
   * color.hsb([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(ScaleHsb,  hsb,  MagickCore::ConvertRGBToHSB)
  /**
   * HSBA scaled color (from HSB model)
   *
   * color.hsba([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(ScaleHsba,    hsba, MagickCore::ConvertRGBToHSB)
  /**
   * HSI scaled color (from HSI model)
   *
   * color.hsi([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(ScaleHsi,  hsi,  MagickCore::ConvertRGBToHSI)
  /**
   * HSIA scaled color (from HSI model)
   *
   * color.hsia([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(ScaleHsia,    hsia, MagickCore::ConvertRGBToHSI)
  /**
   * HWB scaled color (from HWB model)
   *
   * color.hwb([scale=1]) -> Array(3)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_3(ScaleHwb,  hwb,  MagickCore::ConvertRGBToHWB)
  /**
   * HWBA scaled color (from HWB model)
   *
   * color.hwba([scale=1]) -> Array(4)
   **/
  NODEMAGICK_METHOD_COLORARRAY_CONVERT_ALPHA(ScaleHwba,    hwba, MagickCore::ConvertRGBToHWB)

  /**
   * RGB Color object factory
   *
   *   Color.RGB(red, green, blue[, alpha=opaque]) -> color
   **/
  // NODEMAGICK_METHOD_COLORCONSTR(NewRgb, rgb, Magick::ColorRGB)
  /**
   * YUV Color object factory
   *
   *   Color.YUV(y, u, v[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR(NewYuv, yuv, Magick::ColorYUV)
  /**
   * HSL Color object factory
   *
   *   Color.HSL(hue, saturation, lightness[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR_CONVERT(NewHsl, hsl, MagickCore::ConvertHSLToRGB)
  /**
   * HSV Color object factory
   *
   *   Color.HSV(hue, saturation, value[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR_CONVERT(NewHsv, hsv, MagickCore::ConvertHSVToRGB)
  /**
   * HSB Color object factory
   *
   *   Color.HSB(hue, saturation, brightness[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR_CONVERT(NewHsb, hsb, MagickCore::ConvertHSBToRGB)
  /**
   * HSI Color object factory
   *
   *   Color.HSI(hue, saturation, intensity[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR_CONVERT(NewHsi, hsi, MagickCore::ConvertHSIToRGB)
  /**
   * HWB Color object factory
   *
   *   Color.HWB(hue, whiteness, blackness[, alpha=opaque]) -> color
   **/
  NODEMAGICK_METHOD_COLORCONSTR_CONVERT(NewHwb, hwb, MagickCore::ConvertHWBToRGB)

  /**
   * Value of color
   *
   * color.valueOf() -> a 32-bit integer color value
   **/
  NAN_METHOD(Color::ValueOf) {
    NODEMAGICK_SCOPE_COLOR_UNWRAP();
    unsigned int red = color->magickcolor.redQuantum();
    NODEMAGICK_NORMALIZE_8_QUANTUM(red);
    unsigned int green = color->magickcolor.greenQuantum();
    NODEMAGICK_NORMALIZE_8_QUANTUM(green);
    unsigned int blue = color->magickcolor.blueQuantum();
    NODEMAGICK_NORMALIZE_8_QUANTUM(blue);
    unsigned int alpha = color->magickcolor.alphaQuantum();
    NODEMAGICK_NORMALIZE_8_QUANTUM(alpha);
    uint32_t value = blue | (red << 8) | (green << 16) | (alpha << 24);
    NanReturnValue( NanNew<Integer>(value) );
  }

  /**
   * String name of color
   *
   * color.toString() -> an x11/html color value
   **/
  NAN_METHOD(Color::ToString) {
    NODEMAGICK_SCOPE_COLOR_UNWRAP();
    NanReturnValue( NanNew<String>( string(color->magickcolor) ) );
  }

  /**
   * Quantum from normalized value
   *
   *   Color.toQuantum(value) -> integer
   **/
/*
  NAN_METHOD(Color::ToQuantum) {
    NODEMAGICK_SCOPE_ARGC();
    if ( argc == 1 && ! args[0]->IsUndefined() ) {
      double value( args[0]->NumberValue() );
      if ( value < -1.0 ) value = -1.0; else if ( value > 1.0 ) value = 1.0;
      NanReturnValue( NanNew<Integer>(Magick::Color::scaleDoubleToQuantum(value)) );
    }
    NanReturnValue( NanNew<Integer>((double)(1L << MAGICKCORE_QUANTUM_DEPTH)) );
  }
*/
  /**
   * Normalized value form quantum
   *
   *   Color.fromQuantum(quantum) -> number
   **/
/*
  NAN_METHOD(Color::FromQuantum) {
    NODEMAGICK_SCOPE_ARGC();
    if ( argc == 1 && ! args[0]->IsUndefined() ) {
      double value( args[0]->NumberValue() );
      NanReturnValue( NanNew<Number>(Magick::Color::scaleQuantumToDouble(value)) );
    }
    return NanThrowTypeError("Color.fromQuantum() needs a quantum number");
  }
*/
  Local<Value> Color::NewColorObjectV8(const Magick::Color &magickcolor_) {
    Magick::Quantum colorParts[] = {
      magickcolor_.redQuantum(),
      magickcolor_.greenQuantum(),
      magickcolor_.blueQuantum(),
      magickcolor_.alphaQuantum()
    };
    return Color::NewColorObjectV8(colorParts);
  }

  Local<Value> Color::NewColorObjectV8(Magick::Quantum colorParts[]) {
    NanEscapableScope();
    Local<Value> argv[] = {
      NanNew<Integer>(colorParts[0]),
      NanNew<Integer>(colorParts[1]),
      NanNew<Integer>(colorParts[2]),
      NanNew<Integer>(colorParts[3])
    };
    Local<Object> newColor = NanNew<FunctionTemplate>(constructor)->GetFunction()->NewInstance(4, argv);
    return NanEscapeScope( newColor );
  }

}
