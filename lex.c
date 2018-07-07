/*
 * lexical analizer
 */
#include "lang.h"

/**
   reserved word
   if else do while for foreach continue break return null
   2letter
   != >= <= ++ -- >> << && || += -= *= /= %= &= ^= |= ..
   3letter
   ... <<= >>=

 **/

static struct {
    int token;
    char * word;
    size_t len;
} reserved[] = {
    {TOKEN_BREAK,    "break",     5},
    {TOKEN_CONTINUE, "continue",  8},
    {TOKEN_DO,       "do",        2},
    {TOKEN_ELSE,     "else",      4},
    {TOKEN_FOR,      "for",       3},
    {TOKEN_FOREACH,  "foreach",   7},
    {TOKEN_FALSE,    "false",     5},
    {TOKEN_IF,       "if",        2},
    {TOKEN_NULL,     "null",      4},
    {TOKEN_RETURN,   "return",    6},
    {TOKEN_TRUE,     "true",      4},
    {TOKEN_WHILE,    "while",     5},
    // {TOKEN_, ""},
};

static struct {
    char *word;
    size_t len;
} id_list[256];

static int search_word(char *word, size_t len, int *index)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(reserved); i++) {
        if (len == reserved[i].len &&
            strncmp(reserved[i].word, word, len) == 0)
            return reserved[i].token;
    }
    for (i = 0; i < 256; i++) {
        if (id_list[i].len == 0) {
            id_list[i].word = word;
            id_list[i].len = len;
            break;
        } else if(id_list[i].len == len &&
                  strncmp(id_list[i].word, word, len) == 0) {
            break;
        }
    }
    if (i < 256)
        *index = i;
    return TOKEN_ID;
}

int StartLex(char * file_name, struct lex_data *lex)
{
    struct stat stbuf;
    char *buf;

    memset(id_list, 0, sizeof(id_list));

    lex->fd = open(file_name, O_RDONLY);
    if (lex->fd == -1) {
        perror(file_name);
        return -EINVAL;
    }
    if (fstat(lex->fd, &stbuf) == -1) {
        perror(file_name);
        return -EINVAL;
    }
    buf = mmap(NULL, stbuf.st_size, PROT_READ, MAP_SHARED, lex->fd, 0);
    if (buf == MAP_FAILED) {
        perror(file_name);
        return -EPERM;
    }
    lex->buf = buf;
    lex->buf_size = stbuf.st_size;
    lex->lex_point = buf;
    lex->token = 0;
    lex->line_no = 1;
    lex->file_name = file_name;

    return 0;
}

void EndLex(struct lex_data *lex)
{
    // todo unmap
    close(lex->fd);
}

static int is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_alpha_num(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        c == '_' || (c >= '0' && c <= '9');
}

static int is_num(char c)
{
    return c >= '0' && c <= '9';
}


void Lex(struct lex_data *lex)
{
    while (1) {
        char * p = lex->lex_point;
        if (is_alpha(*lex->lex_point)) {
            while (is_alpha_num(*lex->lex_point))
                lex->lex_point++;
            lex->token_str = p;
            lex->token_len = lex->lex_point - p;

            lex->token = search_word(lex->token_str, lex->token_len,
                                     &lex->token_index);
            return;
        } else if (is_num(*lex->lex_point)) {
            while (is_num(*lex->lex_point))
                lex->lex_point++;
            sscanf(p,"%d",&lex->val_int);
            lex->token = TOKEN_INT;
            return;
        } else {
            switch (*lex->lex_point) {
              case '\n':
                lex->line_no ++;
              case ' ':         /* 空文字 */
              case '\t':
                lex->lex_point++;
                continue;
              case '#':         /* コメント */
                while (*lex->lex_point != '\n' && *lex->lex_point != 0) {
                    lex->lex_point++;
                }
                continue;
#define CASE_2CHAR_SYM(a,b,sym)                         \
              case (a):                                 \
                if (*(lex->lex_point + 1) == (b)) {     \
                    lex->token = (sym);                 \
                    lex->lex_point += 2;                \
                } else {                                \
                    lex->token = *lex->lex_point++;     \
                }                                       \
                return;
              CASE_2CHAR_SYM('!', '=', TOKEN_NE)
              CASE_2CHAR_SYM('*', '=', TOKEN_AE)
              CASE_2CHAR_SYM('/', '=', TOKEN_SE)
              CASE_2CHAR_SYM('%', '=', TOKEN_MODE)
              CASE_2CHAR_SYM('^', '=', TOKEN_XORE)
#define CASE_2OR3CHAR_SYM(a,b,sym,c,sym2,d,sym3)                \
              case (a):                                         \
                 if (*(lex->lex_point + 1) == (b)) {            \
                      lex->token = (sym);                       \
                      lex->lex_point += 2;                      \
                  } else if (*(lex->lex_point + 1) == (c)) {    \
                      if (*(lex->lex_point + 2) == (d)) {       \
                          lex->token = (sym3);                  \
                          lex->lex_point += 3;                  \
                    } else {                                    \
                        lex->token = (sym2);                    \
                        lex->lex_point += 2;                    \
                    }                                           \
                } else {                                        \
                    lex->token = *lex->lex_point++;             \
                }                                               \
                return;
              CASE_2OR3CHAR_SYM('>','=',TOKEN_GE,'>', TOKEN_RSHIFT,
                                '=', TOKEN_RSE)
              CASE_2OR3CHAR_SYM('<','=',TOKEN_LE,'<', TOKEN_LSHIFT,
                                '=', TOKEN_LSE)
#define CASE_2CHAR_SYM2(a, b, sym1, c, sym2)                    \
              case (a):                                         \
                if (*(lex->lex_point + 1) == (b)) {             \
                    lex->token = (sym1);                        \
                    lex->lex_point += 2;                        \
                } else if (*(lex->lex_point + 1) == (c)) {      \
                    lex->token = (sym2);                        \
                    lex->lex_point += 2;                        \
                } else {                                        \
                    lex->token = *lex->lex_point++;             \
        }                                                       \
                return;
              CASE_2CHAR_SYM2('+', '=', TOKEN_PE, '+', TOKEN_INC)
              CASE_2CHAR_SYM2('-', '=', TOKEN_ME, '-', TOKEN_DEC)
              CASE_2CHAR_SYM2('&', '=', TOKEN_ANDE, '&', TOKEN_AND)
              CASE_2CHAR_SYM2('|', '=', TOKEN_ORE, '|', TOKEN_OR)
              case '.':
                if (*(lex->lex_point + 1) == '.') {
                      if (*(lex->lex_point + 2) == '.') {
                          lex->token = TOKEN_DOT3;
                          lex->lex_point += 3;
                    } else {
                        lex->token = TOKEN_DOT2;
                        lex->lex_point += 2;
                    }
                } else {
                    lex->token = *lex->lex_point++;
                }
                return;


              case 0:
                lex->token = TOKEN_EOF;
                return;
              default:
                lex->token = *lex->lex_point++;
                return;
            }
        }
    }  /* 無限ループ */
}
