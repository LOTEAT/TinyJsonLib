#include "json.h"


static int test_count = 0;
static int test_pass = 0;
static int main_ret = 1;

#define TEST_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define TEST_ERROR(error_type, json) \
    do {                             \
        json_value v;                \
        v.type = JSON_FALSE;         \
        TEST_EQ_INT(error_type, json_parse(&v, json)); \
        TEST_EQ_INT(JSON_NULL, get_type(&v));          \
    } while(0)



#define TEST_EQ_INT(expect, actual) TEST_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define TEST_TRUE(actual) TEST_EQ_BASE((actual) != 0, "true", "false", "%s")
#define TEST_FALSE(actual) TEST_EQ_BASE((actual) == 0, "false", "true", "%s")
#define TEST_EQ_DOUBLE(expect, actual) TEST_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define TEST_EQ_STRING(expect, actual, a_length) \
    TEST_EQ_BASE(sizeof(expect) - 1 == a_length && memcmp(expect, actual, a_length) == 0, expect, actual, "%s")


#define TEST_NUMBER(expect, json) \
    do {                          \
        json_value v;             \
        TEST_EQ_INT(PARSE_OK, json_parse(&v, json)); \
        TEST_EQ_INT(JSON_NUMBER, get_type(&v));      \
        TEST_EQ_DOUBLE(expect, get_number(&v));      \
    } while(0)

#define TEST_STRING(expect, json)\
    do {\
        json_value v;\
        json_init(&v);\
        TEST_EQ_INT(PARSE_OK, json_parse(&v, json));\
        TEST_EQ_INT(JSON_STRING, get_type(&v));\
        TEST_EQ_STRING(expect, get_string(&v), get_string_length(&v));\
        free_json(&v);\
    } while(0)


static void test_parse_null() {
    json_value v;
    v.type = JSON_FALSE;
    TEST_EQ_INT(PARSE_OK, json_parse(&v, "null"));
    TEST_EQ_INT(JSON_NULL, get_type(&v));
}

static void test_parse_true(){
    json_value v;
    v.type = JSON_FALSE;
    TEST_EQ_INT(PARSE_OK, json_parse(&v, "true"));
    TEST_EQ_INT(JSON_TRUE, get_type(&v));
}

static void test_parse_false(){
    json_value v;
    v.type = JSON_TRUE;
    TEST_EQ_INT(PARSE_OK, json_parse(&v, "false"));
    TEST_EQ_INT(JSON_FALSE, get_type(&v));
}


static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    // under flow
    TEST_NUMBER(0.0, "1e-10000");

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");

    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");

}

static void test_parse_root_not_singular() {
    json_value v;
    v.type = JSON_FALSE;
    TEST_EQ_INT(PARSE_ROOT_NOT_SINGULAR, json_parse(&v, "null x"));
    TEST_EQ_INT(JSON_NULL, get_type(&v));

    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");

}


static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_access_null() {
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    set_null(&v);
    TEST_EQ_INT(JSON_NULL, get_type(&v));
    free_json(&v);
}

static void test_access_boolean() {
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    set_boolean(&v, 1);
    TEST_TRUE(get_boolean(&v));
    set_boolean(&v, 0);
    TEST_FALSE(get_boolean(&v));
    free_json(&v);
}

static void test_access_number() {
    json_value v;
    json_init(&v);
    set_string(&v, "a", 1);
    set_number(&v, 1234.5);
    TEST_EQ_DOUBLE(1234.5, get_number(&v));
    free_json(&v);
}

static void test_access_string() {
    json_value v;
    json_init(&v);
    set_string(&v, "", 0);
    TEST_EQ_STRING("", get_string(&v), get_string_length(&v));
    set_string(&v, "Hello", 5);
    TEST_EQ_STRING("Hello", get_string(&v), get_string_length(&v));
    free_json(&v);
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_access_null();
    test_access_boolean();
    test_access_number();
    //test_access_string();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
