#define CHECK_RETURN(stmt, ctx) jstree_pass_iterate(stmt, ctx, CHECK_RETURN_TREE, jstree_pass_iterate_default)
static jstree CHECK_RETURN_TREE(jstree t, jsctx *ctx)
{
  js_typeinfo info = (js_typeinfo)(ctx->data);
  switch(JSTREE_OP(t)) {
    case OP_LOOP:
    case OP_EXIT:
      CHECK_RETURN(JSTREE_LHS(t), ctx);
      break;
    case OP_RETURN:
      info->retTy = TyValue;
      break;
    case OP_COND:
      CHECK_RETURN(JSTREE_LHS(t), ctx);
      CHECK_RETURN(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      CHECK_RETURN(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      break;
    case OP_PRED_INC: case OP_PRED_DEC:
    case OP_POST_INC: case OP_POST_DEC:
      CHECK_RETURN(JSTREE_LHS(t), ctx);
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
      fprintf(stderr, (info->retTy == TyNone)?"void ":"var  ");
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

jstree jstree_pass_check_function_return(jstree t, jsctx *ctx)
{
  js_typeinfo info = ctx->func_info;
  fprintf(stderr, "**jstree_pass_check_function_return {\n");
  while (info) {
      ctx->data = (void*)info;
      CHECK_RETURN(info->body_node, ctx);
      ctx->data = NULL;
      info = info->next;
  }
  DUMP_FUNCINFO(ctx);
  fprintf(stderr, "}\n");
  return t;
}
#undef CHECK_RETURN

