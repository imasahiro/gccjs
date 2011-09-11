#ifndef JS_H
#define JS_H

/* copied from gcc/js/js.h */
typedef enum js_type {
    JS_OBJECT = 1,
    JS_BOOL,
    JS_INT,
    JS_FLOAT,
    JS_STRING,
    JS_ARRAY,
    JS_FUNCTION,
    JS_INTOBJ,
    JS_FLTOBJ,
    JS_UNDEF  = -1,
    JS_VOID   = -2,
    JS_VARGS  = -3,
    JS_RAWSTR = -4,
    JS_REF    = -5,
} js_type;



#define JS_T(T) js_##T##_t
typedef void js_void_t;
typedef char js_bool_t;
#define JS_TRUE  ((js_bool_t)1)
#define JS_FALSE ((js_bool_t)0)
typedef int js_int_t;
typedef char js_rawstring_t;
typedef struct js_object js_object_t;
typedef struct js_string js_string_t;
typedef struct js_function js_function_t;
typedef double js_float_t;

typedef struct js_prototype_t {
#ifdef DEBUG
    const char *name;
#endif
    const js_type type;
    void *ref0;
    void *ref1;
    void *ref2;
    void *ref3;
} js_prototype_t;

typedef struct js_header {
    int type;
    const js_prototype_t *prototype;
} js_header;

struct js_object {
    js_header h;
    void *unused;
};
#define SET_TYPE(o, T)      (o)->h.type = T
#define SET_PROTOTYPE(o, P) (o)->h.prototype = P
#define TYPE(o)      ((o)->h.type)
#define PROTOTYPE(o) ((o)->h.prototype)

#define CAST(T, V) ((T)(V))
#define NEW(T)  (CAST(T*, malloc(sizeof(T))))

#define IS_Int(o)     (TYPE(o) == JS_INT)
#define IS_Float(o)   (TYPE(o) == JS_FLOAT)
#define IS_OBJECT(o)   (TYPE(o) == JS_OBJECT)
#define IS_STRING(o)   (TYPE(o) == JS_STRING)
#define IS_FUNCTION(o) (TYPE(o) == JS_FUNCTION)

struct js_Number {
    js_header h;
    union {
        js_int_t i;
        js_float_t f;
    };
};

struct js_string {
    js_header h;
    int length;
    js_rawstring_t *__str;
};

#define string_length(s) ((s)->length)
#define string_str(s) ((s)->__str)
#define string_cmp(s1, s2) strncmp(string_str(s1), string_str(s2), string_length(s1))
typedef union js_value {
    void *ptr;
    int ival;
    double fval;
    js_rawstring_t *rawstr;
    js_string_t *string;
    js_object_t *o;
} js_value_t;

struct js_function {
    js_header h;
    int length;
    void *func;
};

//#include "array.h"
#define Array(T) js_Array_##T##_t

#define DEF_ARRAY_STRUCT(T) \
struct Array(T) {\
    js_header h;\
    js_type type;\
    int size;  \
    int capacity;  \
    js_##T##_t **list;\
}
#define DEF_ARRAY_T(T)              \
struct Array(T);                    \
typedef struct Array(T) Array(T);   \

DEF_ARRAY_T(void);
DEF_ARRAY_STRUCT(void);
typedef Array(void) js_Array_t;

#define array_op(o, op, idx) (assert(array_##op((js_Array_t*)o, idx)))
static inline int array_check_index(js_Array_t *a, int idx)
{
    return (idx < a->size);
}
static inline int array_check_type(js_Array_t *a, js_type type)
{
    return (a->type == type);
}

#define DEF_ARRAY_OP(T, JS_TYPE)\
static inline Array(T) *Array_new_##T (void) {             \
    Array(T) *a = CAST(Array(T) *, new_Array(4));    \
    a->type = JS_TYPE;                             \
    return a;                                       \
}\
static inline void Array_##T##_add(Array(T) *a, JS_T(T) *v) {\
    /*array_op(a, check_type, O(v)->h.classinfo);     */\
    if (a->size + 1 >= a->capacity) {\
        a->list = (JS_T(T)**)realloc(a->list, sizeof(JS_T(T)*) * a->capacity * 2);\
        a->capacity *= 2;\
    }\
    a->list[a->size++] = v;                         \
}\
static inline JS_T(T) *Array_##T##_get(Array(T) *a, int idx) {\
    array_op(a, check_index, idx);                  \
    return a->list[idx];                            \
}\
static inline void Array_##T##_set(Array(T) *a, int idx, JS_T(T) *v){ \
    array_op(a, check_index, idx);                  \
    /*array_op(a, check_type, O(v)->h.classinfo);     */\
    a->list[idx] = v;                               \
}\
static inline void Array_##T##_delete(Array(T) *a) {\
    free(a);                                        \
}

