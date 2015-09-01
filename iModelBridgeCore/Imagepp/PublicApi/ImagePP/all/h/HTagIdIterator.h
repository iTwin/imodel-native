//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTagIdIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/h/HIterators.h>

BEGIN_IMAGEPP_NAMESPACE
class HTIFFTagEntry;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class HTagIdIter : public BentleyApi::ImagePP::ForwardIterator<HTagIdIter, uint32_t>
    {
public:
    IMAGEPP_EXPORT explicit         HTagIdIter                   ();

    // Hide base class version in order to return by value instead than by reference
    value_type              operator*                      ()                   {
        return Dereference();
        }
    value_type              operator*                      () const             {
        return Dereference();
        }

    friend class            HTIFFDirectory;

    typedef value_type      TagID;


    value_type              Dereference                    () const;

    IMAGEPP_EXPORT void             Increment                      ();

    bool                    EqualTo                        (const iterator_t&               pi_rRight) const;

    explicit                HTagIdIter                     (const HTIFFTagEntry* const*     pi_pTag,
                                                            const HTIFFTagEntry* const*     pi_pTagBegin,
                                                            const HTIFFTagEntry* const*     pi_pTagEnd);

    const HTIFFTagEntry* const*
    m_ppTag;
    const HTIFFTagEntry* const*
    m_ppTagBegin;
    const HTIFFTagEntry* const*
    m_ppTagEnd;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline HTagIdIter::value_type HTagIdIter::Dereference () const
    {
    HPRECONDITION(0 != m_ppTagBegin);
    return static_cast<value_type>(std::distance(m_ppTagBegin, m_ppTag));
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool HTagIdIter::EqualTo (const iterator_t& pi_rRight) const
    {
    return m_ppTag == pi_rRight.m_ppTag;
    }
END_IMAGEPP_NAMESPACE
