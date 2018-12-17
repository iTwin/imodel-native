//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAnnotationsPDF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDAnnotations.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFAnnotationsPDF : public HMDAnnotations
    {
    HDECLARE_CLASS_ID(HRFAnnotationsPDFId_Base, HMDAnnotations);

public :
    HRFAnnotationsPDF();
    virtual ~HRFAnnotationsPDF();

    HRFAnnotationsPDF(const HRFAnnotationsPDF& pi_rObj);

    virtual HFCPtr<HMDMetaDataContainer> Clone() const;

    virtual const HMDAnnotationInfo*     GetAnnotation(double pi_PosX,
                                                       double pi_PosY) const;

protected:

private :
    HRFAnnotationsPDF& operator=(const HRFAnnotationsPDF& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
