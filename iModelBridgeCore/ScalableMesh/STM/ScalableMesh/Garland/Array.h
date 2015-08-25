/*--------------------------------------------------------------------------------------+
|
|   Code taken from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*---------------------------------------------------------------------------------**//**
*   Array
+---------------+---------------+---------------+---------------+---------------+------*/

#include "TypesAndDefinitions.h"

template<class T> class array
{
    protected:
        T *data;
        int len;
    public:
        array() { data = NULL; len = 0; }
        array(int l) { data = NULL; init(l); }
        ~array() { freeMemory(); }

        inline void init(int l);
        inline void freeMemory();
        inline void resize(int l);

        inline T& ref(int i);
        inline const T& ref(int i) const;

        inline T& operator[](int i);
        inline T& operator()(int i);
        inline const T& operator[](int i) const;
        inline const T& operator()(int i) const;

        inline int length() const { return len; }
        inline int maxLength() const { return len; }
};

template<class T> inline void array<T>::init(int l)
{
    freeMemory();
    data = new T[l];
    len = l;
}

template<class T> inline void array<T>::freeMemory()
{
    if (data)
    {
    delete[] data;
    data = NULL;
    }
};

template<class T> inline T& array<T>::ref(int i)
{
    assert(i >= 0 && i < length());
    return data[i];
};

template<class T> inline const T& array<T>::ref(int i) const
{
    assert(i >= 0 && i < length());
    return data[i];
};

template<class T> inline void array<T>::resize(int l)
{
    if (len > 0)
    {
        T* newArr = new T[l];
        int minV = min(len, l);
        memcpy(newArr, data, minV * sizeof(T));
        len = l;
        delete[] data;
        data = newArr;
    }
    else 
    {
        if (l > 0)
        {
            data = new T[l];
            len = l;
        }       
    } 
}

template<class T> inline T& array<T>::operator[](int i)
{
    assert(i >= 0 && i < length());
    return data[i];
}
template<class T> inline T& array<T>::operator()(int i)
{
    assert(i >= 0 && i < length());
    return ref(i);
}
template<class T> inline const T& array<T>::operator[](int i) const
{
    assert(i >= 0 && i < length());
    return data[i];
}
template<class T> inline const T& array<T>::operator()(int i) const
{
    assert(i >= 0 && i < length());
    return ref(i);
}


