#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <gjs/js.h>

#define RUNTIME(x) js_runtime_##x
#define JSInt(o) (((struct js_Number*)(o))->i)
#define JSFloat(o) (((struct js_Number*)(o))->f)
typedef struct js_Number js_Number_t;

static const char JS_STRING_TRUE[]  = "true";
static const char JS_STRING_FALSE[] = "false";

#define DBG(fmt, ...) _DBG(__FILE__, __LINE__, __func__, fmt, __VA_ARGS__)
static void _DBG(const char *file, unsigned int line, const char *func, const char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "[%s:%s(%d)] ", file, func, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void js_print(FILE *fd, js_type type, va_list vl)
{
    js_value_t v;
    //fprintf(stderr, "line=%d: %d\n", __LINE__, type);
    switch (type) {
        case JS_BOOL:
            v.ptr = va_arg(vl, js_Number_t*);
            //fprintf(stdout, "type=%d, %s", (JSInt(v.ptr))?JS_STRING_TRUE:JS_STRING_FALSE);
            fprintf(stdout, "%s", (JSInt(v.ptr))?JS_STRING_TRUE:JS_STRING_FALSE);
            break;
        case JS_INT:
            v.ival = va_arg(vl, int);
            //fprintf(stdout, "type=%d, %i", type, v.ival);
            fprintf(stdout, "%i", v.ival);
            break;
        case JS_STRING:
            v.string = va_arg(vl, js_string_t*);
            fprintf(stdout, "'%s'", v.string->__str);
            break;
        case JS_RAWSTR:
            v.rawstr = va_arg(vl, js_rawstring_t*);
            fprintf(stdout, "@'%s'", v.rawstr);
            break;
        case JS_INTOBJ:
            v.ptr = va_arg(vl, js_Number_t*);
            fprintf(stdout, "%i", JSInt(v.ptr));
            break;
        case JS_FLTOBJ:
            v.ptr = va_arg(vl, js_Number_t*);
            fprintf(stdout, "%f", tJSFloat(v.ptr));
            break;
        default :
            v.ptr = va_arg(vl, void*);
            if (IS_Int(v.o)) {
                fprintf(stdout, "%i", JSInt(v.ptr));
            } else if (IS_Float(v.o)) {
                fprintf(stdout, "%f", JSFloat(v.ptr));
            } else {
                fprintf(stdout, "%p", v.ptr);
            }
            break;
    }
}

js_Array_t *new_Array(int n)
{
    js_Array_t *a = NEW(js_Array_t);
    a->type = -1;
    a->size = 0;
    a->capacity = n;
    a->list = (void**)malloc(sizeof(void*) * n);
    return a;
}

DEF_ARRAY_T(int);
DEF_ARRAY_STRUCT(int);
DEF_ARRAY_OP(int, JS_INT);

js_Array_t *js_runtime_Array_new(int argc, ...)
{
    int i;
    js_value_t v;
    va_list vl;
    va_start(vl, argc);
    Array(int) *a = Array_new(int);
    for (i = 0; i < argc; ++i) {
        v.ival = va_arg(vl, js_int_t);
        Array_add(int, a, v.ival);
        fprintf(stderr, "list[%d]=%d %p\n", i, v.ival, Array_get(int, a, i));
    }
    va_end(vl);
    return a;
}

void js_runtime_print(int nargs, ... )
{
  int i;
  va_list vl;
  js_type type;
  js_value_t v;

  va_start(vl, nargs);
  type  = va_arg(vl, js_type);
  v.ptr = va_arg(vl, void*);
  if (type != JS_OBJECT || v.ptr != NULL) {
      fprintf(stdout, "self=%p", v.ptr);
  }
  for(i=1; i < nargs; ++i) {
      type = va_arg(vl, js_type);
      js_print(stdout, type, vl);
      if (i != nargs-1) {
          fputs(", ", stdout);
      }
  }
  fputc('\n', stdout);
  va_end(vl);
}
static js_string_t *__new_string(js_rawstring_t *str, int len);

DEF_ARRAY_T(string);
DEF_ARRAY_STRUCT(string);
DEF_ARRAY_OP(string, JS_STRING);
void js_runtime___init__(int argc, char **argv)
{
    int i;
    Array(string) *args = Array_new(string);
    for (i = 0; i < argc; i++) {
        Array_add(string, args,  __new_string(argv[i], strlen(argv[i])));
    }
}

static const js_prototype_t STRING_PROTOTYPE = {
#ifdef DEBUG
    "string", 
#endif
    JS_STRING, NULL, NULL, NULL, NULL
};

static js_string_t *__new_string_ref(js_rawstring_t *strref, int len)
{
    js_string_t* ret = (js_string_t*) malloc(sizeof(*ret));
    SET_TYPE(ret, JS_STRING);
    SET_PROTOTYPE(ret, &STRING_PROTOTYPE);
    ret->__str = strref;
    ret->length = len;
    return ret;
}

static js_string_t *__new_string(js_rawstring_t *str, int len)
{
    js_rawstring_t *s = (js_rawstring_t *) malloc(len);
    memcpy(s, str, len);
    return __new_string_ref(s, len);
}

/* Runtime */
void js_runtime_exception(const char *file_name, int line_no)
{
    fprintf(stderr, "runtime error at file:%s line:%d\n", file_name, line_no);
    abort();
}

const char* js_runtime_typeof(void *ptr)
{
    /* TODO */
    return "string";
}

static const js_prototype_t OBJECT_PROTOTYPE = {
#ifdef DEBUG
    "object", 
#endif
    JS_OBJECT, NULL, NULL, NULL, NULL
};

/**** Object ****/
js_object_t* js_runtime_Object_new(js_type type)
{
    js_object_t *self = NEW(js_object_t);
    SET_TYPE(self, type);
    SET_PROTOTYPE(self, &OBJECT_PROTOTYPE);
    return self;
}

js_object_t* js_runtime_Object_get(js_object_t *self, js_string_t *key)
{
    fprintf(stderr, "self=%p, key=%p\n", self, key);
    if (IS_STRING(key)) {
        if (strcmp(string_str(key), "length") == 0) {
            return (void*)(intptr_t)string_length(CAST(js_string_t*,self));
        }
    }
    return self;
}

void js_runtime_Object_set(js_object_t *self, js_string_t *key, js_object_t *value)
{
    fprintf(stderr, "self=%p, key=%p, value=%p\n", self, key, value);
}
js_object_t* js_runtime_Object_get_prototype(js_object_t *self)
{
    DBG("self=%p\n", self);
    if (IS_OBJECT(self)) {
        asm volatile("int3");
        return PROTOTYPE(self);
    }
    return NULL;
}

void js_runtime_Object_set_prototype(js_object_t *self, js_object_t *proto)
{
    DBG("self=%p, proto=%p", self, proto);
}
/**** Boolean ****/
/* String Boolean.toString
 * :method
 * # convert Boolean value to String
 */
js_string_t* js_runtime_Boolean_toString(js_bool_t self)
{
    js_rawstring_t *str;
    if (self == JS_TRUE) {
        str = "true";
    } else {
        str = "false";
    }
    return __new_string(str, strlen(str));
}

/* Boolean Boolean.valueOf
 * :method
 * # return value of Boolean
 */
js_bool_t js_runtime_Boolean_valueOf(js_bool_t self)
{
    return self;
}

/**** String ****/
/* String static_String.new(RawString str, Int len)
 * :method
 * # build string object from RawString
 */
js_string_t *js_runtime_static_new_String(js_rawstring_t *str, int len)
{
    return __new_string(str, len);
}

/* Int String.length
 * :propaty
 * # get length of string
 */
js_int_t js_runtime_String_length(js_string_t *self)
{
    return string_length(self);
}

/* String String.toString()
 * :method
 * # convert to string expression
 */
js_string_t* js_runtime_String_toString(js_string_t *self)
{
    return self;
}

/* String String.valueOf()
 * :method
 * # get base value of string
 */
js_string_t* js_runtime_String_valueOf(js_string_t *self)
{
    return self;
}

/* String String.concat(String arg1, String arg2)
 * :method
 * # concat values
 */
/* TODO va_list */
js_string_t* js_runtime_String_concat(js_string_t *self, js_string_t* arg1, js_string_t* arg2)
{
    int len0 = string_length(self);
    int len1 = string_length(arg1);
    int len2 = string_length(arg2);
    int len = len0 + len1 + len2 + 1;
    js_rawstring_t* ret = (js_rawstring_t*) malloc(len);
    memcpy(ret, self, len0);
    memcpy(ret + len0, arg1, len1);
    memcpy(ret + len1, arg2, len2);
    ret[len] = '\0';
    return __new_string_ref(ret, len);
}

/* String String.slice(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and end
 */
js_string_t* js_runtime_String_slice(js_string_t *self, js_int_t arg1, js_int_t arg2)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}

