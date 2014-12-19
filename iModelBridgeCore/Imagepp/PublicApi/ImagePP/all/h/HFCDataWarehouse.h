//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCDataWarehouse.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


typedef std::map<uint32_t, Byte*> DataContainer;

class HFCDataWarehouse
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------
    _HDLLu             HFCDataWarehouse();
    _HDLLu virtual   ~HFCDataWarehouse();

    _HDLLu uint32_t GetNewStoringId();

    _HDLLu void     AddData(uint32_t pi_StoringId, void* pi_Data);

    _HDLLu void*    GetData(uint32_t pi_StoringId);

    _HDLLu void     DeleteData(uint32_t pi_StoringId);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    DataContainer    m_DataContainer;
    uint32_t          m_NextNewStoringId;

    HFCDataWarehouse(HFCDataWarehouse&);
    HFCDataWarehouse& operator=(HFCDataWarehouse&);
    };

#include "HFCDataWarehouse.hpp"