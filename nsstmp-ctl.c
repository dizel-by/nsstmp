#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <string.h>
#include <time.h>

#include "storage.h"
#include "nsstmp-ctl.h"

char dir[100];

void expire(void) {
	strcat(dir, "*");

	glob_t results;
	glob(dir, 0, NULL, &results);

	unsigned int i, exp = 0;

	for (i = 0; i < results.gl_pathc; i++) {
		result_entry r;
		getHostRecord(results.gl_pathv[i], &r);
		if (r.expired) {
			printf("%s\n", results.gl_pathv[i]+strlen(dir)-1);
			unlink(results.gl_pathv[i]);
			exp++;
		}
	}
	if (exp)
		printf("===================\n");
	printf("Expired Domains: %d\n", exp);
}

void dump(void) {
	strcat(dir, "*");

	glob_t results;
	glob(dir, 0, NULL, &results);

	if (!results.gl_pathc) {
		printf("No domains defined\n");
		return;
	}

	unsigned int i;
	unsigned int maxlength = 0;
	for (i = 0; i < results.gl_pathc; i++) {
		if (strlen(results.gl_pathv[i]) > maxlength)
			maxlength = strlen(results.gl_pathv[i]);
	}

	maxlength -= strlen(dir)-1;
	if (maxlength < 11) maxlength = 11;

	char fmt[50];
	sprintf(fmt, "%%-%ds | %%15s | %%-31s | %%s \n", maxlength);

	putchar('\n');
	unsigned int chars = printf(fmt, "Domain Name", "IP Address", "Exp. Date", "Expired");

	for (i = 0; i < chars-1; i++)
		putchar('-');
	putchar('\n');

	for (i = 0; i < results.gl_pathc; i++) {
		result_entry r;
		getHostRecord(results.gl_pathv[i], &r);

		struct tm *tmp = localtime(&r.bestbefore);

		char datestr[100];
		strftime(datestr, sizeof(datestr), "%a, %d %b %Y %T %z", tmp);
		printf(fmt, results.gl_pathv[i]+strlen(dir)-1, r.ipaddr, datestr, r.expired ? "Yes" : "No");
	}
	putchar('\n');
}

void help(void) {
	printf("Usage:\n\
  nsstmp-ctl [options]\n\
\n\
  nsstmp-ctl --help\n\
  nsstmp-ctl --dump\n\
  nsstmp-ctl --add    <domain> <ip> <days>\n\
  nsstmp-ctl --update <domain> <days>\n\
  nsstmp-ctl --delete <domain>\n\
\n");
}

void add(char* hostname, char* ip, unsigned int days) {
	addHostRecord(hostname, ip, days);
}

void update(char* hostname, unsigned int days) {
	char ip[20];
	if (!getHostByName(ip, hostname, sizeof(ip), ALLOW_EXPIRED))
		return;
	addHostRecord(hostname, ip, days);
}

void del(char* hostname) {
	delHostRecord(hostname);
}

int main(int argc, char **argv) {

	if (!getWorkingDirectory(dir, sizeof(dir))) {
		printf("ERROR: Error getting working directory\n");
		return 1;
	}

	if (argc == 1) {
		help();
		dump();
		return 0;
	}

	if (strcmp("--add", argv[1]) == 0) {
		char* hostname = argv[2];
		char* ip = argv[3];
		char* days = argv[4];
		add(hostname, ip, atoi(days));
		return 0;
	}
	else if (strcmp("--update", argv[1]) == 0) {
		char* hostname = argv[2];
		char* days = argv[3];
		update(hostname, atoi(days));
		return 0;
	}
	else if (strncmp("--del", argv[1], 5) == 0) {
		char* hostname = argv[2];
		del(hostname);
		return 0;
	}
	else if (strncmp("--expire", argv[1], 5) == 0) {
		expire();
		return 0;
	}
	else if (strcmp("--dump", argv[1]) == 0) {
		dump();
		return 0;
	}

	return 0;

}
