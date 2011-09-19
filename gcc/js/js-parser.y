%{
#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"
#include "opts.h"
#include "tree.h"
#include "gimple.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "toplev.h"
#include "debug.h"
#include "options.h"
#include "flags.h"
#include "convert.h"
#include "diagnostic-core.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "target.h"
#include "cgraph.h"

#include <gmp.h>
#include <mpfr.h>

#include "vec.h"
#include "hashtab.h"

#include "gjs.h"
#include "js-lang.h"
#include "js-op.h"
#include "js-tree.h"

#ifndef DBG0
#define debug0(...)
#else
#define debug0(...) debug(__VA_ARGS__)
#endif

extern int js_lex (void);
int js_error(const char *str);

location_t CURRENT_LOCATION(void);
#define LOC CURRENT_LOCATION()
#define DEBUG( ... ) \
fprintf(stderr, "JS(%d) ", CURRENT_LOCATION());\
__js_debug__( __FILE__, __LINE__, __VA_ARGS__ );
%}

%union
{
  jstree token;
  JSOperator op;
}

%error-verbose
//%start declaration
%debug

/* Locations will be added later to add debugging information */
%locations


%start Program
%token CAMMA
%token COLON
%token SEMICOLON
%token LBRACE RBRACE   /* { } */
%token LCBRACE RCBRACE /* ( ) */
%token LPARENTHESIS RPARENTHESIS /* [ ] */
%token Var
%token If Else 
%token Do While 
%token For 
%token In 
%token Switch Case Break Default 
%token Delete
%token Continue
%token Function Return 
%token This
%token Try CATCH FINALLY Throw 
%token Typeof Instanceof
%token With 
%token New
%token Void
%token Debugger
%token T_NULL T_TRUE T_FALSE
%token IdentifierName
%token HexIntegerLiteral IntegerLiteral FloatLiteral StringLiteral RegexLiteral
%token PLUSPLUS MINUSMINUS
%token EQ_LET                   /* = */
%token MUL_LET DIV_LET REM_LET  /* *= /= %= */
%token ADD_LET SUB_LET          /* += -= */
%token LSHIFT_LET RSHIFT_LET    /* <<= >>= */
%token SHIFT_LET                /* >>>= */
%token AND_LET OR_LET XOR_LET   /* &= |= ^=*/
%token DOT QUESTION             /* . ? */
%token AND LAND OR LOR XOR      /* & && | || ^ */
%token ADD SUB MUL DIV REM      /* + - * / % */
%token INV NOT LT LTE GT GTE    /* ~ ! < <= > >= */
%token LSHIFT RSHIFT SHIFT      /* << >> >>> */
%token EQEQ NEQ STREQ STRNEQ    /* == != === !== */

%nonassoc IF_WITHOUT_ELSE
%nonassoc ELSE

%type <token> Statement StatementList Program SourceElement SourceElements
%type <token> Literal ArrayLiteral ObjectLiteral
%type <token> Expression VariableStatement
%type <token> AssignmentExpression AssignmentExpressionNoIn
%type <token> VariableDeclaration VariableDeclarationNoIn
%type <token> Initialiser InitialiserNoIn
%type <token> VariableDeclarationList VariableDeclarationListNoIn
%type <token> IterationStatement ReturnStatement
%type <token> ContinueStatement BreakStatement
%type <token> WithStatement LabelledStatement SwitchStatement
%type <token> ThrowStatement TryStatement DebuggerStatement
%type <token> FunctionDeclaration FunctionBody

%type <token> ExpressionNoInopt Expressionopt
%type <token> AdditiveExpression MultiplicativeExpression
%type <token> LeftHandSideExpression Block NewExpression
%type <token> PrimaryExpression IfStatement
%type <token> NullLiteral BooleanLiteral NumericLiteral
%type <token> IntegerLiteral HexIntegerLiteral
%type <token> StringLiteral  FloatLiteral Identifier
%type <token> CallExpression MemberExpression FunctionExpression 
%type <token> PostfixExpression ExpressionStatement
%type <token> UnaryExpression ShiftExpression
%type <token> RelationalExpression RelationalExpressionNoIn
%type <token> EqualityExpression EqualityExpressionNoIn
%type <token> BitwiseANDExpression BitwiseANDExpressionNoIn
%type <token> BitwiseORExpression BitwiseORExpressionNoIn
%type <token> BitwiseXORExpression BitwiseXORExpressionNoIn
%type <token> LogicalANDExpression LogicalANDExpressionNoIn
%type <token> LogicalORExpression LogicalORExpressionNoIn
%type <token> PropertyName PropertyNameAndValueList PropertyAssignment

