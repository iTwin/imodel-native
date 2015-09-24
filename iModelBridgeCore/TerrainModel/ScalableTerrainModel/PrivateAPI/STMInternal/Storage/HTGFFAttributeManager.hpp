//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFAttributeManager.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool AttributeManager::IsPresent (AttributeID pi_ID) const
    {
    return GetFile().TagIsPresent(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool AttributeManager::Remove (AttributeID pi_ID)
    {
    return GetFile().RemoveTag(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool AttributeManager::Get (AttributeID pi_ID,
                            T&          po_rData) const
    {
    return GetFile().GetField(pi_ID, &po_rData);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Get  (AttributeID     pi_ID,
                                    uint32_t&          po_rData) const
    {
    return GetFile().GetField(pi_ID, &po_rData);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Get  (AttributeID     pi_ID,
                                    int64_t&        po_rData) const
    {
    uint64_t value;

    if (!GetFile().GetField(pi_ID, &value))
        return false;

    po_rData = static_cast<int64_t>(value);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Get  (AttributeID         pi_ID,
                                    WString&            po_rData) const
    {
    WChar* pAttribute = 0;
    if (!GetFile().GetField(pi_ID, &pAttribute))
        {
        po_rData.clear();
        return false;
        }

    po_rData.assign(pAttribute);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool AttributeManager::Set (AttributeID pi_ID,
                            const T&    pi_rData)
    {
    return GetFile().SetField(pi_ID, pi_rData);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Set  (AttributeID     pi_ID,
                                    const uint32_t&    pi_rData)
    {
    return GetFile().SetField(pi_ID, pi_rData);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Set  (AttributeID     pi_ID,
                                    const int64_t&  pi_rData)
    {
    // NOTE: Signed data is not supported by HTag file...
    return GetFile().SetField(pi_ID, static_cast<uint64_t>(pi_rData));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Set  (AttributeID     pi_ID,
                                    const WString&  pi_rData)
    {
    return GetFile().SetFieldW(pi_ID, pi_rData.c_str());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Set  (AttributeID     pi_ID,
                                    const WCharCP&  pi_rData)
    {
    return GetFile().SetFieldW(pi_ID, pi_rData);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::Set  (AttributeID     pi_ID,
                                    const CharP&    pi_rData)
    {
    return GetFile().SetFieldA(pi_ID, pi_rData);
    }
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool AttributeManager::Get (AttributeID     pi_ID,
                            T*&             po_rpData,
                            size_t&         po_rSize) const
    {
    uint32_t TempValue = 0;
    if (!GetFile().GetField(pi_ID, &TempValue, &po_rpData))
        return false;

    po_rSize = TempValue;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool AttributeManager::Set (AttributeID     pi_ID,
                            const T*        pi_pData,
                            size_t          pi_Size)
    {
    HPRECONDITION(pi_Size <= ULONG_MAX);
    return GetFile().SetField(pi_ID, static_cast<uint32_t>(pi_Size), pi_pData);
    }


#if defined (_WIN32)   //DM-Android
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename UnderlyingType, typename T>
bool AttributeManager::GetTyped    (AttributeID     pi_ID,
                                    T*&             po_rpData,
                                    size_t&         po_Size) const
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    HSTATICASSERT((sizeof T) >= (sizeof UnderlyingType));
    HSTATICASSERT(0 == (sizeof T) % (sizeof UnderlyingType));
#endif

    UnderlyingType* pAttributeData;
    size_t          AttributeDataSize;
    if (!Get(pi_ID, pAttributeData, AttributeDataSize))
        return false;

    po_rpData = reinterpret_cast<T*>(pAttributeData);
    po_Size = AttributeDataSize/((sizeof T)/(sizeof UnderlyingType));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename UnderlyingType, typename T>
bool AttributeManager::SetTyped    (AttributeID     pi_ID,
                                    const T*        pi_pData,
                                    size_t          pi_Size)
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    HSTATICASSERT((sizeof T) >= (sizeof UnderlyingType));
    HSTATICASSERT(0 == (sizeof T) % (sizeof UnderlyingType));
#endif

    return Set(pi_ID, reinterpret_cast<const UnderlyingType*>(pi_pData), pi_Size*((sizeof T)/(sizeof UnderlyingType)));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename UnderlyingType>
inline bool AttributeManager::GetPacket(AttributeID     pi_ID,
                                        Packet&         po_rPacket) const
    {
    UnderlyingType* pAttributeData;
    size_t          AttributeDataSize;
    if (!Get(pi_ID, pAttributeData, AttributeDataSize))
        return false;

    po_rPacket.Wrap(reinterpret_cast<Packet::value_type*>(pAttributeData), AttributeDataSize*(sizeof UnderlyingType));

    HPOSTCONDITION(0 == (po_rPacket.GetSize()%(sizeof UnderlyingType)));
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::GetPacket<Byte>  (AttributeID     pi_ID,
                                                Packet&         po_rPacket) const
    {
    Byte*   pAttributeData;
    size_t  AttributeDataSize;
    if (!Get(pi_ID, pAttributeData, AttributeDataSize))
        return false;

    po_rPacket.Wrap(pAttributeData, AttributeDataSize);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename UnderlyingType>
inline bool AttributeManager::SetPacket    (AttributeID         pi_ID,
                                            const Packet&       pi_rPacket)
    {
    HPRECONDITION(0 == (pi_rPacket.GetSize()%(sizeof UnderlyingType)));
    return Set(pi_ID, reinterpret_cast<const UnderlyingType*>(pi_rPacket.Get()), pi_rPacket.GetSize()/(sizeof UnderlyingType));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<>
inline bool AttributeManager::SetPacket<Byte>  (AttributeID         pi_ID,
                                                const Packet&       pi_rPacket)
    {
    return Set(pi_ID, pi_rPacket.Get(), pi_rPacket.GetSize());
    }