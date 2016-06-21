/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeAtomic.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <atomic>

#ifdef BeAtomic
#undef BeAtomic
#endif

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
template<class T> struct BeAtomic : std::atomic<T>
{
    explicit BeAtomic(T val=0) : std::atomic<T>(val){}
    T IncrementAtomicPre(std::memory_order order=std::memory_order_seq_cst) {return this->fetch_add(1, order)+1;}
    T DecrementAtomicPost(std::memory_order order=std::memory_order_seq_cst) {return this->fetch_sub(1, order);}
};

