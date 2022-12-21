#include <zzsys.h>
#include <zzerr.h>
#include <zzio.h>
#include <zzsh.h>
#include <zzargs.h>
#include <stdlib.h>
#include <string.h>

int zztouch(int argc, char** argv)
{
    int status, i;
    for (i = 1; i < argc; ++i) {
        status = zzsys_mkfile(argv[i]);
        if (status) {
            return status;
        }
    }

    if (i == 1) {
        zzio_print("usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int zzmkdir(int argc, char** argv)
{
    int i, err, p = 0, out = 0;
    for (i = 1; i < argc; ++i) {
        if (!out && argv[i][0] == '-') {
            switch (argv[i][1]) {
                default: return zzerr_set(ZZ_ERR_ILLEGAL_OPTION, argv[i]);
                case 'p': ++p;
            }
        } else {
            ++out;
            err = zzsys_mkdir(argv[i]);
            if (err) {
                if (!p || err != ZZ_ERR_DIR_EXISTS) {
                    return err;
                } 
                err = zzerr_get();
            }
        }
    }

    if (!out) {
        zzio_print("usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int zzrm(int argc, char** argv)
{
    int i, err, out = 0, rec = 0;
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
            ++out;
            err = zzsys_rm(argv[i], rec);
            if (err) {
                return err;
            }
        }
    }
    
    if (!out) {
        zzio_print("usage: %s <option> <file / directory>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int zzrmdir(int argc, char** argv)
{
    /* TODO: is a file error handle */

    if (argc < 2) {
        zzio_print("usage: %s <directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return zzsys_rm(argv[1], 1);
}

int zzcd(int argc, char** argv)
{
    (void)argc;
    return zzsys_chdir(argv[1]);
}

int zzmv(int argc, char** argv)
{
    int i;
    char* from = NULL, *to = NULL;

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
        zzio_print("usage: %s <file/directory> <file/directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return zzsys_move(from, to);
}

int zzcp(int argc, char** argv)
{
    int i, rec = 0;
    char* from = NULL, *to = NULL;

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
        zzio_print("usage: %s <option> <file/directory> <file/directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return zzsys_copy(from, to, rec);
}

int zzpwd(int argc, char** argv)
{
    if (argc > 1) {
        return zzerr_set(ZZ_ERR_TOO_MANY_ARGS, argv[0]);
    }
    zzio_print("%s\n", zzpath(zzdir_curr()));
    return EXIT_SUCCESS;
}

int zzecho(int argc, char** argv)
{
    int i;
    for (i = 1; i < argc; ++i) {
        zzio_print("%s", argv[i]);
    }
    zzio_print("\n");
    return EXIT_SUCCESS;
}

int zzcat(int argc, char** argv)
{
    int i;
    ZZDIR* dir;
    for (i = 1; i < argc; ++i) {
        dir = zzdir_search(argv[i], 0);
        if (!dir) {
            return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[i]);
        }

        if (!zzdir_childcount(dir)) {
            return zzerr_set(ZZ_ERR_IS_A_DIR, argv[i]);
        }

        zzio_print("%s", zzdir_data(dir));
    }
    return EXIT_SUCCESS;
}

int zzls(int argc, char** argv)
{
    static const char newline[] = "\n";

    char div[] = "\t\t", *args[ZZ_ARG_COUNT];
    int i, j, count, hidden = 0;
    ZZDIR* dir = NULL;

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
        } else if (!dir) {
            dir = zzdir_search(argv[i], 0);
            if (!dir) {
                return zzerr_set(ZZ_ERR_NO_SUCH_FD, argv[i]);
            }

            count = zzdirargs(dir, args);
            if (!count) {
                zzio_print("%s\n", zzdir_name(dir));
            }
        }
    }

    if (!dir) {
        dir = zzdir_curr();
    }

    count = zzdirargs(dir, args);
    zzsortargs(args, count);

    j = -1;
    for (i = 0; i < count; ++i) {
        if (args[i][0] != '.' || hidden) {
            if (j != -1) {
                zzio_print("%s", j % 3 == 2 ? newline : div);
            }
            zzio_print("%s", args[i]);
            ++j;
        }
    }

    if (j != -1) {
        zzio_print("\n");
    }

    return EXIT_SUCCESS;
}
