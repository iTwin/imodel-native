//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFDirectory.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
inline bool HTIFFDirectory::IsValid(HTIFFError**  po_ppError) const
    {
    if (po_ppError != 0)
        *po_ppError = m_pError;

    return (bool)(m_pError == 0);
    }


inline bool HTIFFDirectory::TagIsPresent(HTagID pi_Tag) const
    {
    return (m_ppDirEntry[pi_Tag] != 0);
    }

inline const char* HTIFFDirectory::GetTagNameString(HTagID pi_Tag) const
    {
    if (TagIsPresent(pi_Tag))
        return m_ppDirEntry[pi_Tag]->GetTagNameString();
    else
        return 0;
    }

inline HTagInfo::DataType HTIFFDirectory::GetTagDataType(HTagID pi_Tag) const
    {
    if (TagIsPresent(pi_Tag))
        return m_ppDirEntry[pi_Tag]->GetTagType();
    else
        return HTagInfo::_NOTYPE;
    }


inline bool HTIFFDirectory::Touched (HTagID pi_Tag)
    {
    if (TagIsPresent(pi_Tag))
        {
        m_Status.Dirty = true;
        m_ppDirEntry[pi_Tag]->Touched();
        return true;
        }
    else
        return false;
    }

inline HTIFFGeoKey& HTIFFDirectory::GetGeoKeyInterpretation ()
    {
    return *m_pGeoKeys;
    }

END_IMAGEPP_NAMESPACE