/* String String.substring(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and end
 */

js_string_t* js_runtime_String_substring(js_string_t *self, js_int_t arg1, js_int_t arg2)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}

/* String String.substr(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and length
 */
js_string_t* js_runtime_String_substr(js_string_t *self, js_int_t arg1, js_int_t arg2)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}
/* String String.charAt(Int arg1);
 * :method
 * # get string at designated position
 */
js_string_t *js_runtime_String_charAt(js_string_t *self, js_int_t arg1)
{
    /* TODO string type. not char*. */
    int len = string_length(self);
    char *ret = malloc(2);
    ret[0] = ret[1] = '\0';
    if (arg1 >= len) {
        return __new_string_ref(ret, 0);
    }
    ret[0] = self->__str[arg1];
    return __new_string_ref(ret, 1);
}

/* Int String.charCodeAt(Int arg1);
 * :method
 * # get integer at designated position
 */
js_int_t js_runtime_String_charCodeAt(js_string_t *self, js_int_t arg1)
{
    js_runtime_exception(__FILE__, __LINE__);
    return -1;
}
/* String String.fromCharCode(Int arg1);
 * :method
 * # create string from integer
 */
js_string_t* js_runtime_String_fromCharCode(js_string_t *self, js_int_t arg1)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}
/* String String.toLowerCase();
 * :method
 * # convert to string of LowerCase
 */
