%{
#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"
#include "opts.h"
#include "tree.h"
#include "gimple.h"
#include "toplev.h"
#include "debug.h"
#include "options.h"
#include "flags.h"
#include "hashtab.h"
#include "convert.h"
#include "diagnostic.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "target.h"
#include "js-buffer.h"

#if 0
#define YYDEBUG 1
#endif
#include "js-lang.h"
#include "js-op.h"
#include "y.js.h"

int js_wrap (void);
int js_error (const char *);
static jsbuf_t *global_buf = NULL;
static void write_buf(char *str);
static char *buf_tochar(void);
/*static void js_unput (int, register char *) ATTRIBUTE_UNUSED;*/
/*static int input (void) ATTRIBUTE_UNUSED;*/
%}

%pointer
%option yylineno

%start COMMENT MULTI_COMMENT DoubleStringCharacters SingleStringCharacters RegexCharacters

%%
<INITIAL>"null"       {
    js_lval.token = NULL_TREE;
    return T_NULL;
}
<INITIAL>"true"       {
    js_lval.token = boolean_true_node;
    return T_TRUE;
}
<INITIAL>"false"      {
    js_lval.token = boolean_false_node;
    return T_FALSE;
}

<INITIAL>"var"        return Var;
<INITIAL>"if"         return If;
<INITIAL>"else"       {
    return Else;
}
<INITIAL>"do"         return Do;
<INITIAL>"while"      return While;
<INITIAL>"return"     return Return;
<INITIAL>"for"        return For;
<INITIAL>"in"         return In;
<INITIAL>"new"        return New;
<INITIAL>"void"       return Void;
<INITIAL>"this"       return This;
<INITIAL>"typeof"     return Typeof;
<INITIAL>"delete"     return Delete;
<INITIAL>"case"       return Case;
<INITIAL>"continue"   return Continue;
<INITIAL>"break"      return Break;
<INITIAL>"default"    return Default;
<INITIAL>"throw"      return Throw;
<INITIAL>"with"       return With;
<INITIAL>"switch"     return Switch;
<INITIAL>"debugger"   return Debugger;
<INITIAL>"function"   return Function;
<INITIAL>"try"        return Try;
<INITIAL>"catch"      return CATCH;
<INITIAL>"finally"    return FINALLY;
<INITIAL>"instanceof" return Instanceof;

<INITIAL>","       return CAMMA;
<INITIAL>"."       return DOT;
<INITIAL>"+"       return ADD;
<INITIAL>"-"       return SUB;
<INITIAL>"*"       return MUL;
<INITIAL>"/"       return DIV;
<INITIAL>"%"       return REM;

<INITIAL>"~"       return INV;
<INITIAL>"!"       return NOT;
<INITIAL>"<<"     return LSHIFT;
<INITIAL>">>"     return RSHIFT;
<INITIAL>"<="      return LTE;
<INITIAL>"<"       return LT;
<INITIAL>">="      return GTE;
<INITIAL>">"       return GT;
<INITIAL>":"       return COLON;
<INITIAL>";"       return SEMICOLON;
<INITIAL>"{"       {
    return LBRACE;
}
<INITIAL>"}"       {
    return RBRACE;
}
<INITIAL>"("       return LCBRACE;
<INITIAL>")"       return RCBRACE;
<INITIAL>"["       return LPARENTHESIS;
<INITIAL>"]"       return RPARENTHESIS;
<INITIAL>"++"      return PLUSPLUS;
<INITIAL>"--"      return MINUSMINUS;
<INITIAL>"=="      return EQEQ;     /* ==  */
<INITIAL>"!="      return NEQ;      /* !=  */
<INITIAL>"==="     return STREQ;    /* === */
<INITIAL>"!=="     return STRNEQ;   /* !== */
<INITIAL>"="       return EQ_LET;     /* = */
<INITIAL>"*="      return MUL_LET;    /* *= */
<INITIAL>"/="      return DIV_LET;    /* /= */
<INITIAL>"%="      return REM_LET;    /* %= */
<INITIAL>"+="      return ADD_LET;    /* += */
<INITIAL>"-="      return SUB_LET;    /* -= */
<INITIAL>"<<="     return LSHIFT_LET; /* <<=  */
<INITIAL>">>="     return RSHIFT_LET; /* >>=  */
<INITIAL>">>>="    return SHIFT_LET;  /* >>>= */
<INITIAL>"&="      return AND_LET;    /* &= */
<INITIAL>"^="      return XOR_LET;    /* ^= */
<INITIAL>"|="      return OR_LET;     /* |= */

