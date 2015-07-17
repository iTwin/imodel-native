/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/PartitionArray.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once
#include "dtmdefs.h"

#pragma inline_depth(255)
#pragma inline_recursion(on)

class MAllocAllocator
    {
    public:
        static __forceinline void* AllocMemory(size_t size)
            {
            return malloc(size);
            }
        static __forceinline void* ResizeMemory(void* ptr, size_t curSize, size_t newSize)
            {
            return realloc(ptr, newSize);
            }
        static __forceinline void DeleteMemory(void* ptr)
            {
            free(ptr);
            }
    };

template <class TYPE, int shift, class allocator = MAllocAllocator> class PartitionArray
    {
typedef PartitionArray<TYPE, shift, allocator> _Mybase;

    public:
        template <class TYPE> class _iterator : public std::iterator<std::random_access_iterator_tag, TYPE, int, const TYPE*, const TYPE&>
            {
            typedef _iterator<TYPE> _Iterator;
            private:
                mutable TYPE* m_curPtr;
                mutable TYPE* m_startPtr;
                mutable TYPE* m_endPtr;
                _Mybase* m_array;
                mutable long m_curPartition;

                void GetPartitionInfo() const
                    {
                    if(m_curPartition == m_array->m_numPartitions)
                        {
                        m_startPtr = m_endPtr = NULL;
                        return;
                        }
                    m_startPtr = m_array->m_tablePP[m_curPartition];
                    if(m_curPartition < m_array->m_numPartitions)
                        m_endPtr = m_startPtr + m_array->m_partitionSize - 1;
                    else
                        {
                        long remPartition = m_array->m_size & m_array->m_bitmask;

                        if( remPartition == 0 ) { remPartition = m_array->m_partitionSize ; }
                        m_endPtr = m_startPtr + remPartition;
                        }
                    }
            public:
                __forceinline _iterator(){m_curPtr = 0;};
                __forceinline _iterator(_Mybase* theArray, long partition, long subIndex)
                    {
                    m_array = theArray;
                    m_curPartition = partition;
                    GetPartitionInfo();
                    m_curPtr = m_startPtr + subIndex;
                    }

                __forceinline TYPE& operator*()
                    {
                    return *m_curPtr;
                    }
                __forceinline TYPE& operator*() const
                    {
                    return *m_curPtr;
                    }
                __forceinline TYPE& operator[](size_t i)                               // The [] operator function. Returns the TYPE at pos i.
                    {
                    _Iterator newI = *this + i;
                    return *newI;
                    }
                __forceinline TYPE& operator[](size_t i) const                        // The [] operator function. Returns the TYPE at pos i.
                    {
                    _Iterator newI = *this;
                    newI += i;
                    return *newI;
                    }
                __forceinline TYPE* operator->()
                    {
                    return m_curPtr;
                    }
                __forceinline TYPE* operator->() const
                    {
                    return m_curPtr;
                    }
                __forceinline bool operator!=(TYPE* l) const
                    {
                    return m_curPtr != l;
                    }

                __forceinline bool operator==(TYPE* l) const
                    {
                    return m_curPtr == l;
                    }

                __forceinline bool operator<(TYPE* l) const
                    {
                    return m_curPtr < l;
                    }
                __forceinline bool operator<=(TYPE* l) const
                    {
                    return m_curPtr <= l;
                    }
                __forceinline bool operator>(TYPE* l) const
                    {
                    return m_curPtr > l;
                    }
                __forceinline bool operator>=(TYPE* l) const
                    {
                    return m_curPtr >= l;
                    }

                __forceinline bool operator!=(const _Iterator& l) const
                    {
                    return m_curPtr != l.m_curPtr;
                    }

                __forceinline bool operator==(const _Iterator& l) const
                    {
                    return m_curPtr == l.m_curPtr;
                    }

                __forceinline bool operator<(const _Iterator& l) const
                    {
                    if(m_curPartition != l.m_curPartition)
                        return m_curPartition < l.m_curPartition;
                    return m_curPtr < l.m_curPtr;
                    }
                __forceinline bool operator<=(const _Iterator& l) const
                    {
                    if(m_curPartition != l.m_curPartition)
                        return m_curPartition < l.m_curPartition;
                    return m_curPtr <= l.m_curPtr;
                    }
                __forceinline bool operator>(const _Iterator& l) const
                    {
                    if(m_curPartition != l.m_curPartition)
                        return m_curPartition > l.m_curPartition;
                    return m_curPtr > l.m_curPtr;
                    }
                __forceinline bool operator>=(const _Iterator& l) const
                    {
                    if(m_curPartition != l.m_curPartition)
                        return m_curPartition >= l.m_curPartition;
                    return m_curPtr >= l.m_curPtr;
                    }

                __forceinline _Iterator& operator++()
                    {	// preincrement

                    if(m_curPtr < m_endPtr)
                        {
                        m_curPtr++;
                        return (*this);
                        }
                    else
                        {
                        m_curPartition++;
                        GetPartitionInfo();
                        m_curPtr = m_startPtr;
                        }
                    return (*this);
                    }

                __forceinline _Iterator operator++(int)
                    {	// postincrement
                    _Iterator _Tmp = *this;
                    ++*this;
                    return (_Tmp);
                    }

                __forceinline _Iterator& operator--()
                    {	// predecrement
                    if(m_curPtr > m_startPtr)
                        m_curPtr--;
                    else
                        {
                        m_curPartition--;
                        GetPartitionInfo();
                        m_curPtr = m_endPtr;
                        }
                    return (*this);
                    }

                __forceinline _Iterator operator--(int)
                    {	// postdecrement
                    _Iterator _Tmp = *this;
                    --*this;
                    return (_Tmp);
                    }

                __forceinline _Iterator& operator+=(size_t _Off)
                    {	// increment by integer
                    MoveOn((int)_Off);
                    return (*this);
                    }

                __forceinline _Iterator operator+(size_t _Off) const
                    {	// return this + integer
                    _Iterator _Tmp = *this;
                    return (_Tmp += _Off);
                    }

                __forceinline _Iterator& operator-=(size_t _Off)
                    {	// decrement by integer
                    MoveOn(-(int)_Off);
                    return (*this);
                    }

                __forceinline _Iterator operator-(size_t _Off) const
                    {	// return this - integer
                    _Iterator _Tmp = *this;
                    return (_Tmp -= _Off);
                    }

                __forceinline __int64 operator-(const _Iterator& _to) const
                    {	// return this - integer
                    long id = (m_curPartition - _to.m_curPartition) * m_array->m_partitionSize + (long)(m_curPtr - m_startPtr) - (long)(_to.m_curPtr - _to.m_startPtr);
                    //long index = (m_curPartition << m_array->m_shift) | (m_curPtr - m_startPtr);
                    //long index2 = (_to.m_curPartition << _to.m_array->m_shift) | (_to.m_curPtr - _to.m_startPtr);
                    //long id2 = index - index2;
                    //if(id != id2)
                    //    id = id2;
                    return id;
                    }

                __forceinline TYPE* getPtr() { return m_curPtr; }

            private:
                __forceinline void MoveOn(int offset) const
                    {
                    int index = (m_curPartition << m_array->m_shift) | (int)(m_curPtr - m_startPtr);
                    index += offset;
                    long partition = (index >> m_array->m_shift);
                    if(partition != m_curPartition)
                        {
                        m_curPartition = partition;
                        GetPartitionInfo();
                        }
                    m_curPtr = m_startPtr + (index & m_array->m_bitmask);
                    }
            };
#ifdef _WIN32_WCE
			public:
#else
			private:
#endif
        long    m_size;
        long    m_numPartitions;
        long	m_partitionSize;
        TYPE**	m_tablePP;
        long    m_bitmask;
        short   m_shift;
        bool m_attached;

    public:
        typedef _iterator<TYPE> iterator;

        __forceinline TYPE& operator[](int i)                                // The [] operator function. Returns the TYPE at pos i.
            {
            return( m_tablePP[i >> shift ] [i & m_bitmask] ) ;
            }

        __forceinline TYPE* operator+(int i)
            {
            return( m_tablePP[i >> shift ] + (i & m_bitmask) ) ;
            }

        __forceinline iterator start()
            {
            return iterator(this, 0, 0);
            }

        __forceinline iterator begin()
            {
            return iterator(this, 0, 0);
            }

        __forceinline iterator end()
            {
            long lastIndex = m_size;
            return iterator(this, lastIndex >> shift, lastIndex & m_bitmask);
            }
        PartitionArray()
            {
            m_shift = shift;
            m_size = 0;
            m_numPartitions = 0;
            m_partitionSize = 1 << shift;
            m_tablePP = NULL;
            m_bitmask = m_partitionSize - 1;
            m_attached = false;
            }

        PartitionArray(TYPE** tablePP, long size, long numPartitions, long partitionSize) 
            {
            m_shift = shift;
            m_size = size;
            m_numPartitions = numPartitions;
            m_partitionSize = partitionSize;
            m_tablePP = tablePP;
            m_bitmask = m_partitionSize - 1;
            m_attached = true;
            }

        // This may need to be improved for speed.
        PartitionArray<TYPE, shift, allocator>* Clone()
            {
            PartitionArray<TYPE, shift, allocator>* newArray = new PartitionArray<TYPE, shift, allocator>();
            newArray->resize(getSize());

            iterator curI = start();
            iterator newI = newArray->start();
            for(int i = 0; i < getSize(); i++, ++newI, ++curI)
                {
                *newI = *curI;
                }
            return newArray;
            }

        __forceinline long getSize()
            {
            return m_size;
            }
        __forceinline long size()
            {
            return m_size;
            }

        int resize(int size)
            {
            long n;
            int dbg=0;
            if(size == m_size)
                return 0;

            if(m_size == 0)
                {
                m_size = size;
                long numPartitions = (size >> shift) + 1;
                long remPartition  = size & m_bitmask;

                if( remPartition == 0 ) { --numPartitions ; remPartition = m_partitionSize ; }

                m_numPartitions = numPartitions;

                m_tablePP = (TYPE**) allocator::AllocMemory(m_numPartitions * sizeof(TYPE*));
                if(m_tablePP == NULL)
                    {
                    // Need to do something here
                    return -1;
                    }

                for( n = 0 ; n < m_numPartitions ; ++n )
                    m_tablePP[n] = NULL;

                for( n = 0 ; n < m_numPartitions ; ++n )
                    {
                    size_t partitionSize;
                    /*
                    **     Determine Partition Size
                    */
                    if( n < numPartitions - 1 ) partitionSize = m_partitionSize ;
                    else partitionSize = remPartition ;

                    if( dbg ) bcdtmWrite_message(0,0,0,"Allocationg Memory Amount Of %8ld for Partition %8ld",partitionSize,n) ;
                    /*
                    **     Allocate Partition Memory
                    */
                    m_tablePP[n] = ( TYPE* ) allocator::AllocMemory( partitionSize * sizeof(TYPE)) ;
                    if( m_tablePP[n] == NULL )
                        {
                                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        //                    goto errexit ;
                        return -1;
                        }
                    }
                }
            else
                {
                long curNumPartitions = (m_size >> shift) + 1;
                long curRemPartition  = m_size & m_bitmask;
                long numPartitions    = (size >> shift) + 1;
                long remPartition     = size & m_bitmask;

                if( curRemPartition == 0 ) { --curNumPartitions ; curRemPartition = m_partitionSize ; }
                if( remPartition == 0 ) { --numPartitions ; remPartition = m_partitionSize ; }

                m_size = size;
                m_numPartitions = numPartitions;

                if(curNumPartitions < numPartitions)
                    {
                    m_tablePP = (TYPE**)allocator::ResizeMemory(m_tablePP, curNumPartitions * sizeof(TYPE*), m_numPartitions * sizeof(TYPE*));
                    if( m_tablePP == NULL )
                        {
                                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        //                    goto errexit ;
                        return -1;
                        }
                    for(n = curNumPartitions; n < numPartitions; ++n)
                        m_tablePP[n] = NULL;
                    }
                else
                    {
                    // Need to resize down.
                    // Release now unneeded partitions.
                    for(int n = m_numPartitions + 1; n < curNumPartitions; n++)
                        {
                        allocator::DeleteMemory(m_tablePP[n]);
                        }

                    m_tablePP = (TYPE**)allocator::ResizeMemory(m_tablePP, curNumPartitions * sizeof(TYPE*), m_numPartitions * sizeof(TYPE*));
                    if( m_tablePP == NULL )
                        {
                                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        //                    goto errexit ;
                        return -1;
                        }
                    }

                for(n = curNumPartitions - 1; n < numPartitions; n++)
                    {
                    size_t partitionSize;
                    if( n < m_numPartitions - 1 ) partitionSize = m_partitionSize ;
                    else partitionSize = remPartition ;

                    if(n == curNumPartitions - 1)
                        {
                        m_tablePP[n] = (TYPE*)allocator::ResizeMemory(m_tablePP[n], curRemPartition * sizeof(TYPE), partitionSize * sizeof(TYPE));
                        }
                    else
                        m_tablePP[n] = ( TYPE* ) allocator::AllocMemory( partitionSize * sizeof(TYPE)) ;
                    if( m_tablePP[n] == NULL )
                        {
                                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                        //                    goto errexit ;
                        return -1;
                        }
                    }
                }
            return 0;
            }

        virtual ~PartitionArray()
            {
            empty();
            }

        void empty()
            {
            if(!m_attached && m_tablePP)
                {
                for( long n = 0 ; n < m_numPartitions ; ++n )
                    {
                    allocator::DeleteMemory(m_tablePP[n]);
                    }
                allocator::DeleteMemory(m_tablePP);
                }
            m_size = 0;
            m_numPartitions = 0;
            m_tablePP = NULL;
            m_attached = false;
            }
    };

