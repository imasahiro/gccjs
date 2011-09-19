#include "js-tree.h"

#ifndef JS_OP_H
#define JS_OP_H

extern int js_lex_parse(const char * gjs_in);

/* Premitives */
extern jstree js_build_id(location_t loc, const char *str);
extern jstree js_build_int(location_t loc, int val);
extern jstree js_build_float_str(location_t loc, const char *buf);
extern jstree js_build_string(location_t loc, const char *str);
extern jstree js_build_bool(location_t loc, int val);
extern jstree js_build_nulval(location_t loc, int n/*n=0=>null, n=1=>undefined*/);
extern jstree js_build_array(location_t loc, jstree list);
extern jstree js_build_defun(location_t loc, jstree name, jstree params, jstree body);

/* Expressions */
extern jstree js_build_nop(location_t loc);
extern jstree js_build1(location_t loc, JSOperator op, JSType type, jstree lhs);
extern jstree js_build2(location_t loc, JSOperator op, JSType type, jstree lhs, jstree rhs);
extern jstree js_build3(location_t loc, JSOperator op, JSType type, jstree lhs, jstree mhs, jstree rhs);
extern jstree js_build_call(location_t loc, JSOperator op, JSType type, jstree f, jstree params);
extern jstree js_build_loop(location_t loc, enum loopmode loop, jstree init , jstree cond, jstree inc, jstree body);
extern jstree js_build_cond(location_t loc, jstree cond, jstree thenExpr, jstree elseExpr);

#endif /* end of include guard: JS_OP_H */
