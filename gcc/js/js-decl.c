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

static jstree JSTREE_NEW(JSType type, JSOperator op, jstree lhs, jstree rhs)
{
  jstree expr = JSTREE_ALLOC();
  JSTREE_TYPE(expr) = type;
  JSTREE_OP(expr)   = op;
  JSTREE_LHS(expr) = lhs;
  JSTREE_RHS(expr) = rhs;
  JSTREE_CHAIN(expr) = NULL;
  return expr;
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

jstree js_build_array(jstree list)
{
  return js_build1(OP_ARRAY, TyArray, list);
}

jstree js_build_nop(void)
{
  return js_build1(OP_NOP, TyNone, NULL);
}

jstree js_build1(JSOperator op, JSType type, jstree lhs)
{
  jstree expr = JSTREE_NEW(type, op, lhs, NULL);
  return expr;
}

jstree js_build2(JSOperator op, JSType type, jstree lhs, jstree rhs)
{
  jstree expr = JSTREE_NEW(type, op, lhs, rhs);
  return expr;
}

jstree js_build3(JSOperator op, JSType type, jstree lhs, jstree mhs, jstree rhs)
{
  jstree cons = JSTREE_NEW(TyNone, OP_NOP, mhs, rhs);
  jstree expr = JSTREE_NEW(type, op, lhs, cons);
  return expr;
}

jstree js_build_call(JSOperator op, JSType type, jstree f, jstree params)
{
  jstree expr = JSTREE_NEW(type, op, f, params);
  return expr;
}

jstree js_build_loop(enum loopmode loop, jstree init , jstree cond, jstree inc, jstree body)
{
  jstree expr = (init)? (init):JSTREE_NEW(TyNone, OP_NOP, NULL, NULL);
  if (loop == LOOP_FOR || loop == LOOP_FOR_VAR) {
    /*
     * init -> body -> inc -> cond 
     *          |               |
     *          +---------------+
     */
    jstree exitExpr = JSTREE_NEW(TyNone, OP_EXIT, cond, NULL);
    jstree loopExpr = JSTREE_NEW(TyNone, OP_LOOP, body, NULL);
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
    jstree exitExpr = JSTREE_NEW(TyNone, OP_EXIT, cond, NULL);
    jstree loopExpr = JSTREE_NEW(TyNone, OP_LOOP, body, NULL);
    JSTREE_APPENDTAIL(body, inc);
    JSTREE_APPENDTAIL(body, exitExpr);
    JSTREE_APPENDTAIL(expr, loopExpr);
    TODO();
    return expr;
  }
  TODO();
  return NULL;
}

jstree js_build_cond(jstree cond, jstree thenExpr, jstree elseExpr)
{
  jstree cons = JSTREE_NEW(TyNone, OP_NOP, thenExpr, elseExpr);
  jstree expr = JSTREE_NEW(TyNone, OP_COND, cond, cons);
  return expr;
}

static int tmpvalue = 0;
jstree js_build_defun(jstree name, jstree params, jstree body)
{
  if (name == NULL) {
      char buf[32] = {};
      sprintf(buf, "__tmp%d", tmpvalue++);
      name = js_build_id(buf);
  }
  return js_build3(OP_DEFUN, TyFunction, name, params, body);
}

