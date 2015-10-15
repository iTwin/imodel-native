//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirHelpers.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool ArrayIndexSubDirBase<T>::Get  (ArrayID pi_id,
                                    Array&  po_rArray) const
    {
    return super_class::Get(pi_id, po_rArray.EditPacket());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool ArrayIndexSubDirBase<T>::Add  (ArrayID&        pi_id,
                                    const Array&    pi_rArray)
    {
    return super_class::Add(pi_id, pi_rArray.GetPacket());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool ArrayIndexSubDirBase<T>::Set  (ArrayID         pi_id,
                                    const Array&    pi_rArray)
    {
    return super_class::Set(pi_id, pi_rArray.GetPacket());
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t AttributeSubDirBase::GetBlockIndexFor (size_t pi_index) const
    {
    return pi_index >> m_typedBlockSizeBase2Log;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t AttributeSubDirBase::GetElementIndexFor (size_t pi_index) const
    {
    return pi_index & m_elementIndexMask;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool AttributeSubDirBase::IsBlockLoaded (size_t pi_blockIndex) const
    {
    return (m_cachedBlocks.size() > pi_blockIndex) && (0 != m_cachedBlocks[pi_blockIndex].m_pPacket);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline AttributeSubDirBase::Packet& AttributeSubDirBase::EditPacketFor     (size_t      pi_index)
    {
    const size_t blockIndex = GetBlockIndexFor(pi_index);
    AttributeSubDirBase::Packet& packet = IsBlockLoaded(blockIndex) ? *m_cachedBlocks[blockIndex].m_pPacket : LoadBlock(blockIndex);    
    assert(blockIndex < m_cachedBlocks.size());
    //NEEDS_WORK_SM - Needed during partial update to ensure the block is saved back to file. 
    //Note that the call to StoreHeader also triggers a call this function when the file is read-only, but seems that some other 
    //mechanism (likely the fact that the file is loaded in read only mode) impedes the modification of the file.
    m_cachedBlocks[blockIndex].m_dirty = true;
    return packet;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline const AttributeSubDirBase::Packet& AttributeSubDirBase::GetPacketFor    (size_t      pi_index) const
    {
    const size_t blockIndex = GetBlockIndexFor(pi_index);
    return IsBlockLoaded(blockIndex) ? *m_cachedBlocks[blockIndex].m_pPacket : LoadBlock(blockIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
AttributeSubDir<T>::Options::Options   (const T&    pi_defaultValue)
    :   AttributeSubDirBase::Options(&m_defaultValue),
        m_defaultValue(pi_defaultValue)
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
AttributeSubDir<T>::AttributeSubDir ()
    :   AttributeSubDirBase(sizeof(T)),
        m_defaultValue(T())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void AttributeSubDir<T>::_InitializePacket (Packet&     pi_rPacket) const
    {
    T* begin = reinterpret_cast<T*>(pi_rPacket.BeginEdit());
    T* end = reinterpret_cast<T*>(pi_rPacket.EndEdit());

    fill(begin, end, m_defaultValue);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void AttributeSubDir<T>::_SetDefaultValue (const void* pi_pDefaultValue)
    {
    m_defaultValue = *static_cast<const T*>(pi_pDefaultValue);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
const T& AttributeSubDir<T>::Get   (size_t      pi_index) const
    {
    const Packet& rPacket = GetPacketFor(pi_index);
    return reinterpret_cast<const T*>(rPacket.Get())[GetElementIndexFor(pi_index)];
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
T& AttributeSubDir<T>::Edit    (size_t      pi_index)
    {
    Packet& rPacket = EditPacketFor(pi_index);
    return reinterpret_cast<T*>(rPacket.Edit())[GetElementIndexFor(pi_index)];
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
VarSizeAttributeSubDir<T>::Options::Options   (const T&    pi_defaultValue)
    : m_defaultValue(pi_defaultValue)
    {    
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
VarSizeAttributeSubDir<T>::VarSizeAttributeSubDir ()
    :   m_defaultValue(T())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
VarSizeAttributeSubDir<T>::~VarSizeAttributeSubDir ()    
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void VarSizeAttributeSubDir<T>::_InitializePacket (Packet&     pi_rPacket) const
    {
    T* begin = reinterpret_cast<T*>(pi_rPacket.BeginEdit());
    T* end = reinterpret_cast<T*>(pi_rPacket.EndEdit());

    fill(begin, end, m_defaultValue);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void VarSizeAttributeSubDir<T>::_SetDefaultValue (const void* pi_pDefaultValue)
    {
    m_defaultValue = *static_cast<const T*>(pi_pDefaultValue);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
const T* VarSizeAttributeSubDir<T>::Get   (PacketID pi_packetID) const
    {           
    T* varSizeAttr = 0;

    bool result = GetVarSizeData(varSizeAttr, pi_packetID);

    assert(result == true);
    
    return varSizeAttr;    
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool VarSizeAttributeSubDir<T>::GetVarSizeData   (T*& varSizeAttr, PacketID pi_packetID) const
    {
    bool result = true;
    //Not in cached, load it.
    if (m_cachedVarSizeDataPackets.size() <= pi_packetID || m_cachedVarSizeDataPackets[pi_packetID].m_pPacket == 0)
        {
        if (m_cachedVarSizeDataPackets.size() <= pi_packetID)
            {
            m_cachedVarSizeDataPackets.resize(pi_packetID + 1);
            }               

        m_cachedVarSizeDataPackets[pi_packetID].m_pPacket = new Packet();        
        
        HPRECONDITION(pi_packetID != IDTMFile::GetNullPacketID());        

        //NEEDS_WORK_SM : Cache for variable size data packet?
        result = PacketMgr().Get(pi_packetID, *m_cachedVarSizeDataPackets[pi_packetID].m_pPacket);                
        }    

    if (result == true)
        {            
        varSizeAttr = (T*)m_cachedVarSizeDataPackets[pi_packetID].m_pPacket->Get();       
        }

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
T* VarSizeAttributeSubDir<T>::Edit    (PacketID& pio_packetID, size_t nbAttributes)
    {  
    size_t nbVarData = PacketMgr().GetSize(pio_packetID) / sizeof(T);

    //NEEDS_WORK_SM : only when more data?    
    if (nbVarData != nbAttributes)
        {      
        if (pio_packetID != IDTMFile::GetNullPacketID())
            {
            bool isRemoved = PacketMgr().Remove(pio_packetID);
            assert(isRemoved == true);                  
            }
       
        if (nbAttributes > 0)
            {
            Packet::Ptr newPacket(new Packet(sizeof(T) * nbAttributes));
            newPacket->SetSize(sizeof(T) * nbAttributes);
            
            bool isAdded = PacketMgr().Add(pio_packetID, *newPacket);
            assert(isAdded == true);

            if (m_cachedVarSizeDataPackets.size() <= pio_packetID)
                 {
                 m_cachedVarSizeDataPackets.resize(pio_packetID + 1);
                 }              

            m_cachedVarSizeDataPackets[pio_packetID].m_pPacket = newPacket;
            }
        else
            {
            if (pio_packetID + 1 == m_cachedVarSizeDataPackets.size())
                {
                m_cachedVarSizeDataPackets.resize(m_cachedVarSizeDataPackets.size() - 1);
                }
            else
                {
                m_cachedVarSizeDataPackets[pio_packetID].m_pPacket = 0;
                m_cachedVarSizeDataPackets[pio_packetID].m_dirty = false;
                }            

            pio_packetID = IDTMFile::GetNullPacketID();            
            }
        }
    else
        {
        if (pio_packetID != IDTMFile::GetNullPacketID())
            {
            assert(nbAttributes > 0);
            T* attr = 0;
            bool result = GetVarSizeData(attr, pio_packetID);        
            assert(result); 
            }
        }

    if (pio_packetID != IDTMFile::GetNullPacketID())
        {
        m_cachedVarSizeDataPackets[pio_packetID].m_dirty = true;

        return (T*)m_cachedVarSizeDataPackets[pio_packetID].m_pPacket->Get();
        }
    else
        {
        return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
bool VarSizeAttributeSubDir<T>::_Save ()
    {    
    bool Success = true;
       
    PacketID id = 0;
    for (BlockList::iterator blockIt = m_cachedVarSizeDataPackets.begin(), endBlockIt = m_cachedVarSizeDataPackets.end();
         blockIt != endBlockIt;
         ++blockIt, ++id)
        {
        if (!blockIt->m_dirty)
            continue;
        
        assert(blockIt->m_pPacket != 0);
        
        if (!PacketMgr().Set(id, *blockIt->m_pPacket))
            {
            Success = false;
            continue;
            }

        size_t packetSize = PacketMgr().GetSize(id);

        assert(packetSize > 0);

        blockIt->m_dirty = false;
        }

    return Success;
    }


