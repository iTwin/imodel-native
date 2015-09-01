//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFTagEntry.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
inline bool HTIFFTagEntry::IsValid(HTIFFError**  po_ppError) const
    {
    if (po_ppError != 0)
        *po_ppError = m_pError;

    return (bool)(m_pEntry != 0);
    }


inline HTagID HTIFFTagEntry::GetTagID() const
    {
    HPRECONDITION(m_pEntry != 0);

    return m_pTagDef->GetID();
    }

inline HTagInfo::DataType HTIFFTagEntry::GetTagType() const
    {
    HPRECONDITION(m_pEntry != 0);

    return m_pTagDef->GetDataType();
    }

inline const char* HTIFFTagEntry::GetTagNameString() const
    {
    HPRECONDITION(m_pEntry != 0);

    return m_pTagDef->GetTagName();
    }

// This method is used to force the rewrite of the Tag
inline void HTIFFTagEntry::Touched ()
    {
    HPRECONDITION(m_pEntry != 0);

    m_pEntry->Status.Dirty  = true;
    }


END_IMAGEPP_NAMESPACE
