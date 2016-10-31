#pragma once

#include <stddef.h>
#include <nss.h>
#include <netdb.h>

enum nss_status _nss_tmp_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop);
enum nss_status _nss_tmp_gethostbyname4_r(const char *name,
                                           struct gaih_addrtuple **pat,
                                           char *buffer, size_t buflen,
                                           int *errnop, int *h_errnop,
                                           int32_t *ttlp);
