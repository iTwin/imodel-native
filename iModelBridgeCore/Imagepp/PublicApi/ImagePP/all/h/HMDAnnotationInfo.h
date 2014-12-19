//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotationInfo.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

class HMDAnnotations;

class HMDAnnotationInfo : public HMDMetaData
    {
    friend class HMDAnnotations;

    HDECLARE_CLASS_ID(7010, HMDMetaData);

public :
    _HDLLu HMDAnnotationInfo(const WString& pi_rMsg,
                      bool          pi_IsSupported,
                      const WString& pi_rAnnotationType = L"");
    _HDLLu virtual                 ~HMDAnnotationInfo();

    _HDLLu HMDAnnotationInfo(const HMDAnnotationInfo& pi_rObj);


    _HDLLu const WString&          GetAnnotationType() const;
    _HDLLu const WString&          GetAnnotationMsg() const;

    _HDLLu bool                   IsSupported() const;

protected:

    WString m_AnnotationType;

private :
    HMDAnnotationInfo& operator=(const HMDAnnotationInfo& pi_rObj);

    void CopyMemberData(const HMDAnnotationInfo& pi_rObj);

    WString m_Msg;
    bool   m_IsSupported;
    };

