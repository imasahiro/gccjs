#ifndef JS_TREE_H
#define JS_TREE_H

typedef enum {
    OP_NOP,
    OP_NULL,
    OP_BOOL,
    OP_INTEGER,
    OP_FLOAT,
    OP_STRING,
    OP_ARRAY,
    OP_OBJECT,
    OP_LET,
    OP_DEFUN,
    OP_NEW,
    OP_IDENTIFIER,
    OP_PARM,
    OP_FIELD,
    OP_GetField,
    OP_SetField,
    OP_CALL,
    OP_LOOP,
    OP_EXIT,
    OP_RETURN,
    OP_COND,
    OP_EQLET,   /*    = */
    OP_MULLET,  /*   *= */
    OP_DIVLET,  /*   /= */
    OP_MODLET,  /*   %= */
    OP_ADDLET,  /*   += */
    OP_SUBLET,  /*   -= */
    OP_LSFTLET, /*  <<= */
    OP_RSFTLET, /*  >>= */
    OP_SHFTLET, /* >>>= */
    OP_ANDLET,  /*   &= */
    OP_XORLET,  /*   ^= */
    OP_ORLET,   /*   |= */
    OP_Plus,
    OP_Minus,
    OP_Mul,
    OP_Div,
    OP_Mod,
    OP_Lshift,
    OP_Rshift,
    OP_Shift,
    OP_Or,
    OP_Xor,
    OP_And,
    OP_Not,
    OP_LAND, /* && */
    OP_LOR,  /* || */
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_EQ,
    OP_NE,
    OP_STREQ,
    OP_STRNE,
    OP_PRED_INC,
    OP_PRED_DEC,
    OP_POST_INC,
    OP_POST_DEC,
    OP_Undef = -1
} JSOperator;

typedef enum {
    TyUndefined,
    TyNULL,
    TyBoolean,
    TyInt,
    TyFloat,
    TyString,
    TyArray,
    TyObject,
    TyFunction,
    TyValue,
    TyNone,
    TY_MAX
} JSType;

enum loopmode {
    LOOP_FOR,
    LOOP_FOR_IN,
    LOOP_FOR_VAR,
    LOOP_FOR_VAR_IN,
    LOOP_DOWHILE,
    LOOP_WHILE
};

typedef enum jsfieldop {
    FIELD_NONE,
    FIELD_SET,
    FIELD_GET
} jsfieldop;

typedef struct GTY(()) gjs_tree_common {
    location_t loc;
    JSType T;
    JSOperator op;
    union {
        int   ivalue;
        char *fvalue;
        char *svalue;
    } o;
} gjs_tree_common;

typedef struct GTY(()) gjs_tree_t {
    location_t loc;
    JSType T;
    JSOperator op;
    union {
        gjs_tree_common   *tc;
        struct gjs_tree_t *t;
    } l;
    union {
        gjs_tree_common   *tc;
        struct gjs_tree_t *t;
    } r;
    struct gjs_tree_t *next;
} gjs_tree_t;

#define JSTREE_COMMON_ALLOC()   ((gjs_tree_common*)(jstree_alloc_()))
#define JSTREE_COMMON_INT(x)    (((gjs_tree_common*)x)->o.ivalue)
#define JSTREE_COMMON_FLOAT(x)  (((gjs_tree_common*)x)->o.fvalue)
#define JSTREE_COMMON_STRING(x) (((gjs_tree_common*)x)->o.svalue)

#define JSTREE_TYPE(x)  ((x)->T)
#define JSTREE_LOC(x)   ((x)->loc)
#define JSTREE_OP(x)    ((x)->op)
#define JSTREE_CHAIN(x) ((x)->next)
#define JSTREE_LHS(x)   ((x)->l.t)
#define JSTREE_RHS(x)   ((x)->r.t)
#define JSTREE_LHS_C(x) ((x)->l.tc)
#define JSTREE_RHS_C(x) ((x)->r.tc)
#define JSTREE_ALLOC()  ((gjs_tree_t*)(jstree_alloc_()))
#define JSTREE_IDENTIFIER_POINTER(x) (JSTREE_COMMON_STRING(JSTREE_LHS_C(x)))

#define JSTREE_APPENDTAIL(top, node) {\
  jstree t_ = top;\
  while(JSTREE_CHAIN(t_)) {\
      t_ = JSTREE_CHAIN(t_);\
  }\
  JSTREE_CHAIN(t_) = (node);\
}

typedef struct gjs_tree_t * jstree;
DEF_VEC_P (jstree);
DEF_VEC_ALLOC_P (jstree, gc);

extern jstree js_build_nulval(location_t loc, int n/*n=0=>null, n=1=>undefined*/);
static inline jstree jstree_alloc_(void)
{
  jstree t = (gjs_tree_t*) (xmalloc(sizeof(gjs_tree_t)));
  memset(t, 0, sizeof(gjs_tree_t));
  return t;
}
extern jstree global_tree;

#define JS_NULL      (js_build_nulval(LOC, 0))
#define JS_UNDEFINED (js_build_nulval(LOC, 1))

#endif /* end of include guard: JS_TREE_H */
