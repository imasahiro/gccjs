
#define EMIT_GENERIC(stmt, ctx) jstree_pass_iterate(stmt, ctx, EMIT_GENERIC_NODE, jstree_pass_iterate_default)
static jstree EMIT_GENERIC_NODE(jstree t, jsctx *ctx)
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
      fprintf(stderr, "let ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, "= ");
      EMIT_GENERIC(JSTREE_RHS(t), ctx);
      break;
    case OP_DEFUN:
      fprintf(stderr, "defun ");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, " = function(");
      EMIT_GENERIC(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      fprintf(stderr, ") {");
      EMIT_GENERIC(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      fprintf(stderr, "}");
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
      fprintf(stderr, "id:%s", JSTREE_COMMON_STRING(t));
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
      fprintf(stderr, "(call (");
      EMIT_GENERIC(JSTREE_LHS(t), ctx);
      fprintf(stderr, " ");
      if (JSTREE_RHS(t)) {
          EMIT_GENERIC(JSTREE_RHS(t), ctx);
      }
      fprintf(stderr, "))");
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

jstree jstree_pass_emit_generic(jstree t, jsctx *ctx)
{
  fprintf(stderr, "**jstree_pass_emit {\n");
  EMIT_GENERIC(t, ctx);
  fprintf(stderr, "\n}\n");
  return t;
}
#undef EMIT_GENERIC

