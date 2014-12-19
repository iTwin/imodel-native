//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/h/HTGFFSubDirHelpers.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
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
    return IsBlockLoaded(blockIndex) ? *m_cachedBlocks[blockIndex].m_pPacket : LoadBlock(blockIndex);
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