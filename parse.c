/*
 * parser
 */
#include "lang.h"

/**
   syntax

   factor     := var | num | '(' expr ')'
   list       := [expr [',' expr ]*] 
   Postfixed  := factor [ '++' | '--' | '.' | '(' list ')' |
                 '[' expr ']' | '{' expr'}' ]
   unary      := ['+' | '-' | '!' ]* factor
   term       := unary | unary ['*' '/' '%' ] unary 
   expr       := term  [ ('+' | '-') expr ]*
   compare    := expr [ ('==' | '!=' | ...) expr ] 
   l_and      := compare [ '&&' comapre ]*
   l_or       := l_and [ '||' l_and]  
   assign     := l_or ['=' l_or]
   for_stat   := <TBD>
   if_stat    := <TBD>
   statement  := (expr | for_stat | if_stat) ';' 
 **/

static struct ptree *alloc_tree() {
    return malloc(sizeof(struct ptree));
}

static void syntax_error(struct lex_data *lex)
{
    printf("%s:%d: syntax error\n", lex->file_name, lex->line_no);
    exit(0);
}

static struct ptree *parse_factor(struct lex_data *lex)
{
    struct ptree * pt;
    pt = alloc_tree();
    
    if (lex->token == TOKEN_ID) {
        pt->type = PT_VAR;
        pt->var_id = lex->token_index;
    } else if (lex->token == TOKEN_INT) {
        pt->type = PT_INT;
        pt->val_int = lex->val_int;
    } else 
        syntax_error(lex);
    
    Lex(lex);
    
    return pt;
}

static struct ptree *parse_term(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_factor(lex);
    if (lex->token == '*' || lex->token == '/') {
        pt = alloc_tree();
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_factor(lex);

        return pt;
    } else
        return pt_l;
}
    
static struct ptree *parse_expr(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_term(lex);
    if (lex->token == '+' || lex->token == '-') {
        pt = alloc_tree();
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_term(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_assign(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_expr(lex);
    if (lex->token == '=') {
        pt = alloc_tree();
        pt->type = PT_ASSIGN;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_expr(lex);
    } else
        pt = pt_l;
    
    if (lex->token != ';')
        syntax_error(lex);
    
    Lex(lex);
    
    return pt;
}

static struct ptree *parse_stat(struct lex_data *lex)
{
    struct ptree *pt;
    
    if (lex->token == TOKEN_IF) {
        // TBD
    } else if (lex->token == TOKEN_WHILE) {
        // TBD
    } else {
        pt = parse_assign(lex);
    }
    
    return pt;
}

struct ptree *ParseAll(char *file_name)
{
    struct ptree *pt = NULL;
    int ret;
    struct lex_data lex;

    ret = StartLex(file_name, &lex);
    if (ret < 0)
        return NULL;
    
    Lex(&lex);
    while (lex.token != TOKEN_EOF) {
        pt = parse_stat(&lex);
    }
    EndLex(&lex);

    return pt;
}

