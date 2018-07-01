#ifndef __LANG_H__
#define __LANG_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
 * lexical analizer
 */
enum lex_taken {
    TOKEN_ID = 256,  /* identifier */
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_IF,
    TOKEN_FOR,
    TOKEN_FOREACH,
    TOKEN_WHILE,
    TOKEN_EOF,
};

struct lex_data {
    int fd;
    char * file_name;
    char * lex_point;
    char * buf;
    size_t buf_size;
    int line_no;
    int token;
    int token_index;
    int val_int;
    double val_real;
    char *token_str;
    size_t token_len;
};

extern int StartLex(char * file_name, struct lex_data *lex);
extern void Lex(struct lex_data *lex);
extern void EndLex(struct lex_data *lex);

/*
 * parser
 */

struct ptree {
    enum {
        PT_VAR,
        PT_ARRAY,              /* var[ ] */
        PT_HASH,               /* var{ } */
        PT_STRUCT,             /* var.X  */
        PT_INT,
        PT_EXPR,
        PT_ASSIGN,
        PT_IF,
        PT_WHILE,
        PT_FOR,
        PT_FOREACH,
    } type;
    int op;
    union {
        struct ptree * subtree[3];
        int var_id;
        int val_int;
    };
};
struct ptree *ParseAll(char *file_name);

/*
 * virtual machine code generator
 */
struct vcode {
    enum {
        VOP_LD_INT = 0x10000,
        VOP_ASSIGN,
        VOP_REF_VAR,
        VOP_REF_ARRAY,
        VOP_REF_HASH,
        VOP_REF_STRUCT,
    } vop;
    int reg;
    union {
        int regs[3];
        int val_int;
        int var_id;
        struct vcode *sub_vcode;
    };
    struct vcode *next;
};
struct vcode* GenCode(struct ptree* pt);


/*
 * executor
 */
struct value {
    enum {
        TYPE_NULL,
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING,
        TYPE_ARRAY,
        TYPE_HASH,
        TYPE_STRUCT,
        TYPE_REF,
    } type;
    int ref_count;
    union {
        int val_int;
        double val_float;
        struct value *ref;
    };
};

int Execute(struct vcode* vc);
#endif /* __LANG_H__ */