//#define UseSimpleLongArray
//#define UseMultiThreading
#ifndef UseSimpleLongArray
typedef PartitionArray<long, 11, MAllocAllocator> LongArray;
#else
class LongArray
    {
    private:
        long* m_ptr;
        long m_size;
    public:
        typedef long* iterator;

        LongArray()
            {
            m_ptr = NULL;
            }
        ~LongArray()
            {
            if(m_ptr) free(m_ptr);
            }

        int resize(int size)
            {
            m_ptr = (long*)malloc(size * sizeof(long));
            m_size = size;
            return 0;
            }

        __forceinline long* start()
            {
            return m_ptr;
            }
        __forceinline long* end()
            {
            return m_ptr + m_size - 1;
            }
        __forceinline long& operator[](int i)
            {
            return m_ptr[i];
            }

        __forceinline long* operator+(int i)
            {
            return &m_ptr[i];
            }
    };
#endif

#ifdef notdef
template <class TYPE, class arrayType> class ArraySort
    {
    private:
        arrayType* m_array;
        LongArray sortP;
        LongArray tempP;

        int DivConqMergeSortOld(long startPnt,long numPts)
            /*
            ** This Routine Sorts The Dtm Object By A Combined Divide And Conquer Merging Method
            */
            {
            long  i,temp,numPts1,numPts2,startPnt1,startPnt2 ;
            TYPE *p1P , *p2P ;

            /*
            **  More than two data Points
            */

            if( numPts > 2 )
                {
                numPts1 = numPts / 2  ; 
                if( numPts % 2 != 0 ) numPts1 = numPts1 + 1 ; 
                numPts2 = numPts - numPts1 ;
                startPnt1 = startPnt  ; 
                startPnt2 = startPnt + numPts1 ;
                divConqMergeSort(startPnt1,numPts1);
                divConqMergeSort(startPnt2,numPts2);
                /*
                **  Merge data sets
                */
                p1P = (*m_array) + *(sortP+(startPnt2-1)) ;
                p2P = (*m_array) + *(sortP+startPnt2) ;
                if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
                    {
                    LongArray::iterator startPnt1I = sortP.start()+startPnt1; 
                    LongArray::iterator startPnt2I = sortP.start()+startPnt2; 
                    LongArray::iterator tempPi = tempP.start() + startPnt;
                    p1P = (*m_array) + *startPnt1I ;
                    p2P = (*m_array) + *startPnt2I ;
                    for( i = startPnt ; i < startPnt + numPts ; ++i, ++tempPi )
                        {
                        if     ( startPnt1 >= startPnt + numPts1 ) {*tempPi=*startPnt2I;++startPnt2I;++startPnt2;}
                        else if( startPnt2 >= startPnt + numPts  ) {*tempPi=*startPnt1I;++startPnt1I;++startPnt1;}
                        //          if     ( startPnt1 >= startPnt + numPts1 ) {*tempPi=*(sortP+startPnt2);++startPnt2;}
                        //          else if( startPnt2 >= startPnt + numPts  ) {*tempPi=*(sortP+startPnt1);++startPnt1;}
                        else
                            {
                            //             p1P = (*m_array) + *(sortP+startPnt1) ;
                            //             p2P = (*m_array) + *(sortP+startPnt2) ;
                            if( p1P->x < p2P->x || ( p1P->x == p2P->x && p1P->y <= p2P->y))
                                {
                                *tempPi = *startPnt1I ; 
                                ++startPnt1I ; 
                                //			    *tempPi = *(sortP+startPnt1) ; 
                                ++startPnt1 ; 
                                p1P = (*m_array) + *startPnt1I ;
                                }
                            else 
                                { 
                                *tempPi = *startPnt2I ;
                                ++startPnt2I ;
                                //			    *tempPi = *(sortP+startPnt2) ;
                                ++startPnt2 ;
                                if(startPnt2 < startPnt + numPts)
                                    p2P = (*m_array) + *startPnt2I ;
                                }
                            }
                        }
                    /*
                    **     Restore Sort Pointers
                    */
                    tempPi = tempP.start() + startPnt;
                    LongArray::iterator sortPi = sortP.start() + startPnt;
                    for( i = startPnt ; i < startPnt + numPts ; ++i, ++tempPi, ++sortPi) *sortPi = *tempPi; //*(sortP+i) = *(tempP+i) ;
                    }
                }
            /*
            ** Two data points
            */
            else if( numPts == 2 )
                {
                long* a = sortP+startPnt;
                long* b = sortP+(startPnt+1);
                p1P = (*m_array) + *a ;
                p2P = (*m_array) + *b ;
                if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
                    {
                    temp = *a ;
                    *a   = *b ;
                    *b = temp ;
                    }
                }/*
                 ** Job Completed
                 */
            return(0) ;
            }
        int DivConqMergeSort(long startPnt,long numPts)
            /*
            ** This Routine Sorts The Dtm Object By A Combined Divide And Conquer Merging Method
            */
            {
            long  i,temp,startPnt1,startPnt2 ;//numPts1,numPts2
            TYPE *p1P , *p2P ;


            for(startPnt1 = startPnt, i = 0; i < numPts - 1; i += 2, startPnt1 += 2)
                {
                long* a = sortP+startPnt1;
                long* b = sortP+(startPnt1+1);
                p1P = (*m_array) + *a ;
                p2P = (*m_array) + *b ;
                if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
                    {
                    temp = *a ;
                    *a   = *b ;
                    *b = temp ;
                    }
                }

            int startPntTemp = startPnt;
            long step = 4;
            while(1)
                {
                long smallStep = step >> 1;
                long i2;
                for(startPnt = startPntTemp, i2 = 0; i2 < numPts; i2+=step, startPnt += step)
                    {
                    long num = step;

                    if(step + startPnt > startPntTemp + numPts)
                        {
                        num = (startPntTemp + numPts) - startPnt;
                        if(num < 3 || num <= smallStep)
                            break;
                        }

                    startPnt1 = startPnt;
                    startPnt2 = startPnt1 + smallStep;
                    /*
                    **  Merge data sets
                    */
                    p1P = (*m_array) + *(sortP+(startPnt2 - 1));
                    p2P = (*m_array) + *(sortP+startPnt2);
                    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
                        {
                        LongArray::iterator startPnt1I = sortP.start()+startPnt1; 
                        LongArray::iterator startPnt2I = sortP.start()+startPnt2; 
                        LongArray::iterator tempPi = tempP.start() + startPnt;

                        p1P = (*m_array) + *startPnt1I ;
                        p2P = (*m_array) + *startPnt2I ;
                        for( i = startPnt ; i < startPnt + num ; ++i, ++tempPi )
                            {
                            if     ( startPnt1 >= startPnt + smallStep) {*tempPi=*startPnt2I;++startPnt2I;++startPnt2;}
                            else if( startPnt2 >= startPnt + num  ) {*tempPi=*startPnt1I;++startPnt1I;++startPnt1;}
                            else
                                {
                                if( p1P->x < p2P->x || ( p1P->x == p2P->x && p1P->y <= p2P->y))
                                    {
                                    *tempPi = *startPnt1I ; 
                                    ++startPnt1I ; 
                                    ++startPnt1 ; 
                                    p1P = (*m_array) + *startPnt1I ;
                                    }
                                else 
                                    { 
                                    *tempPi = *startPnt2I ;
                                    ++startPnt2I ;
                                    ++startPnt2 ;
                                    if(startPnt2 < startPntTemp + numPts)
                                        p2P = (*m_array) + *startPnt2I ;
                                    }
                                }
                            }
                        /*
                        **     Restore Sort Pointers
                        */
                        tempPi = tempP.start() + startPnt;
                        LongArray::iterator sortPi = sortP.start() + startPnt;
                        for( i = startPnt ; i < startPnt + num; ++i, ++tempPi, ++sortPi) *sortPi = *tempPi; //*(sortP+i) = *(tempP+i) ;

                        }
                    }
                if(step > numPts)
                    break;
                step <<= 1;
                }

            //for(startPnt1 = startPntTemp, i = 0; i < numPts - 1; i++, startPnt1++)
            //    {
            //    long* a = sortP+startPnt1;
            //    long* b = sortP+(startPnt1+1);
            //    p1P = (*m_array) + *a ;
            //    p2P = (*m_array) + *b ;
            //    if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y))
            //        {
            //        temp = *a ;
            //        *a   = *b ;
            //        *b = temp ;
            //        }
            //    }

            return(0) ;
            }

    public:
        LongArray& GetTempP() { return tempP; }
        LongArray& GetSortP() { return sortP; }
        int DoSort(arrayType& theArray)
            {
            int    dbg=DTM_TRACE_VALUE(0) ;
            long   ofs;
            LongArray::iterator sP;
            LongArray::iterator endP;

            TYPE   dtmPoint,*p1P,*p2P;
            (*m_array) = theArray;
            /*
            **     Allocate Memory For Sort Offset Pointers
            */
            if(sortP.resize(m_array->getSize()) != 0)
                { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return DTM_ERROR; }
            if(tempP.resize(m_array->getSize()) != 0)
                { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return DTM_ERROR ; }
            /*
            **     Initialise Sort Offset Pointer
            */
            endP = sortP.end();
            for( sP = sortP.start(), ofs = 0 ; sP <= endP; ++sP, ++ofs) *sP = ofs ;
            /*
            **     Sort the Dtm Points
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;
            divConqMergeSort(0,m_array->getSize()) ;
            /*
            **     Calculate Dtm Point Sort Position
            */
            long i = 0;
            for( sP = sortP.start(); sP <= endP; ++sP, ++i) *(tempP+*sP) = i; //(long)(sP - sortP.start());
            /*
            **     Place In Sort Order
            */
            for( sP = sortP.start(), ofs = 0 ; sP <= endP ; ++sP , ++ofs )
                {
                p1P = (*m_array) + ofs ; 
                p2P = (*m_array) + *sP ; 
                dtmPoint = *p1P ;
                *p1P = *p2P ;
                *p2P = dtmPoint ;
                *(sortP+*(tempP+ofs)) = *sP ;
                *(tempP+*sP) = *(tempP+ofs) ;
                }
            return 0;
            }
    };
