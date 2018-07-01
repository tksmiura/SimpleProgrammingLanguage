/*
 * lexical analizer
 */
#include "lang.h"

/**
   reserved word
   if else do while for foreach continue break return null
   2letter
   != >= <= ++ -- >> << && || += -= *= /= %= &= ^= |= 
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

void Lex(struct lex_data *lex)
{
    while (1) {
        char * p = lex->lex_point;
        if ((*lex->lex_point >= 'a' && *lex->lex_point <= 'z') ||
            (*lex->lex_point >= 'A' && *lex->lex_point <= 'Z')) {
            while ((*lex->lex_point >= 'a' && *lex->lex_point <= 'z') ||
                   (*lex->lex_point >= 'A' && *lex->lex_point <= 'Z') ||
                   (*lex->lex_point >= '0' && *lex->lex_point <= '9') ||
                   (*lex->lex_point == '_')) 
                lex->lex_point++;
            lex->token_str = p;
            lex->token_len = lex->lex_point - p;
            
            lex->token = search_word(lex->token_str, lex->token_len,
                                     &lex->token_index);
            return;
        } else if (*lex->lex_point >= '0' && *lex->lex_point <= '9') {
            while (*lex->lex_point >= '0' && *lex->lex_point <= '9')
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

