/*
 * common.h
 *
 *  Created on: May 29, 2015
 *      Author: reboot
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

char debug_mode;

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FAULT(...) char err_b[1024]; fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__, \
    errno ? strerror_r(errno,err_b, sizeof(err_b)) : "" ); \
    errno ? errno = 0 : errno;
#define ERROR(...) fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__)
#define DEBUG(...) if ( debug_mode ) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)
#define NOTIFY(...) fprintf(stdout, PACKAGE_NAME ": " __VA_ARGS__)
#define ABORT(...) fprintf(stderr, PACKAGE_NAME ": " __VA_ARGS__); abort()


#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error
#endif


#if __x86_64__ || __ppc64__
#define _ARCH_STR        "x86_64"
#else
#define _ARCH_STR        "i686"
#endif


#endif /* SRC_COMMON_H_ */
