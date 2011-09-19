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

jstree global_tree = NULL;

typedef struct js_typeinfo {
    jstree this_node;
    jstree body_node;
    jstree fields_node;
    struct js_typeinfo *next;
    int id;
    JSType base;
    int fields_size;
    JSType* fields;
    JSType retTy;
} * js_typeinfo;

typedef struct jscontext {
    int id;
    FILE *fp;
    js_typeinfo type_info;
    js_typeinfo func_info;
} jsctx;

jstree jstree_pass_dump(jstree t, jsctx *ctx);
jstree jstree_pass_collect_typeinfo(jstree t, jsctx *ctx);
jstree jstree_pass_emit_generic(jstree t, jsctx *ctx);

typedef jstree (*jstree_pass_t)(jstree, jsctx*);
typedef jstree (*ftree_node)   (jstree, jsctx*);
typedef jstree (*ftree_next)   (jstree, jsctx*);
static jstree_pass_t jstree_pass_manager[] = {
    &jstree_pass_dump,
    &jstree_pass_collect_typeinfo,
    &jstree_pass_collect_typeinfo,
    &jstree_pass_emit_generic,
    NULL
};

void jstree_write_globals(void)
{
  jstree decls = global_tree;
  jstree_pass_t *p;
  jsctx ctx = {0, NULL, NULL, NULL};
  for (p = jstree_pass_manager; *p != NULL; ++p) {
    decls = (*p)(decls, &ctx);
  }
}

static jstree jstree_pass_iterate(jstree decl, jsctx *ctx, ftree_node fnode, ftree_next fnext)
{
  jstree t, prev = NULL;;
  for (t = decl; t; t = fnext(t, ctx)) {
    t = fnode(t, ctx);
    prev = t;
  }
  return t;
}

static jstree jstree_pass_iterate_default(jstree t, jsctx *ctx ATTRIBUTE_UNUSED)
{
  return JSTREE_CHAIN(t);
}

#include "js-pass-dump.c"
#include "js-pass-emit.c"
#define FIND_FUNCTION(stmt, ctx) jstree_pass_iterate(stmt, ctx, FIND_FUNCTION_TREE, jstree_pass_iterate_default)

static int list_size(jstree decl)
{
  jstree t;
  int i = 0;
  for (t = decl; t; t = JSTREE_CHAIN(t)) {
      i++;
  }
  return i;
}

static JSType *JSTypeList_init(int argc)
{
  int i = 0;
  JSType *types = (JSType*)(xmalloc(sizeof(JSType)*argc));
  for (i = 0; i < argc; i++) {
      types[i] = TyObject;
  }
  return types;
}

static js_typeinfo NEW_TYPEINFO(jstree name, jstree args, jstree body)
{
  js_typeinfo info = (js_typeinfo)(xmalloc(sizeof(*info)));
  memset(info, 0, sizeof(*info));
  info->this_node = name;
  info->body_node = body;
  info->fields_node = args;
  info->fields_size = list_size(args);
  info->fields      = JSTypeList_init(info->fields_size);
  return info;
}

#define JSCTX_APPEND(ctx, F, info) {\
    js_typeinfo t_ = ctx->F;\
      if (t_ == NULL) {\
          ctx->F = info;\
      } else {\
          while (t_->next) {\
              t_ = t_->next;\
          }\
          t_->next = info;\
      }\
}

static bool jsctx_isDefined(jsctx *ctx, jstree t)
{
  jstree name = JSTREE_LHS((t));
  jstree args = JSTREE_LHS(JSTREE_RHS(t));
  jstree body = JSTREE_RHS(JSTREE_RHS(t));
  int argc = list_size(args);
  js_typeinfo info = ctx->func_info;
  while (info) {
      if (name == info->this_node) {
          if (body == info->body_node && argc == info->fields_size) {
              return true;
          }
      }
      info = info->next;
  }
  return false;
}

static void jsctx_addFunction(jsctx *ctx, jstree t)
{
  jstree name, args, body;
  js_typeinfo info;
  if (JSTREE_OP(JSTREE_LHS(t)) != OP_IDENTIFIER) {
      TODO();
  }
  if (!jsctx_isDefined(ctx, t)) {
      name = JSTREE_LHS((t));
      args = JSTREE_LHS(JSTREE_RHS(t));
      body = JSTREE_RHS(JSTREE_RHS(t));
      info = NEW_TYPEINFO(name, args, body);
      JSCTX_APPEND(ctx, func_info, info);
  }
}

static bool jsctx_isSameShape(jsctx *ctx, jstree t)
{
  jstree shape     = JSTREE_LHS(t);
  int fields_size  = list_size(shape);
  js_typeinfo info = ctx->type_info;
  while (info) {
      if (fields_size == info->fields_size) {
          jstree f1 = shape;
          jstree f2 = info->body_node;
          while (f1 != NULL && f2 != NULL && f1 == f2) {
              f1 = f1->next;
              f2 = f2->next;
          }
          if (f1 == NULL && f2 == NULL) {
              return true;
          }
          return false;
      }
      info = info->next;
  }
  return false;
}
static void jsctx_addShape(jsctx *ctx, jstree t)
{
  if (!jsctx_isSameShape(ctx, t)) {
      jstree name = t;
      jstree body = JSTREE_LHS(t);
      js_typeinfo info = NEW_TYPEINFO(name, body, body);
      JSCTX_APPEND(ctx, type_info, info);
  }
}

