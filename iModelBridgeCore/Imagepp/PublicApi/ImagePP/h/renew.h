//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/renew.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HTypes.h"

BEGIN_IMAGEPP_NAMESPACE

inline void* renew(Byte* pi_pMemblock, size_t pi_CurrentSize, size_t pi_NewSize)
    {
    Byte* pNewMemBlock;
    if ((pNewMemBlock = new Byte[pi_NewSize]) != 0 && pi_pMemblock)
        {
        memcpy(pNewMemBlock, pi_pMemblock, hmin(pi_CurrentSize, pi_NewSize));
        delete[] pi_pMemblock;
        }

    return pNewMemBlock;
    }

template <class T> inline
T* renewT(T* pi_pBuffer, size_t pi_CurrentCount, size_t pi_NewCount)
    {

    return (T*)renew(pi_pBuffer, pi_CurrentCount*sizeof(T), pi_NewCount*sizeof(T));
    }

END_IMAGEPP_NAMESPACE