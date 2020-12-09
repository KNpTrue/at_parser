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
#ifndef __AT_PARAM_H_
#define __AT_PARAM_H_

/**
 * AT parameter type.
*/
enum at_param_type {
    AT_PARAM_TYPE_EMTPY,    /**< empty */
    AT_PARAM_TYPE_UNKNOWN,  /**< unknown */
    AT_PARAM_TYPE_STRING,   /**< string */
};

/**
 * The argument of the command.
*/
struct at_param {
    enum at_param_type type;
    char *raw;  /**< raw string */
};

/**
 * Parse AT command parameters.
 *
 * @param s is a pointer to C style string.
 * @param param is an array to store parameters after parsing.
 * @param size is the size of the array.
 * @return the count of parameters, -1 on error.
*/
int at_param_parse(char *s, struct at_param *params, int size);

/**
 * Get C style string from a string parameter.
 *
 * @param param is a pointer to the parameter.
 * @return is the C style string.
*/
const char *at_param_str(struct at_param *param);

#endif /* __AT_PARAM_H_ */
