#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

/* test function prototype */
typedef bool (*Test)(void);

#define main __original_main

/* test target source */
#include "lex.c"
#include "parse.c"

#undef main

/* test assert */
#define UT_ASSERT(f) {if (!(f)) {printf("%s:%u: '%s' is NG\n", __FILE__,__LINE__,#f);return false;}}

void dump_ptree(struct ptree *pt, int l)
{
    int i;

    if (pt == NULL)
        return;
    for (i = 0; i < l; i++)
        printf("  ");
    switch (pt->type) {
      case PT_VAR:
        printf("var id=%d\n", pt->var_id);
        break;
      case PT_INT:
        printf("value int=%d\n", pt->val_int);
        break;
      case PT_EXPR:
        printf("expr %c\n", pt->op);
        dump_ptree(pt->subtree[0], l+1);
        dump_ptree(pt->subtree[1], l+1);
        break;
      case PT_UNARY:
        printf("UNARY %c\n", pt->op);
        dump_ptree(pt->subtree[0], l+1);
        break;
      case PT_POSTFIXED:
        printf("POSTFIXED %d\n", pt->op);
        dump_ptree(pt->subtree[0], l+1);
        break;
      case PT_ASSIGN:
        printf("assign lvalue\n");
        dump_ptree(pt->subtree[0], l+1);
        dump_ptree(pt->subtree[1], l+1);
        break;
      case PT_RETURN:
        printf("return\n");
        dump_ptree(pt->subtree[0], l+1);
        break;
      default:
        break;
    }
    if (pt->subtree[2] != NULL)
        dump_ptree(pt->subtree[2], l);
}
/*
 * test codes
 */
bool test001(void)
{
    struct ptree *pt;

    pt = ParseAll("test.txt");
    UT_ASSERT(pt != NULL);

    dump_ptree(pt, 0);

    return true;
}

bool test002(void)
{
    struct ptree *pt;

    pt = ParseAll("test_expr.txt");
    UT_ASSERT(pt != NULL);

    dump_ptree(pt, 0);

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
