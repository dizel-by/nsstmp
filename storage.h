#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#define ALLOW_EXPIRED 1

typedef struct result_entry_type {
	time_t bestbefore;
	char ipaddr[16];
	char expired;
} result_entry;

int getWorkingDirectory(char* buf, unsigned int bufsize);
int getHostRecord(char* filename, result_entry* result);
int getHostByName(char* buf, const char *hostname, unsigned int bufsize, int allowExpired);

int addHostRecord(char* hostname, char* ip, int days);
int delHostRecord(char* hostname);

char *trim(char *str);
