/**
 * Copyright (c) 2020 KNpTrue
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#ifndef __AT_CONFIG_H_
#define __AT_CONFIG_H_

#include <stdlib.h>
#include <string.h>

#define AT_DEBUG

#define AT_CMD_PARAMS_MAX_CNT 24
#define AT_CMD_NAME_MAX_LEN 32
#define AT_RX_BUF_LEN 1024

/* Allocate SIZE bytes of memory. */
#define at_malloc(size) malloc(size)

/* Free a block allocated by `malloc', `realloc' or `calloc'. */
#define at_free(ptr)    free(ptr)

#ifdef AT_DEBUG
#include <assert.h>
#define AT_ASSERT(expr) assert(expr)
#else
#define AT_ASSERT(expr)
#endif

#endif /* __AT_CONFIG_H_ */
