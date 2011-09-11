/* This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"
#include "opts.h"
#include "tree.h"
#include "gimple.h"
#include "toplev.h"
#include "debug.h"
#include "options.h"
#include "flags.h"
#include "convert.h"
#include "diagnostic-core.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "target.h"

#include <gmp.h>
#include <mpfr.h>

#include "vec.h"
#include "hashtab.h"

#include "gjs.h"
#include "js-tree.h"
#include "js-op.h"

tree js_build_id(const char *str)
{
  char *name = xstrdup(str);
  int i, len = strlen(name);
  for (i = 0; i < len; i++) {
      if (name[i] == '$') {
          name[i] = '_';
      }
  }
  return get_identifier(name);
}

tree js_build_int(int val)
{
  return build_int_cst(integer_type_node, val);
}

tree js_build_float_str(const char *buf)
{
  REAL_VALUE_TYPE d;
  real_from_string(&d, buf);
  return build_real(double_type_node, d);
}

tree js_build_string(const char *str)
{
  int len = strlen(str);
  tree t = build_string(len, str);
  tree idxtype = build_index_type(build_int_cst(NULL_TREE, len));
  tree type = build_array_type(char_type_node, idxtype);
  TREE_TYPE(t) = type;
  TREE_CONSTANT(t) = true;
  TREE_READONLY(t) = true;
  TREE_STATIC(t) = true;
  return t;
}