js_string_t* js_runtime_String_toLowerCase(js_string_t *self)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}
/* String String.UpperCase();
 * :method
 * # create string of UpperCase
 */
js_string_t* js_runtime_String_toUpperCase(js_string_t *self)
{
    js_runtime_exception(__FILE__, __LINE__);
    return NULL;
}
/* Int String.indexOf(arg1);
 * :method
 * # search designated string
 */
js_int_t js_runtime_String_indexOf(js_string_t *self, js_string_t* arg1)
{
    js_runtime_exception(__FILE__, __LINE__);
    return -1;
}
/* Int String.lastIndexOf(arg1);
 * :method
 * # search designated string
 */
js_int_t js_runtime_String_lastIndexOf(js_string_t *self, js_string_t* arg1)
{
    js_runtime_exception(__FILE__, __LINE__);
    return -1;
}

js_string_t* js_runtime_String_opPlus(js_string_t* self, js_string_t* arg1)
{
    int len0 = string_length(self);
    int len1 = string_length(arg1);
    int len = len0 + len1 - 1;
    js_rawstring_t* ret = (js_rawstring_t*) malloc(len);
    memcpy(ret, self->__str, len0);
    memcpy(ret + len0 - 1, arg1->__str, len1);
    return __new_string_ref(ret, len);
}

js_bool_t js_runtime_String_eq(js_string_t* self, js_string_t* arg1)
{
    int len0 = string_length(self);
    int len1 = string_length(arg1);
    return (len0 == len1 && string_cmp(self, arg1) == 0);
}

js_bool_t js_runtime_String_neq(js_string_t* self, js_string_t* arg1)
{
    int len0 = string_length(self);
    int len1 = string_length(arg1);
    return (len0 != len1 && string_cmp(self, arg1) != 0);
}

/**** Function ****/
static const js_prototype_t FUNCTION_PROTOTYPE = {
#ifdef DEBUG
    "function", 
#endif
    JS_FUNCTION, NULL, NULL, NULL, NULL
};


