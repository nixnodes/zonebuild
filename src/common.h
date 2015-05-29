/*
 * common.h
 *
 *  Created on: May 29, 2015
 *      Author: reboot
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_


#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FAULT(...) char err_b[1024]; fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__, \
    errno ? strerror_r(errno,err_b, sizeof(err_b)) : "" ); \
    errno ? errno = 0 : errno;
#define ERROR(...) fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__)
#define DEBUG(...) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)
#define NOTIFY(...) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)

#endif /* SRC_COMMON_H_ */
