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

#undef main

/* test assert */
#define UT_ASSERT(f) {if (!(f)) {printf("%s:%u: '%s' is NG\n", __FILE__,__LINE__,#f);return false;}}

static void dump_vcode(struct vcode *vc)
{
    char *op;
    char op_str[3];
    int num = 1;

    for (; vc != NULL; vc = vc->next) {
        switch (vc->vop) {
          case VOP_LD_INT:
            printf("LD_INT r%d, %d\n", vc->reg, vc->val_int);
            break;
          case VOP_ASSIGN:
            printf("ASSIGN r%d, r%d\n", vc->reg, vc->regs[0]);
            break;
          case VOP_READ_VAR:
            printf("READ r%d, v%d\n", vc->reg, vc->var_id);
            break;
          case VOP_REF_VAR:
            printf("REF r%d, v%d\n", vc->reg, vc->var_id);
            break;
          case VOP_RETURN:
            printf("RETURN r%d\n", vc->reg);
            break;
          default:
            printf("op '%c' r%d, r%d, r%d\n",
                   vc->vop, vc->reg, vc->regs[0], vc->regs[1]);
            break;
        }
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
    dump_vcode(vc);

    return true;
}

bool test002(void)
{
    struct ptree *pt;
    struct vcode *vc;

    pt = ParseAll("test_expr.txt");
    UT_ASSERT(pt != NULL);
    vc = GenCode(pt);
    UT_ASSERT(vc != NULL);
    dump_vcode(vc);

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
