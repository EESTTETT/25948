#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <ulimit.h>
#include <errno.h>

extern char **environ;

/* Структура для сохранения порядка и аргументов опций */
typedef struct {
    char opt;
    char *arg;
} CmdOption;

int main(int argc, char *argv[]) {
    CmdOption *opts = malloc(argc * sizeof(CmdOption));
    if (!opts) {
        perror("malloc");
        return 1;
    }
    
    int opt_count = 0;
    int c;
    
    /* 1. Считываем опции слева направо через getopt и сохраняем в массив */
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (c == '?') {
            /* getopt сам выведет сообщение о недопустимой опции */
            continue; 
        }
        opts[opt_count].opt = c;
        opts[opt_count].arg = optarg;
        opt_count++;
    }
    
    /* 2. Выполняем опции СПРАВА НАЛЕВО */
    for (int i = opt_count - 1; i >= 0; i--) {
        switch (opts[i].opt) {
            case 'i':
                printf("[ -i ] Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
                printf("       Real GID: %d, Effective GID: %d\n", getgid(), getegid());
                break;
                
            case 's':
                if (setpgid(0, 0) == -1) {
                    perror("[ -s ] setpgid failed");
                } else {
                    printf("[ -s ] Процесс стал лидером группы. PGID: %d\n", getpgid(0));
                }
                break;
                
            case 'p':
                printf("[ -p ] PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgid(0));
                break;
                
            case 'u': {
                long ulimit_val = ulimit(UL_GETFSIZE, 0);
                if (ulimit_val == -1) {
                    perror("[ -u ] ulimit get failed");
                } else {
                    printf("[ -u ] Значение ulimit: %ld\n", ulimit_val);
                }
                break;
            }
            
            case 'U': {
                /* Используем atol(3C) согласно подсказке из руководства */
                long new_val = atol(opts[i].arg);
                if (new_val < 0) {
                    fprintf(stderr, "[ -U ] Ошибка: неудачное (отрицательное) значение для U: %ld\n", new_val);
                } else {
                    if (ulimit(UL_SETFSIZE, new_val) == -1) {
                        perror("[ -U ] ulimit set failed");
                    } else {
                        printf("[ -U ] Значение ulimit изменено на %ld\n", new_val);
                    }
                }
                break;
            }
            
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("[ -c ] Размер core-файла (в байтах): %ld\n", (long)rl.rlim_cur);
                } else {
                    perror("[ -c ] getrlimit core");
                }
                break;
            }
            
            case 'C': {
                long new_val = atol(opts[i].arg);
                if (new_val < 0) {
                    fprintf(stderr, "[ -C ] Ошибка: недопустимый размер core-файла: %ld\n", new_val);
                } else {
                    struct rlimit rl;
                    getrlimit(RLIMIT_CORE, &rl);
                    rl.rlim_cur = new_val;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("[ -C ] setrlimit core failed");
                    } else {
                        printf("[ -C ] Размер core-файла изменен на %ld байт\n", new_val);
                    }
                }
                break;
            }
            
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("[ -d ] Рабочая директория: %s\n", cwd);
                } else {
                    perror("[ -d ] getcwd failed");
                }
                break;
            }
            
            case 'v': {
                char **env = environ;
                printf("[ -v ] --- Переменные среды ---\n");
                while (*env) {
                    printf("%s\n", *env);
                    env++;
                }
                break;
            }
            
            case 'V': {
                /* Выделяем память для putenv, так как строка должна оставаться в памяти */
                char *env_str = malloc(strlen(opts[i].arg) + 1);
                if (env_str) {
                    strcpy(env_str, opts[i].arg);
                    if (putenv(env_str) != 0) {
                        perror("[ -V ] putenv failed");
                        free(env_str);
                    } else {
                        printf("[ -V ] Переменная среды установлена/изменена: %s\n", opts[i].arg);
                    }
                }
                break;
            }
        }
    }
    
    free(opts);
    return 0;
}
