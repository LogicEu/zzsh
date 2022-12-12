#include <zzsh.h>
#include <stdlib.h>
#include <string.h>

static struct tnode_t* syscurr = NULL;

static struct tnode_t* zzsys_stdroot(void)
{
    char* dirname;
    struct tnode_t* root;
    
    dirname = strdup("~");
    root = tnode_create(&dirname, sizeof(char*));

    dirname = strdup(".");
    tnode_push(root, tnode_create(&dirname, sizeof(char*)));
    
    return root;
}

static void zzsys_free(struct tnode_t* node)
{
    size_t i;
    for (i = 0; node->children[i]; ++i) {
        zzsys_free(node->children[i]);
    }
    
    free(*(char**)node->data);
    free(node->data);
    memset(node, 0, sizeof(struct tnode_t));
    free(node);
}

void zzinit(void)
{
    syscurr = zzsys_stdroot();
}

int zzexit(void)
{
    zzsys_free(zzsys_root());
    return EXIT_SUCCESS;
}

struct tnode_t* zzsys_curr(void)
{
    return syscurr;
}

void zzsys_set(struct tnode_t* node)
{
    syscurr = node;
}

struct tnode_t* zzsys_root(void)
{
    struct tnode_t* node = zzsys_curr();
    while (node->parent) {
        node = node->parent;
    }
    return node;
}

struct tnode_t* zzsys_search(char* arg, const int off)
{
    static const char div[] = "/", parent[] = "..", root[] = "~";

    int i, j;
    char* args[ZZ_ARG_COUNT];
    const int count = zzgetargs(arg, args, div);
    struct tnode_t* node = zzsys_curr();

    if (!arg || !*arg) {
        return NULL;
    }
    
    for (i = 0; args[i + off]; ++i) {
        if (!memcmp(args[i], parent, sizeof(parent) - 1)) {
            node = node->parent;
        } else if (!memcmp(args[i], root, sizeof(root) - 1)) {
            node = tnode_root(node);
        } else if (!(args[i][0] == '.' && !args[i][1])) {
            int found = 0;
            for (j = 0; node->children[j]; ++j) {
                if (!strcmp(args[i], *(char**)node->children[j]->data)) {
                    node = node->children[j];
                    ++found;
                    break;
                }
            }
            if (!found) {
                return NULL;
            }
        }
    }

    args[0] = zzstruntok(args[0], count, '/');
    return node;
}

char* zzsys_path_name(const char* path)
{
    size_t i, j;
    for (i = 0, j = 0; path[i]; ++i) {
        if (path[i] == '/') {
            j = i + 1;
        }
    }
    return (char*)(size_t)(path + j);
}

char* zzsys_path(struct tnode_t* node)
{
    static char buf[0xfff];
    
    const char* s;
    size_t len, blen;

    strcpy(buf, *(char**)node->data);
    blen = strlen(buf);

    while (node->parent) {
        node = node->parent;
        s = *(char**)node->data;
        len = strlen(s);
        memmove(buf + len + 1, buf, blen + 1);
        memcpy(buf, s, len);
        buf[len] = '/';
        blen += len + 1;
    }
    
    return buf;
}
