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
  info->retTy = TyNone;
  info->isSelfRecursive = false;
  return info;
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

#define FIND_FUNCTION(stmt, ctx) jstree_pass_iterate(stmt, ctx, FIND_FUNCTION_TREE, jstree_pass_iterate_default)
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

jstree jstree_pass_collect_typeinfo(jstree t, jsctx *ctx)
{
  fprintf(stderr, "**jstree_pass_collect_typeinfo {\n");
  FIND_FUNCTION(t, ctx);
  /*DUMP_TYPEINFO(ctx);*/
  fprintf(stderr, "\n}\n");
  return t;
}

#undef FIND_FUNCTION

