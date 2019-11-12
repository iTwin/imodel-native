/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <atomic>

#ifdef BeAtomic
#undef BeAtomic
#endif

// This BEATOMIC_TEMPLATED construct is to avoid problems in the case where both the DgnV8 and BIM header files are included.
#if ! defined (BEATOMIC_TEMPLATED)
#define BEATOMIC_TEMPLATED

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
template<class T> struct BeAtomic : std::atomic<T>
{
    explicit BeAtomic(T val=T(0)) : std::atomic<T>(val){}
    T IncrementAtomicPre(std::memory_order order=std::memory_order_seq_cst) {return this->fetch_add(1, order)+1;}
    T DecrementAtomicPost(std::memory_order order=std::memory_order_seq_cst) {return this->fetch_sub(1, order);}
};

#endif // BEATOMIC_TEMPLATED

