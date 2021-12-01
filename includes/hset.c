/**
 * Version 1.0 October 2021 - simple hashset, based off hashtable
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
#include <stdlib.h>

#include "hmap.h"
#include "hset.h"

hset *hsetCreate() {
	hset *hs;

	if ((hs = malloc(sizeof(hset))) == NULL)
		return NULL;

	if ((hs->hm = hmapCreate()) == NULL) {
		free(hs);
		return 0;
	}

	// A set does not need values
	hs->hm->type->freevalue = NULL;

	return hs;
}

int hsetAdd(hset *hs, void *value) {
	return hmapAdd(hs->hm, value, NULL);
}

int hsetHas(hset *hs, void *value) {
	return hmapContains(hs->hm, value);
}

int hsetDelete(hset *hs, void *value) {
	return hmapDelete(hs->hm, value);
}

void hsetRelease(hset *hs) {
	if (hs) {
		hmapRelease(hs->hm);
		free(hs);
	}
}
