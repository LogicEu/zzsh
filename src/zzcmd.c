#include <zzsh.h>
#include <stdlib.h>
#include <string.h>

static int zzsys_file_create(char* arg)
{
    int i;
    const char* dirname;
    struct tnode_t* node = zzsys_search(arg, 1);

    if (!node) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, arg);
    } else if (!node->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, arg);
    }
    
    dirname = zzsys_path_name(arg);
    for (i = 0; node->children[i]; ++i) {
        if (!strcmp(dirname, *(char**)node->children[i]->data)) {
            return zzerr_set(ZZ_ERR_FILE_EXISTS, arg);
        }
    }

    dirname = strdup(dirname);
    tnode_push(node, tnode_create(&dirname, sizeof(char*)));
    
    return EXIT_SUCCESS;
}

int zztouch(int argc, char** argv)
{
    int status = EXIT_SUCCESS, i;
    
    if (argc < 2) {
        zzio_print("usage: touch <file>\n");
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; ++i) {
        status = zzsys_file_create(argv[i]);
        if (status) {
            return status;
        }
    }

    return EXIT_SUCCESS;
}

int zzmkdir(int argc, char** argv)
{
    const char* dirname;
    struct tnode_t* newdir, *node;

    int i, j, out = 0;
    for (i = 1; i < argc; ++i) {

        node = zzsys_search(argv[i], 1);
        if (!node) {
            return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[i]);
        } 
        
        if (!node->children[0]) {
            return zzerr_set(ZZ_ERR_NOT_A_DIR, argv[i]);
        }

        dirname = zzsys_path_name(argv[i]);
        for (j = 0; node->children[j]; ++j) {
            if (!strcmp(dirname, *(char**)node->children[j]->data)) {
                return zzerr_set(ZZ_ERR_DIR_EXISTS, argv[i]);
            }
        }

        dirname = strdup(dirname);
        newdir = tnode_create(&dirname, sizeof(char*));
    
        dirname = strdup(".");
        tnode_push(newdir, tnode_create(&dirname, sizeof(char*)));
        tnode_push(node, newdir);
        ++out;
    }

    if (!out) {
        zzio_print("usage: mkdir <directory>\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int zzrm(int argc, char** argv)
{
    int i, j, out = 0, rec = 0;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1]) {
            switch (argv[i][1]) {
                case 'r': 
                    ++rec;
                    break;
                default:
                    return zzerr_set(ZZ_ERR_ILLEGAL_OPTION, argv[i]);
            }
        } else {
            struct tnode_t* node = zzsys_search(argv[i], 0), *parent;
            if (!node) {
                return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[i]);
            }
            
            if (!rec && node->children[0]) {
                return zzerr_set(ZZ_ERR_IS_A_DIR, argv[i]);
            }
            
            if (!node->parent) {
                return zzerr_set(ZZ_ERR_NOT_PERMITED, argv[i]);
            }
            
            parent = node->parent;
            for (j = 0; parent->children[j] != node; ++j);

            free(*(char**)node->data);
            tnode_remove(parent, j);
            ++out;
        }
    }
    
    if (!out) {
        zzio_print("usage: rm <option> <file / directory>\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int zzcd(int argc, char** argv)
{
    static struct tnode_t* prev = NULL;

    struct tnode_t* node;
    if (argc < 2) {
        node = zzsys_root();
    } else if (argv[1][0] == '-' && !argv[1][1]) {
        if (!prev) {
            return EXIT_SUCCESS;
        }
        node = prev;
    } else {
        node = zzsys_search(argv[1], 0);
    }

    if (!node) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[1]);
    }
    
    if (!node->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, argv[1]);
    }

    prev = zzsys_curr();
    zzsys_set(node);

    return EXIT_SUCCESS;
}

int zzmv(int argc, char** argv)
{
    int i, index, unnamed;
    char* from = NULL, *to = NULL, *name;
    struct tnode_t* node, *mvdir, *parent;

    for (i = 1; i < argc; ++i) {
        if (!from) {
            from = argv[i];
        } else if (!to) {
            to = argv[i];
        } else {
            from = NULL;
            break;
        }
    }

    if (!from || !to) {
        zzio_print("usage: mv <file/directory> <file/directory>\n");
        return EXIT_FAILURE;
    }

    node = zzsys_search(from, 0);
    if (!node) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, from);
    }
    
    if (!node->parent) {
        return zzerr_set(ZZ_ERR_NOT_PERMITED, from);
    }

    unnamed = 0;
    mvdir = zzsys_search(to, unnamed);
    
    if (!mvdir) {
        mvdir = zzsys_search(to, ++unnamed);    
    }

    if (!mvdir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, to);
    }
    
    if (!mvdir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, to);
    }

    parent = node->parent;
    
    index = 0;
    for (i = 0; parent->children[i]; ++i) {
        if (parent->children[i] == node) {
            index = i;
        }
    }

    memmove(
        parent->children + index, 
        parent->children + index + 1,
        (i - index) * sizeof(struct tnode_t*)
    );

    free(*(char**)node->data);
    name = strdup(zzsys_path_name(unnamed ? to : from));
    memcpy(node->data, &name, sizeof(char*));
    tnode_push(mvdir, node);
    
    return EXIT_SUCCESS;
}

