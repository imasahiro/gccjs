#ifndef JS_PASS_H
#define JS_PASS_H

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
    bool isSelfRecursive;
    tree decl;
} * js_typeinfo;

typedef enum scope_type {
    SCOPE_LOCAL,
    SCOPE_PARAM,
    SCOPE_FIELD,
    SCOPE_CLOSURE,
    SCOPE_GLOBAL,
    SCOPE_BUILTIN,
    SCOPE_DEFAULT
} scope_type;

typedef struct js_scope {
    struct js_scope *next;
    int level;
    scope_type scopeTy;
    jstree list;
} js_scope;

typedef struct JSBultinDef {
    const char *name;
    const char *real_name;
    JSType rettype;
    int    argc;
    JSType args[5];
    tree fn_decl;
    struct JSBultinDef *next;
} JSBultinDef;


typedef struct jscontext {
    int id;
    void *data;
    FILE *fp;
    js_typeinfo type_info;
    js_typeinfo func_info;
    js_scope    *scope;
    JSBultinDef *builtins;
    VEC(tree, gc) *decls;
} jsctx;

jstree jstree_pass_dump(jstree t, jsctx *ctx);
jstree jstree_pass_collect_typeinfo(jstree t, jsctx *ctx);
jstree jstree_pass_check_variables(jstree t, jsctx *ctx);
jstree jstree_pass_check_function_return(jstree t, jsctx *ctx);
jstree jstree_pass_emit_generic(jstree t, jsctx *ctx);

typedef jstree (*jstree_pass_t)(jstree, jsctx*);
typedef jstree (*ftree_node)   (jstree, jsctx*);
typedef jstree (*ftree_next)   (jstree, jsctx*);

extern jsctx *g_ctx;
extern jsctx *new_jsctx(void);
extern tree js_build_function_type(tree rettype, VEC(tree,gc) *vec);

#endif /* end of include guard: JS_PASS_H */
