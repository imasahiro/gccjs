#ifndef JS_OP_H
#define JS_OP_H

typedef enum {
    OpEQLET=1, /*    = */
    OpMULLET,  /*   *= */
    OpDIVLET,  /*   /= */
    OpMODLET,  /*   %= */
    OpADDLET,  /*   += */
    OpSUBLET,  /*   -= */
    OpLSFTLET, /*  <<= */
    OpRSFTLET, /*  >>= */
    OpSHFTLET, /* >>>= */
    OpANDLET,  /*   &= */
    OpXORLET,  /*   ^= */
    OpORLET,   /*   |= */
    OpPlus,
    OpMinus,
    OpMul,
    OpDiv,
    OpMod,
    OpLshift,
    OpRshift,
    OpShift,
    OpOr,
    OpXor,
    OpAnd,
    OpNot,
    OpLT,
    OpLE,
    OpGT,
    OpGE,
    OpEQ,
    OpNE,
    OpSTREQ,
    OpSTRNE,
    OpPRED_INC,
    OpPRED_DEC,
    OpPOST_INC,
    OpPOST_DEC,
    OpUndef = -1
} JSOperator;

extern int js_lex_parse(const char * gjs_in);
extern tree js_build_id(const char *str);
extern tree js_build_int(int val);
extern tree js_build_float_str(const char *buf);
extern tree js_build_string(const char *str);

extern void __js_debug__( const char * file, unsigned int lineno, const char * fmt, ... );
//#define debug(...)
//#define debug0(...)
//#define debug1(...)
#define debug(...)  __js_debug__( __FILE__, __LINE__, __VA_ARGS__ )
//#ifdef DBG0
#define debug0(...)  debug(__VA_ARGS__)
//#define DBG1 1
//#define DBG2 1
//#else
//#define debug0(...)
//#endif
//#ifdef DBG1
//#define debug1(...)  debug(__VA_ARGS__)
//#else
//#define debug1(...)
//#endif


#endif /* end of include guard: JS_OP_H */
