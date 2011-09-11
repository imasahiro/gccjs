#ifndef JS_BUFFER_H
#define JS_BUFFER_H

struct js_buffer;
typedef struct js_buffer jsbuf_t;

extern jsbuf_t *js_buf_new(void);
extern void  js_buf_putc (jsbuf_t *buf, char c);
extern void  js_buf_write(jsbuf_t *buf, char *str, int len);
extern void  js_buf_delete(jsbuf_t *buf);
extern char *js_buf_tochar(jsbuf_t *buf, int *len);
extern int js_buf_size(jsbuf_t *buf);
extern void js_buf_clear(jsbuf_t *buf);

#endif /* end of include guard: JS_BUFFER_H */