/* Function Function.new(RawPtr ptr)
 * :method
 * # build function object from RawPtr
 */
js_function_t *js_runtime_new_Function(void *ptr)
{
    js_function_t* ret = (js_function_t*) malloc(sizeof(*ret));
    SET_TYPE(ret, JS_FUNCTION);
    SET_PROTOTYPE(ret, &FUNCTION_PROTOTYPE);
    ret->func = ptr;
    return ret;
}
#define object_t js_object_t
typedef object_t* (*TyFunc0)(void);
typedef object_t* (*TyFunc1)(object_t *o1);
typedef object_t* (*TyFunc2)(object_t *o1, object_t *o2);
typedef object_t* (*TyFunc3)(object_t *o1, object_t *o2, object_t *o3);
typedef object_t* (*TyFunc4)(object_t *o1, object_t *o2, object_t *o3, object_t *o4);
typedef object_t* (*TyFunc5)(object_t *o1, object_t *o2, object_t *o3, object_t *o4, object_t *o5);

js_object_t *js_runtime_Function_call(js_function_t *func, js_Number_t *argc, ...)
{
    int i, nargs = JSInt(argc);
    va_list vl;
    void *fptr = func->func;
    fprintf(stderr, "nargs=%d\n", nargs);
    js_object_t *ret = NULL;
    object_t *ptr0, *ptr1, *ptr2, *ptr3, *ptr4;
    switch (nargs) {
        case 0: {
            ret = ((TyFunc0)fptr)();
            break;
        }
        case 1: {
            va_start(vl, argc);
            ptr0 = va_arg(vl, object_t*);
            va_end(vl);
            fprintf(stderr, "%p\n", ptr0);
            ret = ((TyFunc1)fptr)(ptr0);
            break;
        }
        case 2: {
            va_start(vl, argc);
            ptr0 = va_arg(vl, object_t*);
            ptr1 = va_arg(vl, object_t*);
            va_end(vl);
            fprintf(stderr, "%p %p\n", ptr0, ptr1);
            ret = ((TyFunc2)fptr)(ptr0, ptr1);
            break;
        }
        case 3: {
            va_start(vl, argc);
            ptr0 = va_arg(vl, object_t*);
            ptr1 = va_arg(vl, object_t*);
            ptr2 = va_arg(vl, object_t*);
            ret = ((TyFunc3)fptr)(ptr0, ptr1, ptr2);
            va_end(vl);
            break;
        }
        case 4: {
            va_start(vl, argc);
            ptr0 = va_arg(vl, object_t*);
            ptr1 = va_arg(vl, object_t*);
            ptr2 = va_arg(vl, object_t*);
            ptr3 = va_arg(vl, object_t*);
            va_end(vl);
            ret = ((TyFunc4)fptr)(ptr0, ptr1, ptr2, ptr3);
            break;
        }
        case 5: {
            va_start(vl, argc);
            ptr0 = va_arg(vl, object_t*);
            ptr1 = va_arg(vl, object_t*);
            ptr2 = va_arg(vl, object_t*);
            ptr3 = va_arg(vl, object_t*);
            ptr4 = va_arg(vl, object_t*);
            va_end(vl);
            ret = ((TyFunc5)fptr)(ptr0, ptr1, ptr2, ptr3, ptr4);
            break;
        }
    }
    return ret;
}
/* Math */

/* Float Math.abs();
 * :method
 */
js_float_t js_runtime_Math_abs(js_float_t x)
{
    return fabs(x);
}

/* Float Math.acos();
 * :method
 */
js_float_t js_runtime_Math_acos(js_float_t x)
{
    return acos(x);
}

/* Float Math.asin();
 * :method
 */
js_float_t js_runtime_Math_asin(js_float_t x)
{
    return asin(x);
}

/* Float Math.atan();
 * :method
 */
js_float_t js_runtime_Math_atan(js_float_t x)
{
    return atan(x);
}

/* Float Math.atan2();
 * :method
 */

js_float_t js_runtime_Math_atan2(js_float_t y, js_float_t x)
{
    return atan2(y, x);
}

