#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

/* test function prototype */
typedef bool (*Test)(void);

#define main __original_main

/* test target source */
#include "lex.c"

#undef main

/* test assert */
#define UT_ASSERT(f) {if (!(f)) {printf("%s:%u: '%s' is NG\n", __FILE__,__LINE__,#f);return false;}}

/*
 * test codes
 */
bool test001(void)
{
    int ret;
    struct lex_data lex_data;

    ret = StartLex("test.txt", &lex_data);

    UT_ASSERT(ret == 0);
    UT_ASSERT(lex_data.buf_size == 15);

    EndLex(&lex_data);

    return true;
}

bool test002(void)
{
    int ret;
    struct lex_data lex_data;
    static char * token[] = {
        "TOKEN_ID", "TOKEN_INT", "TOKEN_FLOAT", "TOKEN_STRING",
    };

    ret = StartLex("test.txt", &lex_data);

    UT_ASSERT(ret == 0);
    UT_ASSERT(lex_data.buf_size == 15);

    do {
        Lex(&lex_data);

        if (lex_data.token >= TOKEN_ID && lex_data.token < TOKEN_EOF)
            printf("%s", token[lex_data.token - TOKEN_ID]);
        else
            printf("%c", lex_data.token);

        printf("\n");
    } while (lex_data.token != TOKEN_EOF);

    EndLex(&lex_data);

    return true;
}

bool test003(void)
{
    int ret, index;
    struct lex_data lex_data;

    /* test for search_word */

    ret = StartLex("test.txt", &lex_data);

    ret = search_word("if", 2, &index);
    UT_ASSERT(ret == TOKEN_IF);

    index = -1;
    ret = search_word("abc", 3, &index);
    UT_ASSERT(ret == TOKEN_ID && index == 0);

    index = -1;
    ret = search_word("ab", 2, &index);
    UT_ASSERT(ret == TOKEN_ID && index == 1);

    index = -1;
    ret = search_word("abc", 3, &index);
    UT_ASSERT(ret == TOKEN_ID && index == 0);

    EndLex(&lex_data);

    return true;
}

bool test004(void)
{
    int ret, index;
    struct lex_data lex_data;

    /* test for search_word */

    ret = StartLex("test_sym.txt", &lex_data);

    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_NE);       /* != */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_GE);       /* >= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_LE);       /* <= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_INC);      /* ++ */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_DEC);      /* -- */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_RSHIFT);   /* >> */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_LSHIFT);   /* << */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_AND);      /* && */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_OR);       /* || */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_PE);       /* += */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_ME);       /* -= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_AE);       /* *= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_SE);       /* /= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_MODE);     /* %= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_ANDE);     /* &= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_XORE);     /* ^= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_ORE);      /* |= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_DOT2);     /* .. */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_DOT3);     /* ... */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_RSE);      /* >>= */
    Lex(&lex_data); UT_ASSERT(lex_data.token == TOKEN_LSE);      /* <<= */

    EndLex(&lex_data);

    return true;
}

/* add more test code ... */

int main(int argc, char *argv[])
{
    Test t;
    bool ret;
    int i;
    char func_name[100];
    unsigned int count_ok = 0, count = 0;

    for (i = 0; i < 100; i++) {
        sprintf(func_name, "test%03d", i);
        t = (Test) dlsym(RTLD_DEFAULT, func_name);
        if (t != NULL)  {
            count++;
            ret = (*t)();
            if (ret)
                count_ok++;
            else
                printf("test NG %s\n", func_name);
        }
    }
    printf("result %u/%u \n", count_ok, count);
}
