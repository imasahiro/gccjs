#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "js-buffer.h"

struct js_buffer {
    int  capacity;
    int  size;
    char *buf;
};

static void buf_expand(jsbuf_t *buf, int capacity)
{
  assert(capacity > 0);
  buf->buf = (char*) realloc(buf->buf, capacity);
  buf->capacity = capacity;
}

jsbuf_t *js_buf_new(void)
{
  jsbuf_t *buf;
  buf = (jsbuf_t*) malloc(sizeof(*buf));
  buf->buf = NULL;
  buf->capacity = buf->size = 0;
  buf_expand(buf, 128);
  return buf;
}

void js_buf_putc(jsbuf_t *buf, char c)
{
  assert(buf->size >= 0);
  if (buf->size == buf->capacity) {
      buf_expand(buf, buf->size*2);
  }
  buf->buf[buf->size] = c;
  buf->size++;
}

void js_buf_write(jsbuf_t *buf, char *str, int len)
{
  assert(buf->size >= 0);
  if (buf->size + len >= buf->capacity) {
      buf_expand(buf, buf->size + len);
  }
  memcpy(buf->buf + buf->size, str, len);
  buf->size += len;
}

char *js_buf_raw(jsbuf_t *buf)
{
  return buf->buf;
}

char *js_buf_tochar(jsbuf_t *buf, int *len)
{
  char *str = (char *) malloc(buf->size + 1);
  memcpy(str, buf->buf, buf->size);
  str[buf->size] = '\0';
  *len = buf->size;
  return str;
}

void js_buf_clear(jsbuf_t *buf)
{
  memset(buf->buf, 0, buf->size);
  buf->size = 0;
}

#define JS_ASSERT(msg, expr) {\
    if (!(expr)) {\
        fprintf(stderr, "[%s:%d:%s] '%s' (%s)\n", \
                __FILE__, __LINE__, __func__, msg, #expr);\
        asm volatile("int3");\
    }\
}

void js_buf_delete(jsbuf_t *buf)
{
  JS_ASSERT("buffer is deleted", buf->size >= 0);
  js_buf_clear(buf);
  free(buf->buf);
  buf->buf = NULL;
  buf->capacity = 0;
  buf->size = -1;
  free(buf);
}

int js_buf_size(jsbuf_t *buf)
{
  return (buf->size);
}

