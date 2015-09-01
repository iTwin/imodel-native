//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDAnnotations.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>ayer
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDAnnotations.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDAnnotations::HMDAnnotations()
    : HMDMetaDataContainer(HMDMetaDataContainer::HMD_ANNOTATION_INFO)
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDAnnotations::~HMDAnnotations()
    {
    AnnotationList::iterator AnnotationIter = m_pAnnotations.begin();
    AnnotationList::iterator AnnotationIterEnd = m_pAnnotations.end();
    while (AnnotationIter < AnnotationIterEnd)
        {
        delete *AnnotationIter;
        AnnotationIter++;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDAnnotations::HMDAnnotations(const HMDAnnotations& pi_rObj)
    : HMDMetaDataContainer(pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Add an annotation
//-----------------------------------------------------------------------------
void HMDAnnotations::AddAnnotation(const HMDAnnotationInfo* pi_pAnnotation)
    {
    m_pAnnotations.push_back(pi_pAnnotation);
    }

//-----------------------------------------------------------------------------
// Public
// Get an annotation
//-----------------------------------------------------------------------------
const HMDAnnotationInfo* HMDAnnotations::GetAnnotation(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_pAnnotations.size());

    return m_pAnnotations[pi_Index];
    }

//-----------------------------------------------------------------------------
// Public
// Get the number of annotations
//-----------------------------------------------------------------------------
uint32_t HMDAnnotations::GetNbAnnotations() const
    {
    return (uint32_t)m_pAnnotations.size();
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDAnnotations::CopyMemberData(const HMDAnnotations& pi_rObj)
    {
    AnnotationList::const_iterator AnnotIter    = pi_rObj.m_pAnnotations.begin();
    AnnotationList::const_iterator AnnotIterEnd = pi_rObj.m_pAnnotations.end();

    while (AnnotIter != AnnotIterEnd)
        {
        m_pAnnotations.push_back(new HMDAnnotationInfo(*(*AnnotIter)));
        AnnotIter++;
        }
    }