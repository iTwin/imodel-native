/*
 * dict.c: dictionary of reusable strings, just used to avoid allocation
 *         and freeing operations.
 *
 * Copyright (C) 2003-2012 Daniel Veillard.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 * Author: daniel@veillard.com
 */

#define IN_LIBXML
#include "libxml.h"

#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include "private/dict.h"
#include "private/threads.h"

/*
 * Following http://www.ocert.org/advisories/ocert-2011-003.html
 * it seems that having hash randomization might be a good idea
 * when using XML with untrusted data
 * Note1: that it works correctly only if compiled with WITH_BIG_KEY
 *  which is the default.
 * Note2: the fast function used for a small dict won't protect very
 *  well but since the attack is based on growing a very big hash
 *  list we will use the BigKey algo as soon as the hash size grows
 *  over MIN_DICT_SIZE so this actually works
 */
#if !defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
#define DICT_RANDOMIZATION
#endif

#include <string.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#elif defined(_WIN32)
typedef unsigned __int32 uint32_t;
#endif
#endif
#include <libxml/tree.h>
#include <libxml/dict.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>
#include <libxml/globals.h>

/* #define DEBUG_GROW */
/* #define DICT_DEBUG_PATTERNS */

#define MAX_HASH_LEN 3
#define MIN_DICT_SIZE 128
#define MAX_DICT_HASH 8 * 2048
#define WITH_BIG_KEY

#ifdef WITH_BIG_KEY
#define xmlDictComputeKey(dict, name, len)                              \
    (((dict)->size == MIN_DICT_SIZE) ?                                  \
     xmlDictComputeFastKey(name, len, (dict)->seed) :                   \
     xmlDictComputeBigKey(name, len, (dict)->seed))

#define xmlDictComputeQKey(dict, prefix, plen, name, len)               \
    (((prefix) == NULL) ?                                               \
      (xmlDictComputeKey(dict, name, len)) :                             \
      (((dict)->size == MIN_DICT_SIZE) ?                                \
       xmlDictComputeFastQKey(prefix, plen, name, len, (dict)->seed) :	\
       xmlDictComputeBigQKey(prefix, plen, name, len, (dict)->seed)))

#else /* !WITH_BIG_KEY */
#define xmlDictComputeKey(dict, name, len)                              \
        xmlDictComputeFastKey(name, len, (dict)->seed)
#define xmlDictComputeQKey(dict, prefix, plen, name, len)               \
        xmlDictComputeFastQKey(prefix, plen, name, len, (dict)->seed)
#endif /* WITH_BIG_KEY */

/*
 * An entry in the dictionary
 */
typedef struct _xmlDictEntry xmlDictEntry;
typedef xmlDictEntry *xmlDictEntryPtr;
struct _xmlDictEntry {
    struct _xmlDictEntry *next;
    const xmlChar *name;
    unsigned int len;
    int valid;
    unsigned long okey;
};

typedef struct _xmlDictStrings xmlDictStrings;
typedef xmlDictStrings *xmlDictStringsPtr;
struct _xmlDictStrings {
    xmlDictStringsPtr next;
    xmlChar *free;
    xmlChar *end;
    size_t size;
    size_t nbStrings;
    xmlChar array[1];
};
/*
 * The entire dictionary
 */
struct _xmlDict {
    int ref_counter;

    struct _xmlDictEntry *dict;
    size_t size;
    unsigned int nbElems;
    xmlDictStringsPtr strings;

    struct _xmlDict *subdict;
    /* used for randomization */
    int seed;
    /* used to impose a limit on size */
    size_t limit;
};

/*
 * A mutex for modifying the reference counter for shared
 * dictionaries.
 */
static xmlMutex xmlDictMutex;

#ifdef DICT_RANDOMIZATION
#ifdef HAVE_RAND_R
/*
 * Internal data for random function, protected by xmlDictMutex
 */
static unsigned int rand_seed = 0;
#endif
#endif

/**
 * xmlInitializeDict:
 *
 * DEPRECATED: Alias for xmlInitParser.
 */
int xmlInitializeDict(void) {
    xmlInitParser();
    return(0);
}

/**
 * __xmlInitializeDict:
 *
 * This function is not public
 * Do the dictionary mutex initialization.
 */
int __xmlInitializeDict(void) {
    xmlInitMutex(&xmlDictMutex);

#ifdef DICT_RANDOMIZATION
#ifdef HAVE_RAND_R
    rand_seed = time(NULL);
    rand_r(& rand_seed);
#else
    srand(time(NULL));
#endif
#endif
    return(1);
}

#ifdef DICT_RANDOMIZATION
int __xmlRandom(void) {
    int ret;

    xmlMutexLock(&xmlDictMutex);
#ifdef HAVE_RAND_R
    ret = rand_r(& rand_seed);
#else
    ret = rand();
#endif
    xmlMutexUnlock(&xmlDictMutex);
    return(ret);
}
#endif

/**
 * xmlDictCleanup:
 *
 * DEPRECATED: This function is a no-op. Call xmlCleanupParser
 * to free global state but see the warnings there. xmlCleanupParser
 * should be only called once at program exit. In most cases, you don't
 * have call cleanup functions at all.
 */
void
xmlDictCleanup(void) {
}

/**
 * xmlCleanupDictInternal:
 *
 * Free the dictionary mutex.
 */
void
xmlCleanupDictInternal(void) {
    xmlCleanupMutex(&xmlDictMutex);
}

/*
 * xmlDictAddString:
 * @dict: the dictionary
 * @name: the name of the userdata
 * @len: the length of the name
 *
 * Add the string to the array[s]
 *
 * Returns the pointer of the local string, or NULL in case of error.
 */
static const xmlChar *
xmlDictAddString(xmlDictPtr dict, const xmlChar *name, unsigned int namelen) {
    xmlDictStringsPtr pool;
    const xmlChar *ret;
    size_t size = 0; /* + sizeof(_xmlDictStrings) == 1024 */
    size_t limit = 0;

#ifdef DICT_DEBUG_PATTERNS
    fprintf(stderr, "-");
#endif
    pool = dict->strings;
    while (pool != NULL) {
	if ((size_t)(pool->end - pool->free) > namelen)
	    goto found_pool;
	if (pool->size > size) size = pool->size;
        limit += pool->size;
	pool = pool->next;
    }
    /*
     * Not found, need to allocate
     */
    if (pool == NULL) {
        if ((dict->limit > 0) && (limit > dict->limit)) {
            return(NULL);
        }

        if (size == 0) size = 1000;
	else size *= 4; /* exponential growth */
        if (size < 4 * namelen)
	    size = 4 * namelen; /* just in case ! */
	pool = (xmlDictStringsPtr) xmlMalloc(sizeof(xmlDictStrings) + size);
	if (pool == NULL)
	    return(NULL);
	pool->size = size;
	pool->nbStrings = 0;
	pool->free = &pool->array[0];
	pool->end = &pool->array[size];
	pool->next = dict->strings;
	dict->strings = pool;
#ifdef DICT_DEBUG_PATTERNS
        fprintf(stderr, "+");
#endif
    }
found_pool:
    ret = pool->free;
    memcpy(pool->free, name, namelen);
    pool->free += namelen;
    *(pool->free++) = 0;
    pool->nbStrings++;
    return(ret);
}

/*
 * xmlDictAddQString:
 * @dict: the dictionary
 * @prefix: the prefix of the userdata
 * @plen: the prefix length
 * @name: the name of the userdata
 * @len: the length of the name
 *
 * Add the QName to the array[s]
 *
 * Returns the pointer of the local string, or NULL in case of error.
 */
static const xmlChar *
xmlDictAddQString(xmlDictPtr dict, const xmlChar *prefix, unsigned int plen,
                 const xmlChar *name, unsigned int namelen)
{
    xmlDictStringsPtr pool;
    const xmlChar *ret;
    size_t size = 0; /* + sizeof(_xmlDictStrings) == 1024 */
    size_t limit = 0;

    if (prefix == NULL) return(xmlDictAddString(dict, name, namelen));

#ifdef DICT_DEBUG_PATTERNS
    fprintf(stderr, "=");
#endif
    pool = dict->strings;
    while (pool != NULL) {
	if ((size_t)(pool->end - pool->free) > namelen + plen + 1)
	    goto found_pool;
	if (pool->size > size) size = pool->size;
        limit += pool->size;
	pool = pool->next;
    }
    /*
     * Not found, need to allocate
     */
    if (pool == NULL) {
        if ((dict->limit > 0) && (limit > dict->limit)) {
            return(NULL);
        }

        if (size == 0) size = 1000;
	else size *= 4; /* exponential growth */
        if (size < 4 * (namelen + plen + 1))
	    size = 4 * (namelen + plen + 1); /* just in case ! */
	pool = (xmlDictStringsPtr) xmlMalloc(sizeof(xmlDictStrings) + size);
	if (pool == NULL)
	    return(NULL);
	pool->size = size;
	pool->nbStrings = 0;
	pool->free = &pool->array[0];
	pool->end = &pool->array[size];
	pool->next = dict->strings;
	dict->strings = pool;
#ifdef DICT_DEBUG_PATTERNS
        fprintf(stderr, "+");
#endif
    }
found_pool:
    ret = pool->free;
    memcpy(pool->free, prefix, plen);
    pool->free += plen;
    *(pool->free++) = ':';
    memcpy(pool->free, name, namelen);
    pool->free += namelen;
    *(pool->free++) = 0;
    pool->nbStrings++;
    return(ret);
}

#ifdef WITH_BIG_KEY
/*
 * xmlDictComputeBigKey:
 *
 * Calculate a hash key using a good hash function that works well for
 * larger hash table sizes.
 *
 * Hash function by "One-at-a-Time Hash" see
 * http://burtleburtle.net/bob/hash/doobs.html
 */