%type <token> IdentifierName

%type <token> ArgumentList Arguments
%type <token> FormalParameterListopt FormalParameterList
%type <token> ElementList
%type <op> AssignmentOperator

%%
/*TODO RegexLiteral*/
Literal 
    : NullLiteral
    | BooleanLiteral
    | NumericLiteral
    | StringLiteral
    ;

NullLiteral 
    : T_NULL {}
    ;

BooleanLiteral 
    : T_TRUE {}
    | T_FALSE {}
    ;

/* DecimalLiteral */
NumericLiteral 
    : IntegerLiteral
    | FloatLiteral 
    | HexIntegerLiteral;

Expression 
    : AssignmentExpression 
    | Expression CAMMA AssignmentExpression ;

Expressionopt
    : {}
    | Expression {}
    ;

ExpressionNoIn
    : AssignmentExpressionNoIn
    | ExpressionNoIn CAMMA AssignmentExpressionNoIn
    ;

ExpressionNoInopt
    : {}
    | ExpressionNoIn {}
    ;

Statement
    : Block
    | VariableStatement
    | EmptyStatement {}
    | ExpressionStatement
    | IfStatement
    | IterationStatement
    | ContinueStatement
    | BreakStatement
    | ReturnStatement 
    | WithStatement
    | LabelledStatement
    | SwitchStatement
    | ThrowStatement
    | TryStatement
    | DebuggerStatement
    ;

Block 
    : LBRACE RBRACE {
        $$ = NULL;
    }
    | LBRACE StatementList RBRACE {
        $$ = $2;
    }
    ;