static void zzcopy(struct tnode_t* dst, const struct tnode_t* src)
{
    int i;
    char* s;
    struct tnode_t* cpy;
    for (i = 0; src->children[i]; ++i) {
        s = strdup(*(char**)src->children[i]->data);
        cpy = tnode_create(&s, sizeof(char*));
        zzcopy(cpy, src->children[i]);
        tnode_push(dst, cpy);
    }
}

int zzcp(int argc, char** argv)
{
    int i, unnamed, rec = 0;
    struct tnode_t* node, *cpdir, *copy;
    char* from = NULL, *to = NULL, *name;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1]) {
            switch (argv[i][1]) {
                case 'r':
                    ++rec;
                    break;
                default:
                    return zzerr_set(ZZ_ERR_ILLEGAL_OPTION, argv[i]);
            }
        } else if (!from) {
            from = argv[i];
        } else if (!to) {
            to = argv[i];
        } else {
            from = NULL;
            break;
        }
    }

    if (!from || !to) {
        zzio_print("usage: cp <option> <file/directory> <file/directory>\n");
        return EXIT_FAILURE;
    }

    node = zzsys_search(from, 0);
    if (!node) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, from);
    } 
    
    if (node->children[0] && !rec) {
        return zzerr_set(ZZ_ERR_IS_A_DIR, from);
    }
    
    if (!node->parent) {
        return zzerr_set(ZZ_ERR_NOT_PERMITED, from);
    }

    unnamed = 0;
    cpdir = zzsys_search(to, unnamed);
    
    if (!cpdir) {
        cpdir = zzsys_search(to, ++unnamed);
    }

    if (!cpdir) {
        return zzerr_set(ZZ_ERR_NO_SUCH_FD, to);
    }
    
    if (!cpdir->children[0]) {
        return zzerr_set(ZZ_ERR_NOT_A_DIR, to);
    }

    name = strdup(zzsys_path_name(unnamed ? to : from));
    copy = tnode_create(&name, sizeof(char*));
    zzcopy(copy, node);
    tnode_push(cpdir, copy);

    return EXIT_SUCCESS;
}

int zzpwd(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    zzio_print("%s\n", zzsys_path(zzsys_curr()));
    return EXIT_SUCCESS;
}

int zzecho(int argc, char** argv)
{
    argv[0] = zzstruntok(argv[0], argc, ' '); 
    zzio_print("%s\n", argv[0] ? argv[0] : "");
    return EXIT_SUCCESS;
}

int zzls(int argc, char** argv)
{
    static const char newline[] = "\n";
    char div[] = "\t\t", *nargv[ZZ_ARG_COUNT];

    int i, j, count, hidden = 0;
    struct tnode_t* node = NULL;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-' && argv[i][1]) {
            for (j = 1; argv[i][j]; ++j) {
                switch (argv[i][j]) {
                case 'a':
                    ++hidden;
                    break;
                case 'l':
                    strcpy(div, newline);
                    break;
                default:
                    return zzerr_set(ZZ_ERR_ILLEGAL_OPTION, argv[i]);
                }
            }
        } else if (!node) {
            node = zzsys_search(argv[i], 0);
            if (!node) {
                return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[i]);
            }
            
            if (!node->children[0]) {
                zzio_print("%s\n", *(char**)node->data);
            }
        }
    }

    if (!node) {
        node = zzsys_curr();
    }

    count = zzdirargs(node, nargv);
    zzsortargs(nargv, count);

    j = -1;
    for (i = 0; i < count; ++i) {
        if (nargv[i][0] != '.' || hidden) {
            if (j != -1) {
                zzio_print("%s", j % 3 == 2 ? newline : div);
            }
            zzio_print("%s", nargv[i]);
            ++j;
        }
    }

    if (j != -1) {
        zzio_print("\n");
    }

    return EXIT_SUCCESS;
}
