
#include <stdio.h>

int main(void)
{
    FILE *fp;
    char line[256];

    fp = fopen("/proc/uptime", "r");

    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    if (fgets(line, sizeof(line), fp) != NULL) {
        printf("Kernel uptime: %s", line);
    }

    fclose(fp);
    return 0;
}
