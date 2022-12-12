#include <zzsh.h>
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
        {"touch",   &zztouch},
        {"mv",      &zzmv},
        {"cp",      &zzcp},
        {"pwd",     &zzpwd},
        {"zzsh",    &zzsh}
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
    struct tnode_t* currdir;

    if (!processes++) {
        zzinit();
        commands = zzsh_command_map();
    }

    while (1) {
        currdir = zzsys_curr();
        zzio_print("%s $ ", *(char**)currdir->data);
        linelen = zzio_getline(&line, &linecap);
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
                    zzexit();
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
