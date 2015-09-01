//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotationInfo.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    IMAGEPP_EXPORT HMDAnnotationInfo(const WString& pi_rMsg,
                      bool          pi_IsSupported,
                      const WString& pi_rAnnotationType = L"");
    IMAGEPP_EXPORT virtual                 ~HMDAnnotationInfo();

    IMAGEPP_EXPORT HMDAnnotationInfo(const HMDAnnotationInfo& pi_rObj);


    IMAGEPP_EXPORT const WString&          GetAnnotationType() const;
    IMAGEPP_EXPORT const WString&          GetAnnotationMsg() const;

    IMAGEPP_EXPORT bool                   IsSupported() const;

protected:

    WString m_AnnotationType;

private :
    HMDAnnotationInfo& operator=(const HMDAnnotationInfo& pi_rObj);

    void CopyMemberData(const HMDAnnotationInfo& pi_rObj);

    WString m_Msg;
    bool   m_IsSupported;
    };

END_IMAGEPP_NAMESPACE
