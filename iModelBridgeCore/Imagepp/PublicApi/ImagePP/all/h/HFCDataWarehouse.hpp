//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCDataWarehouse.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Public
// Constructor
//----------------------------------------------------------------------------
inline HFCDataWarehouse::HFCDataWarehouse()
    {
    m_NextNewStoringId = 0;
    }

//----------------------------------------------------------------------------
// Public
// Get the next available storing ID
//----------------------------------------------------------------------------
inline uint32_t HFCDataWarehouse::GetNewStoringId()
    {
    HPRECONDITION(m_NextNewStoringId < ULONG_MAX - 1);
    return m_NextNewStoringId++;
    }

//----------------------------------------------------------------------------
// Public
// Get the next available storing ID.
//----------------------------------------------------------------------------
inline void HFCDataWarehouse::AddData(uint32_t pi_StoringId, void* pi_Data)
    {
    HPRECONDITION(m_DataContainer.find(pi_StoringId) == m_DataContainer.end());
    m_DataContainer.insert(DataContainer::value_type(pi_StoringId, (Byte*)pi_Data));
    }

//----------------------------------------------------------------------------
// Public
// Get the stored data.
//----------------------------------------------------------------------------
inline void* HFCDataWarehouse::GetData(uint32_t pi_StoringId)
    {
    void*                    pData = 0;
    DataContainer::iterator DataItr =  m_DataContainer.find(pi_StoringId);

    if (DataItr != m_DataContainer.end())
        {
        pData = (*DataItr).second;
        }

    return pData;
    }

//----------------------------------------------------------------------------
// Public
// Delete the data
//----------------------------------------------------------------------------
inline void HFCDataWarehouse::DeleteData(uint32_t pi_StoringId)
    {
    DataContainer::iterator DataItr =  m_DataContainer.find(pi_StoringId);
    delete (*DataItr).second;
    m_DataContainer.erase(pi_StoringId);
    }

