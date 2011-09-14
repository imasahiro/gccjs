#ifndef JS_TREE_H
#define JS_TREE_H

typedef enum {
    OP_NULL,
    OP_INTEGER,
    OP_FLOAT,
    OP_STRING,
    OP_LET,
    OP_DEFUN,
    OP_NEW,
    OP_FIELD,
    OP_PARM,
    OP_GetField,
    OP_SetField,
    OP_CALL,
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
    TyValue,
    TyNone,
    TY_MAX
} JSType;

typedef struct GTY(()) gjs_tree_common {
    location_t loc;
    JSType T;
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

#define JSTREE_TYPE(x)  ((x)->T)
#define JSTREE_CHAIN(x) ((x)->next)
#define JSTREE_LHS(x)   ((x)->l.t)
#define JSTREE_RHS(x)   ((x)->r.t)
#define JSTREE_LHS_C(x) ((x)->l.tc)
#define JSTREE_RHS_C(x) ((x)->r.tc)
#define JSTREE_ALLOC(x) ((gjs_tree_t*)(xmalloc(sizeof(gjs_tree_t))))
#define JSTREE_IDENTIFIER_POINTER(x) (JSTREE_LHS_C(x)->o.svalue)

typedef struct gjs_tree_t * jstree;
DEF_VEC_P (jstree);
DEF_VEC_ALLOC_P (jstree, gc);

#endif /* end of include guard: JS_TREE_H */