<INITIAL>[ \t] ;
<INITIAL>[ \t\r\n] {}

<INITIAL>\" {
    BEGIN DoubleStringCharacters;
}
<INITIAL>\' {
    BEGIN SingleStringCharacters;
}
<INITIAL>"/" {
    BEGIN RegexCharacters;
}
<INITIAL>"//"  BEGIN COMMENT;
<COMMENT>. ;
<COMMENT>\n  {
    BEGIN INITIAL;
}
<INITIAL>"/*"  BEGIN MULTI_COMMENT;
<MULTI_COMMENT>. ;
<MULTI_COMMENT>\n  {}
<MULTI_COMMENT>"*/"  {
    BEGIN INITIAL;
}
<INITIAL>[A-Za-z_$][A-Za-z_0-9]* {
    /*debug("id=%s", yytext);*/
    js_lval.token = js_build_id(yytext);
    return IdentifierName;
}
<INITIAL>[1-9][0-9]* {
    int temp;
    sscanf(yytext, "%d", &temp);
    js_lval.token = js_build_int(temp);
    return IntegerLiteral;
}
<INITIAL>(0[Xx])?[0-9a-zA-Z]+ {
    unsigned int temp;
    sscanf(yytext, "%x", &temp);
    js_lval.token = js_build_int(temp);
    return HexIntegerLiteral;
}
<INITIAL>[-+]?[0-9]+(\.[0-9]*)([eE][-+]?[0-9]+)? {
    js_lval.token = js_build_float_str(yytext);
    return FloatLiteral;
}

<DoubleStringCharacters>\" {
    BEGIN INITIAL;
    js_lval.token = js_build_string(buf_tochar());
    return StringLiteral;
}
<DoubleStringCharacters>\n  {
    js_error("terminate string without '\"'");
}
<DoubleStringCharacters>\\\" {
    /* '"'  */
    write_buf(yytext);
}
<DoubleStringCharacters>\\n  {
    /* '\n' */
    write_buf(yytext);
}
<DoubleStringCharacters>\\t {
    /* '\t' */
    write_buf(yytext);
}
<DoubleStringCharacters>\\\\  {
    /* '\\' */
    write_buf(yytext);
}
<DoubleStringCharacters>.  {
    /*  */
    write_buf(yytext);
}

<SingleStringCharacters>\' {
    BEGIN INITIAL;
    js_lval.token = js_build_string(buf_tochar());
    return StringLiteral;
}
<SingleStringCharacters>\n  {
    js_error("terminate string without '\''");
}
<SingleStringCharacters>\\\' {
    /* '"'  */
    write_buf(yytext);
}
<SingleStringCharacters>\\n  {
    /* '\n' */
    write_buf(yytext);
}
<SingleStringCharacters>\\t  {
    /* '\t' */
    write_buf(yytext);
}
<SingleStringCharacters>\\\\ {
    /* '\\' */
    write_buf(yytext);
}
<SingleStringCharacters>.    {
    /*  */
    write_buf(yytext);
}

<RegexCharacters>"/" {
    BEGIN INITIAL;
    js_lval.token = js_build_string(buf_tochar());
    return RegexLiteral;
}
<RegexCharacters>\n  {
    js_error("terminate string without '\''");
}
<RegexCharacters>\/ {
    /* '/'  */
    write_buf(yytext);
}
<RegexCharacters>\\\\ {
    /* '\\' */
    write_buf(yytext);
}
<RegexCharacters>.    {
    /*  */
    write_buf(yytext);
}

<INITIAL>. {
    fprintf(stderr, "lexical error\n");
    exit(1);
}
%%
/**/ 
static char *buf_tochar(void)
{
  int len;
  char *str;
  if (global_buf == NULL)
      write_buf("");
  str = js_buf_tochar(global_buf, &len);
  js_buf_clear(global_buf);
  return str;
}
static void write_buf(char *str)
{
  if (global_buf == NULL) {
      global_buf = js_buf_new();
  }
  js_buf_write(global_buf, str, strlen(str));
}

int js_wrap (void)
{
  return 1;
}
int js_lex_parse( const char * gjs_in )
{
  int retval = 1;
  FILE * fd = fopen( gjs_in, "rb" );
  debug("trying to open <%s>!", gjs_in);
  if( fd ) {
      yyin = fd;
      retval = js_parse( );
      fclose( fd );
      yylex_destroy( );
  } else {
      fprintf(stderr, "error opening <%s>!\n", gjs_in );
      retval = 0;
  }
  return retval;
}
int js_error (const char *msg)
{
  error("%s", msg);
  exit(FATAL_EXIT_CODE);
}