#define DEF_ARRAY_S_STRUCT(T) \
struct Array(T) {\
    js_hObject_t h;\
    enum object_type type;\
    int size;  \
    int capacity;  \
    js_##T##_t *list;\
}
#define DEF_ARRAY_S_T(T) DEF_ARRAY_T(T)

#define DEF_ARRAY_S_OP_NOEQ(T)\
static inline Array(T) *Array_new_##T (void) {      \
    Array(T) *a = CAST(Array(T) *, new_Array(4));    \
    return a;                                       \
}\
static inline void Array_##T##_add(Array(T) *a, JS_T(T) v) {\
    if (a->size + 1 >= a->capacity) {                \
        a->capacity *= 2;\
        a->list = (JS_T(T)*)realloc(a->list, sizeof(JS_T(T)) * a->capacity);\
    }\
    a->list[a->size++] = v;                         \
}\
static inline JS_T(T) Array_##T##_get(Array(T) *a, int idx) {\
    array_op(a, check_index, idx);                  \
    return a->list[idx];                            \
}\
static inline void Array_##T##_set(Array(T) *a, int idx, JS_T(T) v){ \
    array_op(a, check_index, idx);                  \
    a->list[idx] = v;                               \
}\
static inline void Array_##T##_delete(Array(T) *a) {\
    free(a);                                        \
}\

#define DEF_ARRAY_S_OP(T)\
DEF_ARRAY_S_OP_NOEQ(T);\
static inline int Array_##T##_eq(Array(T) *a1, Array(T) *a2){ \
    int i; JS_T(T) x;                      \
    if (Array_size(a1) == Array_size(a2)) return 0;\
    FOR_EACH_ARRAY(a1, x, i) {              \
        if (x != Array_n(a2, i)) return 0;  \
    }\
    return 1;\
}

#define Array_get(T, a, idx)    Array_##T##_get(a, idx)
#define Array_set(T, a, idx, v) Array_##T##_set(a, idx, v)
#define Array_add(T, a, v)      Array_##T##_add(a, v)
#define Array_delete(T, a)      Array_##T##_delete(a)
#define Array_new(T)            Array_new_##T ()
#define Array_eq(T, a1, a2)     Array_##T##_eq(a1, a2)
#define Array_n(a, n) (a->list[n])
#define Array_size(a) (a->size)
#define Array_last(a) (Array_n(a, Array_size(a) - 1))
js_Array_t *new_Array(int n);

#define FOR_EACH_ARRAY(a, x, i) \
    for(i=0, x = a->list[i]; i < (a)->size; x=a->list[(++i)])
#define FOR_EACH_ARRAY_INIT(a, x, i, _init_) \
    for(_init_, x = a->list[i]; i < (a)->size; x=a->list[(++i)])


/* runtime functions */
extern void js_runtime_print(int, ...);
extern const char* js_runtime_typeof(void *ptr);

/* Object */
js_object_t* js_runtime_Object_new(js_type type);
js_object_t* js_runtime_Object_get(js_object_t *self, js_string_t *key);
void js_runtime_Object_set(js_object_t *self, js_string_t *key, js_object_t *value);

js_object_t* js_runtime_Object_get_prototype(js_object_t *self);
void js_runtime_Object_set_prototype(js_object_t *self, js_object_t *proto);
/* Array */
js_Array_t *js_runtime_Array_new(int argc, ...);
/* Boolean */
/* String Boolean.toString
 * :method
 * # convert Boolean value to String
 */
js_string_t* js_runtime_Boolean_toString(js_bool_t self);

/* Boolean Boolean.valueOf
 * :method
 * # return value of Boolean
 */
