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
#include "js-op.h"
typedef enum jsfieldop {
    FIELD_NONE,
    FIELD_SET,
    FIELD_GET
} jsfieldop;

static void DUMP_TREE(jstree t);
static void DUMP_TREE_FIELD(jsfieldop op, jstree t)
{
  switch(op) {
    case FIELD_NONE:
      break;
    case FIELD_SET:
      fprintf(stderr, "set ");
      break;
    case FIELD_GET:
      fprintf(stderr, "get ");
      break;
  }
  DUMP_TREE(JSTREE_LHS(t));
  fprintf(stderr, ".");
  DUMP_TREE(JSTREE_RHS(t));
  fprintf(stderr, "%p", t);
}

static void DUMP_TREE_CALL(const char *name, jstree t)
{
  fprintf(stderr, "%s ", name);
  DUMP_TREE(JSTREE_LHS(t));
  fprintf(stderr, "(");
  DUMP_TREE(JSTREE_RHS(t));
  fprintf(stderr, ")");
}

static void DUMP_TREE(jstree t)
{
  switch(JSTREE_OP(t)) {
    case OP_Undef:
      fprintf(stderr, "undefined");
      break;
    case OP_NULL:
      fprintf(stderr, "null");
      break;
    case OP_BOOL:
      fprintf(stderr, "bool(%s)", JSTREE_COMMON_INT(t)?"true":"false");
      break;
    case OP_INTEGER:
      fprintf(stderr, "int(%d)", JSTREE_COMMON_INT(t));
      break;
    case OP_FLOAT:
      fprintf(stderr, "float(%s)", JSTREE_COMMON_STRING(t));
      break;
    case OP_STRING:
      fprintf(stderr, "string(%s)", JSTREE_COMMON_STRING(t));
      break;
    case OP_LET:
    case OP_EQLET:
      fprintf(stderr, "let ");
      DUMP_TREE(JSTREE_LHS(t));
      fprintf(stderr, "= ");
      DUMP_TREE(JSTREE_RHS(t));
      break;
    case OP_DEFUN:
      fprintf(stderr, "defun ");
      DUMP_TREE(JSTREE_LHS(t));
      fprintf(stderr, "= ");
      DUMP_TREE(JSTREE_RHS(t));
      break;
    case OP_NEW:
      DUMP_TREE_CALL("new", t);
      break;
    case OP_IDENTIFIER:
      fprintf(stderr, "id:%s", JSTREE_COMMON_STRING(t));
      break;
    case OP_PARM:
      fprintf(stderr, "param=%p", t);
      break;
    case OP_FIELD:
      DUMP_TREE_FIELD(FIELD_NONE, t);
      break;
    case OP_GetField:
      DUMP_TREE_FIELD(FIELD_GET, t);
      break;
    case OP_SetField:
      DUMP_TREE_FIELD(FIELD_SET, t);
      break;
    case OP_CALL:
      DUMP_TREE_CALL("call", t);
      break;
    case OP_RETURN:
      fprintf(stderr, "return ");
      DUMP_TREE(JSTREE_LHS(t));
      break;
    default:
      fprintf(stderr, "'OP=%d, %p'", JSTREE_OP(t), t);
      break;
  }
  if (JSTREE_CHAIN(t)) {
      fprintf(stderr, ";\n");
      DUMP_TREE(JSTREE_CHAIN(t));
  }
}

void jstree_dump(jstree t)
{
  DUMP_TREE(t);
  fprintf(stderr, "\n");
}

jstree js_build_id(const char *str)
{
  gjs_tree_common *id;
  char *name = xstrdup(str);
  int i, len = strlen(name);
  for (i = 0; i < len; i++) {
      if (name[i] == '$') {
          name[i] = '_';
      }
  }
  id = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(id) = TyValue;
  JSTREE_OP(id)   = OP_IDENTIFIER;
  JSTREE_COMMON_STRING(id) = name;
  return (jstree)id;
}

jstree js_build_int(int val)
{
  gjs_tree_common *n;
  n = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(n) = TyInt;
  JSTREE_OP(n) = OP_INTEGER;
  JSTREE_COMMON_INT(n) = val;
  return (jstree) n;
}

jstree js_build_bool(int val)
{
  gjs_tree_common *n;
  n = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(n) = TyBoolean;
  JSTREE_OP(n) = OP_BOOL;
  JSTREE_COMMON_INT(n) = val;
  return (jstree) n;
}

jstree js_build_nulval(int v/*v=0=>null, v=1=>undefined*/)
{
  gjs_tree_common *n = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(n) = (v)?TyNULL:TyUndefined;
  JSTREE_OP(n) = OP_NULL;
  return (jstree) n;
}
jstree js_build_float_str(const char *buf)
{
  char *fstr = xstrdup(buf);
  gjs_tree_common *n = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(n) = TyFloat;
  JSTREE_OP(n) = OP_FLOAT;
  JSTREE_COMMON_FLOAT(n) = fstr;
  return (jstree) n;
}

jstree js_build_string(const char *str)
{
  char *newstr = xstrdup(str);
  gjs_tree_common *n;
  n = JSTREE_COMMON_ALLOC();
  JSTREE_TYPE(n) = TyString;
  JSTREE_OP(n) = OP_STRING;
  JSTREE_COMMON_FLOAT(n) = newstr;
  return (jstree) n;
}

jstree js_build2(JSOperator op, JSType type, jstree lhs, jstree rhs)
{
  jstree expr = JSTREE_ALLOC();
  JSTREE_TYPE(expr) = type;
  JSTREE_OP(expr)   = op;
  JSTREE_LHS(expr) = lhs;
  JSTREE_RHS(expr) = rhs;
  return expr;
}

jstree js_build_call(JSType type, jstree f, jstree params)
{
  jstree expr = JSTREE_ALLOC();
  JSTREE_TYPE(expr) = type;
  JSTREE_OP(expr)   = OP_CALL;
  JSTREE_LHS(expr) = f;
  JSTREE_RHS(expr) = params;
  return expr;
}

