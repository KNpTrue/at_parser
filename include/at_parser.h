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
#ifndef __AT_PARSER_H_
#define __AT_PARSER_H_

#include <at_param.h>

/**
 * Response result.
*/
enum at_resp_result {
    AT_RESP_OK,     /**< OK */
    AT_RESP_ERROR,  /**< ERROR */
};

/**
 * AT command type.
*/
enum at_cmd_type {
    AT_CMD_NONE,    /**< NONE */
    AT_CMD_SET,     /**< AT+CMD=1 */
    AT_CMD_TEST,    /**< AT+CMD=? */
    AT_CMD_READ,    /**< AT+CMD? */
    AT_CMD_EXE,     /**< AT+CMD */
};

struct at_parser_config {
    unsigned char echo;

    unsigned int (*tx)(void *data, unsigned int len, void *arg);
    void (*enable_read)(unsigned char value, void *arg);
};

struct at_parser;

/**
 * New a parser.
 *
 * @param cfg is a pointer to the parser configuration.
 * @param arg is the extra argument in callback.
*/
struct at_parser *at_parser_new(struct at_parser_config *cfg, void *arg);

/**
 * Free a parser.
 *
 * @param parser is a pointer to the parser.
*/
void at_parser_free(struct at_parser *parser);

/**
 * Set echo flag.
 *
 * @param parser is a pointer to the parser.
 * @param echo is a boolean, 1 on enable, 0 on disable.
*/
void at_parser_set_echo(struct at_parser *parser, unsigned char echo);

/**
 * Post a char to the parser.
 *
 * @param parser is a pointer to the parser.
 * @param c is a character.
 * @return 0 on success, -1 on failed.
*/
int at_parser_post_char(struct at_parser *parser, int c);

/**
 * Register a command.
 *
 * @param parser is a pointer to the parser.
 * @param cmd is command name.
 * @param handle is a callback called when a command is handled.
 * @return 0 on success, -1 on failed.
*/
int at_cmd_register(struct at_parser *parser, const char *cmd,
    void (*handle)(struct at_parser *parser, const char *cmd, enum at_cmd_type type,
    struct at_param *params, unsigned char count, void *arg));

/**
 * Unregister a command.
 *
 * @param parser is a pointer to the parser.
 * @param cmd is command name.
*/
void at_cmd_unregister(struct at_parser *parser, const char *cmd);

/**
 * Synchronous response.
 *
 * @code
 *   {msg}<CR><LF>
 *   {result}<CR><LF>
 * @endcode
 *
 * @param parser is a pointer to the parser.
 * @param result is response result.
 * @param msg is extra message. 
*/
void at_sync_response(struct at_parser *parser, enum at_resp_result result,
    const char *msg);

/**
 * Asynchronous response.
 *
 * @code
 *   {msg}<CR><LF>
 * @endcode
 *
 * @param parser is a pointer to the parser.
 * @param msg is extra message. 
*/
void at_async_response(struct at_parser *parser, const char *msg);

#endif /* __AT_PARSER_H_ */
