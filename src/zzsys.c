#ifndef ZZ_SYSTEM_C
#define ZZ_SYSTEM_C

typedef struct tnode_t ZZDIR;

#include <zzsys.h>
#include <zznode.h>
#include <zzerr.h>
#include <zzio.h>
#include <zzargs.h>
#include <stdlib.h>
#include <string.h>
#include <tree.h>

static ZZDIR* syscurr = NULL;

static ZZDIR* zzdir_pushf(ZZDIR* dir, const char* name, const void* data, size_t len)
{
    struct zznode node = zznode_create(name, data, len);
    ZZDIR* newdir = tnode_create(&node, sizeof(struct zznode));
    tnode_push(dir, newdir);
    return newdir;
}

static ZZDIR* zzdir_pushd(ZZDIR* dir, const char* name)
{
    ZZDIR* newdir = zzdir_pushf(dir, name, NULL, 0);
    zzdir_pushf(newdir, ".", NULL, 0);
    return newdir;
}

static ZZDIR* zzdir_stdroot(void)
{
    struct zznode node = zznode_create("~", NULL, 0);
    ZZDIR* root = tnode_create(&node, sizeof(struct zznode));
    zzdir_pushf(root, ".", NULL, 0);
    return root;
}

static void zzdir_free(ZZDIR* dir)
{
    size_t i;
    for (i = 0; dir->children[i]; ++i) {
        zzdir_free(dir->children[i]);
    }
    
    zznode_free(dir->data);
    free(dir->data);
    memset(dir, 0, sizeof(ZZDIR));
    free(dir);
}

void zzsys_init(void)
{
    syscurr = zzdir_stdroot();
}

int zzsys_exit(void)
{
    zzdir_free(zzdir_root());
    return EXIT_SUCCESS;
}

ZZDIR* zzdir_curr(void)
{
    return syscurr;
}

int zzdir_childcount(const ZZDIR* dir)
{
    int i;
    for (i = 0; dir->children[i]; ++i);
    return i;
}

void zzdir_set(ZZDIR* dir)
{
    if (dir->children[0]) {
        syscurr = dir;
    }
}

ZZDIR* zzdir_root(void)
{
    ZZDIR* dir = zzdir_curr();
    while (dir->parent) {
        dir = dir->parent;
    }
    return dir;
}

void* zzdir_data(const ZZDIR* dir)
{
    return zznode_data(dir->data);
}

ZZDIR* zzdir_search(const char* arg, const int off)
{
    static const char div[] = "/", parent[] = "..", root[] = "~";

    int i, j;
    char* args[ZZ_ARG_COUNT], buf[ZZ_ARG_SIZE];
    ZZDIR* dir = zzdir_curr();

    strcpy(buf, arg);
    zzgetargs(buf, args, div);

    if (!arg || !*arg) {
        return NULL;
    }
    
    for (i = off; args[i]; ++i) {
        if (!memcmp(args[i], parent, sizeof(parent) - 1)) {
            dir = dir->parent;
        } else if (!memcmp(args[i], root, sizeof(root) - 1)) {
            dir = tnode_root(dir);
        } else if (!(args[i][0] == '.' && !args[i][1])) {
            int found = 0;
            for (j = 0; dir->children[j]; ++j) {
                const struct zznode* node = dir->children[j]->data;
                if (!strcmp(args[i], node->data)) {
                    dir = dir->children[j];
                    ++found;
                    break;
                }
            }
            if (!found) {
                return NULL;
            }
        }
    }

    return dir;
}

const char* zzdir_name(const ZZDIR* dir)
{
    return zznode_name(dir->data);
}

void zzdir_rename(ZZDIR* dir, const char* name)
{
    zznode_rename(dir->data, name);
}

int zzsys_mkfile(const char* arg)
{
    int i;
    const char* dirname;
    struct zznode node;
    ZZDIR* dir = zzdir_search(arg, 1);

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, arg);
    } else if (!dir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, arg);
    }
    
    dirname = zzpath_name(arg);
    for (i = 0; dir->children[i]; ++i) {
        node = *(struct zznode*)dir->children[i]->data;
        if (!strcmp(dirname, node.data)) {
            return zzerr_set(ZZ_ERR_FILE_EXISTS, arg);
        }
    }

    zzdir_pushf(dir, dirname, NULL, 0);
    return EXIT_SUCCESS;
}

int zzsys_mkdir(const char* arg)
{
    int i;
    const char* dirname;
    struct zznode node;
    ZZDIR* dir = zzdir_search(arg, 1);

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, arg);
    }

    if (!dir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, arg);
    }

    dirname = zzpath_name(arg);
    for (i = 0; dir->children[i]; ++i) {
        node = *(struct zznode*)dir->children[i]->data;
        if (!strcmp(dirname, node.data)) {
            return zzerr_set(ZZ_ERR_DIR_EXISTS, arg);
        }
    }

    zzdir_pushd(dir, dirname);
    return EXIT_SUCCESS;
}

