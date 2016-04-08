//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotationInfo.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDAnnotations;

class HMDAnnotationInfo : public HMDMetaData
    {
    friend class HMDAnnotations;

    HDECLARE_CLASS_ID(HMDAnnotationId_Info, HMDMetaData);

public :
    IMAGEPP_EXPORT HMDAnnotationInfo(const Utf8String& pi_rMsg,
                      bool          pi_IsSupported,
                      const Utf8String& pi_rAnnotationType = "");
    IMAGEPP_EXPORT virtual                 ~HMDAnnotationInfo();

    IMAGEPP_EXPORT HMDAnnotationInfo(const HMDAnnotationInfo& pi_rObj);


    IMAGEPP_EXPORT const Utf8String&          GetAnnotationType() const;
    IMAGEPP_EXPORT const Utf8String&          GetAnnotationMsg() const;

    IMAGEPP_EXPORT bool                   IsSupported() const;

protected:

    Utf8String m_AnnotationType;

private :
    HMDAnnotationInfo& operator=(const HMDAnnotationInfo& pi_rObj);

    void CopyMemberData(const HMDAnnotationInfo& pi_rObj);

    Utf8String m_Msg;
    bool   m_IsSupported;
    };

END_IMAGEPP_NAMESPACE
