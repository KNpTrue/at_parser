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
#include <at_config.h>
#include <at_param.h>

int at_param_parse(char *s, struct at_param *params, int size)
{
    int i = 0;
    char *p;
    enum {
        AT_PARAM_FIND_START,
        AT_PARAM_FIND_END,
        AT_PARAM_FIND_STRING_END,
    } state = AT_PARAM_FIND_START;

    for (; *s; s++) {
        switch (state) {
        case AT_PARAM_FIND_START:
            switch (*s) {
            case ',':
                params[i].type = AT_PARAM_TYPE_EMTPY;
                params[i].raw = NULL;
                i++;
                if (i == size) {
                    return i;
                }
                break;
            case '"':
                params[i].type = AT_PARAM_TYPE_UNKNOWN;
                params[i].raw = s;
                state = AT_PARAM_FIND_STRING_END;
                break;
            case ' ':
                break;
            default:
                params[i].type = AT_PARAM_TYPE_UNKNOWN;
                params[i].raw = s;
                state = AT_PARAM_FIND_END;
            }
            break;
        case AT_PARAM_FIND_END:
            if (*s == ',') {
                p = s - 1;
                while (*p == ' ') {
                    p--;
                }
                *(p + 1) = '\0';
                i++;
                if (i == size) {
                    return i;
                }
                state = AT_PARAM_FIND_START;
            }
            break;
        case AT_PARAM_FIND_STRING_END:
            if (*s == '"') {
                state = AT_PARAM_FIND_END;
            }
            break;
        default:
            break;
        }
    }
    switch (state) {
    case AT_PARAM_FIND_END:
        p = s - 1;
        while (*p == ' ') {
            p--;
        }
        *(p + 1) = '\0';
        i++;
        break;
    case AT_PARAM_FIND_STRING_END:  
        return -1;
    default:
        break;
    }
    return i;
}

static int at_param_hex2num(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return 0x0a + (c - 'A');
    }
    if (c >= 'a' && c <= 'f') {
        return 0x0a + (c- 'a');
    }
    return -1;
}

const char *at_param_str(struct at_param *param)
{
    size_t len;
    int num1, num2;
    char *s, *t, *end;
    char c;
    AT_ASSERT(param);

    if (param->type == AT_PARAM_TYPE_STRING) {
        return param->raw;
    } else if (param->type != AT_PARAM_TYPE_UNKNOWN) {
        return NULL;
    }

    len = strlen(param->raw);
    if (len < 2 || param->raw[0] != '"' ||
        param->raw[len - 1] != '"') {
        return NULL;
    }
    end = param->raw + len - 1;
    s = param->raw + 1;
    t = s;

    while (t != end) {
        c = *t++;
        if (c == '\\' && end -t >= 2 && isalnum((int)t[0]) && isalnum((int)t[1])) {
                num1 = at_param_hex2num(t[0]);
                num2 = at_param_hex2num(t[1]);
                if (num1 >= 0 && num2 >= 0) {
                    *s++ = (num1 << 4) | num2;
                    t += 2;
                } else {
                    *s++ = c;
                }
        } else {
            *s++ = c;
        }
    }
    *s = '\0';
    param->raw += 1;
    param->type = AT_PARAM_TYPE_STRING;
    return param->raw;
}
