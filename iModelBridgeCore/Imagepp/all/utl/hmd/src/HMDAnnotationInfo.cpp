//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDAnnotationInfo.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDAnnotationInfo.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDAnnotationInfo::HMDAnnotationInfo(const Utf8String& pi_rMsg,
                                            bool          pi_IsSupported,
                                            const Utf8String& pi_rAnnotationType)
    {
    m_Msg            = pi_rMsg;
    m_IsSupported    = pi_IsSupported;
    m_AnnotationType = pi_rAnnotationType;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDAnnotationInfo::~HMDAnnotationInfo()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDAnnotationInfo::HMDAnnotationInfo(const HMDAnnotationInfo& pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Return the annotation's message
//-----------------------------------------------------------------------------
const Utf8String& HMDAnnotationInfo::GetAnnotationMsg() const
    {
    return m_Msg;
    }

//-----------------------------------------------------------------------------
// Public
// Return the annotation's type
//-----------------------------------------------------------------------------
const Utf8String& HMDAnnotationInfo::GetAnnotationType() const
    {
    return m_AnnotationType;
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the annotation is supported
//-----------------------------------------------------------------------------
bool HMDAnnotationInfo::IsSupported() const
    {
    return m_IsSupported;
    }

//-----------------------------------------------------------------------------
// Private
// Copy the member data
//-----------------------------------------------------------------------------
void HMDAnnotationInfo::CopyMemberData(const HMDAnnotationInfo& pi_rObj)
    {
    m_Msg            = pi_rObj.m_Msg;
    m_AnnotationType = pi_rObj.m_AnnotationType;
    m_IsSupported    = pi_rObj.m_IsSupported;
    }
