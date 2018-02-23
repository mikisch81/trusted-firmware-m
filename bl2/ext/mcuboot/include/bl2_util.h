/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BL2_UTIL_H__
#define __BL2_UTIL_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include <stddef.h>

    /* Evaluates to 0 if cond is true-ish; compile error otherwise */
#define ZERO_OR_COMPILE_ERROR(cond) ((int) sizeof(char[1 - 2 * !(cond)]) - 1)

    /* Evaluates to 0 if array is an array; compile error if not array (e.g.
     * pointer)
     */
#define IS_ARRAY(array) \
        ZERO_OR_COMPILE_ERROR(!__builtin_types_compatible_p(__typeof__(array), \
                              __typeof__(&(array)[0])))

#define ARRAY_SIZE(array) \
    ((unsigned long) (IS_ARRAY(array) + \
        (sizeof(array) / sizeof((array)[0]))))

#define CONTAINER_OF(ptr, type, field) \
        ((type *)(((char *)(ptr)) - offsetof(type, field)))

struct device {
    int device_id;
};

#ifdef __cplusplus
}
#endif

#endif /* __BL2_UTIL_H__ */
