/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Foundations/FoundationsPrivateTools.h $
|    $RCSfile: FoundationsPrivateTools.h,v $
|   $Revision: 1.4 $
|       $Date: 2012/02/23 18:20:14 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Foundations/Definitions.h>
#include <ImagePP/h/HNumeric.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
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


END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE