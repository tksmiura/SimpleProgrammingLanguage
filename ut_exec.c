#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

/* test function prototype */
typedef bool (*Test)(void);

#define main __original_main

/* test target source */
#include "lex.c"
#include "parse.c"
#include "gen_code.c"
#include "exec.c"

#undef main

/* test assert */
#define UT_ASSERT(f) {if (!(f)) {printf("%s:%u: '%s' is NG\n", __FILE__,__LINE__,#f);return false;}}


void print_var(char *s,struct value *v)
{
    switch (v->type) {
      case TYPE_INT:
        printf("%s = (INT) %d\n", s, v->val_int);
        break;
      case TYPE_REF:
        printf("%s = (REF) %p\n", s, v->ref);
        break;
      default:
        break;
    }
}

void dump_var(void)
{
    int i;
    char wk[10];

    for (i=0; i < 255; i++) {
        sprintf(wk, "reg[%d]", i);
        print_var(wk, &reg[i]);
     }
   for (i=0; i < 255; i++) {
        sprintf(wk, "var[%d]", i);
        print_var(wk, &var[i]);
     }
}

/*
 * test codes
 */
bool test001(void)
{
    struct ptree *pt;
    struct vcode *vc;

    pt = ParseAll("test.txt");
    UT_ASSERT(pt != NULL);
    vc = GenCode(pt);
    UT_ASSERT(vc != NULL);
    Execute(vc);

    dump_var();
    return true;
}

/*
 * test codes
 */
bool test002(void)
{
    struct ptree *pt;
    struct vcode *vc;
    int ret;

    pt = ParseAll("test_expr.txt");
    UT_ASSERT(pt != NULL);
    vc = GenCode(pt);
    UT_ASSERT(vc != NULL);
    ret = Execute(vc);
    UT_ASSERT(ret == - 1 * (2 + 6));

    dump_var();
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