/* Float Math.ceil();
 * :method
 */
js_float_t js_runtime_Math_ceil(js_float_t x)
{
    return ceil(x);
}

/* Float Math.cos();
 * :method
 */
js_float_t js_runtime_Math_cos(js_float_t x)
{
    return cos(x);
}

/* Float Math.exp();
 * :method
 */
js_float_t js_runtime_Math_exp(js_float_t x)
{
    return exp(x);
}

/* Float Math.floor();
 * :method
 */
js_float_t js_runtime_Math_floor(js_float_t x)
{
    return floor(x);
}

/* Float Math.log();
 * :method
 */
js_float_t js_runtime_Math_log(js_float_t x)
{
    return log(x);
}

/* Float Math.max();
 * :method
 */
js_float_t js_runtime_Math_max(js_float_t arg1, js_float_t arg2)
{
#define MAX(x,y) (x<y)?(y):(x)
    return MAX(arg1, arg2);
#undef MAX
}

/* Float Math.min();
 * :method
 */
js_float_t js_runtime_Math_min(js_float_t arg1, js_float_t arg2)
{
#define MIN(x,y) (x<y)?(x):(y)
    return MIN(arg1, arg2);
#undef MIN
}

/* Float Math.pow();
 * :method
 */
js_float_t js_runtime_Math_pow(js_float_t x, js_float_t y)
{
    return pow(x, y);
}

/* Float Math.random();
 * :method
 */
js_float_t js_runtime_Math_random(void)
{
    return rand();
}

/* Float Math.round();
 * :method
 */

js_float_t js_runtime_Math_round(js_float_t x)
{
    return round(x);
}

/* Float Math.sin();
 * :method
 */
js_float_t js_runtime_Math_sin(js_float_t x)
{
    return sin(x);
}

/* Float Math.sqrt();
 * :method
 */
js_float_t js_runtime_Math_sqrt(js_float_t x)
{
    return sqrt(x);
}

/* Float Math.tan();
 * :method
 */
js_float_t js_runtime_Math_tan(js_float_t x)
{
    return tan(x);
}

js_object_t *new_Int(js_int_t a)
{
    js_Number_t* ret = (js_Number_t*) malloc(sizeof(*ret));
    ret->h.type = JS_INT;
    ret->i = a;
    return (js_object_t *) ret;
}
js_object_t *new_Float(js_float_t a)
{
    js_Number_t* ret = (js_Number_t*) malloc(sizeof(*ret));
    ret->h.type = JS_FLOAT;
    ret->f = a;
    return (js_object_t *) ret;
}
js_object_t *RUNTIME(int_new)(js_int_t a)
{
    return new_Int(a);
}
js_object_t *RUNTIME(float_new)(js_float_t a)
{
    return new_Float(a);
}
js_object_t *RUNTIME(object_add) (js_object_t *a, js_object_t *b)
{
    if (IS_Int(a)) {
        if (IS_Int(b)) {
            return new_Int(JSInt(a) + JSInt(b));
        } else if (IS_Float(b)) {
            return new_Float((js_float_t)JSInt(a) + JSFloat(b));
        } else if (IS_String(b)) {
            return (js_object_t*)new_String_int_str(JSInt(a), JSString(b));
        } else {
            fprintf(stderr, "%s type error\n", __func__);
            return NULL;
        }
    } else {
    }
    return NULL;
}

js_object_t *RUNTIME(Int_opAdd) (js_object_t *a, js_object_t *b)
{
    if (IS_Int(a)) {
        if (IS_Int(b)) {
            return new_Int(JSInt(a) + JSInt(b));
        }
    } else {
        fprintf(stderr, "%s type error\n", __func__);
        return RUNTIME(object_add) (a, b);
    }
}

js_bool_t RUNTIME(Int_opLT) (js_object_t *a, js_object_t *b)
{
    if (IS_Int(a)) {
        if (IS_Int(b)) {
            return JSInt(a) > JSInt(b);
        }
    } else {
        fprintf(stderr, "%s type error\n", __func__);
        return RUNTIME(object_add) (a, b);
    }
}


