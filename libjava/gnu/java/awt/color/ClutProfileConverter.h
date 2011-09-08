
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_color_ClutProfileConverter__
#define __gnu_java_awt_color_ClutProfileConverter__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace awt
      {
        namespace color
        {
            class ClutProfileConverter;
            class ColorLookUpTable;
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
      namespace color
      {
          class ICC_Profile;
      }
    }
  }
}

class gnu::java::awt::color::ClutProfileConverter : public ::java::lang::Object
{

public:
  ClutProfileConverter(::java::awt::color::ICC_Profile *);
  virtual JArray< jfloat > * toCIEXYZ(JArray< jfloat > *);
  virtual JArray< jfloat > * toRGB(JArray< jfloat > *);
  virtual JArray< jfloat > * fromCIEXYZ(JArray< jfloat > *);
  virtual JArray< jfloat > * fromRGB(JArray< jfloat > *);
private:
  ::gnu::java::awt::color::ColorLookUpTable * __attribute__((aligned(__alignof__( ::java::lang::Object)))) toPCS;
  ::gnu::java::awt::color::ColorLookUpTable * fromPCS;
  jint nChannels;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_color_ClutProfileConverter__
