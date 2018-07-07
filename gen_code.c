/*
 * code generator
 */
#include "lang.h"

static struct vcode *alloc_vcode()
{
    struct vcode *vc = malloc(sizeof(struct vcode));
    memset(vc, 0, sizeof(*vc));
    return vc;
}

static struct vcode* concat_vcode(struct vcode* vc0, ...)
{
    va_list ap;
    struct vcode *vc_last, *vc_next;

    vc_last = vc0;
    va_start(ap, vc0);
    vc_next = va_arg(ap, struct vcode*);

    while (vc_next != NULL) {
        while (vc_last->next != NULL)
            vc_last = vc_last->next;
        vc_last->next = vc_next;
        vc_next = va_arg(ap, struct vcode*);
    }
    va_end(ap);
    return vc0;
}

static void context_error(struct ptree* pt)
{
    printf("context_error at %d\n", pt->type);
    exit(0);
}

static struct vcode* gen_int(struct ptree* pt, int* reg)
{
    struct vcode* vc;

    vc = alloc_vcode();
    vc->vop = VOP_LD_INT;
    vc->reg = (*reg)++;
    vc->val_int = pt->val_int;

    return vc;
}

static struct vcode* gen_expr(struct ptree *pt, int *reg)
{
    struct vcode *vc, *vc_l, *vc_r;

    switch (pt->type) {
      case PT_VAR:
        // TBD
        break;
      case PT_INT:
        return gen_int(pt, reg);
      case PT_EXPR:
        vc = alloc_vcode();
        vc->vop = pt->op;
        vc->reg = (*reg)++;
        vc->regs[0] = *reg;
        vc_r = gen_expr(pt->subtree[1], reg);
        vc->regs[1] = *reg;
        vc_l = gen_expr(pt->subtree[0], reg);
        return concat_vcode(vc_l, vc_r, vc, NULL);
      default:
        context_error(pt);
    }
    return NULL;
}

static struct vcode* gen_lvalue(struct ptree *pt, int *reg)
{
    struct vcode *vc = NULL;

    switch (pt->type) {
      case PT_VAR:
        vc = alloc_vcode();
        vc->vop = VOP_REF_VAR;
        vc->reg = (*reg)++;
        vc->var_id = pt->var_id;
        break;
      case PT_ARRAY:
        break;
      case PT_HASH:
        break;
      default:
        context_error(pt);
    }
    return vc;
}

static struct vcode* gen_assign(struct ptree *pt)
{
    struct vcode *vc_assign, *vc_l, *vc_r;
    int reg = 0;

    vc_assign = alloc_vcode();
    vc_assign->vop = VOP_ASSIGN;
    vc_assign->reg = reg;
    vc_l = gen_lvalue(pt->subtree[0], &reg);
    vc_assign->regs[0] = reg;
    vc_r = gen_expr(pt->subtree[1], &reg);

    return concat_vcode(vc_l, vc_r, vc_assign, NULL);
}

static struct vcode* gen_all(struct ptree *pt)
{
    int reg = 0;
    struct vcode *vc;
    switch (pt->type) {
      case PT_ASSIGN:
        vc = gen_assign(pt);
        break;
      case PT_EXPR:
        vc = gen_expr(pt, &reg);
        break;
      default:
        context_error(pt);
    }
    return vc;
}

struct vcode* GenCode(struct ptree* pt)
{
    return gen_all(pt);
}
