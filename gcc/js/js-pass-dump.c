static const char *OPSTR[] = {
    "EQLET", "MULLET", "DIVLET", "MODLET",
    "ADDLET", "SUBLET", "LSFTLET", "RSFTLET",
    "SHFTLET", "ANDLET", "XORLET", "ORLET",
    "Plus", "Minus", "Mul", "Div", "Mod",
    "<<", ">>", ">>>",
    "Or", "Xor", "And", "Not", "LAND", "LOR",
    "LT", "LE", "GT", "GE", "EQ", "NE", "STREQ", "STRNE",
    "++", "--", "++", "--",
};

#define DUMP_TREE(stmt, ctx) jstree_pass_iterate(stmt, ctx, DUMP_TREE_, DUMP_TREE_NEXT)
static jstree DUMP_TREE_NEXT(jstree t, jsctx *ctx)
{
  FILE *fp = ctx->fp;
  jstree next = JSTREE_CHAIN(t);
  if (next)
      fprintf(fp, ";\n");
  return next;
}

static jstree DUMP_TREE_(jstree t, jsctx *ctx)
{
  FILE *fp = ctx->fp;
  switch(JSTREE_OP(t)) {
    case OP_Undef:
      fprintf(fp, "undefined");
      break;
    case OP_NULL:
      fprintf(fp, "null");
      break;
    case OP_BOOL:
      fprintf(fp, "bool(%s)", JSTREE_COMMON_INT(t)?"true":"false");
      break;
    case OP_INTEGER:
      fprintf(fp, "int(%d)", JSTREE_COMMON_INT(t));
      break;
    case OP_FLOAT:
      fprintf(fp, "float(%s)", JSTREE_COMMON_STRING(t));
      break;
    case OP_STRING:
      fprintf(fp, "string(%s)", JSTREE_COMMON_STRING(t));
      break;
    case OP_ARRAY:
      fprintf(fp, "array(");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ")");
      break;
    case OP_OBJECT:
      fprintf(fp, "object{");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, "}");
      break;
    case OP_LET:
      fprintf(fp, "let ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, "= ");
      DUMP_TREE(JSTREE_RHS(t), ctx);
      break;
    case OP_DEFUN:
      fprintf(fp, "defun ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, " = function(");
      DUMP_TREE(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      fprintf(fp, ") {");
      DUMP_TREE(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      fprintf(fp, "}");
      break;
    case OP_NEW:
      fprintf(fp, "(new (");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, " ");
      if (JSTREE_RHS(t)) {
          DUMP_TREE(JSTREE_RHS(t), ctx);
      }
      fprintf(fp, "))");
      break;
    case OP_IDENTIFIER:
      fprintf(fp, "id:%s", JSTREE_COMMON_STRING(t));
      break;
    case OP_PARM:
      fprintf(fp, "param=%p", (void*)t);
      break;
    case OP_FIELD:
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ".");
      DUMP_TREE(JSTREE_RHS(t), ctx);
      break;
    case OP_GetField:
      fprintf(fp, "get ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ".");
      DUMP_TREE(JSTREE_RHS(t), ctx);
      break;
    case OP_SetField:
      fprintf(fp, "set ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, "=");
      DUMP_TREE(JSTREE_RHS(t), ctx);
      break;
    case OP_CALL:
      fprintf(fp, "(call (");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, " ");
      if (JSTREE_RHS(t)) {
          DUMP_TREE(JSTREE_RHS(t), ctx);
      }
      fprintf(fp, "))");
      break;
    case OP_LOOP:
      fprintf(fp, "loop {");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, "}");
      break;
    case OP_EXIT:
      fprintf(fp, "exit ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ";");
      break;
    case OP_RETURN:
      fprintf(fp, "return ");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      break;
    case OP_COND:
      fprintf(fp, "if (");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ") then {");
      DUMP_TREE(JSTREE_LHS(JSTREE_RHS(t)), ctx);
      fprintf(fp, "} else {");
      DUMP_TREE(JSTREE_RHS(JSTREE_RHS(t)), ctx);
      fprintf(fp, "}");
      break;
    case OP_EQLET: case OP_MULLET: case OP_DIVLET: case OP_MODLET:
    case OP_ADDLET: case OP_SUBLET: case OP_LSFTLET: case OP_RSFTLET:
    case OP_SHFTLET: case OP_ANDLET: case OP_XORLET: case OP_ORLET:
    case OP_Plus: case OP_Minus: case OP_Mul: case OP_Div: case OP_Mod:
    case OP_Lshift: case OP_Rshift: case OP_Shift: case OP_Or: case OP_Xor:
    case OP_And: case OP_Not: case OP_LAND: case OP_LOR:
    case OP_LT: case OP_LE: case OP_GT: case OP_GE:
    case OP_EQ: case OP_NE: case OP_STREQ: case OP_STRNE:
      fprintf(fp, "(%s ", OPSTR[JSTREE_OP(t) - OP_EQLET]);
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ",");
      DUMP_TREE(JSTREE_RHS(t), ctx);
      fprintf(fp, ")");
      break;
    case OP_PRED_INC: case OP_PRED_DEC:
      fprintf(fp, "(%s", OPSTR[JSTREE_OP(t) - OP_EQLET]);
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, ")");
      break;
    case OP_POST_INC: case OP_POST_DEC:
      fprintf(fp, "(");
      DUMP_TREE(JSTREE_LHS(t), ctx);
      fprintf(fp, "%s)", OPSTR[JSTREE_OP(t) - OP_EQLET]);
      break;
    default:
      fprintf(fp, "'OP=%d, %p'", JSTREE_OP(t), (void*)t);
      break;
  }
  return t;
}

jstree jstree_pass_dump(jstree t, jsctx *ctx)
{
  fprintf(stderr, "**jstree_pass_dump {\n");
  ctx->fp = stderr;
  DUMP_TREE(t, ctx);
  fprintf(stderr, "\n}\n");
  ctx->fp = NULL;
  return t;
}

