/*
 * executor
 */
#include "lang.h"

static struct value reg[255];
static struct value var[255];

static void execute_error(struct vcode *vc)
{
    printf("execute_error\n");
    exit(0);
}

int Execute(struct vcode* vc)
{
    memset(reg, 0, sizeof(reg));
    memset(var, 0, sizeof(var));

    while (vc != NULL) {
        switch (vc->vop) {
          case VOP_READ_VAR:
            reg[vc->reg] = var[vc->regs[0]];
            break;
          case VOP_REF_VAR:
            reg[vc->reg].type = TYPE_REF;
            reg[vc->reg].ref = &var[vc->regs[0]];
            break;
          case VOP_LD_INT:
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int = vc->val_int;
            break;
          case VOP_ASSIGN:
            if (reg[vc->reg].type != TYPE_REF)
                execute_error(vc);
            *reg[vc->reg].ref = reg[vc->regs[0]];
            break;
          case VOP_RETURN:
            if (reg[vc->regs[0]].type != TYPE_INT)
                execute_error(vc);
            return reg[vc->reg].val_int;
          case '+':
            if (reg[vc->regs[0]].type != TYPE_INT &&
                reg[vc->regs[1]].type != TYPE_INT)
                execute_error(vc);
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int =
                reg[vc->regs[0]].val_int + reg[vc->regs[1]].val_int;
            break;
          case '*':
            if (reg[vc->regs[0]].type != TYPE_INT &&
                reg[vc->regs[1]].type != TYPE_INT)
                execute_error(vc);
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int =
                reg[vc->regs[0]].val_int * reg[vc->regs[1]].val_int;
            break;
          case VOP_UNARY_MINUS:
            if (reg[vc->regs[0]].type != TYPE_INT)
                execute_error(vc);
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int = - reg[vc->regs[0]].val_int;
            break;
          case VOP_UNARY_PLUS:
            if (reg[vc->regs[0]].type != TYPE_INT)
                execute_error(vc);
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int = reg[vc->regs[0]].val_int;
            break;
          case VOP_UNARY_NOT:
            if (reg[vc->regs[0]].type != TYPE_INT)
                execute_error(vc);
            reg[vc->reg].type = TYPE_INT;
            reg[vc->reg].val_int = !reg[vc->regs[0]].val_int;
            break;
          case VOP_UNARY_INC:
          case VOP_UNARY_DEC:
          case VOP_POSTFIXED_INC:
          case VOP_POSTFIXED_DEC:
            // TBD
          default:
            execute_error(vc);
        }
        vc = vc->next;
    }
    return 0;
}
