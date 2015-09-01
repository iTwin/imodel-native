//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTagDefinition.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Static method used by external module.
//

BEGIN_IMAGEPP_NAMESPACE
inline size_t  HTagInfo::sGetDataLen(DataType pi_Type)
    {
    return sDataLen[pi_Type];
    }

inline bool HTagInfo::IsVariableSizeTag (HTagID pi_TagID) const
    {
    HPRECONDITION(pi_TagID < GetTagDefinitionQty());
    return TAG_IO_VARIABLE == GetTagDefinitionArray()[pi_TagID].ReadCount &&
           TAG_IO_VARIABLE == GetTagDefinitionArray()[pi_TagID].WriteCount;
    }

inline bool HTagDefinition::IsValid(HTIFFError**  po_ppError) const
    {
    if (po_ppError != 0)
        *po_ppError = m_pError;

    return (bool)(m_pTagInfo != 0);
    }

inline uint32_t HTagDefinition::GetFileTag() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->FileTag;
    }

inline HTagID HTagDefinition::GetID() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->TagID;
    }

inline short HTagDefinition::GetReadCount() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->ReadCount;
    }

inline short HTagDefinition::GetWriteCount() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->WriteCount;
    }


inline HTagInfo::DataType HTagDefinition::GetDataType() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->Type;
    }

inline uint32_t HTagDefinition::GetDataLen() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return static_cast<uint32_t>(HTagInfo::sGetDataLen(m_pTagInfo->Type));
    }


inline bool HTagDefinition::IsReadOnly() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return !m_pTagInfo->ReadWriteTag;
    }

inline bool HTagDefinition::IsPassDirCount() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->PassDirCount != 0;
    }

inline const char* HTagDefinition::GetTagName() const
    {
    HPRECONDITION(m_pTagInfo != 0);

    return m_pTagInfo->pTagName;
    }

END_IMAGEPP_NAMESPACE
