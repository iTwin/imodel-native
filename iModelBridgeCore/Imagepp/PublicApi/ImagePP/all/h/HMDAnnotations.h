//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotations.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include "HMDAnnotationInfo.h"

class HMDAnnotations : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(7011, HMDMetaDataContainer);

public :
    _HDLLu HMDAnnotations();
    _HDLLu virtual ~HMDAnnotations();

    _HDLLu HMDAnnotations(const HMDAnnotations& pi_rObj);

    _HDLLu void                         AddAnnotation  (const HMDAnnotationInfo* pi_pAnnotations);

    _HDLLu const HMDAnnotationInfo*     GetAnnotation  (uint32_t pi_Index) const;
    _HDLLu virtual const HMDAnnotationInfo*     GetAnnotation   (double pi_PosX,
                                                          double pi_PosY) const = 0;

    _HDLLu uint32_t                             GetNbAnnotations() const;

protected:

    //Avoid the copy
    typedef vector<const HMDAnnotationInfo*> AnnotationList;
    AnnotationList                           m_pAnnotations;

private :
    HMDAnnotations& operator=(const HMDAnnotations& pi_rObj);

    void CopyMemberData(const HMDAnnotations& pi_rObj);
    };

