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
#include <string.h>
#include <strings.h>
#include <at_parser.h>

enum at_parser_state {
    ATP_STATE_WAIT_A,       /**< wait 'A' */
    ATP_STATE_WAIT_T,       /**< wait 'T' */
    ATP_STATE_WAIT_TYPE,    /**< wait type, like '=', '?' */
    ATP_STATE_WAIT_ARG_END, /**< wait argument end */
};

struct at_cmd_desc {
    char name[AT_CMD_NAME_MAX_LEN];
    void (*handle)(struct at_parser *parser, const char *cmd, enum at_cmd_type type,
        struct at_param *args, unsigned char count, void *arg);
    struct at_cmd_desc *next;
};

struct at_parser {
    enum at_parser_state state;
    enum at_cmd_type cmd_type;

    unsigned int cmd_pos;
    unsigned int param_pos;
    char cmd[AT_CMD_NAME_MAX_LEN];
    char param_buf[AT_RX_BUF_LEN];

    unsigned char echo:1;
    unsigned char del:1;
    unsigned char wait_sync_resp:1;

    struct at_cmd_desc *desc_head;
    unsigned int (*tx)(void *data, unsigned int len, void *arg);
    void (*enable_read)(unsigned char value, void *arg);
    void *arg;
};

const char *at_resp_result_str[] = {
    [AT_RESP_OK] = "OK",
    [AT_RESP_ERROR] = "ERROR",
};

void ate_cmd_handle(struct at_parser *parser, const char *cmd, enum at_cmd_type type,
    struct at_param *args, unsigned char count, void *arg)
{
    switch (type) {
    case AT_CMD_EXE:
        if (!strcasecmp(cmd, "E1")) {
            at_parser_set_echo(parser, 1);
        } else {
            at_parser_set_echo(parser, 0);
        }
        at_sync_response(parser, AT_RESP_OK, NULL);
        break;
    default:
        at_sync_response(parser, AT_RESP_ERROR, NULL);
        break;
    }
}

struct at_parser *at_parser_new(struct at_parser_config *cfg, void *arg)
{
    struct at_parser *p;

    AT_ASSERT(cfg);
    AT_ASSERT(cfg->tx);
    AT_ASSERT(cfg->enable_read);
    p = at_malloc(sizeof(*p));
    if (!p) {
        return NULL;
    }
    memset(p, 0, sizeof(*p));
    p->tx = cfg->tx;
    p->enable_read = cfg->enable_read;
    p->echo = cfg->echo;
    p->arg = arg;

    at_cmd_register(p, "E", ate_cmd_handle);
    at_cmd_register(p, "E0", ate_cmd_handle);
    at_cmd_register(p, "E1", ate_cmd_handle);
    p->enable_read(1, p->arg);
    return p;
}

void at_parser_free(struct at_parser *parser)
{
    struct at_cmd_desc *cur;

    AT_ASSERT(parser);
    if (parser) {
        while (parser->desc_head) {
            cur = parser->desc_head;
            parser->desc_head = parser->desc_head->next;
            at_free(cur);
        }
        at_free(parser);
    }
}

static void at_parser_echo(struct at_parser *parser, char c)
{
    if (parser->echo) {
        parser->tx(&c, 1, parser->arg);
    }
}

void at_parser_set_echo(struct at_parser *parser, unsigned char echo)
{
    parser->echo = echo;
}

static struct at_cmd_desc *at_parser_find_cmd(struct at_parser *parser, const char *cmd)
{
    struct at_cmd_desc *desc = parser->desc_head;
    while (desc) {
        if (!strncasecmp(desc->name, cmd, sizeof(desc->name))) {
            return desc;
        }
        desc = desc->next;
    }
    return NULL;
}

static void at_parser_cmd_reset(struct at_parser *parser)
{
    parser->state = ATP_STATE_WAIT_A;
    parser->cmd_pos = 0;
    parser->param_pos = 0;
    parser->cmd_type = AT_CMD_NONE;
}

