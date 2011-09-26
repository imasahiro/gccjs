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
#include "tree-iterator.h"
#include "tree-pass.h"
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
#include "tm.h"
#include "cgraph.h"
#include "vec.h"

#include "gmp.h"
#include "mpfr.h"

#include "gjs.h"
#include "js-op.h"
#include "js-pass.h"

jstree global_tree = NULL;

static jstree_pass_t jstree_pass_manager[] = {
    &jstree_pass_collect_typeinfo,
    &jstree_pass_check_function_return,
    &jstree_pass_check_variables,
#if 0
    &jstree_pass_dump,
#endif
    &jstree_pass_emit_generic,
    NULL
};

jsctx *g_ctx = NULL;
jsctx *new_jsctx(void)
{
  if (g_ctx == NULL) {
      g_ctx = (jsctx*)(xmalloc(sizeof(*g_ctx)));
      memset(g_ctx, 0, sizeof(jsctx));
  }
  return g_ctx;
}

void jstree_write_globals(void)
{
  jstree decls = global_tree;
  jstree_pass_t *p;
  jsctx *ctx = new_jsctx();
  for (p = jstree_pass_manager; *p != NULL; ++p) {
    decls = (*p)(decls, ctx);
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

static int list_size(jstree decl)
{
  jstree t;
  int i = 0;
  for (t = decl; t; t = JSTREE_CHAIN(t)) {
      i++;
  }
  return i;
}

static jstree JSTREE_COPY(jstree old)
{
  jstree expr = NULL;
  if (old) {
      expr = JSTREE_ALLOC();
      JSTREE_LOC(expr)   = JSTREE_LOC(old);
      JSTREE_TYPE(expr)  = JSTREE_TYPE(old);
      JSTREE_OP(expr)    = JSTREE_OP(old);
      JSTREE_LHS(expr)   = JSTREE_LHS(old);
      JSTREE_RHS(expr)   = JSTREE_RHS(old);
      JSTREE_CHAIN(expr) = JSTREE_CHAIN(old);
  }
  return expr;
}

static jstree list_copy(jstree decl)
{
  jstree t, new_ = NULL, root = NULL;
  t = decl;
  root = JSTREE_COPY(decl);
  new_ = root;
  for (; t; t = JSTREE_CHAIN(t)) {
      jstree tmp = JSTREE_COPY(t);
      tmp->next = NULL;
      new_->next = tmp;
      new_ = tmp;
  }
  return root;
}

static void list_delete(jstree decl)
{
  jstree t = decl, tmp;
  if (t) {
      while (t->next) {
          tmp = t->next;
          memset(t, 0, sizeof(*t));
          free(t);
          t = tmp;
      }
  }
}
#define JSCTX_APPEND(ctx, F, info) do {\
    js_typeinfo t_ = ctx->F;\
      if (t_ == NULL) {\
          ctx->F = info;\
      } else {\
          while (t_->next) {\
              t_ = t_->next;\
          }\
          t_->next = info;\
      }\
} while(0)

#include "./js-pass-dump.c"
#include "./js-pass-checkreturn.c"
#include "./js-pass-emit.c"
#include "./js-pass-check-shape.c"

#define CHECK_VAR(stmt, ctx) jstree_pass_iterate(stmt, ctx, CHECK_VAR_TREE, jstree_pass_iterate_default)

static void SCOPE_NEW(jsctx *ctx, jstree vals, scope_type type)
{
  js_scope *global_scope, *scope = (js_scope*)(xmalloc(sizeof(*scope)));
  memset(scope, 0, sizeof(*scope));
  scope->next = NULL;
  scope->level = 0;
  global_scope = ctx->scope;
  if (global_scope != NULL) {
      scope->next  = global_scope;
      scope->level = global_scope->level + 1;
  }
  ctx->scope = scope;
  scope->scopeTy = type;
  scope->list    = list_copy(vals);
  fprintf(stderr, "scope %d {\n", scope->level);
}

static const char *scope_type_name[] = {
    "LOCAL",
    "PARAM",
    "FIELD",
    "CLOSURE",
    "GLOBAL",
    "BUILTIN",
    "DEFAULT"
};

static void scope_type_dump(FILE *fp, scope_type type)
{
  fprintf(fp, "scope_type=%s", scope_type_name[type]);
}

static jstree SCOPE_APPEND(jsctx *ctx, jstree sym, scope_type type)
{
  js_scope *scope = ctx->scope;
  jstree newsym = NULL;
  if (scope->scopeTy != type) {
      fprintf(stderr, "TODO new scope(%d) variable in ", type);
      scope_type_dump(stderr, scope->scopeTy);
      fprintf(stderr, "\n");
  }
  newsym = JSTREE_COPY(sym);
  if (scope->list) {
      JSTREE_APPENDTAIL(scope->list, newsym);
  } else {
      scope->list = newsym;
  }
  fprintf(stderr, "scope %d new var %s\n",
          scope->level, JSTREE_COMMON_STRING(sym));
  return newsym;
}

static void SCOPE_DISPOSE(jsctx *ctx)
{
  js_scope *global_scope = ctx->scope;
  js_scope *s_ = global_scope;
  int level;
  gcc_assert(s_ != NULL);
  level = s_->level;
  ctx->scope = s_->next;
  list_delete(s_->list);
  memset(s_, 0, sizeof(*s_));
  fprintf(stderr, "} scope %d;\n", level);
}

static jstree find_symbol_or_add_newsym(jstree sym, jsctx *ctx)
{
  js_scope *s_ = ctx->scope;
  while (s_) {
      jstree t = s_->list;
      for (; t; t = JSTREE_CHAIN(t)) {
          if (strcmp(JSTREE_COMMON_STRING(sym), JSTREE_COMMON_STRING(t)) == 0) {
              return t;
          }
      }
      s_ = s_->next;
  }
  return SCOPE_APPEND(ctx, sym, SCOPE_LOCAL);
}

static jstree find_symbol(jstree sym, jsctx *ctx)
{
  js_scope *s_ = ctx->scope;
  while (s_) {
      jstree t = s_->list;
      for (; t; t = JSTREE_CHAIN(t)) {
          if (strcmp(JSTREE_COMMON_STRING(sym), JSTREE_COMMON_STRING(t)) == 0) {
              return t;
          }
      }
      s_ = s_->next;
  }
  TODO();
  return SCOPE_APPEND(ctx, sym, SCOPE_LOCAL);
}

typedef enum js_varcheck {
    NO_ERROR,
    CHECK_UNDEF
} js_varcheck;

static jstree CHECK(jstree node, jsctx *ctx, js_varcheck mode)
{
  if (JSTREE_OP(node) == OP_IDENTIFIER) {
      fprintf(stderr, "'%s'\n", JSTREE_COMMON_STRING(node));
      if (mode == NO_ERROR) {
          return find_symbol_or_add_newsym(node, ctx);
      } else {
          return find_symbol(node, ctx);
      }
  }
  else if (JSTREE_OP(node) == OP_FIELD) {
      TODO();
  }
  else if (JSTREE_OP(node) == OP_SetField) {
      TODO();
  }
  else if (JSTREE_OP(node) == OP_GetField) {
      TODO();
  }
  return node;
}

static jstree CHECK_VAR_TREE(jstree t, jsctx *ctx)
{
  switch(JSTREE_OP(t)) {
    case OP_Undef:
      break;
    case OP_NULL:
      break;
    case OP_BOOL:
      break;
    case OP_INTEGER:
      break;
    case OP_FLOAT:
      break;
    case OP_STRING:
      break;
    case OP_ARRAY:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      break;
    case OP_OBJECT:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      break;
    case OP_LET:
      CHECK(JSTREE_LHS(t), ctx, NO_ERROR);
      CHECK(JSTREE_RHS(t), ctx, CHECK_UNDEF);
      break;
    case OP_DEFUN:
      SCOPE_NEW(ctx, JSTREE_LHS(JSTREE_RHS(t)), SCOPE_PARAM);
      CHECK_VAR(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      SCOPE_DISPOSE(ctx);
      break;
    case OP_NEW:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      if (JSTREE_RHS(t)) {
          CHECK_VAR(JSTREE_RHS(t), ctx);
      }
      break;
    case OP_IDENTIFIER:
      CHECK(t, ctx, NO_ERROR);
      break;
    case OP_PARM:
      fprintf(stderr, "param=%p", (void*)t);
      asm volatile("int3");
      break;
    case OP_FIELD:
      CHECK(JSTREE_LHS(t), ctx, CHECK_UNDEF);
      /* lhs.rhs */
      CHECK(JSTREE_RHS(t), ctx, NO_ERROR);
      break;
    case OP_GetField:
      /* get */
      CHECK(JSTREE_LHS(t), ctx, CHECK_UNDEF);
      /* lhs.rhs */
      CHECK_VAR(JSTREE_RHS(t), ctx);
      break;
    case OP_SetField:
      /* set lhs*/
      CHECK(JSTREE_LHS(t), ctx, CHECK_UNDEF);
      /* = rhs */
      CHECK_VAR(JSTREE_RHS(t), ctx);
      break;
    case OP_CALL:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      if (JSTREE_RHS(t)) {
          CHECK_VAR(JSTREE_RHS(t), ctx);
      }
      break;
    case OP_LOOP:
      /* loop { */
      SCOPE_NEW(ctx, NULL, SCOPE_LOCAL);
      CHECK_VAR(JSTREE_LHS(t), ctx);
      SCOPE_DISPOSE(ctx);
      /* } */
      break;
    case OP_EXIT:
      /* exit lhs; */
      CHECK_VAR(JSTREE_LHS(t), ctx);
      break;
    case OP_RETURN:
      /* return lhs; */
      CHECK_VAR(JSTREE_LHS(t), ctx);
      break;
    case OP_COND:
      /* if (*/
      SCOPE_NEW(ctx, NULL, SCOPE_LOCAL);
      CHECK_VAR(JSTREE_LHS(t), ctx);
      /* ) then { */
      CHECK_VAR(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      /* } else { */
      CHECK_VAR(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      SCOPE_DISPOSE(ctx);
      /* } */
      break;
    case OP_EQLET: case OP_MULLET: case OP_DIVLET: case OP_MODLET:
    case OP_ADDLET: case OP_SUBLET: case OP_LSFTLET: case OP_RSFTLET:
    case OP_SHFTLET: case OP_ANDLET: case OP_XORLET: case OP_ORLET:
    case OP_Plus: case OP_Minus: case OP_Mul: case OP_Div: case OP_Mod:
    case OP_Lshift: case OP_Rshift: case OP_Shift: case OP_Or: case OP_Xor:
    case OP_And: case OP_Not: case OP_LAND: case OP_LOR:
    case OP_LT: case OP_LE: case OP_GT: case OP_GE:
    case OP_EQ: case OP_NE: case OP_STREQ: case OP_STRNE:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      CHECK_VAR(JSTREE_RHS(t), ctx);
      break;
    case OP_PRED_INC: case OP_PRED_DEC:
    case OP_POST_INC: case OP_POST_DEC:
      CHECK_VAR(JSTREE_LHS(t), ctx);
      break;
    default:
      break;
  }
  return t;
}

jstree jstree_pass_check_variables(jstree t, jsctx *ctx)
{
  fprintf(stderr, "**jstree_pass_check_variables {\n");
  SCOPE_NEW(ctx, NULL, SCOPE_GLOBAL);
  CHECK_VAR(t, ctx);
  SCOPE_DISPOSE(ctx);
  fprintf(stderr, "}\n");
  return t;
}

#undef CHECK_VAR

