/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// TEMPLATIZED class to emulate omdlVArray indexing in bvector<T>
//
//

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
template <typename T>
class VArrayWrapper
{
public:
static void init (bvector<T>*pArray, int s)
    {
    pArray->clear ();
    }

static void releaseMem (bvector<T>*pArray)
    {
    bvector<T>().swap (*pArray);
    }

static void empty (bvector<T>*pArray)
    {
    pArray->clear ();
    }

static int getCount (bvector<T> const *pArray)
    {
    return (int)pArray->size ();
    }

static T *getPtr (bvector<T> *pArray, int index)
    {
    if (NULL == pArray)
        return NULL;
    size_t n = pArray->size ();
    if (n == 0)
        return NULL;
    if (index < 0)
        return &pArray->at(n - 1);
    if ((size_t)index >= n)
        return NULL;    
    return &pArray->at((size_t)index);
    }

static T const *getConstPtr (bvector<T> const *pArray, int index)
    {
    if (NULL == pArray)
        return NULL;
    size_t n = pArray->size ();
    if (n == 0)
        return NULL;

    if (index < 0)
        return &pArray->at(n - 1);
    if ((size_t)index >= n)
        return NULL;    
    return &pArray->at((size_t)index);
    }

static StatusInt get (bvector<T> const *pArray, T *value, int index)
    {
    if (NULL == pArray)
        return ERROR;
    size_t n = pArray->size ();
    if (n == 0)
        return ERROR;
    if (index < 0)
        {
        *value = pArray->at(n - 1);
        return SUCCESS;
        }
    size_t k = (size_t)index;
    if (k < n)
        {
        *value = pArray->at(k);
        return SUCCESS;
        }
    return ERROR;
    }


static void add (bvector<T> *pArray, T value)
    {
    pArray->push_back (value);
    }

static StatusInt insert (bvector<T> *pArray, T const * value, int index)
    {
    if (pArray == NULL)
        return ERROR;
    if (index < 0)
        pArray->push_back (*value);
    else
        {
        size_t k = (size_t)index;
        size_t n = pArray->size ();
        if (k < n)
            {
            pArray->insert (pArray->begin() + k, *value);
            }
        else
            {
            T nullPoint;
            memset (&nullPoint, 0, sizeof (nullPoint));
            for (size_t k0 = n; k0 < k; k0++)
                pArray->push_back (nullPoint);
            pArray->push_back (*value);
            }
        }
    return SUCCESS;
    }

static StatusInt insert (bvector<T> *pArray, T const * value, int index, int count)
    {
    if (pArray == NULL)
        return ERROR;
    if (count <= 0)
        return SUCCESS;
    size_t n = pArray->size ();
    if (index < 0 || (size_t)index == n)
        {
        for (int i = 0; i < count; i++)
            pArray->push_back (value[i]);
        }
    else if ((size_t)index > n)
        {
        T zero;
        memset (&zero, 0, sizeof (T));
        for (int i = 0; i < index; i++)
            pArray->push_back (zero);
        for (int i = 0; i < count; i++)
            pArray->push_back (value[i]);
        }
    else // Strict internal insertion ...
        {
        size_t k = (size_t)index;
		pArray->insert (pArray->begin() + k, value, &value[count]);
        }
    return SUCCESS;
    }

static int getArray (bvector<T> const *pArray, T * value, int id, int count)
    {
    size_t n = pArray->size ();
    if (!pArray || count <= 0 || (size_t)id >= n)
        {
        count = 0;
        }
    else
        {
        size_t baseIndex = 0;
        if (id >= 0)
            baseIndex = (size_t)id;
        else if (id < 0 && (size_t)count <= n)
            baseIndex = n - (size_t)count;

        size_t maxToGet = n - baseIndex;
        size_t numToGet = (size_t) count;
        if (maxToGet < numToGet)
            numToGet = maxToGet;
        for (size_t i = 0; i < numToGet; i++)
            value[i] = pArray->at (baseIndex + i);
        }
    return count;
    }

static int dropRange (bvector<T> *pArray, int id, int count)
    {
    size_t n = pArray->size ();
    if (!pArray || count <= 0 || (size_t)id >= n)
        {
        count = 0;
        }
    else
        {
        size_t baseIndex = 0;
        if (id >= 0)
            baseIndex = (size_t)id;
        else if (id < 0 && (size_t)count <= n)
            baseIndex = n - (size_t)count;

        size_t maxToGet = n - baseIndex;
        size_t numToGet = (size_t) count;
        if (maxToGet < numToGet)
            numToGet = maxToGet;
        pArray->erase (pArray->begin () + baseIndex, pArray->begin () + baseIndex + numToGet);
        }
    return count;
    }



static void trim (bvector<T> *pArray, int n)
    {
    if (NULL == pArray)
        {
        }
    else if (n <= 0)
        {
        pArray->clear ();
        }
    else
        {
        size_t m = pArray->size ();
        if ((size_t)n < m)
            pArray->resize ((size_t)n);
        }
    }

static StatusInt set (bvector<T> *pArray, T const * value, int index)
    {
    if (pArray == NULL)
        return ERROR;
    size_t n = pArray->size ();

    if (index < 0 || (size_t)index >= n)
        {
        insert (pArray, value, index);
        }
    else    // index is valid for simple assignment ...
        {
        pArray->at((size_t)index) = *value;
        }
    return SUCCESS;
    }

static bool    pop   // false if stack empty
(
bvector<T>*pHeader,               // => array header
T *  value// <= buffer containing one item
)
    {
    if (!pHeader)
        return  false;

    size_t n = pHeader->size ();
    if (n == 0)
        return false;
    *value = pHeader->at(n - 1);
    pHeader->resize (n-1);
    
    return  true;
    }

static StatusInt setBufferSize
(
bvector<T>*pHeader,
int newSize
)
    {
    if (!pHeader)
        return  ERROR;
    if (newSize <= 0)
        return SUCCESS;
    if ((size_t)newSize > pHeader->size ())
        pHeader->reserve ((size_t)newSize);
    else if ((size_t)newSize < pHeader->size ())
        pHeader->resize ((size_t)newSize);
    return SUCCESS;
    }

static StatusInt append   // <= SUCCESS or ERROR
(
bvector<T>          *pDest,    // <=> destination array.
bvector<T> const    *pSource   // => array header.
)
    {
    if (!pDest || !pSource )
        {
        return ERROR;
        }
    size_t nSource = pSource->size ();
    for (size_t i = 0; i < nSource; i++)
        pDest->push_back (pSource->at(i));
    return SUCCESS;
    }

static bool    booleanCopy
(
bvector<T>    *pDest,    // <=> destination array.
bvector<T> const    *pSource   // => array header.
)
    {
    if (NULL != pDest)
        pDest->clear ();
    return SUCCESS == append (pDest, pSource);
    }

static StatusInt copy
(
bvector<T>    *pDest,    // <=> destination array.
bvector<T> const    *pSource   // => array header.
)
    {
    if (NULL != pDest && NULL != pSource)
        {
        *pDest = *pSource;
        return SUCCESS;
        }
    return ERROR;
    }


static T* getNewBlock
(
bvector<T>    *pDest,    // <=> destination array.
int n
)
    {
    if (n <= 0 || NULL == pDest)
        return NULL;
    size_t n0 = pDest->size ();
    size_t n1 = n0 + (size_t)n;
    T nullPoint;
    memset (&nullPoint, 0, sizeof (nullPoint));
    for (size_t i = n0; i < n1; i++)
        pDest->push_back (nullPoint);
    return &pDest->at (n0);
    }

static void reverse (bvector<T> *pArray)
    {
    if (NULL != pArray)
        {
        size_t i0 = 0;
        size_t i1 = pArray->size () - 1;
        for (;i0 < i1; i0++, i1--)
            {
            T a = pArray->at (i0);
            pArray->at(i0) = pArray->at (i1);
            pArray->at(i1) = a;
            }
        }
    }

static StatusInt swapValues(bvector<T> *pArray, int index0, int index1)
    {
    T value0, value1;
    if (   SUCCESS == get (pArray, &value0, index0)
        && SUCCESS == get (pArray, &value1, index1)
        && SUCCESS == set (pArray, &value1, index0)
        && SUCCESS == set (pArray, &value0, index1))
        return SUCCESS;
    return ERROR;
    }

static StatusInt moveItem
    (
    bvector<T> *pDest, int destIndex,
    bvector<T> const *pSource, int sourceIndex
    )
    {
    T value;
    if (SUCCESS == get (pSource, &value, sourceIndex)
        && SUCCESS == set (pDest, &value, destIndex))
        return SUCCESS;
    return ERROR;        
    }
};

END_BENTLEY_GEOMETRY_NAMESPACE
