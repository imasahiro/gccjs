
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_validation_xmlschema_AnyAttribute__
#define __gnu_xml_validation_xmlschema_AnyAttribute__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace validation
      {
        namespace datatype
        {
            class Annotation;
        }
        namespace xmlschema
        {
            class AnyAttribute;
        }
      }
    }
  }
}

class gnu::xml::validation::xmlschema::AnyAttribute : public ::java::lang::Object
{

public: // actually package-private
  AnyAttribute(::java::lang::String *, jint);
  static const jint STRICT = 0;
  static const jint LAX = 1;
  static const jint SKIP = 2;
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) namespace$;
  jint processContents;
  ::gnu::xml::validation::datatype::Annotation * annotation;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_validation_xmlschema_AnyAttribute__
