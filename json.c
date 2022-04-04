#include "json.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>


#define EXPECT(ctx, expected_char) do { assert(*ctx->json == (expected_char)); ctx->json++; } while (0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

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
    v->type = JSON_NULL;
    skip_whitespace(&c);
    int res;
    if ((res = parse_value(&c, v)) == PARSE_OK) {
        skip_whitespace(&c);
        if (*c.json != '\0') {
            v->type = JSON_NULL;
            res = PARSE_ROOT_NOT_SINGULAR;
        }
    }
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