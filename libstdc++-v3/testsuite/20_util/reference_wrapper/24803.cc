// { dg-options "-std=gnu++0x" }
// { dg-do compile }

// Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.


#include <functional>

struct test_type
{
   int member();
   int cmember()const;
   int member2(char);
   int cmember2(char)const;
};

struct functor1 : public std::unary_function<int, double>
{
  double operator()(int) const;
};

struct functor2 : public std::binary_function<int, char, double>
{
   double operator()(int, char) const;
};

template <class T>
void verify_return_type(T, T)
{
}

void test01()
{
  int zero;

  std::reference_wrapper<double (int)>* pr1(0);
  verify_return_type((*pr1)(0), double());
  std::reference_wrapper<double (*)(int)>* pr2(0);
  verify_return_type((*pr2)(0), double());
  std::reference_wrapper<functor1>* pr5(0);

  // libstdc++/24803
  verify_return_type((*pr5)(0), double());
  verify_return_type((*pr5)(zero), double());

  std::reference_wrapper<double (int, char)>* pr1b(0);
  verify_return_type((*pr1b)(0, 0), double());
  std::reference_wrapper<double (*)(int, char)>* pr2b(0);
  verify_return_type((*pr2b)(0, 0), double());
  std::reference_wrapper<functor2>* pr5b(0);

  // libstdc++/24803
  verify_return_type((*pr5b)(0, 0), double());
  verify_return_type((*pr5b)(zero, zero), double());
}