#endif

#ifdef _WIN32_WCE
	        template <class TYPE> class ICompareClass
            {
            public:
                virtual int Compare(TYPE* p1P, TYPE* p2P) = 0;
                virtual bool LessThan(TYPE* p1P, TYPE* p2P) = 0;
                virtual bool GreaterThan(TYPE* p1P, TYPE* p2P) = 0;
            };
#endif

template <class TYPE, class arrayType> class ArraySort
    {
    private:
        arrayType* m_array;
        LongArray sortP;
        LongArray tempP;
        LongArray::iterator sortPstart;
        LongArray::iterator tempPstart;

#ifdef _WIN32_WCE
        int DivConqMergeSort(long startPnt, long numPts, ICompareClass<TYPE>* m_compare)
#else
        template<class compareClass> int DivConqMergeSort(long startPnt, long numPts, compareClass* m_compare)
#endif
            /*
            ** This Routine Sorts The Dtm Object By A Combined Divide And Conquer Merging Method
            */
            {
            /*
            **  More than two data Points
            */

            if( numPts > 2 )
                {
                long  i,numPts1,numPts2,startPnt1,startPnt2 ;
                TYPE *p1P , *p2P ;

                numPts1 = numPts >> 1;

                numPts2 = numPts - numPts1;

                startPnt1 = startPnt; 
                startPnt2 = startPnt1 + numPts1;
                if(numPts1 != 1) DivConqMergeSort(startPnt1,numPts1, m_compare);
                DivConqMergeSort(startPnt2,numPts2, m_compare);
                /*
                **  Merge data sets
                */
                p1P = (*m_array) + *(sortP + (startPnt2 - 1));
                p2P = (*m_array) + *(sortP + startPnt2);

                if(m_compare->GreaterThan(p1P, p2P))
                    {
                    long startPntnumPts1 = startPnt + numPts1;
                    long startPntnumPts = startPnt + numPts;
                    LongArray::iterator startPnt1I = sortPstart+startPnt1; 
                    LongArray::iterator startPnt2I = sortPstart+startPnt2; 
                    LongArray::iterator tempPi = tempPstart + startPnt;
                    p1P = (*m_array) + *startPnt1I ;

                    for( i = startPnt ; i < startPntnumPts ; ++i, ++tempPi )
                        {
                        if(m_compare->LessThan(p1P, p2P))
                            {
                            *tempPi = *startPnt1I;
                            ++startPnt1I; 
                            ++startPnt1; 
                            if( startPnt1 < startPntnumPts1 )
                                {
                                p1P = (*m_array) + *startPnt1I;
                                }
                            else
                                {
                                // We are at the end of this part fill in the rest of the other part.
                                for( ++i, ++tempPi; i < startPntnumPts ; ++i, ++tempPi )
                                    {
                                    *tempPi = *startPnt2I;
                                    ++startPnt2I;
                                    }
                                break;
                                }
                            }
                        else 
                            { 
                            *tempPi = *startPnt2I ;
                            ++startPnt2I ;
                            ++startPnt2 ;
                            if(startPnt2 < startPntnumPts)
                                {
                                p2P = (*m_array) + *startPnt2I;
                                }
                            else
                                {
                                // We are at the end of this part fill in the rest of the other part.
                                for( ++i, ++tempPi ; i < startPntnumPts ; ++i, ++tempPi )
                                    {
                                    *tempPi = *startPnt1I;
                                    ++startPnt1I;
                                    }
                                break;
                                }
                            }
                        }
                    /*
                    **     Restore Sort Pointers
                    */
                    tempPi = tempPstart + startPnt;
                    LongArray::iterator sortPi = sortPstart + startPnt;
                    for( i = startPnt ; i < startPntnumPts ; ++i, ++tempPi, ++sortPi)
                        *sortPi = *tempPi;
                    }
                }
            /*
            ** Two data points
            */
            else if( numPts == 2 )
                {
                TYPE *p1P , *p2P ;
                long temp;
                long* a = sortP + startPnt;
                long* b = sortP + (startPnt + 1);
                p1P = (*m_array) + *a ;
                p2P = (*m_array) + *b ;
                if( m_compare->GreaterThan(p1P, p2P))
                    {
                    temp = *a ;
                    *a   = *b ;
                    *b = temp ;
                    }
                }
            /*
            ** Job Completed
            */
            return(0) ;
            }

#ifdef UseMultiThreading
        struct callbackStruct
            {
            int startPnt;
            int numPts;
            void* compare;
            ArraySort<TYPE, arrayType>* ptr;
            };

#ifdef _WIN32_WCE
        static int callback(void* p)
#else
        template<class compareClass> static int Callback(void* p)
#endif
            {
            callbackStruct* info = (callbackStruct*)p;
            info->ptr->DivConqMergeSort(info->startPnt, info->numPts, (compareClass*)info->compare);
            return 1;
            }

#ifdef _WIN32_WCE
        int DivConqMergeSortMultithreaded(long startPnt, long numPts, ICompareClass<TYPE>* m_compare)
#else
        template<class compareClass> int DivConqMergeSortMultithreaded(long startPnt, long numPts, compareClass* m_compare)
#endif
        {
            int numThreads = 8;
            HANDLE threads[8];
            callbackStruct data[8];
            long  numPts1,startPnt1 ;

            numPts1 = numPts / numThreads;

            startPnt1 = startPnt; 
            if(numPts1 > 100)
                {
                DWORD id;

                for (int i = 0; i < numThreads; i++)
                    {
                    data[i].startPnt = startPnt1;
                    data[i].numPts = numPts1;
                    data[i].compare = m_compare;
                    data[i].ptr = this;

                    startPnt1 += numPts1;
                    }
                data[numThreads - 1].numPts += numPts - (startPnt1 - startPnt);

                for (int i = 0; i < numThreads; i++)
                    threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&callback<compareClass>, &data[1], 0, &id);
//                callback<compareClass>(&data[1]);
//                divConqMergeSort(startPnt1,numPts1, m_compare);
//                divConqMergeSort(startPnt2,numPts2, m_compare);
                WaitForMultipleObjects (numThreads, threads, true, INFINITE);
                for (int i = 0; i < numThreads; i++)
                    CloseHandle(threads[i]);

                MergeData (data[0].startPnt, data[0].numPts, data[1].startPnt, data[1].numPts, m_compare);
                MergeData (data[2].startPnt, data[2].numPts, data[3].startPnt, data[3].numPts, m_compare);
                MergeData (data[4].startPnt, data[4].numPts, data[5].startPnt, data[5].numPts, m_compare);
                MergeData (data[6].startPnt, data[6].numPts, data[7].startPnt, data[7].numPts, m_compare);
                data[0].numPts += data[1].numPts;
                data[2].numPts += data[3].numPts;
                data[4].numPts += data[5].numPts;
                data[6].numPts += data[7].numPts;

                MergeData (data[0].startPnt, data[0].numPts, data[2].startPnt, data[2].numPts, m_compare);
                MergeData (data[4].startPnt, data[4].numPts, data[6].startPnt, data[6].numPts, m_compare);
                data[0].numPts += data[2].numPts;
                data[4].numPts += data[6].numPts;
                MergeData (data[0].startPnt, data[0].numPts, data[4].startPnt, data[4].numPts, m_compare);
//                return 0;
                }
            else
                {
//                divConqMergeSort(startPnt2,numPts2, m_compare);
                }
            return 0;
            }
        template<class compareClass> int MergeData (int startPnt1, int numPts1, int startPnt2, int numPts2, compareClass* m_compare)
            {
            long i;
            int startPnt = startPnt1;
            int numPts = numPts1 + numPts2;
            TYPE *p1P , *p2P ;
            /*
            **  Merge data sets
            */
            p1P = (*m_array) + *(sortP + (startPnt2 - 1));
            p2P = (*m_array) + *(sortP + startPnt2);

            if(m_compare->GreaterThan(p1P, p2P))
                {
                long startPntnumPts1 = startPnt + numPts1;
                long startPntnumPts = startPnt + numPts;
                LongArray::iterator startPnt1I = sortPstart+startPnt1; 
                LongArray::iterator startPnt2I = sortPstart+startPnt2; 
                LongArray::iterator tempPi = tempPstart + startPnt;
                p1P = (*m_array) + *startPnt1I ;

                for( i = startPnt ; i < startPntnumPts ; ++i, ++tempPi )
                    {
                    if(m_compare->LessThan(p1P, p2P))
                        {
                        *tempPi = *startPnt1I;
                        ++startPnt1I; 
                        ++startPnt1; 
                        if( startPnt1 < startPntnumPts1 )
                            {
                            p1P = (*m_array) + *startPnt1I;
                            }
                        else
                            {
                            // We are at the end of this part fill in the rest of the other part.
                            for( ++i, ++tempPi; i < startPntnumPts ; ++i, ++tempPi )
                                {
                                *tempPi = *startPnt2I;
                                ++startPnt2I;
                                }
                            break;
                            }
                        }
                    else 
                        { 
                        *tempPi = *startPnt2I ;
                        ++startPnt2I ;
                        ++startPnt2 ;
                        if(startPnt2 < startPntnumPts)
                            {
                            p2P = (*m_array) + *startPnt2I;
                            }
                        else
                            {
                            // We are at the end of this part fill in the rest of the other part.
                            for( ++i, ++tempPi ; i < startPntnumPts ; ++i, ++tempPi )
                                {
                                *tempPi = *startPnt1I;
                                ++startPnt1I;
                                }
                            break;
                            }
                        }
                    }
                /*
                **     Restore Sort Pointers
                */
                tempPi = tempPstart + startPnt;
                LongArray::iterator sortPi = sortPstart + startPnt;
                for( i = startPnt ; i < startPntnumPts ; ++i, ++tempPi, ++sortPi)
                    *sortPi = *tempPi;
                }
            return 0;
        }
