/*
 * parser
 */
#include "lang.h"

/**
   syntax

   list       := expr [',' expr ]*
   factor     := var | num | '(' expr ')' | '[' list ']'
   Postfixed  := factor [ '++' | '--' | '.' id | '(' list ')' |
                 '[' expr ']' | '{' expr'}' ]*
   unary      := ['+' | '-' | '!' | '++' | '--' ] unary | Postfixed
   term       := unary | unary ['*' '/' '%' ] term
   expr       := term  [ ('+' | '-') expr ]*
   compare    := expr [ ('==' | '!=' | '>=' | '<=' | '>' | '<') expr ]
   l_and      := compare [ '&&' comapre ]*
   l_or       := l_and [ '||' l_and]
   assign     := l_or ['=' l_or]
   for_stat   := <TBD>
   if_stat    := <TBD>
   return_stat := 'return' expr
   statement  := (expr | for_stat | if_stat | return_stat) ';'
   statements  := statement+

 **/

static struct ptree *alloc_tree(struct lex_data *lex) {
    struct ptree * pt;
    pt = calloc(1, sizeof(struct ptree));
    pt->file_name = lex->file_name;
    pt->line_no = lex->line_no;
    pt->line_offset = lex->lex_point - lex->line_start;
    return pt;
}

static void syntax_error(struct lex_data *lex)
{
    char *p;

    printf("%s:%d: syntax error\n", lex->file_name, lex->line_no);
    for (p = lex->line_start; *p != '\n' && *p != 0; p++)
        printf("%c", *p);
    printf("\n");
    for (p = lex->line_start; p < lex->lex_point; p++)
        printf(" ");
    printf("^\n");
    exit(0);
}

static struct ptree *parse_expr_all(struct lex_data *lex);

static struct ptree *parse_list(struct lex_data *lex)
{
    struct ptree * pt = NULL;

    // TBD
    syntax_error(lex);

    Lex(lex);

    return pt;

}

static struct ptree *parse_factor(struct lex_data *lex)
{
    struct ptree * pt = NULL;

    if (lex->token == TOKEN_ID) {
        pt = alloc_tree(lex);
        pt->type = PT_VAR;
        pt->var_id = lex->token_index;
    } else if (lex->token == TOKEN_INT) {
        pt = alloc_tree(lex);
        pt->type = PT_INT;
        pt->val_int = lex->val_int;
    } else if (lex->token == '(') {
        Lex(lex);
        pt = parse_expr_all(lex);
        if (lex->token != ')')
            syntax_error(lex);
    } else if (lex->token == '[') {
        Lex(lex);
        pt = parse_list(lex);
        if (lex->token != ']')
            syntax_error(lex);
    } else
        syntax_error(lex);

    Lex(lex);

    return pt;
}

static struct ptree *parse_postfixed(struct lex_data *lex)
{
    struct ptree *pt, *pt_r;

    pt = parse_factor(lex);
    while (1) {
        switch (lex->token) {
          case TOKEN_INC:
          case TOKEN_DEC:
            pt_r = alloc_tree(lex);
            pt_r->type = PT_POSTFIXED;
            pt_r->op = lex->token;
            pt_r->subtree[0] = pt;
            Lex(lex);
            break;
          case '.':
            // TBD
          case '(':
            // TBD
          case '[':
            // TBD
          case '{':
            // TBD
            syntax_error(lex);
          default:
            goto exit_postfixed;
        }
    }
  exit_postfixed:

    return pt;
}

static struct ptree *parse_unary(struct lex_data *lex)
{
    struct ptree *pt;

    switch (lex->token) {
      case '+':
      case '-':
      case '!':
      case TOKEN_INC:
      case TOKEN_DEC:
        pt = alloc_tree(lex);
        pt->type = PT_UNARY;
        pt->op = lex->token;
        Lex(lex);
        pt->subtree[0] = parse_unary(lex);
        break;
      default:
        pt = parse_postfixed(lex);
        break;
    }

    return pt;
}

static struct ptree *parse_term(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_unary(lex);
    if (lex->token == '*' || lex->token == '/') {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_term(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_expr(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_term(lex);
    if (lex->token == '+' || lex->token == '-') {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_expr(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_shift(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_expr(lex);
    if (lex->token == TOKEN_RSHIFT || lex->token == TOKEN_LSHIFT) {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_shift(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_compare(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_shift(lex);
    if (lex->token == TOKEN_EQ || lex->token == TOKEN_NE ||
        lex->token == TOKEN_GE || lex->token == TOKEN_LE ||
        lex->token == '>' || lex->token == '<') {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_expr(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_land(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_compare(lex);
    if (lex->token == TOKEN_AND) {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_land(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_lor(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_land(lex);
    if (lex->token == TOKEN_OR) {
        pt = alloc_tree(lex);
        pt->type = PT_EXPR;
        pt->op = lex->token;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_lor(lex);

        return pt;
    } else
        return pt_l;
}

static struct ptree *parse_expr_all(struct lex_data *lex)
{
    return parse_lor(lex);
}

static struct ptree *parse_assign(struct lex_data *lex)
{
    struct ptree *pt, *pt_l;

    pt_l = parse_expr_all(lex);
    if (lex->token == '=' || lex->token == TOKEN_PE ||
        lex->token == TOKEN_ME || lex->token == TOKEN_AE ||
        lex->token == TOKEN_SE || lex->token == TOKEN_MODE ||
        lex->token == TOKEN_ANDE || lex->token == TOKEN_XORE ||
        lex->token == TOKEN_ORE || lex->token == TOKEN_RSE ||
        lex->token == TOKEN_LSE) {
        pt = alloc_tree(lex);
        pt->type = PT_ASSIGN;
        pt->subtree[0] = pt_l;
        Lex(lex);
        pt->subtree[1] = parse_expr_all(lex);
    } else
        pt = pt_l;

    return pt;
}

static struct ptree *parse_statement(struct lex_data *lex)
{
    struct ptree *pt = NULL;

    if (lex->token == TOKEN_IF) {
        // TBD
    } else if (lex->token == TOKEN_WHILE) {
        // TBD
    } else if (lex->token == TOKEN_RETURN) {
        pt = alloc_tree(lex);
        pt->type = PT_RETURN;
        Lex(lex);
        pt->subtree[0] = parse_expr_all(lex);
    } else {
        pt = parse_assign(lex);
    }

    if (lex->token != ';')
        syntax_error(lex);

    Lex(lex);

    return pt;
}

static struct ptree *parse_statements(struct lex_data *lex)
{
    struct ptree *pt = NULL, *pt_start = NULL;


    pt = parse_statement(lex);
    pt_start = pt;
    while (lex->token != TOKEN_EOF && lex->token != '}') {
        pt->subtree[2] = parse_statement(lex);
        pt = pt->subtree[2];
    }
    return pt_start;
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
        pt = parse_statements(&lex);
    }
    EndLex(&lex);

    return pt;
}
