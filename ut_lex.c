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
        "TOKEN_ID", "TOKEN_INT", "TOKEN_FLOAT", "TOKEN_STRING", "TOKEN_EOF",
    };

    ret = StartLex("test.txt", &lex_data);
    
    UT_ASSERT(ret == 0);
    UT_ASSERT(lex_data.buf_size == 15);

    do {
        Lex(&lex_data);
        
        if (lex_data.token >= TOKEN_ID && lex_data.token <= TOKEN_EOF)
            printf("%s", token[lex_data.token - TOKEN_ID]);
        else 
            printf("%c", lex_data.token);
                  
        printf("\n");
    } while (lex_data.token != TOKEN_EOF);

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