#endif //UseMultiThreading
    public:
        LongArray& GetTempP()
            {
            return tempP;
            }
        LongArray& GetSortP()
            {
            return sortP;
            }


        int DoSort(arrayType& theArray, void* compare)
            {
			return 0;
			}

        int InitForSort()
		    {
            long   ofs;
            LongArray::iterator sP;
            long size = m_array->getSize();

            /*
            **     Allocate Memory For Sort Offset Pointers
            */
            if(sortP.resize(size) != 0)
                { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return -1; }
            if(tempP.resize(size) != 0)
                { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; return -1 ; }
            sortPstart = sortP.start();
            tempPstart = tempP.start();
            /*
            **     Initialise Sort Offset Pointer
            */
            for( sP = sortPstart, ofs = 0 ; ofs < size; ++sP, ++ofs)
                *sP = ofs;
            return 0;
            }
#ifdef _WIN32_WCE
        int DoSort(arrayType& theArray, ICompareClass<TYPE>* compare, int start, int length)
#else
        template<class compareClass> int DoSort(arrayType& theArray, compareClass* compare, int start, int length)
#endif
		    {
            m_array = &theArray;
            InitForSort();
            int dbg=0 ;
            /*
            **     Sort the Dtm Points
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Sorting") ;

#ifdef UseMultiThreading
            DivConqMergeSortMultithreaded<compareClass>(start,length,compare) ;
#else
#ifdef _WIN32_WCE
            DivConqMergeSort(start, length,compare) ;
#else
            DivConqMergeSort<compareClass>(start,length,compare) ;
#endif
#endif
#ifdef _WIN32_WCE
            if(start != 0)
                mergeSort(compare, start, length);
#else
            if(start != 0)
                MergeSort<compareClass>(compare, start, length);
#endif
            return PlaceSort();

            }


#ifdef _WIN32_WCE
        int mergeSort(ICompareClass<TYPE>* compare, int start, int length)
#else
        template<class compareClass> int MergeSort(compareClass* compare, int start, int length)
#endif
            {
            long  i;
            TYPE *p1P , *p2P ;
            int startPnt2 = start;
            int startPnt1 = 0;
            int startPnt = 0;
            int numPts1 = start;
            int numPts = start + length;
            /*
            **  Merge data sets
            */
            p1P = (*m_array) + *(sortP + (startPnt2 - 1));
            p2P = (*m_array) + *(sortP + startPnt2);

            if(compare->GreaterThan(p1P, p2P))
                {
                long startPntnumPts1 = startPnt + numPts1;
                long startPntnumPts = startPnt + numPts;
                LongArray::iterator startPnt1I = sortPstart+startPnt1; 
                LongArray::iterator startPnt2I = sortPstart+startPnt2; 
                LongArray::iterator tempPi = tempPstart + startPnt;
                p1P = (*m_array) + *startPnt1I ;

                for( i = startPnt ; i < startPntnumPts ; ++i, ++tempPi )
                    {
                    if(compare->LessThan(p1P, p2P))
                        {
                        *tempPi = *startPnt1I;
                        ++startPnt1I; 
                        ++startPnt1; 
                        if( startPnt1 < startPntnumPts1 )
                            {
                            p1P = (*m_array) + *startPnt1I;
                            }
                        else
                            {
                            // We are at the end of this part fill in the rest of the other part.
                            for( ++i, ++tempPi; i < startPntnumPts ; ++i, ++tempPi )
                                {
                                *tempPi = *startPnt2I;
                                ++startPnt2I;
                                }
                            break;
                            }
                        }
                    else 
                        { 
                        *tempPi = *startPnt2I ;
                        ++startPnt2I ;
                        ++startPnt2 ;
                        if(startPnt2 < startPntnumPts)
                            {
                            p2P = (*m_array) + *startPnt2I;
                            }
                        else
                            {
                            // We are at the end of this part fill in the rest of the other part.
                            for( ++i, ++tempPi ; i < startPntnumPts ; ++i, ++tempPi )
                                {
                                *tempPi = *startPnt1I;
                                ++startPnt1I;
                                }
                            break;
                            }
                        }
                    }
                startPnt1I = sortPstart; 
                tempPi = tempPstart + startPnt;
                for( i = startPnt ; i < numPts ; ++i, ++tempPi, ++startPnt1I )
                    {
                    *startPnt1I = *tempPi;
                    }
                }
            return 0;
            }
        int PlaceSort()
            {
            long   ofs;
            LongArray::iterator sP;
            long size = m_array->getSize();
            /*
            **     Calculate Dtm Point Sort Position
            */

/*
            long* ysortP = ( long * ) malloc(size*sizeof(long)) ;
            long* ytempP = ( long * ) malloc(size*sizeof(long)) ;
            long* srP;
            for( srP = ysortP, ofs = 0 ; srP < ysortP+size; ++srP, ++ofs) *srP = ofs ;
            if(gdtmP)
                {
                bcdtmList_qsortDtmObject(gdtmP,gdtmP->numPoints,0,ysortP,ytempP) ;
                    for( sP = sortPstart, ofs = 0; ofs < size; ++sP, ++ofs)
                    {
                    if(ysortP[ofs] != *sP)
                        ofs = ofs;
                    }

                }
*/
            LongArray storeSort;
            LongArray::iterator ssP;
            storeSort.resize(size);
            for( sP = sortPstart, ssP = storeSort.start(), ofs = 0; ofs < size; ++ssP, ++sP, ++ofs)
                {
                *ssP = *sP;
                *(tempP+*sP) = ofs;
                }

            /*
            **     Place In Sort Order
            */
            arrayType::iterator p1P = m_array->start();
            TYPE *p2P, dtmPoint ;

            for( sP = sortPstart, ofs = 0 ; ofs < size; ++sP , ++ofs, ++p1P)
                {
                p2P = (*m_array) + *sP ; 
                dtmPoint = *p1P ;
                *p1P = *p2P ;
                *p2P = dtmPoint ;
                long tempPofs = *(tempP+ofs);
                long sPV = *sP;
                *(sortP+tempPofs) = sPV ;
                *(tempP+sPV) = tempPofs ;
                }
//
            for( sP = sortPstart, ssP = storeSort.start(), ofs = 0; ofs < size; ++ssP, ++sP, ++ofs)
                {
                *sP = *ssP;
                *(tempP+*sP) = ofs;
                }

            return 0;
            }
    };