static enum at_param_type at_param_get_type(struct at_param *param)
{
    return AT_PARAM_TYPE_UNKNOWN;
}

static int at_parse_sub_params(char *s, struct at_param *params, int cnt)
{
    int i = 0;
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
                if (i == cnt) {
                    return i;
                }
                break;
            case '"':
                params[i].type = AT_PARAM_TYPE_STRING;
                params[i].raw = s;
                state = AT_PARAM_FIND_STRING_END;
                break;
            default:
                params[i].type = AT_PARAM_TYPE_UNKNOWN;
                params[i].raw = s;
                state = AT_PARAM_FIND_END;
            }
            break;
        case AT_PARAM_FIND_END:
            switch (*s) {
            case ',':
                *s = '\0';
                if (params[i].type == AT_PARAM_TYPE_UNKNOWN) {
                    params[i].type = at_param_get_type(params + i);
                }
                i++;
                if (i == cnt) {
                    return i;
                }
                state = AT_PARAM_FIND_START;    
                break;
            case '"':
                if (params[i].type == AT_PARAM_TYPE_STRING) {
                    params[i].type = AT_PARAM_TYPE_UNKNOWN;
                }
                break;
            default:
                break;
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
        i++;
        break;
    case AT_PARAM_FIND_STRING_END:  
        return -1;
    default:
        break;
    }
    return i;
}

