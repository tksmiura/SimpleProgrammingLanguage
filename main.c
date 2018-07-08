#include "lang.h"

int main(int argc, char *argv[])
{
    struct ptree *pt;
    struct vcode *vc;

    if (argc < 2) {
        printf("usage: %s script \n", argv[0]);
        return 1;
    }

    pt = ParseAll(argv[1]);
    vc = GenCode(pt);
    Execute(vc);

    return 0;
}
