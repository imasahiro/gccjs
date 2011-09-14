#ifndef JS_OP_H
#define JS_OP_H

extern int js_lex_parse(const char * gjs_in);
extern tree js_build_id(const char *str);
extern tree js_build_int(int val);
extern tree js_build_float_str(const char *buf);
extern tree js_build_string(const char *str);

extern void __js_debug__( const char * file, unsigned int lineno, const char * fmt, ... );
#if 0
#define debug(...)
#define debug0(...)
#define debug1(...)
#endif
#define debug(...)  __js_debug__( __FILE__, __LINE__, __VA_ARGS__ )
#define debug0(...)  debug(__VA_ARGS__)


#endif /* end of include guard: JS_OP_H */
