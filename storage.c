#include "storage.h"

int getWorkingDirectory(char* buf, unsigned int bufsize) {
	char* home = getenv("HOME");
	if (home == NULL)
		return 0;
	const char config_path[] = "/.config/nsstmp/";
	if ((strlen(home) + strlen(config_path) + 1)*sizeof(char) > bufsize)
		return 0;
	strncpy(buf, home, bufsize);
	strncat(buf, config_path, bufsize);
	return 1;
}

int delHostRecord(char* hostname) {
	char tmp[100];
	if (!getWorkingDirectory(tmp, sizeof(tmp)))
		return 0;
	strncat(tmp, hostname, sizeof(tmp));
	unlink(tmp);
	return 1;
}

int addHostRecord(char* hostname, char* ip, int days) {
	char tmp[100];
	if (!getWorkingDirectory(tmp, sizeof(tmp)))
		return 0;

	mkdir(tmp, 0700);

	strncat(tmp, hostname, sizeof(tmp));

	time_t rawtime;
	time (&rawtime);
	rawtime += days * 60*60*24;

	FILE *file = fopen(tmp, "w");
	fprintf(file, "%ld\n%s\n", rawtime, ip);
	fclose(file);
	return 1;
}

int getHostRecord(char* filename, result_entry* result) {
	char buf[20];
	FILE *file;
	memset(result, 0, sizeof(&result));

	file = fopen(filename, "r");
	if (!file)
		return 0;

	if (fgets(buf, sizeof(buf), file) != NULL) {
		result->bestbefore = atol(trim(buf));

		time_t rawtime;
		time (&rawtime);

		result->expired = result->bestbefore < rawtime;

		if (fgets(buf, sizeof(buf), file) != NULL) {
			strncpy(result->ipaddr, trim(buf), sizeof(result->ipaddr));
		}
	}
	return 1;
}

int getHostByName(char* buf, const char *hostname, unsigned int bufsize, int allowExpired) {
	char tmp[100];
	if (!getWorkingDirectory(tmp, sizeof(tmp)))
		return 0;
	strncat(tmp, hostname, sizeof(tmp));
	result_entry r;
	if (!getHostRecord(tmp, &r))
		return 0;

	if (r.expired && (!allowExpired))
		return 0;

	strncpy(buf, r.ipaddr, bufsize);
	return 1;
}

char *trim(char *str) {
	char *end;
	while(isspace((unsigned char)*str)) str++;
	if (*str == 0)
		return str;
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;
	*(end+1) = 0;
	return str;
}