js_bool_t js_runtime_Boolean_valueOf(js_bool_t self);

/* String */
/* Int String.length
 * :propaty
 * # get length of string
 */
extern js_int_t js_runtime_String_length(js_string_t *self);

/* String String.toString()
 * :method
 * # convert to string expression
 */
extern js_string_t* js_runtime_String_toString(js_string_t *self);

/* String String.valueOf()
 * :method
 * # get base value of string
 */
extern js_string_t* js_runtime_String_valueOf(js_string_t *self);

/* String String.concat(String arg1, String arg2)
 * :method
 * # concat values
 */
extern js_string_t* js_runtime_String_concat(js_string_t *self, js_string_t* arg1, js_string_t* arg2);

/* String String.slice(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and end
 */
extern js_string_t* js_runtime_String_slice(js_string_t *self, js_int_t arg1, js_int_t arg2);

/* String String.substring(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and end
 */

extern js_string_t* js_runtime_String_substring(js_string_t *self, js_int_t arg1, js_int_t arg2);

/* String String.substr(Int arg1, Int arg2);
 * :method
 * # get substring with position of start and length
 */
extern js_string_t* js_runtime_String_substr(js_string_t *self, js_int_t arg1, js_int_t arg2);

/* String String.charAt(Int arg1);
 * :method
 * # get string at designated position
 */
extern js_string_t* js_runtime_String_charAt(js_string_t *self, js_int_t arg1);

/* Int String.charCodeAt(Int arg1);
 * :method
 * # get integer at designated position
 */
extern js_int_t js_runtime_String_charCodeAt(js_string_t *self, js_int_t arg1);

/* String String.fromCharCode(Int arg1);
 * :method
 * # create string from integer
 */
extern js_string_t* js_runtime_String_fromCharCode(js_string_t *self, js_int_t arg1);

/* String String.toLowerCase();
 * :method
 * # convert to string of LowerCase
 */
extern js_string_t* js_runtime_String_toLowerCase(js_string_t *self);

/* String String.UpperCase();
 * :method
 * # create string of UpperCase
 */
extern js_string_t* js_runtime_String_toUpperCase(js_string_t *self);

/* Int String.indexOf(arg1);
 * :method
 * # search designated string
 */
extern js_int_t js_runtime_String_indexOf(js_string_t *self, js_string_t* arg1);

/* Int String.lastIndexOf(arg1);
 * :method
 * # search designated string
 */
extern js_int_t js_runtime_String_lastIndexOf(js_string_t *self, js_string_t* arg1);

extern js_string_t* js_runtime_String_opPlus(js_string_t* self, js_string_t* arg1);

extern js_bool_t js_runtime_String_eq(js_string_t* self, js_string_t* arg1);
extern js_bool_t js_runtime_String_neq(js_string_t* self, js_string_t* arg1);

/**** Function ****/
js_function_t *js_runtime_new_Function(void *ptr);

/* Math */
extern js_float_t js_runtime_Math_abs(js_float_t x);
extern js_float_t js_runtime_Math_acos(js_float_t x);
extern js_float_t js_runtime_Math_asin(js_float_t x);
extern js_float_t js_runtime_Math_atan(js_float_t x);
extern js_float_t js_runtime_Math_atan2(js_float_t y, js_float_t x);
extern js_float_t js_runtime_Math_ceil(js_float_t x);
extern js_float_t js_runtime_Math_cos(js_float_t x);
extern js_float_t js_runtime_Math_exp(js_float_t x);
extern js_float_t js_runtime_Math_floor(js_float_t x);
extern js_float_t js_runtime_Math_log(js_float_t x);
extern js_float_t js_runtime_Math_max(js_float_t arg1, js_float_t arg2);
extern js_float_t js_runtime_Math_min(js_float_t arg1, js_float_t arg2);
extern js_float_t js_runtime_Math_pow(js_float_t x, js_float_t y);
extern js_float_t js_runtime_Math_random(void);
extern js_float_t js_runtime_Math_round(js_float_t x);
extern js_float_t js_runtime_Math_sin(js_float_t x);
extern js_float_t js_runtime_Math_sqrt(js_float_t x);
extern js_float_t js_runtime_Math_tan(js_float_t x);
#endif /* end of include guard: JS_H */