template <class TYPE, class arrayType> class XYPointArraySort : public ArraySort<TYPE, arrayType>
    {
    private:
#ifdef _WIN32_WCE
        class CompareClass : public ICompareClass<TYPE>
            {
            public:
                virtual int Compare(TYPE* p1P, TYPE* p2P)
                    {
                    if( p1P->x > p2P->x ||
                        ( p1P->y > p2P->y &&
                        p1P->x == p2P->x))
                        return 1;
                    if( p1P->x == p2P->x &&
                        p1P->y == p2P->y)
                        return 0;
                    return -1;
                    }
                virtual bool LessThan(TYPE* p1P, TYPE* p2P)
                    {
                    return ( p1P->x < p2P->x ||
                        ( p1P->x == p2P->x &&
                        p1P->y < p2P->y));
                    }
                virtual bool GreaterThan(TYPE* p1P, TYPE* p2P)
                    {
                    return( p1P->x > p2P->x ||
                        ( p1P->x == p2P->x &&
                        p1P->y > p2P->y));
                    }
            };
#else
        class CompareClass
            {
            public:
                static __forceinline int Compare(TYPE* p1P, TYPE* p2P)
                    {
                    if( p1P->x > p2P->x ||
                        ( p1P->y > p2P->y &&
                        p1P->x == p2P->x))
                        return 1;
                    if( p1P->x == p2P->x &&
                        p1P->y == p2P->y)
                        return 0;
                    return -1;
                    }
                static __forceinline bool LessThan(TYPE* p1P, TYPE* p2P)
                    {
                    return ( p1P->x < p2P->x ||
                        ( p1P->x == p2P->x &&
                        p1P->y <= p2P->y));
                    }
                static __forceinline bool GreaterThan(TYPE* p1P, TYPE* p2P)
                    {
                    return( p1P->x > p2P->x ||
                        ( p1P->x == p2P->x &&
                        p1P->y > p2P->y));
                    }
            };
#endif
    public:
        int DoSort(arrayType& theArray)
            {
            CompareClass compareClass;
#ifdef _WIN32_WCE
            return ArraySort<TYPE, arrayType>::DoSort(theArray, &compareClass, 0, theArray.getSize());
#else
            return ArraySort<TYPE, arrayType>::DoSort<CompareClass>(theArray, &compareClass, 0, theArray.getSize());
#endif
            }
        int DoResort(arrayType& theArray, int start, int length)
            {
            CompareClass compareClass;
#ifdef _WIN32_WCE
            return ArraySort<TYPE, arrayType>::DoSort(theArray, &compareClass, start, length);
#else
            return ArraySort<TYPE, arrayType>::DoSort<CompareClass>(theArray, &compareClass, start, length);
#endif
            }
    };

    template <class TYPE, class arrayType> class FuncArraySort : public ArraySort<TYPE, arrayType>
        {
        public:
            typedef int (*CompareFunc)(const TYPE* p1P, const TYPE* p2P);
        private:
            class CompareClass
                {
                private:
                    CompareFunc m_func;
                public:
                    CompareClass(CompareFunc func)
                        {
                        m_func = func;
                        }
                    __forceinline int Compare(const TYPE* p1P, const TYPE* p2P)
                        {
                        return m_func(p1P, p2P);
                        }
                    __forceinline bool LessThan(const TYPE* p1P, const TYPE* p2P)
                        {
                        return Compare(p1P, p2P) < 0;
                        }
                    __forceinline bool GreaterThan(const TYPE* p1P, const TYPE* p2P)
                        {
                        return Compare(p1P, p2P) > 0;
                        }
                };
        public:
            int DoSort(arrayType& theArray, CompareFunc func)
                {
                return ArraySort<TYPE, arrayType>::DoSort<CompareClass>(theArray, &CompareClass(func), 0, theArray.getSize());
                }
            int DoResort(arrayType& theArray, CompareFunc func, long start, long length)
                {
                return ArraySort<TYPE, arrayType>::DoSort<CompareClass>(theArray, &CompareClass(func), start, length);
                }
        };
