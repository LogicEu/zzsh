#include <zzsh.h>
#include <string.h>

int zzgetargs(char* str, char** args, const char* div)
{
    int i = 0;
    str = strtok(str, div);
    while (str) {
        args[i++] = str;
        str = strtok(NULL, div);
    }
    args[i] = NULL;
    return i;
}

int zzdirargs(const struct tnode_t* node, char** args)
{
    int i;
    for (i = 0; node->children[i]; ++i) {
        args[i] = *(char**)node->children[i]->data;
    }
    args[i] = NULL;
    return i;
}

void zzsortargs(char** args, const int count)
{
    int i, j, k;
    for (i = 0; i < count - 1; ++i) {
        for (j = i; j < count - i - 1; ++j) {
            for (k = 0; args[j][k] && args[j + 1][k]; ++k) {
                if (args[j][k] > args[j + 1][k]) {
                    char* tmp = args[j];
                    args[j] = args[j + 1];
                    args[j + 1] = tmp;
                    break;
                }
            }
        }
    }
}

char* zzstruntok(char* tok, const int count, const char div)
{
    int i, j;
    char* ret = tok;
    for (i = 1; i < count; ++i) {
        for (j = 0; tok[j]; ++j);
        while (!tok[j]) {
            tok[j++] = div;
        }
        tok = tok + j;
    }
    return ret;
}
