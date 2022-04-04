#include "json.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>



#define EXPECT(ctx, expected_char) do { assert(*ctx->json == (expected_char)); ctx->json++; } while (0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do { *(char*)json_context_push(c, sizeof(char)) = (ch); }while(0)


static void skip_whitespace(json_context* c){
    const char *p = c->json;
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}

static int parse_literal(json_context* c, json_value* v, const char* literal, json_type type){
    EXPECT(c, literal[0]);
    size_t s = 0;
    for(; literal[s + 1]; s++){
        if(c->json[s] != literal[s + 1]){
            return PARSE_INVALID_VALUE;
        }
    }
    c->json += s;
    v->type = type;
    return PARSE_OK;
}


static int parse_number(json_context* c, json_value* v){
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->num = strtod(c->json, NULL);
    if (errno == ERANGE && (v->num == HUGE_VAL || v->num == -HUGE_VAL))
        return PARSE_NUMBER_TOO_BIG;
    v->type = JSON_NUMBER;
    c->json = p;
    return PARSE_OK;
}


static void* json_context_push(json_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0)
            c->size = PARSE_STACK_INIT_SIZE;
        while (c->top + size >= c->size)
            c->size += c->size >> 1;  /* c->size * 1.5 */
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* json_context_pop(json_context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static int parse_string(json_context* c, json_value* v){
    size_t head = c->top, len;
    const char* p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = c->top - head;
                set_string(v, (const char*)json_context_pop(c, len), len);
                c->json = p;
                return PARSE_OK;
            case '\0':
                c->top = head;
                return PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c, ch);
        }
    }
}


static int parse_value(json_context *c, json_value *v){
    switch(*c->json){
        case 'n': return parse_literal(c, v, "null", JSON_NULL);
        case 't': return parse_literal(c, v, "true", JSON_TRUE);
        case 'f': return parse_literal(c, v, "false", JSON_FALSE);
        default:  return parse_number(c, v);
        case '\0': return PARSE_EXPECT_VALUE;
    }
}


int json_parse(json_value* v, const char* json){
    json_context c;
    assert(v != NULL);
    c.json = json;
    c.size = c.top = 0;
    c.stack = NULL;
    json_init(v);
    skip_whitespace(&c);
    int res;
    if ((res = parse_value(&c, v)) == PARSE_OK) {
        skip_whitespace(&c);
        if (*c.json != '\0') {
            v->type = JSON_NULL;
            res = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return res;
}

json_type get_type(const json_value* v){
    assert(v != NULL);
    return v->type;
}

double get_number(const json_value* v){
    assert(v != NULL && v->type == JSON_NUMBER);
    return v->num;
}


void set_string(json_value* v, const char* s, size_t len){
    assert(v != NULL && (s != NULL || len == 0));
    v->str = malloc(len + 1);
    v->str[len] = '\0';
    v->len = len;
    v->type = JSON_STRING;
}

void free_json(json_value* v){
    assert(v != NULL);
    if(v->type == JSON_STRING){
        free(v->str);
    }
    v->type = JSON_NULL;
}


int get_boolean(const json_value* v) {
    assert(v != NULL && (v->type == JSON_FALSE || v->type == JSON_TRUE));
    return v->type == JSON_TRUE;
}

void set_boolean(json_value* v, int b) {
    assert(v != NULL);
    v->type = b ? JSON_TRUE : JSON_FALSE;
}


void set_number(json_value* v, double n) {
    assert(v != NULL);
    v->num = n;
    v->type = JSON_NUMBER;
}

size_t get_string_length(const json_value* v) {
    assert(v != NULL && v->type == JSON_STRING);
    return v->len;
}
