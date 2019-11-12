/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/Foundations/Definitions.h>
#include <ImagePP/h/HNumeric.h>
USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
template <typename T, size_t N>  
char (&CARRAY_SIZE_HELPER(T (&array)[N]))[N]; 

#ifdef CARRAY_SIZE
    #error CARRAY_SIZE already defined
#endif

#define CARRAY_SIZE(array) (sizeof(CARRAY_SIZE_HELPER(array))) 


template <typename T, size_t N>  
char (&CSTRING_SIZE_HELPER(T (&array)[N]))[N - 1]; 

#ifdef CSTRING_LEN
    #error CSTRING_LEN already defined
#endif

#define CSTRING_LEN(array) (sizeof(CSTRING_SIZE_HELPER(array))) 


// TDORAY: It would be a great addition for other modules and our headers if the following were part of our public header.
template<typename T>
bool                                    EqEps                          (T                               lhs,    
                                                                        T                               rhs)
    {
    return HNumeric<T>::EQUAL_EPSILON(lhs, rhs);
    }

template<typename T>
bool                                    EqOneEps                       (T                               rhs)
    {
    // TDORAY: Optimize 
    return HNumeric<T>::EQUAL_EPSILON(1.0, rhs);
    }

template<typename T>
bool                                    EqZeroEps                      (T                               rhs)
    {
    // TDORAY: Optimize 
    return HNumeric<T>::EQUAL_EPSILON(0.0, rhs);
    }


END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
