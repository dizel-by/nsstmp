#include "nss.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <resolv.h>
#include <syslog.h>
#include <arpa/inet.h>

#include "storage.h"

#define DEBUG

struct nss_buf {
	char *block;
	size_t len;
	size_t offset;
};

static inline size_t align_ptr(size_t l)
{
	const size_t a = sizeof(void *) - 1;
	return (l + a) & ~a;
}

static void nss_alloc_init(struct nss_buf *buf, char *backing, size_t len)
{
	*buf = (struct nss_buf){
		.block = backing,
		.len = len
	};
}

static void *nss_alloc(struct nss_buf *buf, size_t len)
{
	void *mem = &buf->block[buf->offset];
	buf->offset = align_ptr(buf->offset + len);
	return mem;
}

enum nss_status _nss_tmp_gethostbyname4_r(const char *name,
										  struct gaih_addrtuple **pat,
										  char *buffer, size_t buflen,
										  int *errnop, int *h_errnop,
										  int32_t *ttlp)
{
	struct nss_buf buf;
	nss_alloc_init(&buf, buffer, buflen);

	char ipaddr[16];
	if (!getHostByName(ipaddr, name, sizeof(ipaddr), 0)) {

#ifdef DEBUG
		char log[200];
		sprintf(log, "Resolving %s as NONE [4]", name);
		syslog (LOG_INFO, log);
#endif

		*errnop = ESRCH;
		*h_errnop = HOST_NOT_FOUND;
		return NSS_STATUS_NOTFOUND;
	}

#ifdef DEBUG
	char log[200];
	sprintf(log, "Resolving %s as %s [4]", name, ipaddr);
	syslog (LOG_INFO, log);
#endif

	size_t name_len = strlen(name);
	char *r_name = nss_alloc(&buf, name_len + 1);
	memcpy(r_name, name, name_len);
	r_name[name_len] = '\0';

	struct gaih_addrtuple *r_tuple = nss_alloc(&buf, sizeof(*r_tuple));
	*r_tuple = (struct gaih_addrtuple){
		.next = NULL,
		.name = r_name,
		.family = AF_INET,
		.scopeid = 0
	};
	inet_pton(AF_INET, ipaddr, r_tuple->addr);

	if (*pat) {
		**pat = *r_tuple;
	} else {
		*pat = r_tuple;
	}

	if (ttlp)
		*ttlp = 0;

	*errnop = 0;
	*h_errnop = NETDB_SUCCESS;
	h_errno = 0;

	return NSS_STATUS_SUCCESS;

}

enum nss_status _nss_tmp_gethostbyname2_r(const char *name,
										  int af, struct hostent *result,
										  char *buffer, size_t buflen,
										  int *errnop, int *h_errnop)
{
	struct nss_buf buf;
	nss_alloc_init(&buf, buffer, buflen);

	char ipaddr[16];
	if (!getHostByName(ipaddr, name, sizeof(ipaddr), 0)) {
#ifdef DEBUG
		char log[200];
		sprintf(log, "Resolving %s as NONE [2]", name);
		syslog (LOG_INFO, log);
#endif

		*errnop = ESRCH;
		*h_errnop = HOST_NOT_FOUND;
		return NSS_STATUS_NOTFOUND;
	}

#ifdef DEBUG
	char log[200];
	sprintf(log, "Resolving %s as %s [2]", name, ipaddr);
	syslog (LOG_INFO, log);
#endif

	size_t addrlen = af == AF_INET ? INADDRSZ : IN6ADDRSZ;
	char *r_addr = nss_alloc(&buf, addrlen);
	inet_pton(af, ipaddr, r_addr);

	char **r_addr_list = nss_alloc(&buf, sizeof(char *) * 2);
	r_addr_list[0] = r_addr;
	r_addr_list[1] = NULL;

	char **r_aliases = nss_alloc(&buf, sizeof(char *));
	r_aliases[0] = NULL;

	size_t name_len = strlen(name);
	char *r_name = nss_alloc(&buf, name_len + 1);
	memcpy(r_name, name, name_len);
	r_name[name_len] = '\0';

	*result = (struct hostent){
		.h_name = r_name,
		.h_aliases = r_aliases,
		.h_addrtype = af,
		.h_length = addrlen,
		.h_addr_list = r_addr_list,
	};

	*errnop = 0;
	*h_errnop = NETDB_SUCCESS;
	return NSS_STATUS_SUCCESS;
}
