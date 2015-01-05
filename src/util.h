#if !defined(NODEMAGICK_UTIL_HEADER)
#define NODEMAGICK_UTIL_HEADER

#include "nodemagick.h"
#include "color.h"

using namespace node;
using namespace v8;

namespace NodeMagick {

  using namespace std;

  static NAN_INLINE bool SetColorFromString(Magick::Color &color, const char *colorstr, const char **errmsg) {
    try {
      color = colorstr;
      return true;
    } catch (exception& err) {
      if ( errmsg != NULL ) *errmsg = err.what();
    } catch (...) {
      if ( errmsg != NULL ) *errmsg = "unhandled error";
    }
    return false;
  }

  static NAN_INLINE const Magick::Color &GetColorFromV8ColorObject(const Handle<Object> &object) {
    NanScope();
    return ObjectWrap::Unwrap<Color>( object )->magickcolor;
  }

  static NAN_INLINE bool SetColorFromV8Value(Magick::Color &color, Handle<Value> value, const char **errmsg) {
    if ( value->IsString() ) {
      return SetColorFromString(color, *NanUtf8String( value ), errmsg);
    } else if ( NODEMAGICK_VALUE_IS_COLOR(value) ) {
      color = GetColorFromV8ColorObject( value.As<Object>() );
      return true;
    }
    if ( errmsg != NULL ) *errmsg = "can't deduce color from value";
    return false;
  }

  static NAN_INLINE Magick::GravityType GetGravityFromString(const char *gravcstr) {
    if ( strcasecmp(gravcstr, "northwest") == 0 )
      return Magick::NorthWestGravity;
    else if ( strcasecmp(gravcstr, "north") == 0 )
      return Magick::NorthGravity;
    else if ( strcasecmp(gravcstr, "northeast") == 0 )
      return Magick::NorthEastGravity;
    else if ( strcasecmp(gravcstr, "west") == 0 )
      return Magick::WestGravity;
    else if ( strcasecmp(gravcstr, "center") == 0 )
      return Magick::CenterGravity;
    else if ( strcasecmp(gravcstr, "east") == 0 )
      return Magick::EastGravity;
    else if ( strcasecmp(gravcstr, "southwest") == 0 )
      return Magick::SouthWestGravity;
    else if ( strcasecmp(gravcstr, "south") == 0 )
      return Magick::SouthGravity;
    else if ( strcasecmp(gravcstr, "southeast") == 0 )
      return Magick::SouthEastGravity;
    else
      return Magick::ForgetGravity;
  }

  static NAN_INLINE bool SetGeometryFromV8Array(Magick::Geometry &geometry, const Local<Array> &size) {
    NanScope();
    if ( size->Length() >= 2 ) {
      Local<Value> cols( size->Get(0) );
      Local<Value> rows( size->Get(1) );
      if ( cols->IsUint32() && rows->IsUint32() ) {
        geometry.width( cols->Uint32Value() );
        geometry.height( rows->Uint32Value() );
        if ( size->Length() >= 4 ) {
          Local<Value> xoff( size->Get(2) );
          Local<Value> yoff( size->Get(3) );
          if ( xoff->IsInt32() && yoff->IsInt32() ) {
            geometry.xOff( xoff->Int32Value() );
            geometry.yOff( yoff->Int32Value() );
          }
        }
        return true;
      }
    }
    return false;
  }

  static char tagdelimiters[] = " \t\n\r,.;:/?";

  /* tags is NULL terminated array of char[], searched string is searched for tags, returns tag index */
  static int FindTag(const char * const tags[], char *searchedstr) {
    char *found = strtok(searchedstr, tagdelimiters);
    while ( found != NULL ) {
      const char *tag;
      for ( int i = 0; ( tag = tags[i] ) != NULL; ++i ) {
        if ( strcasecmp(tag, found) == 0 )
          return i;
      }
      found = strtok(NULL, tagdelimiters);
    }
    return -1;
  }

  static NAN_INLINE int FindNextTag(const char * const tags[]) {
    return FindTag(tags, NULL);
  }
}

#endif