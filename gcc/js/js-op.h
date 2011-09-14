#include "js-tree.h"

#ifndef JS_OP_H
#define JS_OP_H

extern int js_lex_parse(const char * gjs_in);
extern void jstree_dump(jstree t);

/* Premitives */
extern jstree js_build_id(const char *str);
extern jstree js_build_int(int val);
extern jstree js_build_float_str(const char *buf);
extern jstree js_build_string(const char *str);
extern jstree js_build_bool(int val);
extern jstree js_build_nulval(int n/*n=0=>null, n=1=>undefined*/);

/* Expressions */
extern jstree js_build2(JSOperator op, JSType type, jstree lhs, jstree rhs);
extern jstree js_build_call(JSType type, jstree f, jstree params);

extern void __js_debug__( const char * file, unsigned int lineno, const char * fmt, ... )
     __attribute__ ((format (printf, 3, 4))) ;

#endif /* end of include guard: JS_OP_H */
