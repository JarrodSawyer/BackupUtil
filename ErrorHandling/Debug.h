#ifndef __DEBUG_h__
#define __DEBUG_h__

/**
 * Error and debug prints taken from http://c.learncodethehardway.org/book/ex20.html and
 * adapted to my needs.
 **/

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DEBUG
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%s:%d: " M "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define ERROR(ERROR_CODE, M, ...) fprintf(stderr, "[ERROR] (%s:%s:%d: errno: %s, error code: %d) " M "\n", __FILE__, __func__, __LINE__, clean_errno(), ERROR_CODE, ##__VA_ARGS__)

#define WARN(M, ...) fprintf(stderr, "[WARN] (%s:%s:%d: errno: %s) " M "\n", __FILE__, __func__,  __LINE__, clean_errno(), ##__VA_ARGS__)

#define INFO(M, ...) fprintf(stderr, "[INFO] (%s:%s:%d) " M "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif
