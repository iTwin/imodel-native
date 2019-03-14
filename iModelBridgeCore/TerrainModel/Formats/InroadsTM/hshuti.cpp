//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* hshuti.c                                           dgc       04/1994       */
/*----------------------------------------------------------------------------*/
/* Various hashing utilities.                                                 */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

static class HashTable* pThisHashTable = NULL;

static int aecHash_compare(const void*, const void*);

struct my_cmp
{
    bool operator() (void* const a, void* const b) const
    {
        return (aecHash_compare(a,b)<0) ? true : false;
    }
};

class HashTable : public map<void*,void*,my_cmp>
    {
    public:
        int (*pCmpFunc)(const void*, const void*);
    };

/*%-----------------------------------------------------------------------------
FUNC: aecHash_create
DESC:
HIST: original                                               dgc 04/1994
MISC:
KEYW: HASH TABLE CREATE
----------------------------------------------------------------------------%*/

int aecHash_create          /* <= TRUE if error                    */
    (
    void** ppTbl,                        /* <= caller must aecHash_destroy      */
    int (*pCmpFunc)(const void*, const void*)  /* => compare function      */
    )
    {
    *ppTbl = NULL;

    HashTable* pHashTable = new HashTable;
    pHashTable->pCmpFunc = pCmpFunc;

    *ppTbl = pHashTable;

    return SUCCESS;
    }



/*%-----------------------------------------------------------------------------
FUNC: aecHash_destroy
DESC:
HIST: original                                               dgc 04/1994
MISC:
KEYW: HASH TABLE DESTROY
----------------------------------------------------------------------------%*/

void aecHash_destroy
    (
    void* pTbl                           /* => hash table                       */
    )
    {
    if( pTbl )
        {
        HashTable* pHashTable = (HashTable*)pTbl;
        delete pHashTable;
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecHash_insert
DESC:
HIST: original                                               dgc 04/1994
MISC:
KEYW: HASH TABLE INSERT
----------------------------------------------------------------------------%*/

int aecHash_insert          /* <= TRUE if error                    */
    (
    void* pTbl,                          /* => hash table                       */
    void* pEle                           /* => element to insert                */
    )
    {
    if( pTbl )
        {
        HashTable* pHashTable = (HashTable*)pTbl;
        pThisHashTable = pHashTable;
        (*pHashTable)[pEle] = pEle;
        pThisHashTable = NULL;
        }

    return ( SUCCESS );
    }



/*%-----------------------------------------------------------------------------
FUNC: aecHash_find
DESC:
HIST: original                                               dgc 04/1994
MISC:
KEYW: HASH TABLE FIND
----------------------------------------------------------------------------%*/

void* aecHash_find          /* <= found element                    */
    (
    void* pTbl,                          /* => hash table                       */
    void* pKey                           /* => element to find                  */
    )
    {
    void* pEle = NULL;

    if( pTbl )
        {
        HashTable* pHashTable = (HashTable*)pTbl;
        pThisHashTable = pHashTable;
        auto i = pThisHashTable->find (pKey);
        if (i != pThisHashTable->end ())
            pEle = i->second;
        pThisHashTable = NULL;
        }

    return( pEle );
    }


/*%-----------------------------------------------------------------------------
FUNC: aecHash_compare
DESC:
HIST: original                                               dgc 04/1994
MISC:
KEYW: HASH TABLE COMPARE
----------------------------------------------------------------------------%*/

static int aecHash_compare
    (
    const void* p1,                      /* =>                                  */
    const void* p2                       /* =>                                  */
    )
    {
    int value = 0;

    if( pThisHashTable && pThisHashTable->pCmpFunc )
        {
        value = (*pThisHashTable->pCmpFunc)( p1, p2 );
        }

    return( value );
    }
