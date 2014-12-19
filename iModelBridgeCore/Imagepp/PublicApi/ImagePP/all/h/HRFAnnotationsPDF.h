//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAnnotationsPDF.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDAnnotations.h"

class HRFAnnotationsPDF : public HMDAnnotations
    {
    HDECLARE_CLASS_ID(8051, HMDAnnotations);

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