#ifdef __clang__
ATTRIBUTE_NO_SANITIZE("unsigned-integer-overflow")
ATTRIBUTE_NO_SANITIZE("unsigned-shift-base")
#endif
static uint32_t
xmlDictComputeBigKey(const xmlChar* data, int namelen, int seed) {
    uint32_t hash;
    int i;

    if (namelen <= 0 || data == NULL) return(0);

    hash = seed;

    for (i = 0;i < namelen; i++) {
        hash += data[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

/*
 * xmlDictComputeBigQKey:
 *
 * Calculate a hash key for two strings using a good hash function
 * that works well for larger hash table sizes.
 *
 * Hash function by "One-at-a-Time Hash" see
 * http://burtleburtle.net/bob/hash/doobs.html
 *
 * Neither of the two strings must be NULL.
 */
#ifdef __clang__
ATTRIBUTE_NO_SANITIZE("unsigned-integer-overflow")
ATTRIBUTE_NO_SANITIZE("unsigned-shift-base")
#endif
static unsigned long
xmlDictComputeBigQKey(const xmlChar *prefix, int plen,
                      const xmlChar *name, int len, int seed)
{
    uint32_t hash;
    int i;

    hash = seed;

    for (i = 0;i < plen; i++) {
        hash += prefix[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }
    hash += ':';
    hash += (hash << 10);
    hash ^= (hash >> 6);

    for (i = 0;i < len; i++) {
        hash += name[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
#endif /* WITH_BIG_KEY */

/*
 * xmlDictComputeFastKey:
 *
 * Calculate a hash key using a fast hash function that works well
 * for low hash table fill.
 */
static unsigned long
xmlDictComputeFastKey(const xmlChar *name, int namelen, int seed) {
    unsigned long value = seed;

    if ((name == NULL) || (namelen <= 0))
        return(value);
    value += *name;
    value <<= 5;
    if (namelen > 10) {
        value += name[namelen - 1];
        namelen = 10;
    }
    switch (namelen) {
        case 10: value += name[9];
        /* Falls through. */
        case 9: value += name[8];
        /* Falls through. */
        case 8: value += name[7];
        /* Falls through. */
        case 7: value += name[6];
        /* Falls through. */
        case 6: value += name[5];
        /* Falls through. */
        case 5: value += name[4];
        /* Falls through. */
        case 4: value += name[3];
        /* Falls through. */
        case 3: value += name[2];
        /* Falls through. */
        case 2: value += name[1];
        /* Falls through. */
        default: break;
    }
    return(value);
}

/*
 * xmlDictComputeFastQKey:
 *
 * Calculate a hash key for two strings using a fast hash function
 * that works well for low hash table fill.
 *
 * Neither of the two strings must be NULL.
 */
static unsigned long
xmlDictComputeFastQKey(const xmlChar *prefix, int plen,
                       const xmlChar *name, int len, int seed)
{
    unsigned long value = seed;

    if (plen == 0)
	value += 30 * ':';
    else
	value += 30 * (*prefix);

    if (len > 10) {
        int offset = len - (plen + 1 + 1);
	if (offset < 0)
	    offset = len - (10 + 1);
	value += name[offset];
        len = 10;
	if (plen > 10)
	    plen = 10;
    }
    switch (plen) {
        case 10: value += prefix[9];
        /* Falls through. */
        case 9: value += prefix[8];
        /* Falls through. */
        case 8: value += prefix[7];
        /* Falls through. */
        case 7: value += prefix[6];
        /* Falls through. */
        case 6: value += prefix[5];
        /* Falls through. */
        case 5: value += prefix[4];
        /* Falls through. */
        case 4: value += prefix[3];
        /* Falls through. */
        case 3: value += prefix[2];
        /* Falls through. */
        case 2: value += prefix[1];
        /* Falls through. */
        case 1: value += prefix[0];
        /* Falls through. */
        default: break;
    }
    len -= plen;
    if (len > 0) {
        value += ':';
	len--;
    }
    switch (len) {
        case 10: value += name[9];
        /* Falls through. */
        case 9: value += name[8];
        /* Falls through. */
        case 8: value += name[7];
        /* Falls through. */
        case 7: value += name[6];
        /* Falls through. */
        case 6: value += name[5];
        /* Falls through. */
        case 5: value += name[4];
        /* Falls through. */
        case 4: value += name[3];
        /* Falls through. */
        case 3: value += name[2];
        /* Falls through. */
        case 2: value += name[1];
        /* Falls through. */
        case 1: value += name[0];
        /* Falls through. */
        default: break;
    }
    return(value);
}

/**
 * xmlDictCreate:
 *
 * Create a new dictionary
 *
 * Returns the newly created dictionary, or NULL if an error occurred.
 */
xmlDictPtr
xmlDictCreate(void) {
    xmlDictPtr dict;

    xmlInitParser();

#ifdef DICT_DEBUG_PATTERNS
    fprintf(stderr, "C");
#endif

    dict = xmlMalloc(sizeof(xmlDict));
    if (dict) {
        dict->ref_counter = 1;
        dict->limit = 0;

        dict->size = MIN_DICT_SIZE;
	dict->nbElems = 0;
        dict->dict = xmlMalloc(MIN_DICT_SIZE * sizeof(xmlDictEntry));
	dict->strings = NULL;
	dict->subdict = NULL;
        if (dict->dict) {
	    memset(dict->dict, 0, MIN_DICT_SIZE * sizeof(xmlDictEntry));
#ifdef DICT_RANDOMIZATION
            dict->seed = __xmlRandom();
#else
            dict->seed = 0;
#endif
	    return(dict);
        }
        xmlFree(dict);
    }
    return(NULL);
}

/**
 * xmlDictCreateSub:
 * @sub: an existing dictionary
 *
 * Create a new dictionary, inheriting strings from the read-only
 * dictionary @sub. On lookup, strings are first searched in the
 * new dictionary, then in @sub, and if not found are created in the
 * new dictionary.
 *
 * Returns the newly created dictionary, or NULL if an error occurred.
 */
xmlDictPtr
xmlDictCreateSub(xmlDictPtr sub) {
    xmlDictPtr dict = xmlDictCreate();

    if ((dict != NULL) && (sub != NULL)) {
#ifdef DICT_DEBUG_PATTERNS
        fprintf(stderr, "R");
#endif
        dict->seed = sub->seed;
        dict->subdict = sub;
	xmlDictReference(dict->subdict);
    }
    return(dict);
}

/**
 * xmlDictReference:
 * @dict: the dictionary
 *
 * Increment the reference counter of a dictionary
 *
 * Returns 0 in case of success and -1 in case of error
 */
int
xmlDictReference(xmlDictPtr dict) {
    if (dict == NULL) return -1;
    xmlMutexLock(&xmlDictMutex);
    dict->ref_counter++;
    xmlMutexUnlock(&xmlDictMutex);
    return(0);
}

/**
 * xmlDictGrow:
 * @dict: the dictionary
 * @size: the new size of the dictionary
 *
 * resize the dictionary
 *
 * Returns 0 in case of success, -1 in case of failure
 */
static int
xmlDictGrow(xmlDictPtr dict, size_t size) {
    unsigned long key, okey;
    size_t oldsize, i;
    xmlDictEntryPtr iter, next;
    struct _xmlDictEntry *olddict;
#ifdef DEBUG_GROW
    unsigned long nbElem = 0;
#endif
    int ret = 0;
    int keep_keys = 1;

    if (dict == NULL)
	return(-1);
    if (size < 8)
        return(-1);
    if (size > 8 * 2048)
	return(-1);

#ifdef DICT_DEBUG_PATTERNS
    fprintf(stderr, "*");
#endif

    oldsize = dict->size;
    olddict = dict->dict;
    if (olddict == NULL)
        return(-1);
    if (oldsize == MIN_DICT_SIZE)
        keep_keys = 0;

    dict->dict = xmlMalloc(size * sizeof(xmlDictEntry));
    if (dict->dict == NULL) {
	dict->dict = olddict;
	return(-1);
    }
    memset(dict->dict, 0, size * sizeof(xmlDictEntry));
    dict->size = size;

    /*	If the two loops are merged, there would be situations where
	a new entry needs to allocated and data copied into it from
	the main dict. It is nicer to run through the array twice, first
	copying all the elements in the main array (less probability of
	allocate) and then the rest, so we only free in the second loop.
    */
    for (i = 0; i < oldsize; i++) {
	if (olddict[i].valid == 0)
	    continue;

	if (keep_keys)
	    okey = olddict[i].okey;
	else
	    okey = xmlDictComputeKey(dict, olddict[i].name, olddict[i].len);
	key = okey % dict->size;

	if (dict->dict[key].valid == 0) {
	    memcpy(&(dict->dict[key]), &(olddict[i]), sizeof(xmlDictEntry));
	    dict->dict[key].next = NULL;
	    dict->dict[key].okey = okey;
	} else {
	    xmlDictEntryPtr entry;

	    entry = xmlMalloc(sizeof(xmlDictEntry));
	    if (entry != NULL) {
		entry->name = olddict[i].name;
		entry->len = olddict[i].len;
		entry->okey = okey;
		entry->next = dict->dict[key].next;
		entry->valid = 1;
		dict->dict[key].next = entry;
	    } else {
	        /*
		 * we don't have much ways to alert from here
		 * result is losing an entry and unicity guarantee
		 */
	        ret = -1;
	    }
	}
#ifdef DEBUG_GROW
	nbElem++;
#endif
    }

    for (i = 0; i < oldsize; i++) {
	iter = olddict[i].next;
	while (iter) {
	    next = iter->next;

	    /*
	     * put back the entry in the new dict
	     */

	    if (keep_keys)
		okey = iter->okey;
	    else
		okey = xmlDictComputeKey(dict, iter->name, iter->len);
	    key = okey % dict->size;
	    if (dict->dict[key].valid == 0) {
		memcpy(&(dict->dict[key]), iter, sizeof(xmlDictEntry));
		dict->dict[key].next = NULL;
		dict->dict[key].valid = 1;
		dict->dict[key].okey = okey;
		xmlFree(iter);
	    } else {
		iter->next = dict->dict[key].next;
		iter->okey = okey;
		dict->dict[key].next = iter;
	    }

#ifdef DEBUG_GROW
	    nbElem++;
#endif

	    iter = next;
	}
    }

    xmlFree(olddict);

#ifdef DEBUG_GROW
    xmlGenericError(xmlGenericErrorContext,
	    "xmlDictGrow : from %lu to %lu, %u elems\n", oldsize, size, nbElem);
#endif

    return(ret);
}

/**
 * xmlDictFree:
 * @dict: the dictionary
 *
 * Free the hash @dict and its contents. The userdata is
 * deallocated with @f if provided.
 */
void
xmlDictFree(xmlDictPtr dict) {
    size_t i;
    xmlDictEntryPtr iter;
    xmlDictEntryPtr next;
    int inside_dict = 0;
    xmlDictStringsPtr pool, nextp;

    if (dict == NULL)
	return;

    /* decrement the counter, it may be shared by a parser and docs */
    xmlMutexLock(&xmlDictMutex);
    dict->ref_counter--;
    if (dict->ref_counter > 0) {
        xmlMutexUnlock(&xmlDictMutex);
        return;
    }

    xmlMutexUnlock(&xmlDictMutex);

    if (dict->subdict != NULL) {
        xmlDictFree(dict->subdict);
    }

    if (dict->dict) {
	for(i = 0; ((i < dict->size) && (dict->nbElems > 0)); i++) {
	    iter = &(dict->dict[i]);
	    if (iter->valid == 0)
		continue;
	    inside_dict = 1;
	    while (iter) {
		next = iter->next;
		if (!inside_dict)
		    xmlFree(iter);
		dict->nbElems--;
		inside_dict = 0;
		iter = next;
	    }
	}
	xmlFree(dict->dict);
    }
    pool = dict->strings;
    while (pool != NULL) {
        nextp = pool->next;
	xmlFree(pool);
	pool = nextp;
    }
    xmlFree(dict);
}

/**
 * xmlDictLookup:
 * @dict: the dictionary
 * @name: the name of the userdata
 * @len: the length of the name, if -1 it is recomputed
 *
 * Add the @name to the dictionary @dict if not present.
 *
 * Returns the internal copy of the name or NULL in case of internal error
 */
const xmlChar *
xmlDictLookup(xmlDictPtr dict, const xmlChar *name, int len) {
    unsigned long key, okey, nbi = 0;
    xmlDictEntryPtr entry;
    xmlDictEntryPtr insert;
    const xmlChar *ret;
    unsigned int l;

    if ((dict == NULL) || (name == NULL))
	return(NULL);

    if (len < 0)
        l = strlen((const char *) name);
    else
        l = len;

    if (((dict->limit > 0) && (l >= dict->limit)) ||
        (l > INT_MAX / 2))
        return(NULL);

    /*
     * Check for duplicate and insertion location.
     */
    okey = xmlDictComputeKey(dict, name, l);
    key = okey % dict->size;
    if (dict->dict[key].valid == 0) {
	insert = NULL;
    } else {
	for (insert = &(dict->dict[key]); insert->next != NULL;
	     insert = insert->next) {
#ifdef __GNUC__
	    if ((insert->okey == okey) && (insert->len == l)) {
		if (!memcmp(insert->name, name, l))
		    return(insert->name);
	    }
#else
	    if ((insert->okey == okey) && (insert->len == l) &&
	        (!xmlStrncmp(insert->name, name, l)))
		return(insert->name);
#endif
	    nbi++;
	}
#ifdef __GNUC__
	if ((insert->okey == okey) && (insert->len == l)) {
	    if (!memcmp(insert->name, name, l))
		return(insert->name);
	}
#else
	if ((insert->okey == okey) && (insert->len == l) &&
	    (!xmlStrncmp(insert->name, name, l)))
	    return(insert->name);
#endif
    }

    if (dict->subdict) {
        unsigned long skey;

        /* we cannot always reuse the same okey for the subdict */
        if (((dict->size == MIN_DICT_SIZE) &&
	     (dict->subdict->size != MIN_DICT_SIZE)) ||
            ((dict->size != MIN_DICT_SIZE) &&
	     (dict->subdict->size == MIN_DICT_SIZE)))
	    skey = xmlDictComputeKey(dict->subdict, name, l);
	else
	    skey = okey;

	key = skey % dict->subdict->size;
	if (dict->subdict->dict[key].valid != 0) {
	    xmlDictEntryPtr tmp;

	    for (tmp = &(dict->subdict->dict[key]); tmp->next != NULL;
		 tmp = tmp->next) {
#ifdef __GNUC__
		if ((tmp->okey == skey) && (tmp->len == l)) {
		    if (!memcmp(tmp->name, name, l))
			return(tmp->name);
		}
#else
		if ((tmp->okey == skey) && (tmp->len == l) &&
		    (!xmlStrncmp(tmp->name, name, l)))
		    return(tmp->name);
#endif
		nbi++;
	    }
#ifdef __GNUC__
	    if ((tmp->okey == skey) && (tmp->len == l)) {
		if (!memcmp(tmp->name, name, l))
		    return(tmp->name);
	    }
#else
	    if ((tmp->okey == skey) && (tmp->len == l) &&
		(!xmlStrncmp(tmp->name, name, l)))
		return(tmp->name);
#endif
	}
	key = okey % dict->size;
    }

    ret = xmlDictAddString(dict, name, l);
    if (ret == NULL)
        return(NULL);
    if (insert == NULL) {
	entry = &(dict->dict[key]);
    } else {
	entry = xmlMalloc(sizeof(xmlDictEntry));
	if (entry == NULL)
	     return(NULL);
    }
    entry->name = ret;
    entry->len = l;
    entry->next = NULL;
    entry->valid = 1;
    entry->okey = okey;


    if (insert != NULL)
	insert->next = entry;

    dict->nbElems++;

    if ((nbi > MAX_HASH_LEN) &&
        (dict->size <= ((MAX_DICT_HASH / 2) / MAX_HASH_LEN))) {
	if (xmlDictGrow(dict, MAX_HASH_LEN * 2 * dict->size) != 0)
	    return(NULL);
    }
    /* Note that entry may have been freed at this point by xmlDictGrow */

    return(ret);
}

/**
 * xmlDictExists:
 * @dict: the dictionary
 * @name: the name of the userdata
 * @len: the length of the name, if -1 it is recomputed
 *
 * Check if the @name exists in the dictionary @dict.
 *
 * Returns the internal copy of the name or NULL if not found.
 */
const xmlChar *
xmlDictExists(xmlDictPtr dict, const xmlChar *name, int len) {
    unsigned long key, okey;
    xmlDictEntryPtr insert;
    unsigned int l;

    if ((dict == NULL) || (name == NULL))
	return(NULL);

    if (len < 0)
        l = strlen((const char *) name);
    else
        l = len;
    if (((dict->limit > 0) && (l >= dict->limit)) ||
        (l > INT_MAX / 2))
        return(NULL);

    /*
     * Check for duplicate and insertion location.
     */
    okey = xmlDictComputeKey(dict, name, l);
    key = okey % dict->size;
    if (dict->dict[key].valid == 0) {
	insert = NULL;
    } else {
	for (insert = &(dict->dict[key]); insert->next != NULL;
	     insert = insert->next) {
#ifdef __GNUC__
	    if ((insert->okey == okey) && (insert->len == l)) {
		if (!memcmp(insert->name, name, l))
		    return(insert->name);
	    }
#else
	    if ((insert->okey == okey) && (insert->len == l) &&
	        (!xmlStrncmp(insert->name, name, l)))
		return(insert->name);
#endif
	}
#ifdef __GNUC__
	if ((insert->okey == okey) && (insert->len == l)) {
	    if (!memcmp(insert->name, name, l))
		return(insert->name);
	}
#else
	if ((insert->okey == okey) && (insert->len == l) &&
	    (!xmlStrncmp(insert->name, name, l)))
	    return(insert->name);
#endif
    }

    if (dict->subdict) {
        unsigned long skey;

        /* we cannot always reuse the same okey for the subdict */
        if (((dict->size == MIN_DICT_SIZE) &&
	     (dict->subdict->size != MIN_DICT_SIZE)) ||
            ((dict->size != MIN_DICT_SIZE) &&
	     (dict->subdict->size == MIN_DICT_SIZE)))
	    skey = xmlDictComputeKey(dict->subdict, name, l);
	else
	    skey = okey;

	key = skey % dict->subdict->size;
	if (dict->subdict->dict[key].valid != 0) {
	    xmlDictEntryPtr tmp;

	    for (tmp = &(dict->subdict->dict[key]); tmp->next != NULL;
		 tmp = tmp->next) {
#ifdef __GNUC__
		if ((tmp->okey == skey) && (tmp->len == l)) {
		    if (!memcmp(tmp->name, name, l))
			return(tmp->name);
		}
#else
		if ((tmp->okey == skey) && (tmp->len == l) &&
		    (!xmlStrncmp(tmp->name, name, l)))
		    return(tmp->name);
#endif
	    }
#ifdef __GNUC__
	    if ((tmp->okey == skey) && (tmp->len == l)) {
		if (!memcmp(tmp->name, name, l))
		    return(tmp->name);
	    }
#else
	    if ((tmp->okey == skey) && (tmp->len == l) &&
		(!xmlStrncmp(tmp->name, name, l)))
		return(tmp->name);
#endif
	}
    }

    /* not found */
    return(NULL);
}

/**
 * xmlDictQLookup:
 * @dict: the dictionary
 * @prefix: the prefix
 * @name: the name
 *
 * Add the QName @prefix:@name to the hash @dict if not present.
 *
 * Returns the internal copy of the QName or NULL in case of internal error
 */
const xmlChar *
xmlDictQLookup(xmlDictPtr dict, const xmlChar *prefix, const xmlChar *name) {
    unsigned long okey, key, nbi = 0;
    xmlDictEntryPtr entry;
    xmlDictEntryPtr insert;
    const xmlChar *ret;
    unsigned int len, plen, l;

    if ((dict == NULL) || (name == NULL))
	return(NULL);
    if (prefix == NULL)
        return(xmlDictLookup(dict, name, -1));

    l = len = strlen((const char *) name);
    plen = strlen((const char *) prefix);
    len += 1 + plen;

    /*
     * Check for duplicate and insertion location.
     */
    okey = xmlDictComputeQKey(dict, prefix, plen, name, l);
    key = okey % dict->size;
    if (dict->dict[key].valid == 0) {
	insert = NULL;
    } else {
	for (insert = &(dict->dict[key]); insert->next != NULL;
	     insert = insert->next) {
	    if ((insert->okey == okey) && (insert->len == len) &&
	        (xmlStrQEqual(prefix, name, insert->name)))
		return(insert->name);
	    nbi++;
	}
	if ((insert->okey == okey) && (insert->len == len) &&
	    (xmlStrQEqual(prefix, name, insert->name)))
	    return(insert->name);
    }

    if (dict->subdict) {
        unsigned long skey;

        /* we cannot always reuse the same okey for the subdict */
        if (((dict->size == MIN_DICT_SIZE) &&
	     (dict->subdict->size != MIN_DICT_SIZE)) ||
            ((dict->size != MIN_DICT_SIZE) &&
	     (dict->subdict->size == MIN_DICT_SIZE)))
	    skey = xmlDictComputeQKey(dict->subdict, prefix, plen, name, l);
	else
	    skey = okey;

	key = skey % dict->subdict->size;
	if (dict->subdict->dict[key].valid != 0) {
	    xmlDictEntryPtr tmp;
	    for (tmp = &(dict->subdict->dict[key]); tmp->next != NULL;
		 tmp = tmp->next) {
		if ((tmp->okey == skey) && (tmp->len == len) &&
		    (xmlStrQEqual(prefix, name, tmp->name)))
		    return(tmp->name);
		nbi++;
	    }
	    if ((tmp->okey == skey) && (tmp->len == len) &&
		(xmlStrQEqual(prefix, name, tmp->name)))
		return(tmp->name);
	}
	key = okey % dict->size;
    }

    ret = xmlDictAddQString(dict, prefix, plen, name, l);
    if (ret == NULL)
        return(NULL);
    if (insert == NULL) {
	entry = &(dict->dict[key]);
    } else {
	entry = xmlMalloc(sizeof(xmlDictEntry));
	if (entry == NULL)
	     return(NULL);
    }
    entry->name = ret;
    entry->len = len;
    entry->next = NULL;
    entry->valid = 1;
    entry->okey = okey;

    if (insert != NULL)
	insert->next = entry;

    dict->nbElems++;

    if ((nbi > MAX_HASH_LEN) &&
        (dict->size <= ((MAX_DICT_HASH / 2) / MAX_HASH_LEN)))
	xmlDictGrow(dict, MAX_HASH_LEN * 2 * dict->size);
    /* Note that entry may have been freed at this point by xmlDictGrow */

    return(ret);
}

/**
 * xmlDictOwns:
 * @dict: the dictionary
 * @str: the string
 *
 * check if a string is owned by the dictionary
 *
 * Returns 1 if true, 0 if false and -1 in case of error
 * -1 in case of error
 */
int
xmlDictOwns(xmlDictPtr dict, const xmlChar *str) {
    xmlDictStringsPtr pool;

    if ((dict == NULL) || (str == NULL))
	return(-1);
    pool = dict->strings;
    while (pool != NULL) {
        if ((str >= &pool->array[0]) && (str <= pool->free))
	    return(1);
	pool = pool->next;
    }
    if (dict->subdict)
        return(xmlDictOwns(dict->subdict, str));
    return(0);
}

/**
 * xmlDictSize:
 * @dict: the dictionary
 *
 * Query the number of elements installed in the hash @dict.
 *
 * Returns the number of elements in the dictionary or
 * -1 in case of error
 */
int
xmlDictSize(xmlDictPtr dict) {
    if (dict == NULL)
	return(-1);
    if (dict->subdict)
        return(dict->nbElems + dict->subdict->nbElems);
    return(dict->nbElems);
}

/**
 * xmlDictSetLimit:
 * @dict: the dictionary
 * @limit: the limit in bytes
 *
 * Set a size limit for the dictionary
 * Added in 2.9.0
 *
 * Returns the previous limit of the dictionary or 0
 */
size_t
xmlDictSetLimit(xmlDictPtr dict, size_t limit) {
    size_t ret;

    if (dict == NULL)
	return(0);
    ret = dict->limit;
    dict->limit = limit;
    return(ret);
}

/**
 * xmlDictGetUsage:
 * @dict: the dictionary
 *
 * Get how much memory is used by a dictionary for strings
 * Added in 2.9.0
 *
 * Returns the amount of strings allocated
 */
size_t
xmlDictGetUsage(xmlDictPtr dict) {
    xmlDictStringsPtr pool;
    size_t limit = 0;

    if (dict == NULL)
	return(0);
    pool = dict->strings;
    while (pool != NULL) {
        limit += pool->size;
	pool = pool->next;
    }
    return(limit);
}

<<<<<<< HEAD
=======
/*****************************************************************
 *
 * The code below was rewritten and is additionally licensed under
 * the main license in file 'Copyright'.
 *
 *****************************************************************/

ATTRIBUTE_NO_SANITIZE_INTEGER
static unsigned
xmlDictHashName(unsigned seed, const xmlChar* data, size_t maxLen,
                size_t *plen) {
    unsigned h1, h2;
    size_t i;

    HASH_INIT(h1, h2, seed);

    for (i = 0; i < maxLen && data[i]; i++) {
        HASH_UPDATE(h1, h2, data[i]);
    }

    HASH_FINISH(h1, h2);

    *plen = i;
    return(h2 | MAX_HASH_SIZE);
}

ATTRIBUTE_NO_SANITIZE_INTEGER
static unsigned
xmlDictHashQName(unsigned seed, const xmlChar *prefix, const xmlChar *name,
                 size_t *pplen, size_t *plen) {
    unsigned h1, h2;
    size_t i;

    HASH_INIT(h1, h2, seed);

    for (i = 0; prefix[i] != 0; i++) {
        HASH_UPDATE(h1, h2, prefix[i]);
    }
    *pplen = i;

    HASH_UPDATE(h1, h2, ':');

    for (i = 0; name[i] != 0; i++) {
        HASH_UPDATE(h1, h2, name[i]);
    }
    *plen = i;

    HASH_FINISH(h1, h2);

    /*
     * Always set the upper bit of hash values since 0 means an unoccupied
     * bucket.
     */
    return(h2 | MAX_HASH_SIZE);
}

/**
 * xmlDictComputeHash:
 * @dict:  dictionary
 * @string:  C string
 *
 * Compute the hash value of a C string.
 *
 * Returns the hash value.
 */
unsigned
xmlDictComputeHash(const xmlDict *dict, const xmlChar *string) {
    size_t len;
    return(xmlDictHashName(dict->seed, string, SIZE_MAX, &len));
}

#define HASH_ROL31(x,n) ((x) << (n) | ((x) & 0x7FFFFFFF) >> (31 - (n)))

/**
 * xmlDictCombineHash:
 * @v1:  first hash value
 * @v2: second hash value
 *
 * Combine two hash values.
 *
 * Returns the combined hash value.
 */
ATTRIBUTE_NO_SANITIZE_INTEGER
unsigned
xmlDictCombineHash(unsigned v1, unsigned v2) {
    /*
     * The upper bit of hash values is always set, so we have to operate on
     * 31-bit hashes here.
     */
    v1 ^= v2;
    v1 += HASH_ROL31(v2, 5);

    return((v1 & 0xFFFFFFFF) | 0x80000000);
}

/**
 * xmlDictFindEntry:
 * @dict: dict
 * @prefix: optional QName prefix
 * @name: string
 * @len: length of string
 * @hashValue: valid hash value of string
 * @pfound: result of search
 *
 * Try to find a matching hash table entry. If an entry was found, set
 * @found to 1 and return the entry. Otherwise, set @found to 0 and return
 * the location where a new entry should be inserted.
 */
ATTRIBUTE_NO_SANITIZE_INTEGER
static xmlDictEntry *
xmlDictFindEntry(const xmlDict *dict, const xmlChar *prefix,
                 const xmlChar *name, int len, unsigned hashValue,
                 int *pfound) {
    xmlDictEntry *entry;
    unsigned mask, pos, displ;
    int found = 0;

    mask = dict->size - 1;
    pos = hashValue & mask;
    entry = &dict->table[pos];

    if (entry->hashValue != 0) {
        /*
         * Robin hood hashing: abort if the displacement of the entry
         * is smaller than the displacement of the key we look for.
         * This also stops at the correct position when inserting.
         */
        displ = 0;

        do {
            if (entry->hashValue == hashValue) {
                if (prefix == NULL) {
                    /*
                     * name is not necessarily null-terminated.
                     */
                    if ((strncmp((const char *) entry->name,
                                 (const char *) name, len) == 0) &&
                        (entry->name[len] == 0)) {
                        found = 1;
                        break;
                    }
                } else {
                    if (xmlStrQEqual(prefix, name, entry->name)) {
                        found = 1;
                        break;
                    }
                }
            }

            displ++;
            pos++;
            entry++;
            if ((pos & mask) == 0)
                entry = dict->table;
        } while ((entry->hashValue != 0) &&
                 (((pos - entry->hashValue) & mask) >= displ));
    }

    *pfound = found;
    return(entry);
}

/**
 * xmlDictGrow:
 * @dict: dictionary
 * @size: new size of the dictionary
 *
 * Resize the dictionary hash table.
 *
 * Returns 0 in case of success, -1 if a memory allocation failed.
 */
static int
xmlDictGrow(xmlDictPtr dict, unsigned size) {
    const xmlDictEntry *oldentry, *oldend, *end;
    xmlDictEntry *table;
    unsigned oldsize, i;

    /* Add 0 to avoid spurious -Wtype-limits warning on 64-bit GCC */
    if ((size_t) size + 0 > SIZE_MAX / sizeof(table[0]))
        return(-1);
    table = xmlMalloc(size * sizeof(table[0]));
    if (table == NULL)
        return(-1);
    memset(table, 0, size * sizeof(table[0]));

    oldsize = dict->size;
    if (oldsize == 0)
        goto done;

    oldend = &dict->table[oldsize];
    end = &table[size];

    /*
     * Robin Hood sorting order is maintained if we
     *
     * - compute dict indices with modulo
     * - resize by an integer factor
     * - start to copy from the beginning of a probe sequence
     */
    oldentry = dict->table;
    while (oldentry->hashValue != 0) {
        if (++oldentry >= oldend)
            oldentry = dict->table;
    }

    for (i = 0; i < oldsize; i++) {
        if (oldentry->hashValue != 0) {
            xmlDictEntry *entry = &table[oldentry->hashValue & (size - 1)];

            while (entry->hashValue != 0) {
                if (++entry >= end)
                    entry = table;
            }
            *entry = *oldentry;
        }

        if (++oldentry >= oldend)
            oldentry = dict->table;
    }

    xmlFree(dict->table);

done:
    dict->table = table;
    dict->size = size;

    return(0);
}

/**
 * xmlDictLookupInternal:
 * @dict: dict
 * @prefix: optional QName prefix
 * @name: string
 * @maybeLen: length of string or -1 if unknown
 * @update: whether the string should be added
 *
 * Internal lookup and update function.
 */
ATTRIBUTE_NO_SANITIZE_INTEGER
static const xmlDictEntry *
xmlDictLookupInternal(xmlDictPtr dict, const xmlChar *prefix,
                      const xmlChar *name, int maybeLen, int update) {
    xmlDictEntry *entry = NULL;
    const xmlChar *ret;
    unsigned hashValue;
    size_t maxLen, len, plen, klen;
    int found = 0;

    if ((dict == NULL) || (name == NULL))
	return(NULL);

    maxLen = (maybeLen < 0) ? SIZE_MAX : (size_t) maybeLen;

    if (prefix == NULL) {
        hashValue = xmlDictHashName(dict->seed, name, maxLen, &len);
        if (len > INT_MAX / 2)
            return(NULL);
        klen = len;
    } else {
        hashValue = xmlDictHashQName(dict->seed, prefix, name, &plen, &len);
        if ((len > INT_MAX / 2) || (plen >= INT_MAX / 2 - len))
            return(NULL);
        klen = plen + 1 + len;
    }

    if ((dict->limit > 0) && (klen >= dict->limit))
        return(NULL);

    /*
     * Check for an existing entry
     */
    if (dict->size > 0)
        entry = xmlDictFindEntry(dict, prefix, name, klen, hashValue, &found);
    if (found)
        return(entry);

    if ((dict->subdict != NULL) && (dict->subdict->size > 0)) {
        xmlDictEntry *subEntry;
        unsigned subHashValue;

        if (prefix == NULL)
            subHashValue = xmlDictHashName(dict->subdict->seed, name, len,
                                           &len);
        else
            subHashValue = xmlDictHashQName(dict->subdict->seed, prefix, name,
                                            &plen, &len);
        subEntry = xmlDictFindEntry(dict->subdict, prefix, name, klen,
                                    subHashValue, &found);
        if (found)
            return(subEntry);
    }

    if (!update)
        return(NULL);

    /*
     * Grow the hash table if needed
     */
    if (dict->nbElems + 1 > dict->size / MAX_FILL_DENOM * MAX_FILL_NUM) {
        unsigned newSize, mask, displ, pos;

        if (dict->size == 0) {
            newSize = MIN_HASH_SIZE;
        } else {
            if (dict->size >= MAX_HASH_SIZE)
                return(NULL);
            newSize = dict->size * 2;
        }
        if (xmlDictGrow(dict, newSize) != 0)
            return(NULL);

        /*
         * Find new entry
         */
        mask = dict->size - 1;
        displ = 0;
        pos = hashValue & mask;
        entry = &dict->table[pos];

        while ((entry->hashValue != 0) &&
               ((pos - entry->hashValue) & mask) >= displ) {
            displ++;
            pos++;
            entry++;
            if ((pos & mask) == 0)
                entry = dict->table;
        }
    }

    if (prefix == NULL)
        ret = xmlDictAddString(dict, name, len);
    else
        ret = xmlDictAddQString(dict, prefix, plen, name, len);
    if (ret == NULL)
        return(NULL);

    /*
     * Shift the remainder of the probe sequence to the right
     */
    if (entry->hashValue != 0) {
        const xmlDictEntry *end = &dict->table[dict->size];
        const xmlDictEntry *cur = entry;

        do {
            cur++;
            if (cur >= end)
                cur = dict->table;
        } while (cur->hashValue != 0);

        if (cur < entry) {
            /*
             * If we traversed the end of the buffer, handle the part
             * at the start of the buffer.
             */
            memmove(&dict->table[1], dict->table,
                    (char *) cur - (char *) dict->table);
            cur = end - 1;
            dict->table[0] = *cur;
        }

        memmove(&entry[1], entry, (char *) cur - (char *) entry);
    }

    /*
     * Populate entry
     */
    entry->hashValue = hashValue;
    entry->name = ret;

    dict->nbElems++;

    return(entry);
}

/**
 * xmlDictLookup:
 * @dict: dictionary
 * @name: string key
 * @len: length of the key, if -1 it is recomputed
 *
 * Lookup a string and add it to the dictionary if it wasn't found.
 *
 * Returns the interned copy of the string or NULL if a memory allocation
 * failed.
 */
const xmlChar *
xmlDictLookup(xmlDictPtr dict, const xmlChar *name, int len) {
    const xmlDictEntry *entry;

    entry = xmlDictLookupInternal(dict, NULL, name, len, 1);
    if (entry == NULL)
        return(NULL);
    return(entry->name);
}

/**
 * xmlDictLookupHashed:
 * @dict: dictionary
 * @name: string key
 * @len: length of the key, if -1 it is recomputed
 *
 * Lookup a dictionary entry and add the string to the dictionary if
 * it wasn't found.
 *
 * Returns the dictionary entry.
 */
xmlHashedString
xmlDictLookupHashed(xmlDictPtr dict, const xmlChar *name, int len) {
    const xmlDictEntry *entry;
    xmlHashedString ret;

    entry = xmlDictLookupInternal(dict, NULL, name, len, 1);

    if (entry == NULL) {
        ret.name = NULL;
        ret.hashValue = 0;
    } else {
        ret = *entry;
    }

    return(ret);
}

/**
 * xmlDictExists:
 * @dict: the dictionary
 * @name: the name of the userdata
 * @len: the length of the name, if -1 it is recomputed
 *
 * Check if a string exists in the dictionary.
 *
 * Returns the internal copy of the name or NULL if not found.
 */
const xmlChar *
xmlDictExists(xmlDictPtr dict, const xmlChar *name, int len) {
    const xmlDictEntry *entry;

    entry = xmlDictLookupInternal(dict, NULL, name, len, 0);
    if (entry == NULL)
        return(NULL);
    return(entry->name);
}

/**
 * xmlDictQLookup:
 * @dict: the dictionary
 * @prefix: the prefix
 * @name: the name
 *
 * Lookup the QName @prefix:@name and add it to the dictionary if
 * it wasn't found.
 *
 * Returns the interned copy of the string or NULL if a memory allocation
 * failed.
 */
const xmlChar *
xmlDictQLookup(xmlDictPtr dict, const xmlChar *prefix, const xmlChar *name) {
    const xmlDictEntry *entry;

    entry = xmlDictLookupInternal(dict, prefix, name, -1, 1);
    if (entry == NULL)
        return(NULL);
    return(entry->name);
}

/*
 * Pseudo-random generator
 */

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <bcrypt.h>
#else
  #if defined(HAVE_GETENTROPY)
    #ifdef HAVE_UNISTD_H
      #include <unistd.h>
    #endif
    #ifdef HAVE_SYS_RANDOM_H
      #include <sys/random.h>
    #endif
  #endif
  #include <time.h>
#endif

static xmlMutex xmlRngMutex;

static unsigned globalRngState[2];

/*
 * xmlInitRandom:
 *
 * Initialize the PRNG.
 */
ATTRIBUTE_NO_SANITIZE_INTEGER
void
xmlInitRandom(void) {
    xmlInitMutex(&xmlRngMutex);

    {
#ifdef _WIN32
        NTSTATUS status;

        status = BCryptGenRandom(NULL, (unsigned char *) globalRngState,
                                 sizeof(globalRngState),
                                 BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (!BCRYPT_SUCCESS(status)) {
            fprintf(stderr, "libxml2: BCryptGenRandom failed with "
                    "error code %lu\n", GetLastError());
            abort();
        }
#else
        int var;

#if defined(HAVE_GETENTROPY)
        while (1) {
            if (getentropy(globalRngState, sizeof(globalRngState)) == 0)
                return;

            /*
             * This most likely means that libxml2 was compiled on
             * a system supporting certain system calls and is running
             * on a system that doesn't support these calls, as can
             * be the case on Linux.
             */
            if (errno == ENOSYS)
                break;

            if (errno != EINTR) {
                fprintf(stderr, "libxml2: getentropy failed with "
                        "error code %d\n", errno);
                abort();
            }
        }
#endif

        globalRngState[0] =
                (unsigned) time(NULL) ^
                HASH_ROL((unsigned) ((size_t) &xmlInitRandom & 0xFFFFFFFF), 8);
        globalRngState[1] =
                HASH_ROL((unsigned) ((size_t) &xmlRngMutex & 0xFFFFFFFF), 16) ^
                HASH_ROL((unsigned) ((size_t) &var & 0xFFFFFFFF), 24);
#endif
    }
}

/*
 * xmlCleanupRandom:
 *
 * Clean up PRNG globals.
 */
void
xmlCleanupRandom(void) {
    xmlCleanupMutex(&xmlRngMutex);
}

ATTRIBUTE_NO_SANITIZE_INTEGER
static unsigned
xoroshiro64ss(unsigned *s) {
    unsigned s0 = s[0];
    unsigned s1 = s[1];
    unsigned result = HASH_ROL(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
    s[0] = HASH_ROL(s0, 26) ^ s1 ^ (s1 << 9);
    s[1] = HASH_ROL(s1, 13);

    return(result & 0xFFFFFFFF);
}

/*
 * xmlGlobalRandom:
 *
 * Generate a pseudo-random value using the global PRNG.
 *
 * Returns a random value.
 */
unsigned
xmlGlobalRandom(void) {
    unsigned ret;

    xmlMutexLock(&xmlRngMutex);
    ret = xoroshiro64ss(globalRngState);
    xmlMutexUnlock(&xmlRngMutex);

    return(ret);
}

/*
 * xmlRandom:
 *
 * Generate a pseudo-random value using the thread-local PRNG.
 *
 * Returns a random value.
 */
unsigned
xmlRandom(void) {
#ifdef LIBXML_THREAD_ENABLED
    return(xoroshiro64ss(xmlGetLocalRngState()));
#else
    return(xmlGlobalRandom());
#endif
}

>>>>>>> 1945ec87 (Update libxml2 to 2.13.6 (#1032))
