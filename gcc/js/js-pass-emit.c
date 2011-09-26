
static tree js_get_function_tree2(const char *name, int argc, int *n);
static tree js_get_function_tree(tree name, VEC(tree, gc) *args, int *n);

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

static tree TreeUndef = NULL_TREE;
static tree TreeNull  = NULL_TREE;
static tree tree_undef(void)
{
  if (TreeUndef == NULL_TREE) {
  }
  return TreeUndef;
}
static tree tree_null(void)
{
  if (TreeNull == NULL_TREE) {
  }
  return TreeNull;
}
static tree tree_bool(int val)
{
  return build_int_cst(boolean_type_node, val);
}
static tree tree_int(int val)
{
  return build_int_cst(integer_type_node, val);
}
static tree tree_float(const char *val)
{
  REAL_VALUE_TYPE d;
  real_from_string(&d, val);
  return build_real(double_type_node, d);
}
static tree tree_string(const char *val)
{
  int len = strlen(val);
  tree t = build_string(len, val);
  tree idxtype = build_index_type(build_int_cst(NULL_TREE, len));
  tree type = build_array_type(char_type_node, idxtype);
  TREE_TYPE(t) = type;
  TREE_CONSTANT(t) = true;
  TREE_READONLY(t) = true;
  TREE_STATIC(t) = true;
  return t;
}
static tree tree_build_id(const char *str)
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

static tree tree_symbol(jsctx *ctx, jstree t)
{
  /* TODO */
  tree var  = tree_build_id(JSTREE_COMMON_STRING(t));
  tree decl = build_decl(JSTREE_LOC(t), VAR_DECL, var, ptr_type_node/*TODO*/);
  return decl;
}

static tree js_build_int_object(tree x)
{
    VEC(tree, gc) *args = VEC_alloc(tree, gc, 0);
    VEC_safe_push(tree, gc, args, save_expr(x));
    return NULL;
    //return js_build_call_expr(new_int_node, args, false/*hasSelf*/);
}

static tree js_build_float_object(tree x)
{
    VEC(tree, gc) *args = VEC_alloc(tree, gc, 0);
    VEC_safe_push(tree, gc, args, save_expr(x));
    return NULL;
    //return js_build_call_expr(new_float_node, args, false/*hasSelf*/);
}

static tree *create_args_vec(int n, VEC(tree,gc) *args);
static tree build_call(tree fn_decl, VEC(tree, gc) *argv)
{
  tree retval = error_mark_node;
  /* expr(argv) */
  tree restype = TREE_TYPE(TREE_TYPE(fn_decl));
  tree result  = DECL_RESULT(fn_decl);
  if (!result) {
      asm volatile("int3");
      result = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, restype);
      DECL_CONTEXT(result) = fn_decl;
      DECL_RESULT(fn_decl) = result;
  }
  retval = build_call_expr_loc_vec(BUILTINS_LOCATION, fn_decl, argv);
  return retval;
}

static tree jstree_name_to_tree(jstree name)
{
  return tree_build_id(JSTREE_COMMON_STRING(name));
}
static tree jsctx_get_fndecl(jsctx *ctx, jstree name, VEC(tree, gc) *params)
{
  js_typeinfo info = ctx->func_info;
  while (info) {
      jstree name1 = info->this_node;
      //int    argc = info->fields_size;
      const char *fname1 = JSTREE_COMMON_STRING(name1);
      const char *fname2 = JSTREE_COMMON_STRING(name);
      if (strcmp(fname1, fname2) == 0) {
          return info->decl;
      }
      info = info->next;
  }
  return NULL_TREE;
}

static void PUSH(jsctx *ctx, tree t);
static tree POP(jsctx *ctx);

