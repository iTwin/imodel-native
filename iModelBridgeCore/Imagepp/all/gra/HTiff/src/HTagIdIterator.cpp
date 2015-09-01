//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTagIdIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/HTagIdIterator.h>
#include <ImagePP/all/h/HTIFFTagEntry.h>

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HTagIdIter::HTagIdIter ()
    :   m_ppTag(0),
        m_ppTagBegin(0),
        m_ppTagEnd(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HTagIdIter::HTagIdIter (const HTIFFTagEntry* const*    pi_ppTag,
                        const HTIFFTagEntry* const*    pi_ppTagBegin,
                        const HTIFFTagEntry* const*    pi_ppTagEnd)
    :   m_ppTag(pi_ppTag),
        m_ppTagBegin(pi_ppTagBegin),
        m_ppTagEnd(pi_ppTagEnd)
    {
    // Ensure that we point on a valid packet
    if ((m_ppTag < m_ppTagEnd) && (0 == *m_ppTag))
        Increment();


    HPRECONDITION(m_ppTagBegin <= m_ppTag);
    HPRECONDITION(m_ppTagEnd >= m_ppTag);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HTagIdIter::Increment ()
    {
    HPRECONDITION(m_ppTagEnd != m_ppTag);

    while ((m_ppTag < m_ppTagEnd) && (0 == *(++m_ppTag))) {};
    }