static jstree FIND_FUNCTION_TREE(jstree t, jsctx *ctx)
{
  switch(JSTREE_OP(t)) {
    case OP_ARRAY:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      break;
    case OP_OBJECT:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      jsctx_addShape(ctx, t);
      break;
    case OP_LET:
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_DEFUN:
      jsctx_addFunction(ctx, t);
      FIND_FUNCTION(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      FIND_FUNCTION(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      /* body */
      FIND_FUNCTION(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      break;
    case OP_NEW:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      if (JSTREE_RHS(t)) {
          FIND_FUNCTION(JSTREE_RHS(t), ctx);
      }
      break;
    case OP_PARM:
      fprintf(stderr, "param=%p", (void*)t);
      TODO();
      break;
    case OP_FIELD:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_GetField:
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_SetField:
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_CALL:
      if (JSTREE_RHS(t)) {
          FIND_FUNCTION(JSTREE_RHS(t), ctx);
      }
      break;
    case OP_LOOP:
    case OP_EXIT:
    case OP_RETURN:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      break;
    case OP_COND:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      FIND_FUNCTION(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      FIND_FUNCTION(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      break;
    case OP_EQLET: case OP_MULLET: case OP_DIVLET: case OP_MODLET:
    case OP_ADDLET: case OP_SUBLET: case OP_LSFTLET: case OP_RSFTLET:
    case OP_SHFTLET: case OP_ANDLET: case OP_XORLET: case OP_ORLET:
      /* TODO check lhs = ID | FIELD */
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_Plus: case OP_Minus: case OP_Mul: case OP_Div: case OP_Mod:
    case OP_Lshift: case OP_Rshift: case OP_Shift: case OP_Or: case OP_Xor:
    case OP_And: case OP_Not: case OP_LAND: case OP_LOR:
    case OP_LT: case OP_LE: case OP_GT: case OP_GE:
    case OP_EQ: case OP_NE: case OP_STREQ: case OP_STRNE:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      FIND_FUNCTION(JSTREE_RHS(t), ctx);
      break;
    case OP_PRED_INC: case OP_PRED_DEC:
    case OP_POST_INC: case OP_POST_DEC:
      FIND_FUNCTION(JSTREE_LHS(t), ctx);
      break;
    default:
      break;
  }
  return t;
}

static void DUMP_FUNCINFO(jsctx *ctx)
{
  int i, argc;
  jstree name, body, argv;
  js_typeinfo info = ctx->func_info;
  while (info) {
      name = info->this_node;
      body = info->body_node;
      argv = info->fields_node;
      argc = info->fields_size;
      fprintf(stderr, "function %s (", JSTREE_COMMON_STRING(name));
      for (i = 0; i < argc; i++) {
          if (JSTREE_OP(argv) == OP_IDENTIFIER) {
              fprintf(stderr, "%s", JSTREE_COMMON_STRING(argv));
          } else {
              fprintf(stderr, "arg%d", i);
          }
          if (i < argc-1) {
              fprintf(stderr, ", ");
          }
          argv = argv->next;
      }
      fprintf(stderr, ") { body:%p }\n", (void*)body);
      info = info->next;
  }
}

static void DUMP_TYPEINFO(jsctx *ctx)
{
  int i, j, argc;
  jstree name, body, field;
  js_typeinfo info = ctx->type_info;
  i = 0;
  while (info) {
      name = info->this_node;
      body = info->body_node;
      field = info->fields_node;
      argc = info->fields_size;
      fprintf(stderr, "type type%d {\n", i++);
      for (j = 0; j < argc; j++) {
          fprintf(stderr, "\tfield%d:", j);
          if (JSTREE_OP(field) == OP_SetField) {
              jstree l = JSTREE_LHS(field);
              jstree r = JSTREE_RHS(l);
              jstree o = JSTREE_RHS(field);
              fprintf(stderr, "%s = %p;\n", JSTREE_COMMON_STRING(r), (void*)o);
          } else if (JSTREE_OP(field) == OP_FIELD) {
              asm volatile("int3");
          } else {
              fprintf(stderr, "(%p)", (void*)field);
          }
          field = field->next;
      }
      fprintf(stderr, "}\n");
      info = info->next;
  }
}

jstree jstree_pass_collect_typeinfo(jstree t, jsctx *ctx)
{
  fprintf(stderr, "**jstree_pass_collect_typeinfo {\n");
  FIND_FUNCTION(t, ctx);
  DUMP_FUNCINFO(ctx);
  //DUMP_TYPEINFO(ctx);
  fprintf(stderr, "\n}\n");
  return t;
}

#undef FIND_FUNCTION
