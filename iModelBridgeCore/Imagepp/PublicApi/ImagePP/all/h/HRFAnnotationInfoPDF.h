//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAnnotationInfoPDF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDAnnotationInfo.h"

#include "HGF2DLocation.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DVector;
class HRFAnnotationsPDF;

class HRFAnnotationInfoPDF : public HMDAnnotationInfo
    {
    friend class HRFAnnotationsPDF;

    HDECLARE_CLASS_ID(HRFAnnotationsPDFId_Info, HMDAnnotationInfo);

public :

    typedef std::vector<HFCPtr<HVE2DVector>> SegmentVector;
    typedef vector<HGF2DLocationCollection*> PointCollections;


    HRFAnnotationInfoPDF(const WString&             pi_rMsg,
                         const HFCPtr<HVE2DVector>& pi_rp2DSelectionZone,
                         const WString&             pi_rAnnotationType,
                         bool                      pi_IsSupported,
                         double                    pi_SetTolerance = 2.0,
                         bool                      pi_IsFilled = false);
    virtual               ~HRFAnnotationInfoPDF();

    HRFAnnotationInfoPDF(const HRFAnnotationInfoPDF& pi_rObj);

    //Methods returning information about the selection zone
    IMAGEPP_EXPORT const PointCollections& GetSelectZonePoints() const;
    IMAGEPP_EXPORT unsigned short          GetLineWidth() const;
    IMAGEPP_EXPORT bool                    IsFilled() const;

protected:

    bool                 IsPointOver(const HGF2DLocation& pi_rTestPoint) const;

    void                  GetSelectionZone(HFCPtr<HVE2DVector>& po_rSelectZone) const;

private :
    HRFAnnotationInfoPDF& operator=(const HRFAnnotationInfoPDF& pi_rObj);

    void                  CopyMemberData(const HRFAnnotationInfoPDF& pi_rObj);

    HFCPtr<HVE2DVector>                m_p2DSelectionZone;
    bool                              m_IsFilled;
    mutable PointCollections           m_SelectZonePointCollections;
    };
END_IMAGEPP_NAMESPACE
