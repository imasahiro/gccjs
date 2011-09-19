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

static jstree JSTREE_NEW(location_t loc, JSType type, JSOperator op, jstree lhs, jstree rhs)
{
  jstree expr = JSTREE_ALLOC();
  JSTREE_LOC(expr) = loc;
  JSTREE_TYPE(expr) = type;
  JSTREE_OP(expr)   = op;
  JSTREE_LHS(expr) = lhs;
  JSTREE_RHS(expr) = rhs;
  JSTREE_CHAIN(expr) = NULL;
  return expr;
}
static gjs_tree_common *JSTREE_COMMON_NEW(location_t loc, JSType type, JSOperator op)
{
  gjs_tree_common *node = JSTREE_COMMON_ALLOC();
  JSTREE_LOC(node) = loc;
  JSTREE_TYPE(node) = type;
  JSTREE_OP(node)   = op;
  return node;
}

jstree js_build_id(location_t loc, const char *str)
{
  gjs_tree_common *id;
  char *name = xstrdup(str);
  int i, len = strlen(name);
  for (i = 0; i < len; i++) {
      if (name[i] == '$') {
          name[i] = '_';
      }
  }
  id = JSTREE_COMMON_NEW(loc, TyValue, OP_IDENTIFIER);
  JSTREE_COMMON_STRING(id) = name;
  return (jstree)id;
}

jstree js_build_int(location_t loc, int val)
{
  gjs_tree_common *n;
  n = JSTREE_COMMON_NEW(loc, TyInt, OP_INTEGER);
  JSTREE_COMMON_INT(n) = val;
  return (jstree) n;
}

jstree js_build_bool(location_t loc, int val)
{
  gjs_tree_common *n;
  n = JSTREE_COMMON_NEW(loc, TyBoolean, OP_BOOL);
  JSTREE_COMMON_INT(n) = val;
  return (jstree) n;
}

jstree js_build_nulval(location_t loc, int v/*v=0=>null, v=1=>undefined*/)
{
  JSType type = (v)?TyNULL:TyUndefined;
  JSOperator op = (v)?OP_NULL:OP_Undef;
  gjs_tree_common *n = JSTREE_COMMON_NEW(loc, type, op);
  return (jstree) n;
}

jstree js_build_float_str(location_t loc, const char *buf)
{
  char *fstr = xstrdup(buf);
  gjs_tree_common *n = JSTREE_COMMON_NEW(loc, TyFloat, OP_FLOAT);
  JSTREE_COMMON_FLOAT(n) = fstr;
  return (jstree) n;
}

jstree js_build_string(location_t loc, const char *str)
{
  char *newstr = xstrdup(str);
  gjs_tree_common *n;
  n = JSTREE_COMMON_NEW(loc, TyString, OP_STRING);
  JSTREE_COMMON_FLOAT(n) = newstr;
  return (jstree) n;
}

jstree js_build_array(location_t loc, jstree list)
{
  return js_build1(loc, OP_ARRAY, TyArray, list);
}

jstree js_build_nop(location_t loc)
{
  return js_build1(loc, OP_NOP, TyNone, NULL);
}

jstree js_build1(location_t loc, JSOperator op, JSType type, jstree lhs)
{
  jstree expr = JSTREE_NEW(loc, type, op, lhs, NULL);
  return expr;
}

jstree js_build2(location_t loc, JSOperator op, JSType type, jstree lhs, jstree rhs)
{
  jstree expr = JSTREE_NEW(loc, type, op, lhs, rhs);
  return expr;
}

jstree js_build3(location_t loc, JSOperator op, JSType type, jstree lhs, jstree mhs, jstree rhs)
{
  jstree cons = JSTREE_NEW(loc, TyNone, OP_NOP, mhs, rhs);
  jstree expr = JSTREE_NEW(loc, type, op, lhs, cons);
  return expr;
}

jstree js_build_call(location_t loc, JSOperator op, JSType type, jstree f, jstree params)
{
  jstree expr = JSTREE_NEW(loc, type, op, f, params);
  return expr;
}

jstree js_build_loop(location_t loc, enum loopmode loop, jstree init , jstree cond, jstree inc, jstree body)
{
  jstree expr = (init)? (init):JSTREE_NEW(loc, TyNone, OP_NOP, NULL, NULL);
  if (loop == LOOP_FOR || loop == LOOP_FOR_VAR) {
    /*
     * init -> body -> inc -> cond 
     *          |               |
     *          +---------------+
     */
    jstree exitExpr = JSTREE_NEW(loc, TyNone, OP_EXIT, cond, NULL);
    jstree loopExpr = JSTREE_NEW(loc, TyNone, OP_LOOP, body, NULL);
    JSTREE_APPENDTAIL(body, inc);
    JSTREE_APPENDTAIL(body, exitExpr);
    JSTREE_APPENDTAIL(expr, loopExpr);
    return expr;
  }
  if (loop == LOOP_FOR_IN || loop == LOOP_FOR_VAR_IN) {
    /*
     * init -> body -> inc -> cond 
     *          |               |
     *          +---------------+
     */
    jstree exitExpr = JSTREE_NEW(loc, TyNone, OP_EXIT, cond, NULL);
    jstree loopExpr = JSTREE_NEW(loc, TyNone, OP_LOOP, body, NULL);
    JSTREE_APPENDTAIL(body, inc);
    JSTREE_APPENDTAIL(body, exitExpr);
    JSTREE_APPENDTAIL(expr, loopExpr);
    TODO();
    return expr;
  }
  TODO();
  return NULL;
}

jstree js_build_cond(location_t loc, jstree cond, jstree thenExpr, jstree elseExpr)
{
  jstree cons = JSTREE_NEW(loc, TyNone, OP_NOP, thenExpr, elseExpr);
  jstree expr = JSTREE_NEW(loc, TyNone, OP_COND, cond, cons);
  return expr;
}

jstree js_build_defun(location_t loc, jstree name, jstree params, jstree body)
{
  if (name == NULL) {
      static int tmpvalue = 0;
      static char buf[32] = {};
      sprintf(buf, "__tmp%d", tmpvalue++);
      name = js_build_id(loc, buf);
  }
  return js_build3(loc, OP_DEFUN, TyFunction, name, params, body);
}

