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
    char *p;

    printf("%s:%d:%d: contex error\n",
           pt->file_name, pt->line_no, pt->line_offset);
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

static struct vcode* gen_read_var(struct ptree *pt, int *reg)
{
        struct vcode* vc;

        vc = alloc_vcode();
        vc->vop = VOP_READ_VAR;
        vc->reg = (*reg)++;
        vc->var_id = pt->var_id;
        return vc;
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

static struct vcode* gen_expr(struct ptree *pt, int *reg)
{
    struct vcode *vc, *vc_l, *vc_r;

    switch (pt->type) {
      case PT_VAR:
        return gen_read_var(pt, reg);
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
      case PT_UNARY:
        vc = alloc_vcode();
        vc->reg = (*reg)++;
        vc->regs[0] = *reg;
        switch (pt->op) {
          case '+':
            vc->vop = VOP_UNARY_PLUS;
            break;
          case '-':
            vc->vop = VOP_UNARY_MINUS;
            break;
          case '!':
            vc->vop = VOP_UNARY_NOT;
            break;
          case TOKEN_INC:
            vc->vop = VOP_UNARY_INC;
            break;
          case TOKEN_DEC:
            vc->vop = VOP_UNARY_DEC;
            break;
          default:
            context_error(pt);
        }
        if (pt->op == TOKEN_INC || pt->op == TOKEN_DEC)
            vc_l = gen_lvalue(pt->subtree[0], reg);
        else
            vc_l = gen_expr(pt->subtree[0], reg);

        return concat_vcode(vc_l, vc, NULL);

      case PT_POSTFIXED:
        vc = alloc_vcode();
        vc->reg = (*reg)++;
        vc->regs[0] = *reg;
        switch (pt->op) {
          case TOKEN_INC:
            vc->vop = VOP_POSTFIXED_INC;
            break;
          case TOKEN_DEC:
            vc->vop = VOP_POSTFIXED_DEC;
            break;
          default:
            context_error(pt);
        }
        vc_l = gen_lvalue(pt->subtree[0], reg);

        return concat_vcode(vc_l, vc, NULL);

      default:
        context_error(pt);
    }
    return NULL;
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

static struct vcode* gen_return(struct ptree *pt)
{
    struct vcode *vc, *vc_v;
    int reg = 0;

    vc= alloc_vcode();
    vc->vop = VOP_RETURN;
    vc->reg = reg;
    vc_v = gen_expr(pt->subtree[0], &reg);

    return concat_vcode(vc_v, vc, NULL);
}

static struct vcode* gen_statement(struct ptree *pt)
{
    int reg = 0;
    struct vcode *vc = NULL;
    switch (pt->type) {
      case PT_ASSIGN:
        vc = gen_assign(pt);
        break;
      case PT_EXPR:
        vc = gen_expr(pt, &reg);
        break;
      case PT_RETURN:
        vc = gen_return(pt);
        break;
      default:
        context_error(pt);
    }
    return vc;
}

static struct vcode* gen_all(struct ptree *pt)
{
    struct vcode *vc, *vc_next;

    vc = gen_statement(pt);
    while (pt->subtree[2] != NULL) {
        pt = pt->subtree[2];
        vc_next = gen_statement(pt);
        vc = concat_vcode(vc, vc_next, NULL);
    }

    return vc;
}
struct vcode* GenCode(struct ptree* pt)
{
    return gen_all(pt);
}