static void Function_bind(tree fn_decl, VEC(tree,gc) *decls, tree stmts);
static tree build_function(jsctx *ctx, jstree t, int n)
{
  tree node, stmts;
  int i;
  VEC(tree, gc) *params;
  jstree name = JSTREE_LHS(t);
  for (i = 0; i < n; i++)
    VEC_safe_insert(tree, gc, params, 0, POP(ctx));

  stmts = alloc_stmt_list();
  while ((node = POP(ctx)) != NULL_TREE) {
      append_to_statement_list(node, &stmts);
  }
    {
      VEC(tree,gc) *block_decls;
      tree retdecl = NULL_TREE;
      tree block_stmts = stmts;
      tree fn_decl  = jsctx_get_fndecl(ctx, name, params);
      tree fn_block = build_block(NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);
      tree rettype = TREE_TYPE(TREE_TYPE(fn_decl));

      DECL_CONTEXT(fn_decl) = NULL_TREE;
      TREE_STATIC(fn_decl)  = true;
      TREE_NOTHROW(fn_decl) = true;
      TREE_PUBLIC(fn_decl)  = true;
      DECL_ARGUMENTS(fn_decl) = /*TODO*/NULL_TREE;
      DECL_INITIAL(fn_decl) = fn_block;
      if (rettype != void_type_node) {
          DECL_IGNORED_P(retdecl) = true;
          DECL_CONTEXT(retdecl) = fn_decl;
          DECL_RESULT(fn_decl) = retdecl;
      } else {
          DECL_RESULT(fn_decl) =  build_decl(BUILTINS_LOCATION,
                                             RESULT_DECL, NULL_TREE, rettype);
      }

      block_decls = NULL/*TODO*/;
      Function_bind(fn_decl, block_decls, block_stmts);

      gimplify_function_tree(fn_decl);
      cgraph_finalize_function(fn_decl, false);

      return fn_decl;
    }
}

#define EMIT_GENERIC(stmt, ctx) jstree_pass_iterate(stmt, ctx, EMIT_GENERIC_NODE, jstree_pass_iterate_default)