int zzsys_rm(const char* arg, const int rec)
{
    int i;
    ZZDIR* dir = zzdir_search(arg, 0), *parent;

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, arg);
    }
    
    if (!rec && dir->children[0]) {
        return zzerr_set(ZZ_ERR_IS_A_DIR, arg);
    }
    
    if (!dir->parent) {
        return zzerr_set(ZZ_ERR_NOT_PERMITED, arg);
    }
    
    parent = dir->parent;
    for (i = 0; parent->children[i] != dir; ++i);

    zznode_free(dir->data);
    tnode_remove(parent, i);
    
    return EXIT_SUCCESS;
}

int zzsys_chdir(const char* arg)
{
    static ZZDIR* prev = NULL;

    ZZDIR* dir;
    if (!arg) {
        dir = zzdir_root();
    } else if (arg[0] == '-' && !arg[1]) {
        if (!prev) {
            return EXIT_SUCCESS;
        }
        dir = prev;
    } else {
        dir = zzdir_search(arg, 0);
    }

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, arg);
    }
    
    if (!dir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, arg);
    }

    prev = zzdir_curr();
    zzdir_set(dir);

    return EXIT_SUCCESS;
}

int zzsys_move(const char* from, const char* to)
{
    int i, unnamed = 0, index = 0;
    ZZDIR *mvdir, *parent, *dir = zzdir_search(from, 0);

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, from);
    }
    
    if (!dir->parent) {
        return zzerr_set(ZZ_ERR_NOT_PERMITED, from);
    }

    mvdir = zzdir_search(to, unnamed);    
    if (!mvdir) {
        mvdir = zzdir_search(to, ++unnamed);    
    }

    if (!mvdir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, to);
    }
    
    if (!mvdir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, to);
    }

    parent = dir->parent;
    for (i = 0; parent->children[i]; ++i) {
        if (parent->children[i] == dir) {
            index = i;
        }
    }

    memmove(
        parent->children + index, 
        parent->children + index + 1,
        (i - index) * sizeof(ZZDIR*)
    );

    zzdir_rename(dir, zzpath_name(unnamed ? to : from));
    tnode_push(mvdir, dir);
    return EXIT_SUCCESS;
}

static void zzsys_copy_tree(ZZDIR* dst, const ZZDIR* src)
{
    int i;
    ZZDIR *dir;
    struct zznode node;
    for (i = 0; src->children[i]; ++i) {
        node = zznode_copy(src->children[i]->data);
        dir = tnode_create(&node, sizeof(struct zznode));
        zzsys_copy_tree(dir, src->children[i]);
        tnode_push(dst, dir);
    }
}

int zzsys_copy(const char* from, const char* to, const int rec)
{
    int unnamed = 0;
    struct zznode node;
    ZZDIR *cpdir, *copy, *dir = zzdir_search(from, 0);

    if (!dir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, from);
    } 
    
    if (dir->children[0] && !rec) {
        return zzerr_set(ZZ_ERR_IS_A_DIR, from);
    }
    
    if (!dir->parent) {
        return zzerr_set(ZZ_ERR_NOT_PERMITED, from);
    }

    cpdir = zzdir_search(to, unnamed);
    if (!cpdir) {
        cpdir = zzdir_search(to, ++unnamed);
    }

    if (!cpdir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, to);
    }
    
    if (!cpdir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, to);
    }

    node = zznode_copy(dir->data);
    copy = tnode_create(&node, sizeof(struct zznode));
    zzsys_copy_tree(copy, dir);
    zzdir_rename(copy, zzpath_name(unnamed ? to : from));
    tnode_push(cpdir, copy);

    return EXIT_SUCCESS;
}

char* zzpath_name(const char* path)
{
    size_t i, j;
    for (i = 0, j = 0; path[i]; ++i) {
        if (path[i] == '/') {
            j = i + 1;
        }
    }
    return (char*)(size_t)(path + j);
}

char* zzpath(const ZZDIR* dir)
{
    static char buf[0xfff];
    
    const char* s;
    size_t len, blen;

    const struct zznode* node = dir->data;
    strcpy(buf, node->data);
    blen = strlen(buf);

    while (dir->parent) {
        dir = dir->parent;
        node = dir->data;
        s = node->data;
        len = strlen(s);
        memmove(buf + len + 1, buf, blen + 1);
        memcpy(buf, s, len);
        buf[len] = '/';
        blen += len + 1;
    }
    
    return buf;
}

int zzdirargs(const ZZDIR* dir, char** args)
{
    int i;
    const struct zznode* node;
    for (i = 0; dir->children[i]; ++i) {
        node = dir->children[i]->data;
        args[i] = node->data;
    }
    args[i] = NULL;
    return i;
}

#endif /* ZZ_SYSTEM_C */
