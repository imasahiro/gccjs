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


#define BUILTIN_NOTHROW 1
#define BUILTIN_CONST 2

tree js_build_function_type(tree rettype, VEC(tree, gc) *vec)
{
  int i;
  tree params = NULL_TREE, x = NULL_TREE;
  for (i = 0; VEC_iterate(tree,vec, i, x); ++i) {
      chainon(params, tree_cons(NULL_TREE, x, NULL_TREE) );
  }
  return build_function_type(rettype, params);
}


#define TyIntObj   (TY_MAX+1)
#define TyFloatObj (TY_MAX+2)
#define TyRef      (TY_MAX+3)
#define TyVags     (TY_MAX+4)

static tree type_to_tree(JSType type)
{
  switch(type) {
    case TyObject:
      return ptr_type_node;
    case TyBoolean:
      return boolean_type_node;
    case TyInt:
      return integer_type_node;
    case TyFloat:
      return double_type_node;
    case TyString:
      return ptr_type_node;
    case TyArray:
      return ptr_type_node;
    case TyFunction:
      return ptr_type_node;
    case TyIntObj:
      return ptr_type_node;
    case TyFloatObj:
      return ptr_type_node;
    case TyVoid:
      return void_type_node;
    case TyVags:
      return va_list_type_node;
    case TyRef:
      return ptr_type_node;
    case TyUndefined:
    default:
      error("undefined type");
      exit(EXIT_FAILURE);
  }
}

JSBultinDef builtins[] = {
      {"__init__", "__init__", TyVoid, 2, {TyInt, TyRef}, NULL},
      {"abort", "abort", TyVoid, 0, {TyVoid}, NULL, NULL},
      {"print", "print", TyVoid, 3, {TyInt, TyVags, TyVoid}, NULL, NULL},
};

static void define_builtin(JSBultinDef *def, const char *realname, tree type, int flags)
{
  const char *name = def->name;
  tree decl = build_decl(BUILTINS_LOCATION, FUNCTION_DECL,
                         get_identifier(name), type);
  DECL_EXTERNAL(decl) = true;
  TREE_PUBLIC(decl)   = true;
  SET_DECL_ASSEMBLER_NAME(decl, get_identifier(realname));
  DECL_BUILT_IN_CLASS(decl) = BUILT_IN_NORMAL;
  if (flags & BUILTIN_NOTHROW) {
      TREE_NOTHROW(decl) = true;
  }
    {
      JSBultinDef *newdef = (JSBultinDef*)(xmalloc(sizeof(*newdef)));
      memcpy(newdef, def, sizeof(*def));
      newdef->fn_decl = decl;
      newdef->next    = g_ctx->builtins;
      g_ctx->builtins = newdef;
    }
}


#define _ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof((a)[0])))
static void init_builtin_functions(void)
{
  VEC(tree,gc) *args;
  int i, j;
  for (i = 0; i < _ARRAY_SIZE(builtins); i++) {
      tree type, rettype;
      char buf[32] = {0};
      JSBultinDef *def = &builtins[i];
      args = VEC_alloc(tree,gc, 0);
      for (j = 0; j < def->argc; j++) {
          type = type_to_tree(def->args[j]);
          VEC_safe_push(tree, gc, args, type);
      }
      snprintf(buf, 32, "js_runtime_%s", def->real_name);
      rettype = type_to_tree(def->rettype);
      type = js_build_function_type(rettype, args);
      define_builtin(def, buf, type, BUILTIN_NOTHROW);
  }
}

void init_builtins(void)
{
  g_ctx = new_jsctx();
  /* base types decl */
  build_common_tree_nodes (false);
  build_common_tree_nodes_2 (0);
  void_list_node = build_tree_list( NULL_TREE, void_type_node );

  if (TYPE_MODE (long_unsigned_type_node) == ptr_mode)
      size_type_node = long_unsigned_type_node;
  else if (TYPE_MODE (long_long_unsigned_type_node) == ptr_mode)
      size_type_node = long_long_unsigned_type_node;
  else
      size_type_node = long_unsigned_type_node;
  set_sizetype (size_type_node);

  set_sizetype (size_type_node);
  /* construct builtin functions */
  init_builtin_functions();
  using_eh_for_cleanups();
}