static jstree EMIT_GENERIC_NODE(jstree t, jsctx *ctx)
{
  switch(JSTREE_OP(t)) {
    case OP_Undef:
      PUSH(ctx, tree_undef());
      break;
    case OP_NULL:
      PUSH(ctx, tree_null());
      break;
    case OP_BOOL:
      PUSH(ctx, tree_bool(JSTREE_COMMON_INT(t)/*1/0*/));
      break;
    case OP_INTEGER:
      PUSH(ctx, tree_int(JSTREE_COMMON_INT(t)));
      break;
    case OP_FLOAT:
      PUSH(ctx, tree_float(JSTREE_COMMON_STRING(t)));
      break;
    case OP_STRING:
      PUSH(ctx, tree_string(JSTREE_COMMON_STRING(t)));
      break;
    case OP_ARRAY:
      fprintf(stderr, "array(");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, ")");
      break;
    case OP_OBJECT:
      fprintf(stderr, "object{");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, "}");
      break;
    case OP_LET:
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      EMIT_GENERIC(JSTREE_RHS(t), ctx);
        {
          tree lhs, rhs;
          rhs = POP(ctx);
          lhs = POP(ctx);
          PUSH(ctx, build2(MODIFY_EXPR, ptr_type_node, lhs, rhs));
        }
      break;
    case OP_DEFUN:
        {
          /* TODO params */
          jstree lrhs = JSTREE_LHS(JSTREE_RHS(t)); /* argv */
          jstree rrhs = JSTREE_RHS(JSTREE_RHS(t)); /* body */
          int n = list_size(lrhs);
          EMIT_GENERIC(lrhs, ctx);
          PUSH(ctx, NULL_TREE);
          EMIT_GENERIC(rrhs, ctx);
          build_function(ctx, t, n);
        }
      break;
    case OP_NEW:
      fprintf(stderr, "(new (");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, " ");
      if (JSTREE_RHS(t)) {
          EMIT_GENERIC(JSTREE_RHS(t), ctx);
      }
      fprintf(stderr, "))");
      break;
    case OP_IDENTIFIER:
      PUSH(ctx, tree_symbol(ctx, t));
      break;
    case OP_PARM:
      fprintf(stderr, "param=%p", (void*)t);
      break;
    case OP_FIELD:
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, ".");
      EMIT_GENERIC(JSTREE_RHS(t), ctx);
      break;
    case OP_GetField:
      fprintf(stderr, "get ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, ".");
      EMIT_GENERIC(JSTREE_RHS(t), ctx);
      break;
    case OP_SetField:
      fprintf(stderr, "set ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, "=");
      EMIT_GENERIC(JSTREE_RHS(t), ctx);
      break;
    case OP_CALL:
        {
          tree   node;
          jstree l = JSTREE_LHS(t);
          jstree r = JSTREE_RHS(t);
          int n = list_size(r), argc = 0;
          if (r) {
              EMIT_GENERIC(r, ctx);
          }
          if (JSTREE_OP(l) == OP_IDENTIFIER &&
              (node = js_get_function_tree2(JSTREE_COMMON_STRING(l), n, &argc)) != NULL_TREE) {
              PUSH(ctx, node);
          } else {
              asm volatile("int3");
              EMIT_GENERIC(l, ctx);
          }
          {
            int i;
            tree fn_decl = POP(ctx), expr;
            VEC(tree, gc) *vec = VEC_alloc(tree, gc, 0);
            for (i = 0; i < n; i++) {
                VEC_safe_insert(tree, gc, vec, 0, POP(ctx));
            }
            for (; i < argc; i++) {
                VEC_safe_push(tree, gc, vec, build_int_cst(integer_type_node, 0));
            }

            PUSH(ctx, build_call(fn_decl, vec));
          }
        }
      break;
    case OP_LOOP:
      fprintf(stderr, "loop {");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, "}");
      break;
    case OP_EXIT:
      fprintf(stderr, "exit ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, ";");
      break;
    case OP_RETURN:
      fprintf(stderr, "return ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      break;
    case OP_COND:
      fprintf(stderr, "if (");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, ") then {");
      EMIT_GENERIC(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      fprintf(stderr, "} else {");
      EMIT_GENERIC(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      fprintf(stderr, "}");
      break;
    case OP_EQLET: case OP_MULLET: case OP_DIVLET: case OP_MODLET:
    case OP_ADDLET: case OP_SUBLET: case OP_LSFTLET: case OP_RSFTLET:
    case OP_SHFTLET: case OP_ANDLET: case OP_XORLET: case OP_ORLET:
      break;
    case OP_Plus: case OP_Minus: case OP_Mul: case OP_Div: case OP_Mod:
    case OP_Lshift: case OP_Rshift: case OP_Shift: case OP_Or: case OP_Xor:
    case OP_And: case OP_Not: case OP_LAND: case OP_LOR:
    case OP_LT: case OP_LE: case OP_GT: case OP_GE:
    case OP_EQ: case OP_NE: case OP_STREQ: case OP_STRNE:
      break;
    case OP_PRED_INC: case OP_PRED_DEC:
      break;
    case OP_POST_INC: case OP_POST_DEC:
      break;
    default:
      fprintf(stderr, "'OP=%d, %p'", JSTREE_OP(t), (void*)t);
      break;
  }
  return t;
}

static tree JSType2Tree(JSType type)
{
  switch (type) {
    case TyUndefined: case TyNULL:
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
    case TyObject:
      return ptr_type_node;
    case TyFunction:
      return ptr_type_node;
    case TyValue:
      return ptr_type_node;
    case TyNone:
      return void_type_node;
    default:
      break;
  }
  return integer_type_node;
}

static tree build_fn_type(JSType retTy, VEC(tree,gc) *vec)
{
  int i;
  tree rettype = JSType2Tree(retTy);
  tree params = NULL_TREE, x = NULL_TREE;
  for (i = 0; VEC_iterate(tree,vec, i, x); ++i) {
      chainon(params, tree_cons(NULL_TREE, x, NULL_TREE) );
  }
  return build_function_type(rettype, params);
}

static VEC(tree,gc) *build_type_array(jstree argv, int argc)
{
  jstree t;
  VEC(tree, gc) *params = VEC_alloc(tree, gc, argc);
  for (t = argv; t; t = JSTREE_CHAIN(t)) {
      tree type = JSType2Tree(JSTREE_TYPE(t));
      VEC_safe_push(tree, gc, params, type);
  }
  return params;
}

static void emit_generic_function(jsctx *ctx)
{
  js_typeinfo info = ctx->func_info;
  while (info) {
      jstree name = info->this_node;
      jstree body = info->body_node;
      jstree argv = info->fields_node;
      int    argc = info->fields_size;

      asm volatile("int3");
      fprintf(stderr, "'%s'\n", JSTREE_COMMON_STRING(name));
      const char *fname = tree_build_id(JSTREE_COMMON_STRING(name));
      VEC(tree,gc) *params = build_type_array(argv, argc);
      tree fn_type  = build_fn_type(info->retTy, params);
      tree fn_decl  = build_decl(JSTREE_LOC(name), FUNCTION_DECL, fname, fn_type);

      info->decl    = fn_decl;
      VEC_safe_push(tree, gc, ctx->decls, fn_decl);
      info = info->next;
  }
}

static void create_main(void);
static void emit_global_decls(jsctx *ctx)
{
  VEC(tree, gc) *global_decls = ctx->decls;
  int i, len = VEC_length(tree,global_decls);
  tree  itx = NULL_TREE;
  tree *vec = XNEWVEC( tree, len );
  for(i=0; VEC_iterate(tree,global_decls,i,itx); ++i) {
      vec[i] = itx;
  }
  wrapup_global_declarations(vec, len);
  check_global_declarations(vec, len);
  emit_debug_global_declarations(vec, len);
  cgraph_finalize_compilation_unit();
  free(vec);
  ctx->decls = NULL;
}

struct stack {
    int idx;
    jstree stack[128];
};

static void PUSH(jsctx *ctx, tree t)
{
  struct stack *s = (struct stack*) ctx->data;
  s->stack[s->idx++] = (void*)t;
}
static tree POP(jsctx *ctx)
{
  struct stack *s = (struct stack*) ctx->data;
  tree t = (tree) s->stack[--s->idx];
  return t;
}

jstree jstree_pass_emit_generic(jstree t, jsctx *ctx)
{
  struct stack stack = {0, {NULL}};
  ctx->data = (void*) &stack;
  fprintf(stderr, "**jstree_pass_emit {\n");
  emit_generic_function(ctx);
  EMIT_GENERIC(t, ctx);
  create_main();
  emit_global_decls(ctx);
  fprintf(stderr, "\n}\n");
  ctx->data = NULL;
  return t;
}
#undef EMIT_GENERIC
static tree js_get_function_tree2(const char *name1, int argc, int *n)
{
  JSBultinDef *def = g_ctx->builtins;
  while (def) {
      tree x = def->fn_decl;
      const char *name2 = def->name;
      if (strcmp(name1, name2) == 0) {
          *n = def->argc;
          return x;
      }
      def = def->next;
  }
  return NULL_TREE;
}

static tree js_get_function_tree(tree name, VEC(tree, gc) *args, int *n)
{
  int argc = VEC_length(tree, args);
  const char *name1 = IDENTIFIER_POINTER(name);
  return js_get_function_tree2(name1, argc, n);
}

#if 0
static tree *create_args_vec(int n, VEC(tree,gc) *args)
{
  int idx = 0;
  tree x, *args_vec = XNEWVEC(tree, n);
  for(; VEC_iterate(tree,args,idx,x); ++idx) {
      args_vec[idx]  = x;
  }
  return args_vec;
}
#endif

static tree build_call_builtin(tree func, VEC(tree,gc) *args)
{
  tree retval = error_mark_node;
  int n = VEC_length(tree, args);
  /* expr(args) */
  if (TREE_CODE(func) == IDENTIFIER_NODE) {
      int argc;
      tree fn_decl = js_get_function_tree(func, args, &argc);
      tree restype = TREE_TYPE(TREE_TYPE(fn_decl));
      tree result  = DECL_RESULT(fn_decl);
      if (!result) {
          result = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, restype);
          DECL_CONTEXT(result) = fn_decl;
          DECL_RESULT(fn_decl) = result;
      }

      if (n < argc) {
          asm volatile("int3");
          VEC_safe_push(tree, gc, args, build_int_cst(integer_type_node, 0));
      }
      retval = build_call_expr_loc_vec(BUILTINS_LOCATION, fn_decl, args);
  }
  return retval;
}

static tree js_tree_chain(VEC(tree, gc) *vec, tree t)
{
  int i;
  tree x = NULL_TREE;
  if (VEC_length(tree, vec) > 0) {
      FOR_EACH_VEC_ELT(tree, vec, i, x) {
          TREE_CHAIN(x) = t;
          t = x;
      }
  }
  return t;
}

static void Function_bind(tree fn_decl, VEC(tree,gc) *decls, tree stmts)
{
  tree bl;
  tree declare_vars = NULL_TREE, bind = NULL_TREE;
  if (VEC_length(tree, decls) > 0) {
      declare_vars = js_tree_chain(decls, declare_vars);
  }

  bl = make_node(BLOCK);
  BLOCK_SUPERCONTEXT(bl) = fn_decl;
  DECL_INITIAL(fn_decl) = bl;
  BLOCK_VARS(bl) = declare_vars;
  TREE_USED(bl) = 1;
  bind = build3(BIND_EXPR, void_type_node, BLOCK_VARS(bl), NULL_TREE, bl);
  TREE_SIDE_EFFECTS(bind) = 1;

  BIND_EXPR_BODY(bind) = stmts;
  stmts = bind;
  DECL_SAVED_TREE(fn_decl) = stmts;
}

static void create_main(void)
{
  tree fn_type, fn_decl;
  tree main_ret, main_block;
  tree main_ret_expr, main_stmts, main_set_ret;
  tree main_init;
  tree args = NULL_TREE;
  VEC(tree,gc) *argsTy_list = VEC_alloc(tree, gc, 0);
  VEC(tree,gc) *main_block_decls;
  debug("main");
  VEC_safe_push(tree, gc, argsTy_list, integer_type_node);
  VEC_safe_push(tree, gc, argsTy_list, ptr_type_node);
  fn_type = js_build_function_type(integer_type_node, argsTy_list);
  fn_decl = build_decl(BUILTINS_LOCATION, FUNCTION_DECL, get_identifier("main"), fn_type);
  DECL_CONTEXT(fn_decl) = NULL_TREE;
  TREE_STATIC(fn_decl) = true;
  TREE_PUBLIC(fn_decl) = true;

    {
      tree arg1, arg2;
      VEC(tree,gc) *args_list = VEC_alloc(tree, gc, 0);
      tree argc_node = tree_build_id("argc");
      tree argv_node = tree_build_id("argv");
      args = NULL_TREE;
      arg1 = build_decl(BUILTINS_LOCATION, PARM_DECL,
                        argc_node, integer_type_node);
      DECL_ARG_TYPE(arg1) = integer_type_node;
      DECL_CONTEXT(arg1)  = fn_decl;

      arg2 = build_decl(BUILTINS_LOCATION, PARM_DECL,
                        argv_node, ptr_type_node);
      DECL_ARG_TYPE(arg2) = ptr_type_node;
      DECL_CONTEXT(arg2)  = fn_decl;

      VEC_safe_push(tree, gc, args_list, arg1);
      VEC_safe_push(tree, gc, args_list, arg2);
      main_init = build_call_builtin(tree_build_id("__init__"), args_list);
      args = arg1;
      TREE_CHAIN(arg1) = arg2;
    }
  DECL_ARGUMENTS(fn_decl) = args;
  /* Define the retutrn type (represented by RESULT_DECL) 
   * for the main function */
  main_ret = build_decl(BUILTINS_LOCATION, RESULT_DECL,
                        NULL_TREE, TREE_TYPE(fn_type));
  DECL_CONTEXT(main_ret) = fn_decl;
  DECL_ARTIFICIAL(main_ret) = true;
  DECL_IGNORED_P(main_ret)  = true;
  DECL_RESULT(fn_decl) = main_ret;

  main_block = build_block(NULL_TREE, NULL_TREE, NULL_TREE, NULL_TREE);
  DECL_INITIAL(fn_decl) = main_block;

  /* TODO parse file and append to main stmts */

  main_set_ret = build2(MODIFY_EXPR, TREE_TYPE(main_ret),
                        main_ret, build_int_cst(integer_type_node, 0));
  TREE_USED(main_set_ret) = true;
  main_ret_expr = build1(RETURN_EXPR, void_type_node, main_set_ret);

  main_stmts = alloc_stmt_list();
  append_to_statement_list(main_init, &main_stmts);
  append_to_statement_list(main_ret_expr, &main_stmts);

  main_block_decls = NULL;
  Function_bind(fn_decl, main_block_decls, main_stmts);

  /* Prepare the function for the GCC middle-end */
  gimplify_function_tree(fn_decl);
  //cgraph_add_new_function(fn_decl, false);
  cgraph_finalize_function(fn_decl, false);
  VEC_safe_push(tree, gc, g_ctx->decls, fn_decl);
}

