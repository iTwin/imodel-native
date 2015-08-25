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

template<class T> class array3
{
    protected:
        T *data;
        int w, h, d;

    public:
        array3() { data = NULL; w = h = d = 0; }
        array3(int w, int h, int d) { init(w, h, d); }
        ~array3() { freeMemory(); }

        inline void init(int w, int h, int d);
        inline void freeMemory();

        inline T& ref(int i, int j, int k);
        inline T& operator()(int i, int j, int k) { return ref(i, j, k); }

        inline int width() const { return w; }
        inline int height() const { return h; }
        inline int depth() const { return d; }
};

template<class T> inline void array3<T>::init(int width, int height, int depth)
{
    w = width;
    h = height;
    d = depth;
    data = new T[w*h*d];
};

template<class T> inline void array3<T>::freeMemory()
{
    if (data)
    {
        delete[] data;
        data = NULL;
        w = h = d = 0;
    }
};

template<class T> inline T& array3<T>::ref(int i, int j, int k)
{
    return data[k*w*h + j*w + i];
};