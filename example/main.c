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
#include <stdio.h>
#include <termios.h>
#include <at_parser.h>

static unsigned int at_parser_tx(void *data, unsigned int len, void *arg)
{
    int i, ret;
    for (i = 0; i < len; i++) {
        putchar(((unsigned char *)data)[i]);
    }
    fflush(stdout);
}

static void at_parser_enable_read(unsigned char value, void *arg)
{
    unsigned char *enable_read = arg;
    *enable_read = value;
}

static struct at_parser_config cfg = {
    .echo = 1,
    .tx = at_parser_tx,
    .enable_read = at_parser_enable_read,
};

void at_test_cmd_handle(struct at_parser *parser, const char *cmd, enum at_cmd_type type,
    struct at_cmd_param *params, unsigned char count, void *arg)
{
    int i;
    switch (type) {
    case AT_CMD_EXE:
        at_sync_response(parser, AT_RESP_OK, "TEST: exec");
        break;
    case AT_CMD_TEST:
        at_sync_response(parser, AT_RESP_OK, "TEST: test");
        break;
    case AT_CMD_SET:
        at_sync_response(parser, AT_RESP_OK, NULL);
        for (i = 0; i < count; i++) {
            at_async_response(parser, params[i].raw);
        }
        break;
    case AT_CMD_READ:
        at_sync_response(parser, AT_RESP_OK, "TEST: read");
        break;
    }
}

int main(int argc, char *argv[])
{
    int c;
    int i;
    char buf[BUFSIZ];
    unsigned int pos = 0;
    unsigned char enable_read = 0;
    struct termios term, new_term;

	tcgetattr(fileno(stdin), &term);
	new_term = term;
	new_term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(stdin), TCSANOW, &new_term);

    struct at_parser *p = at_parser_new(&cfg, &enable_read);
    if (!p) {
        return -1;
    }

    at_cmd_register(p, "+TEST", at_test_cmd_handle);

    while (1) {
        c = getchar();
        if (enable_read) {
            if (pos) {
                for (i = 0; i < pos; i++) {
                    at_parser_post_char(p, buf[i]);
                }
                pos = 0;
            }
            at_parser_post_char(p, c);
        } else {
            buf[pos++] = c;
        }
    }
    return 0;
}
