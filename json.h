#ifndef JSON_H__
#define JSON_H__
#include "def.h"
#include "string.h"

typedef struct {
    union {
        double num;
        struct {char* str; size_t len;};
    };
    json_type type;
} json_value;


int json_parse(json_value* v, const char* json);

json_type get_type(const json_value* v);



#define set_null(v) free_json(v)
#define json_init(v) do {(v)->type = JSON_NULL;} while(0)

void free_json(json_value* v);

int get_boolean(const json_value* v);
void set_boolean(json_value* v, int b);

double get_number(const json_value* v);
void set_number(json_value* v, double n);

const char* get_string(const json_value* v);
size_t get_string_length(const json_value* v);
void set_string(json_value* v, const char* s, size_t len);

#endif /*  JSON_H__ */