//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotations.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include "HMDAnnotationInfo.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDAnnotations : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(HMDAnnotationsId_Base, HMDMetaDataContainer);

public :
    IMAGEPP_EXPORT HMDAnnotations();
    IMAGEPP_EXPORT virtual ~HMDAnnotations();

    IMAGEPP_EXPORT HMDAnnotations(const HMDAnnotations& pi_rObj);

    IMAGEPP_EXPORT void                         AddAnnotation  (const HMDAnnotationInfo* pi_pAnnotations);

    IMAGEPP_EXPORT const HMDAnnotationInfo*     GetAnnotation  (uint32_t pi_Index) const;
    IMAGEPP_EXPORT virtual const HMDAnnotationInfo*     GetAnnotation   (double pi_PosX,
                                                          double pi_PosY) const = 0;

    IMAGEPP_EXPORT uint32_t                             GetNbAnnotations() const;

protected:

    //Avoid the copy
    typedef vector<const HMDAnnotationInfo*> AnnotationList;
    AnnotationList                           m_pAnnotations;

private :
    HMDAnnotations& operator=(const HMDAnnotations& pi_rObj);

    void CopyMemberData(const HMDAnnotations& pi_rObj);
    };

END_IMAGEPP_NAMESPACE