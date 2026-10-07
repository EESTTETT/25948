#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

int main(void) {
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n\n", geteuid());

    setenv("TZ", "Asia/Novosibirsk", 1);
    tzset();

    time_t now;
    time(&now);
    struct tm *sp = localtime(&now);
    
    if (sp != NULL) {
        printf("Novosibirsk Time: %02d/%02d/%04d %02d:%02d %s\n",
               sp->tm_mon + 1,
               sp->tm_mday,
               sp->tm_year + 1900,
               sp->tm_hour,
               sp->tm_min,
               tzname[sp->tm_isdst]);
    } else {
        perror("localtime failed");
    }

    return 0;
}
