
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_StreamHolder__
#define __gnu_CORBA_StreamHolder__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class StreamHolder;
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
    }
  }
}

class gnu::CORBA::StreamHolder : public ::java::lang::Object
{

public:
  StreamHolder(::org::omg::CORBA::portable::InputStream *);
  virtual ::org::omg::CORBA::TypeCode * _type();
  virtual void _write(::org::omg::CORBA::portable::OutputStream *);
  virtual void _read(::org::omg::CORBA::portable::InputStream *);
public: // actually package-private
  virtual ::org::omg::CORBA::portable::InputStream * getInputStream();
public: // actually protected
  ::org::omg::CORBA::portable::InputStream * __attribute__((aligned(__alignof__( ::java::lang::Object)))) stream;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_StreamHolder__
