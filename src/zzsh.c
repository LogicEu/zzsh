#include <zzsh.h>
#include <zzsys.h>
#include <zzargs.h>
#include <zzerr.h>
#include <zzio.h>
#include <map.h>
#include <stdlib.h>
#include <string.h>

static const char* whitespace = " ";

static struct map_t zzsh_command_map(void)
{
    static struct command {
        const char* name;
        int (*func)(int, char**);
    } commands[] = {
        {"cd",      &zzcd},
        {"ls",      &zzls},
        {"mkdir",   &zzmkdir},
        {"echo",    &zzecho},
        {"rm",      &zzrm},
        {"rmdir",   &zzrmdir},
        {"touch",   &zztouch},
        {"mv",      &zzmv},
        {"cp",      &zzcp},
        {"pwd",     &zzpwd},
        {"zzsh",    &zzsh},
        {"cat",     &zzcat}
    };

    size_t i;
    struct map_t map = map_create(sizeof(char*), sizeof(int (*)(int, char**)));
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        map_push(&map, &commands[i].name, &commands[i].func);
    }

    return map;
}

static int zzsh_cmd(int argc, char** argv, const struct map_t* commands)
{
    int (**cmd)(int, char**);
    const size_t index = map_search(commands, &argv[0]);
    if (!index) {
        argv[0] = 0;
        return zzerr_set(ZZ_ERR_CMD_NOT_FOUND, argv[0]);
    }

    cmd = map_value_at(commands, index - 1);
    return cmd[0](argc, argv);
}

ssize_t zzgetline(char** linep, size_t* linecap)
{
#define ZZ_GETLINE_BUFSIZ 8

    int c, n;
    ssize_t cur = 0;

    if (!linep[0]) {
        linep[0] = malloc(ZZ_GETLINE_BUFSIZ);
        *linecap = ZZ_GETLINE_BUFSIZ;
    }

    linep[0][0] = 0;

    do {
        c = zzio_getch();
        if (c == '\033') {
            c = zzio_getch();
            c = zzio_getch();
            switch (c) {
                case 'C':
                    if (linep[0][cur]) {
                        zzio_print("\033[1C");
                        ++cur;
                    }
                    break;
                case 'D':
                    if (cur > 0) {
                        zzio_print("\033[1D");
                        --cur;
                    }
                    break;
            }
        } else if (c == 1) {
            zzio_print("\033[%luD", cur);
            cur = 0;
        } else if (c == 127) {
            if (cur > 0) {
                const size_t len = strlen(linep[0] + --cur);
                memmove(linep[0] + cur, linep[0] + cur + 1, len);
                zzio_print("\033[1D");
                n = zzio_print("%s ", linep[0] + cur);
                zzio_print("\033[%dD", n);
            }
        } else {
            const size_t len = strlen(linep[0] + cur);
            memmove(linep[0] + cur + 1, linep[0] + cur, len + 1);
            linep[0][cur] = c;
            n = zzio_print("%s", linep[0] + cur++);
            if (len) {
                zzio_print("\033[%dD", n - 1);
            }

            if (strlen(linep[0]) + 1 >= (size_t)*linecap) {
                *linecap <<= 1;
                linep[0] = realloc(linep[0], *linecap);
            }
        }
    } while (c != '\n');

    return cur;
}

int zzsh(int argc, char** argv)
{
    static const char exitstr[] = "exit";
    static int processes = 0;
    static struct map_t commands;

    int error;
    char* args[ZZ_ARG_COUNT];
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    ZZDIR* currdir;

    if (!processes++) {
        zzsys_init();
        commands = zzsh_command_map();
    }

    while (1) {
        currdir = zzdir_curr();
        zzio_print("%s $ ", zzdir_name(currdir));
        linelen = zzgetline(&line, &linecap);
        if (linelen == -1) {
            return zzerr_set(ZZ_ERR_IO_ERROR, NULL);
        } else if (linelen) {
            line[linelen - 1] = 0;
        }
        
        if (line && *line) {
            argc = zzgetargs(line, args, whitespace);
            if (!memcmp(args[0], exitstr, sizeof(exitstr))) {
                if (!--processes) {
                    map_free(&commands);
                    zzsys_exit();
                }
                break;
            }
            
            error = zzsh_cmd(argc, args, &commands);
            if (error && zzerr_get()) {
                zzerr_print(error, args[0] ? args[0] : argv[0]);
            }
        }
    }

    free(line);
    return EXIT_SUCCESS;
}
