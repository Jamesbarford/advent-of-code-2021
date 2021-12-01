/**
 * Version 1.0 October 2021 - string library
 *
 * Copyright (c) 2021, James Barford-Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __CSTR_H__
#define __CSTR_H__

#define CSTR_ERR  0
#define CSTR_OK   1
#define CSTR_PAD sizeof(int) + 1

typedef char cstr;

void cstrRelease(void *str);
void cstrArrayRelease(cstr **arr, int count);
void cstrSetLen(cstr *str, unsigned int len);
unsigned int cstrlen(cstr *str);

/**
 * The first 5 bytes are reserved. 4 for an integer, with the 5th being
 * a null terminator.
 * Resultant buffer looks like this:
 *   LEN     LEN      LEN     LEN    END   STRING    END
 * ['0xFF', '0xFF', '0xFF', '0xFF', '\0', 'a', 'b', '\0'];
 *
 * With the pointer moved forward twice to the start of the string. OR all
 * of the LEN properties together to get the length of the string;
 */
cstr *cstrEmpty(unsigned int len);
cstr *cstrCreate(char *tmp, unsigned int len);
cstr *cstrdup(cstr *original);
cstr **cstrSplit(char *to_split, char delimiter, int *count);
cstr *cstrCopyUntil(char *original, char terminator);
cstr **cstrCastArray(char *original);
int cstrToString(cstr *str, char *outbuf, unsigned int outbuflen);

cstr *cstrcatcstr(cstr *s1, cstr *s2);
cstr *cstrcat(cstr *str, char *str2);

cstr *cstrjoin(char **arr, unsigned int arrlen, char *delimiter);
cstr *cstrjoincstr(cstr **arr, unsigned int arrlen, char *delimiter);

#endif
