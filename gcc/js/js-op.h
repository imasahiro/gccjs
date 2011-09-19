#include "js-tree.h"

#ifndef JS_OP_H
#define JS_OP_H

extern int js_lex_parse(const char * gjs_in);

/* Premitives */
extern jstree js_build_id(const char *str);
extern jstree js_build_int(int val);
extern jstree js_build_float_str(const char *buf);
extern jstree js_build_string(const char *str);
extern jstree js_build_bool(int val);
extern jstree js_build_nulval(int n/*n=0=>null, n=1=>undefined*/);
extern jstree js_build_array(jstree list);
extern jstree js_build_defun(jstree name, jstree params, jstree body);

/* Expressions */
extern jstree js_build_nop(void);
extern jstree js_build1(JSOperator op, JSType type, jstree lhs);
extern jstree js_build2(JSOperator op, JSType type, jstree lhs, jstree rhs);
extern jstree js_build3(JSOperator op, JSType type, jstree lhs, jstree mhs, jstree rhs);
extern jstree js_build_call(JSOperator op, JSType type, jstree f, jstree params);
extern jstree js_build_loop(enum loopmode loop, jstree init , jstree cond, jstree inc, jstree body);
extern jstree js_build_cond(jstree cond, jstree thenExpr, jstree elseExpr);

#endif /* end of include guard: JS_OP_H */
