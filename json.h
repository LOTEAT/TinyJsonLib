#ifndef JSON_H__
#define JSON_H__
#include "def.h"


typedef struct {
    double num;
    json_type type;
} json_value;


int json_parse(json_value* v, const char* json);

json_type get_type(const json_value* v);
double get_number(const json_value* v);


#endif /*  JSON_H__ */