StatementList 
    : Statement {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | StatementList Statement {
        JSTREE_APPENDTAIL($1, $2);
    }
    ;

VariableStatement 
    : Var VariableDeclarationList SEMICOLON {
        $$ = $2;
    }
    ;

VariableDeclarationList 
    : VariableDeclaration {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | VariableDeclarationList CAMMA VariableDeclaration {
        JSTREE_APPENDTAIL($1, $3);
    }
    ;

VariableDeclaration 
    : Identifier {
        $$ = js_build2(LOC, OP_LET, TyValue, $1, JS_UNDEFINED);
    }
    | Identifier Initialiser {
        $$ = js_build2(LOC, OP_LET, TyValue, $1, $2);
    }
    ;

VariableDeclarationListNoIn 
    : VariableDeclarationNoIn {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | VariableDeclarationListNoIn CAMMA VariableDeclarationNoIn {
        JSTREE_APPENDTAIL($1, $3);
    }
    ;

VariableDeclarationNoIn
    : Identifier {
        $$ = js_build2(LOC, OP_LET, TyValue, $1, JS_UNDEFINED);
    }
    | Identifier InitialiserNoIn {
        $$ = js_build2(LOC, OP_LET, TyValue, $1, $2);
    }
    ;

Initialiser 
    : EQ_LET AssignmentExpression {
        $$ = $2;
    }
    ;

InitialiserNoIn 
    : EQ_LET AssignmentExpressionNoIn {
        $$ = $2;
    }
    ;

AssignmentExpression 
    : ConditionalExpression {}
    | LeftHandSideExpression AssignmentOperator AssignmentExpression {
        $$ = js_build2(LOC, $2, TyValue, $1, $3);
    }
    ;

AssignmentExpressionNoIn
    : ConditionalExpressionNoIn {}
    | LeftHandSideExpression AssignmentOperator AssignmentExpressionNoIn {
        $$ = js_build2(LOC, $2, TyValue, $1, $3);
    }
    ;

AssignmentOperator
    : EQ_LET     /*    = */ { $$ = OP_LET; }
    | MUL_LET    /*   *= */ { $$ = OP_MULLET; }
    | DIV_LET    /*   /= */ { $$ = OP_DIVLET; }
    | REM_LET    /*   %= */ { $$ = OP_MODLET; }
    | ADD_LET    /*   += */ { $$ = OP_ADDLET; }
    | SUB_LET    /*   -= */ { $$ = OP_SUBLET; }
    | LSHIFT_LET /*  <<= */ { $$ = OP_LSFTLET; }
    | RSHIFT_LET /*  >>= */ { $$ = OP_RSFTLET; }
    | SHIFT_LET  /* >>>= */ { $$ = OP_SHFTLET; }
    | AND_LET    /*   &= */ { $$ = OP_ANDLET; }
    | XOR_LET    /*   ^= */ { $$ = OP_XORLET; }
    | OR_LET     /*   |= */ { $$ = OP_ORLET; }
    ;

ConditionalExpression 
    : LogicalORExpression
    | LogicalORExpression QUESTION AssignmentExpression COLON AssignmentExpression
    ;

ConditionalExpressionNoIn 
    : LogicalORExpressionNoIn
    | LogicalORExpressionNoIn QUESTION AssignmentExpressionNoIn COLON AssignmentExpressionNoIn
    ;

EmptyStatement 
    : SEMICOLON {}
    ;

ExpressionStatement 
    : Expression SEMICOLONopt
    ;

IfStatement 
    : If LCBRACE Expression RCBRACE Statement %prec IF_WITHOUT_ELSE {
        DEBUG("IfStatement1");
        $$ = js_build_cond(LOC, $3, $5, NULL);
    }
    | If LCBRACE Expression RCBRACE Statement Else Statement {
        DEBUG("IfStatement2");
        $$ = js_build_cond(LOC, $3, $5, $7);
    }
;

IterationStatement
    : Do Statement While LCBRACE Expression RCBRACE SEMICOLON {
        DEBUG("While1");
        $$ = js_build_loop(LOC, LOOP_DOWHILE, NULL, $5/*cond*/, NULL, $2/*body*/);
    }
    | While LCBRACE Expression RCBRACE Statement {
        DEBUG("While2");
        $$ = js_build_loop(LOC, LOOP_WHILE, NULL, $3/*cond*/, NULL, $5/*body*/);
    }
    | For LCBRACE ExpressionNoInopt SEMICOLON Expressionopt SEMICOLON Expressionopt RCBRACE Statement {
        DEBUG("for1");
        $$ = js_build_loop(LOC, LOOP_FOR, $3/*init*/ , $5/*cond*/, $7/*inc*/, $9/*body*/);
    }

    | For LCBRACE Var VariableDeclarationListNoIn SEMICOLON Expressionopt SEMICOLON Expressionopt RCBRACE Statement {
        DEBUG("for2");
        $$ = js_build_loop(LOC, LOOP_FOR_VAR, $4/*init*/ , $6/*cond*/, $8/*inc*/, $10/*body*/);
    }

    | For LCBRACE LeftHandSideExpression In Expression RCBRACE Statement {
        TODO();
        $$ = js_build_loop(LOC, LOOP_FOR_IN, $3/*init*/ , NULL, $5/*inc*/, $7/*body*/);
    }
    | For LCBRACE Var VariableDeclarationNoIn In Expression RCBRACE Statement {
        TODO();
        $$ = js_build_loop(LOC, LOOP_FOR_VAR_IN, $4/*init*/ , NULL, $6/*inc*/, $8/*body*/);
    }
;

ContinueStatement 
    : Continue SEMICOLON {
        TODO();
    }
    | Continue Identifier SEMICOLON {
        TODO();
    }
;

BreakStatement 
    : Break SEMICOLON {
        TODO();
    }
    | Break Identifier SEMICOLON {
        TODO();
    }
;

SEMICOLONopt 
    : 
    | SEMICOLON
;
ReturnStatement 
    : Return SEMICOLONopt {
        $$ = js_build1(LOC, OP_RETURN, TyNone, NULL);
    }
    | Return Expression SEMICOLONopt {
        $$ = js_build1(LOC, OP_RETURN, TyValue, $2);
    }
;

WithStatement 
    : With LCBRACE Expression RCBRACE Statement {
        TODO();
    }
;

SwitchStatement 
    : Switch LCBRACE Expression RCBRACE CaseBlock {
        TODO();
    }
;

CaseBlock 
    : LBRACE CaseClausesopt RBRACE {
        TODO();
    }
    | LBRACE CaseClausesopt DefaultClause CaseClausesopt RBRACE {
        TODO();
    }
    ;

CaseClausesopt 
    :
    | CaseClause
;

CaseClause 
    : Case Expression COLON 
    | Case Expression COLON StatementList
;

DefaultClause
    : Default COLON
    | Default COLON StatementList
;

LabelledStatement 
    : Identifier COLON Statement
    ;

ThrowStatement 
    : Throw Expression {
        DEBUG("ThrowStatement");TODO();
        $$ = js_build_nop(LOC);//js_build_throw_expr($2);
    }
    ;

TryStatement 
    : Try Block Catch {
        TODO();
    }
    | Try Block Finally {
        TODO();
    }
    | Try Block Catch Finally {
        TODO();
    }
    ;

Catch
    : CATCH LCBRACE Identifier RCBRACE Block
    ;

Finally
    : FINALLY Block
    ;

DebuggerStatement 
    : Debugger SEMICOLON {
        TODO();
    }
    ;

Program 
    : /* not in spec */ {}
    | SourceElements {
        if (!global_tree)
          global_tree = $1;
        else
          JSTREE_APPENDTAIL(global_tree, $1);
    }
;

FunctionDeclaration
    : Function Identifier LCBRACE FormalParameterListopt RCBRACE LBRACE FunctionBody RBRACE {
        DEBUG("FunctionDeclaration");
        $$ = js_build_defun(LOC, $2, $4, $7);
    }
    ;

FunctionExpression
    :Function LCBRACE FormalParameterListopt RCBRACE LBRACE FunctionBody RBRACE {
        DEBUG("FunctionExpression1");
        $$ = js_build_defun(LOC, NULL, $3, $6);
    }
    |Function Identifier LCBRACE FormalParameterListopt RCBRACE LBRACE FunctionBody RBRACE {
        DEBUG("FunctionExpression2");
        $$ = js_build_defun(LOC, $2, $4, $7);
    }
    ;

FormalParameterListopt
    : {
        $$ = NULL;
    }
    | FormalParameterList
    ;

FormalParameterList
    : Identifier {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | FormalParameterList CAMMA Identifier {
        JSTREE_APPENDTAIL($1, $3);
    }

FunctionBody 
    : {
        /*DEBUG("FunctionBody1");*/
        $$ = NULL;
    }
    |SourceElements {
        /*DEBUG("FunctionBody2");*/
    }
    ;

SourceElements
    : SourceElement {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | SourceElements SourceElement {
        JSTREE_APPENDTAIL($1, $2);
    }
;

SourceElement 
    : Statement
    | FunctionDeclaration
;

PrimaryExpression 
    : This {
        $$ = js_build_id(LOC, "this");
    }
    | Identifier {}
    | Literal {}
    | ArrayLiteral {
        DEBUG("PrimaryExpression4");
    }
    | ObjectLiteral {
        DEBUG("PrimaryExpression5");
    }
    | LCBRACE Expression RCBRACE {
        $$ = $2;
    }
    ;

ArrayLiteral 
    : LPARENTHESIS RPARENTHESIS {
        /* [] */
        DEBUG("ArrayLiteral1");
        $$ = js_build_array(LOC, NULL);
    }
    | LPARENTHESIS Elision RPARENTHESIS {
        /* [,] [a,] */
        DEBUG("ArrayLiteral2");
        TODO();
        $$ = js_build_array(LOC, NULL);
    }
    | LPARENTHESIS ElementList RPARENTHESIS {
        /* [a=1, v=3] */
        DEBUG("ArrayLiteral3");
        $$ = js_build_array(LOC, $2);
    }
    | LPARENTHESIS ElementList CAMMA Elisionopt RPARENTHESIS {
        /* [a, b, c] */
        DEBUG("ArrayLiteral4");
        $$ = js_build_array(LOC, $2);
    }
;

ElementList 
    : Elisionopt AssignmentExpression {
        JSTREE_APPENDTAIL($2, NULL);
        $$ = $2;
    }
    | ElementList CAMMA Elisionopt AssignmentExpression {
        JSTREE_APPENDTAIL($1, $4);
        $$ = $1;
    }
    ;

Elisionopt
    :
    | Elision
    ;

Elision 
    : CAMMA
    | Elision CAMMA
    ;

ObjectLiteral 
    : LBRACE RBRACE {
        DEBUG("ObjectLiteral1");
        $$ = js_build1(LOC, OP_OBJECT, TyObject, NULL);
    }
    | LBRACE PropertyNameAndValueList RBRACE {
        DEBUG("ObjectLiteral2");
        $$ = js_build1(LOC, OP_OBJECT, TyObject, $2);
    }
    | LBRACE PropertyNameAndValueList CAMMA RBRACE {
        DEBUG("ObjectLiteral3");
        $$ = js_build1(LOC, OP_OBJECT, TyObject, $2);
    }
    ;

PropertyNameAndValueList 
    : PropertyAssignment {
        DEBUG("PropertyNameAndValueList0");
        JSTREE_APPENDTAIL($1, NULL);
    }
    | PropertyNameAndValueList CAMMA PropertyAssignment {
        DEBUG("PropertyNameAndValueList1");
        JSTREE_APPENDTAIL($1, $3);
    }
    ;

PropertyAssignment 
    : PropertyName COLON AssignmentExpression {
        jstree t;
        DEBUG("PropertyAssignment1");
        t = js_build2(LOC, OP_FIELD, TyValue, js_build_id(LOC, "this"), $1);
        $$ = js_build2(LOC, OP_SetField, TyValue, t, $3);
    }
    | "get" PropertyName LCBRACE RCBRACE LBRACE FunctionBody RBRACE {
        DEBUG("PropertyAssignment2");
        TODO();
    }
    | "set" PropertyName LCBRACE PropertySetParameterList RCBRACE LBRACE FunctionBody RBRACE {
        DEBUG("PropertyAssignment3");
        TODO();
    }
    ;

PropertyName 
    : IdentifierName
    | StringLiteral
    | NumericLiteral
    ;

PropertySetParameterList 
    : Identifier
    ;

Identifier 
    : IdentifierName {
    }
    ;

MemberExpression 
    : PrimaryExpression {
        /*DEBUG("MemberExpression1");*/
    }
    | FunctionExpression {
        //DEBUG("MemberExpression2");
    }
    | MemberExpression LPARENTHESIS Expression RPARENTHESIS {
        //DEBUG("MemberExpression3");
        //tree expr = js_build_array_expr($1, $3);
        //$$ = expr;
    }
    | MemberExpression DOT IdentifierName {
        //DEBUG("MemberExpression4");
        $$ = js_build2(LOC, OP_FIELD, TyValue, $1, $3);
        //$$ = js_build_propaty_expr($1, $3);
    }
    | New MemberExpression Arguments {
        /*DEBUG("MemberExpression5");*/
        //$$ = js_build_new_expr($2, $3);
        $$ = js_build_call(LOC, OP_NEW, TyValue, $2, $3);
    }
    ;

NewExpression 
    : MemberExpression {
        /*DEBUG("NewExpression1");*/
    }
    | New NewExpression {
        DEBUG("NewExpression2");
        /*$$ = js_build_new_expr($2, NULL);*/
    }
    ;

CallExpression 
    : MemberExpression Arguments {
        DEBUG("CallExpression1");
        $$ = js_build_call(LOC, OP_CALL, TyValue, $1, $2);
    }
    | CallExpression Arguments {
        DEBUG("CallExpression2");
        $$ = js_build_call(LOC, OP_CALL, TyValue, $1, $2);
    }
    | CallExpression LPARENTHESIS Expression RPARENTHESIS {
        DEBUG("CallExpression3");
    }
    | CallExpression DOT IdentifierName {
        DEBUG("CallExpression4");
    }
    ;

Arguments 
    : LCBRACE RCBRACE {
        $$ = NULL;
    }
    | LCBRACE ArgumentList RCBRACE {
        $$ = $2;
    }
    ;

ArgumentList 
    : AssignmentExpression {
        JSTREE_APPENDTAIL($1, NULL);
    }
    | ArgumentList CAMMA AssignmentExpression {
        JSTREE_APPENDTAIL($1, $3);
    }
    ;

LeftHandSideExpression 
    : NewExpression {
        /*DEBUG("LeftHandSideExpression1");*/
        /*$$ = js_build_propaty_expr($1);*/
    }
    | CallExpression {
        /*DEBUG("LeftHandSideExpression2");*/
    }
    ;

PostfixExpression
    : LeftHandSideExpression {}
    | LeftHandSideExpression PLUSPLUS {
        $$ = js_build1(LOC, OP_POST_INC, TyValue, $1);
    }
    | LeftHandSideExpression MINUSMINUS {
        $$ = js_build1(LOC, OP_POST_DEC, TyValue, $1);
    }
    ;

UnaryExpression 
    : PostfixExpression {}
    | Delete UnaryExpression {
        DEBUG("delete");
        $$ = js_build_call(LOC, OP_CALL, TyValue, js_build_id(LOC, "delete"), $2);
    }
    | Void UnaryExpression {
        DEBUG("void");
        $$ = js_build_call(LOC, OP_CALL, TyValue, js_build_id(LOC, "void"), $2);
    }
    | Typeof UnaryExpression {
        DEBUG("Typeof");
        $$ = js_build_call(LOC, OP_CALL, TyValue, js_build_id(LOC, "typeof"), $2);
    }
    | PLUSPLUS   UnaryExpression {
        DEBUG("++ UnaryExpression");
        $$ = js_build1(LOC, OP_PRED_INC, TyValue, $2);
    }
    | MINUSMINUS UnaryExpression {
        DEBUG("-- UnaryExpression");
        $$ = js_build1(LOC, OP_PRED_DEC, TyValue, $2);
    }
    | ADD  UnaryExpression {
        DEBUG("+ val");
        $$ = js_build2(LOC, OP_Plus, TyValue, $2, NULL);
    }
    | SUB  UnaryExpression {
        DEBUG("- val");
        $$ = js_build2(LOC, OP_Minus, TyValue, $2, NULL);
    }
    | INV  UnaryExpression {
        DEBUG("INV val");
        TODO();
        //$$ = js_build1(LOC, OP_Inv, TyValue, $2);
    }
    | NOT  UnaryExpression {
        DEBUG("NOT UnaryExpression");
        $$ = js_build1(LOC, OP_Not, TyValue, $2);
    }
    ;

MultiplicativeExpression
    : UnaryExpression
    | MultiplicativeExpression MUL UnaryExpression {
        DEBUG("MultiplicativeExpression1");
        $$ = js_build2(LOC, OP_Mul, TyValue, $1, $3);
    }
    | MultiplicativeExpression DIV UnaryExpression {
        debug0("MultiplicativeExpression2");
        $$ = js_build2(LOC, OP_Div, TyValue, $1, $3);
    }
    | MultiplicativeExpression REM UnaryExpression {
        debug0("MultiplicativeExpression3");
        $$ = js_build2(LOC, OP_Mod, TyValue, $1, $3);
    }
    ;

AdditiveExpression
    : MultiplicativeExpression
    | AdditiveExpression ADD MultiplicativeExpression {
        debug0("AdditiveExpression1");
        $$ = js_build2(LOC, OP_Plus, TyValue, $1, $3);
    }
    | AdditiveExpression SUB MultiplicativeExpression {
        debug0("AdditiveExpression2");
        $$ = js_build2(LOC, OP_Minus, TyValue, $1, $3);
    }
    ;

ShiftExpression 
    : AdditiveExpression
    | ShiftExpression LSHIFT AdditiveExpression {
        DEBUG("ShiftExpression");
        $$ = js_build2(LOC, OP_Lshift, TyValue, $1, $3);
    }
    | ShiftExpression RSHIFT AdditiveExpression {
        $$ = js_build2(LOC, OP_Rshift, TyValue, $1, $3);
    }
    | ShiftExpression SHIFT  AdditiveExpression {
        TODO();
    }
    ;

RelationalExpression 
    : ShiftExpression
    | RelationalExpression LT  ShiftExpression {
        $$ = js_build2(LOC, OP_LT, TyBoolean, $1, $3);
    }
    | RelationalExpression GT  ShiftExpression {
        $$ = js_build2(LOC, OP_GT, TyBoolean, $1, $3);
    }
    | RelationalExpression LTE ShiftExpression {
        $$ = js_build2(LOC, OP_LE, TyBoolean, $1, $3);
    }
    | RelationalExpression GTE ShiftExpression {
        $$ = js_build2(LOC, OP_GE, TyBoolean, $1, $3);
    }
    | RelationalExpression Instanceof ShiftExpression {
        TODO();
    }
    | RelationalExpression In ShiftExpression {
        TODO();
    }
    ;

RelationalExpressionNoIn
    : ShiftExpression
    | RelationalExpression LT  ShiftExpression {
        $$ = js_build2(LOC, OP_LT, TyBoolean, $1, $3);
    }
    | RelationalExpression GT  ShiftExpression {
        $$ = js_build2(LOC, OP_GT, TyBoolean, $1, $3);
    }
    | RelationalExpression LTE ShiftExpression {
        $$ = js_build2(LOC, OP_LE, TyBoolean, $1, $3);
    }
    | RelationalExpression GTE ShiftExpression {
        $$ = js_build2(LOC, OP_GE, TyBoolean, $1, $3);
    }
    | RelationalExpression Instanceof ShiftExpression {
        TODO();
    }
    ;

EqualityExpression 
    : RelationalExpression
    | EqualityExpression EQEQ   RelationalExpression {
        $$ = js_build2(LOC, OP_EQ, TyBoolean, $1, $3);
    }
    | EqualityExpression NEQ    RelationalExpression {
        $$ = js_build2(LOC, OP_NE, TyBoolean, $1, $3);
    }
    | EqualityExpression STREQ  RelationalExpression {
        $$ = js_build2(LOC, OP_STREQ, TyBoolean, $1, $3);
    }
    | EqualityExpression STRNEQ RelationalExpression {
        $$ = js_build2(LOC, OP_STRNE, TyBoolean, $1, $3);
    }
    ;

EqualityExpressionNoIn
    : RelationalExpressionNoIn
    | EqualityExpressionNoIn EQEQ   RelationalExpressionNoIn {
        $$ = js_build2(LOC, OP_EQ, TyBoolean, $1, $3);
    }
    | EqualityExpressionNoIn NEQ    RelationalExpressionNoIn {
        $$ = js_build2(LOC, OP_NE, TyBoolean, $1, $3);
    }
    | EqualityExpressionNoIn STREQ  RelationalExpressionNoIn {
        $$ = js_build2(LOC, OP_STREQ, TyBoolean, $1, $3);
    }
    | EqualityExpressionNoIn STRNEQ RelationalExpressionNoIn {
        $$ = js_build2(LOC, OP_STRNE, TyBoolean, $1, $3);
    }
    ;

BitwiseANDExpression 
    : EqualityExpression
    | BitwiseANDExpression AND EqualityExpression {
        $$ = js_build2(LOC, OP_And, TyValue, $1, $3);
    }
    ;

BitwiseXORExpression 
    : BitwiseANDExpression
    | BitwiseXORExpression XOR BitwiseANDExpression {
        $$ = js_build2(LOC, OP_Xor, TyValue, $1, $3);
    }
    ;

BitwiseORExpression 
    : BitwiseXORExpression
    | BitwiseORExpression OR BitwiseXORExpression {
        $$ = js_build2(LOC, OP_Or, TyValue, $1, $3);
    }
    ;

LogicalANDExpression 
    : BitwiseORExpression {}
    | LogicalANDExpression LAND BitwiseORExpression {
        $$ = js_build2(LOC, OP_LAND, TyValue, $1, $3);
    }
    ;

LogicalORExpression 
    : LogicalANDExpression {}
    | LogicalORExpression LOR LogicalANDExpression {
        DEBUG("LogicalORExpression");
        $$ = js_build2(LOC, OP_LOR, TyValue, $1, $3);
    }
    ;

BitwiseANDExpressionNoIn
    : EqualityExpressionNoIn {}
    | BitwiseANDExpressionNoIn AND EqualityExpressionNoIn {
        $$ = js_build2(LOC, OP_And, TyValue, $1, $3);
    }
    ;

BitwiseXORExpressionNoIn
    : BitwiseANDExpressionNoIn {}
    | BitwiseXORExpressionNoIn XOR BitwiseANDExpressionNoIn {
        $$ = js_build2(LOC, OP_Xor, TyValue, $1, $3);
    }
    ;

BitwiseORExpressionNoIn
    : BitwiseXORExpressionNoIn {}
    | BitwiseORExpressionNoIn OR BitwiseXORExpressionNoIn {
        $$ = js_build2(LOC, OP_Or, TyValue, $1, $3);
    }
    ;

LogicalANDExpressionNoIn
    : BitwiseORExpressionNoIn
    | LogicalANDExpressionNoIn LAND BitwiseORExpressionNoIn {
        $$ = js_build2(LOC, OP_LAND, TyValue, $1, $3);
    }
    ;

LogicalORExpressionNoIn
    : LogicalANDExpressionNoIn
    | LogicalORExpressionNoIn LOR LogicalANDExpressionNoIn {
        $$ = js_build2(LOC, OP_LOR, TyValue, $1, $3);
    }
    ;

%%

location_t CURRENT_LOCATION(void)
{
    location_t loc = ((location_t)js_lloc.last_line);
    linemap_line_start(line_table, loc, 0);
    return loc;
}
