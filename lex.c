/*
 * lexical analizer
 */
#include "lang.h"

int StartLex(char * file_name, struct lex_data *lex)
{
    struct stat stbuf;
    char *buf;
    
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
            lex->token = TOKEN_ID;
            lex->token_index = 0; // TBD
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

