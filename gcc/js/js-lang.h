#ifndef JS_LANG_H
#define JS_LANG_H

typedef struct GTY(()) js_symbol_t {
    tree name;
    tree decl;
} js_symbol_t;
typedef js_symbol_t* js_symbol;

#endif /*JS_LANG_H*/