int at_parser_post_char(struct at_parser *parser, int c)
{
    unsigned int cnt;
    struct at_cmd_desc *desc;
    struct at_param params[AT_CMD_PARAMS_MAX_CNT];

    AT_ASSERT(parser);

    if (parser->wait_sync_resp) {
        return -1;
    }
    if (c == 0x7f) {
        switch (parser->state) {
        case ATP_STATE_WAIT_T:
            parser->state = ATP_STATE_WAIT_A;
            break;
        case ATP_STATE_WAIT_TYPE:
            if (parser->cmd_pos) {
                parser->cmd_pos--;
            } else {
                parser->state = ATP_STATE_WAIT_T;
            }
            break;
        case ATP_STATE_WAIT_ARG_END:
            if (parser->param_pos) {
                parser->param_pos--;
            } else {
                parser->state = ATP_STATE_WAIT_TYPE;
            }
            break;
        default:
            return 0;
        }
        if (parser->echo) {
            parser->tx((void *)"\b \b", 3, parser->arg);
        }
        return 0;
    }
    switch (parser->state) {
    case ATP_STATE_WAIT_A:
        if (c == 'A' || c == 'a') {
            parser->state = ATP_STATE_WAIT_T;
            at_parser_echo(parser, c);
        }
        break;
    case ATP_STATE_WAIT_T:
        if (c == 'T' || c == 't') {
            parser->state = ATP_STATE_WAIT_TYPE;
            at_parser_echo(parser, c);
        } else {
            parser->state = ATP_STATE_WAIT_A;
        }
        break;
    case ATP_STATE_WAIT_TYPE:
        at_parser_echo(parser, c);
        switch (c) {
        case '\r':
        case '\n':
            if (!parser->cmd_pos) {
                at_sync_response(parser, AT_RESP_OK, NULL);
                at_parser_cmd_reset(parser);
                break;
            }
            parser->cmd[parser->cmd_pos] = '\0';
            if (parser->cmd_type == AT_CMD_NONE) {
                parser->cmd_type = AT_CMD_EXE;
            }
            desc = at_parser_find_cmd(parser, parser->cmd);
            if (desc) {
                parser->wait_sync_resp = 1;
                parser->enable_read(0, parser->arg);
                desc->handle(parser, parser->cmd, parser->cmd_type,
                    NULL, 0, parser->arg);
            } else {
                at_sync_response(parser, AT_RESP_ERROR, NULL);
            }
            at_parser_cmd_reset(parser);
            break;
        case '=':
            parser->cmd[parser->cmd_pos] = '\0';
            parser->cmd_type = AT_CMD_SET;
            parser->state = ATP_STATE_WAIT_ARG_END;
            break;
        case '?':
            parser->cmd[parser->cmd_pos] = '\0';
            parser->cmd_type = AT_CMD_READ;
            parser->state = ATP_STATE_WAIT_ARG_END;
            break;
        default:
            if (parser->cmd_pos < sizeof(parser->cmd) - 1) {
                parser->cmd[parser->cmd_pos++] = c;
            }
            break;
        }
        break;
    case ATP_STATE_WAIT_ARG_END:
        at_parser_echo(parser, c);
        switch (c) {
        case '\r':
        case '\n':
            if (parser->cmd_type == AT_CMD_SET) {
                if (parser->param_pos == 1 && parser->param_buf[0] == '?') {
                    parser->cmd_type = AT_CMD_TEST;
                    goto handle;
                }
            } else {
                if (parser->param_pos) {
                    at_sync_response(parser, AT_RESP_ERROR, NULL);
                    at_parser_cmd_reset(parser);
                    break;
                }
                goto handle;
            }
            parser->param_buf[parser->param_pos] = '\0';
            cnt = at_parse_sub_params(parser->param_buf, params, AT_CMD_PARAMS_MAX_CNT);
            if (cnt == -1) {
                at_sync_response(parser, AT_RESP_ERROR, NULL);
                at_parser_cmd_reset(parser);
                break;
            }
handle:
            desc = at_parser_find_cmd(parser, parser->cmd);
            if (desc) {
                parser->wait_sync_resp = 1;
                parser->enable_read(0, parser->arg);
                desc->handle(parser, parser->cmd, parser->cmd_type,
                    params, cnt, parser->arg);
            } else {
                at_sync_response(parser, AT_RESP_ERROR, NULL);
            }
            at_parser_cmd_reset(parser);
            break;
        default:
            if (parser->param_pos < sizeof(parser->param_buf) - 1) {
                parser->param_buf[parser->param_pos++] = c;
            }
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}

int at_cmd_register(struct at_parser *parser, const char *cmd,
    void (*handle)(struct at_parser *parser, const char *cmd, enum at_cmd_type type,
    struct at_param *args, unsigned char count, void *arg))
{
    struct at_cmd_desc *desc;

    AT_ASSERT(parser);
    AT_ASSERT(cmd);
    AT_ASSERT(handle);

    if (strlen(cmd) >= AT_CMD_NAME_MAX_LEN - 1) {
        return -1;
    }
    desc = at_malloc(sizeof(*desc));
    if (!desc) {
        return -2;
    }
    strncpy(desc->name, cmd, sizeof(desc->name));
    desc->handle = handle;
    if (parser->desc_head) {
        desc->next = parser->desc_head->next;
        parser->desc_head->next = desc;
    } else {
        parser->desc_head = desc;
    }
}

void at_cmd_unregister(struct at_parser *parser, const char *cmd)
{
    struct at_cmd_desc **desc = &(parser->desc_head);
    struct at_cmd_desc *cur;

    AT_ASSERT(parser);
    AT_ASSERT(cmd);

    while (*desc) {
        cur = *desc;
        if (!strncmp(cur->name, cmd, sizeof(cur->name))) {
            *desc = cur->next;
            at_free(cur);
            return;
        }
        desc= &(*desc)->next;
    }
}

void at_sync_response(struct at_parser *parser, enum at_resp_result result,
    const char *msg)
{
    AT_ASSERT(parser);

    if (msg) {
        parser->tx((void *)msg, strlen(msg), parser->arg);
        parser->tx("\r\n", 2, parser->arg);
    }
    parser->tx((void *)at_resp_result_str[result], strlen(at_resp_result_str[result]), parser->arg);
    parser->tx("\r\n", 2, parser->arg);
    parser->wait_sync_resp = 0;
    parser->enable_read(1, parser->arg);
}

void at_async_response(struct at_parser *parser, const char *msg)
{
    AT_ASSERT(parser);
    AT_ASSERT(msg);

    parser->tx((void *)msg, strlen(msg), parser->arg);
    parser->tx("\r\n", 2, parser->arg);
}
