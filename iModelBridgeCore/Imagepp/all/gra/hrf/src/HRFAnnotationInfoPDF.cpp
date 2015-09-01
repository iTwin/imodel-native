//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAnnotationInfoPDF.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFAnnotationInfoPDF.h>
#include <Imagepp/all/h/HRFAnnotationsPDF.h>

#include <Imagepp/all/h/HVE2DComplexLinear.h>
#include <Imagepp/all/h/HVE2DDisjointedComplexLinear.h>
#include <Imagepp/all/h/HVE2DSimpleShape.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFAnnotationsPDF::HRFAnnotationsPDF()
    {
    }
//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFAnnotationsPDF::~HRFAnnotationsPDF()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HRFAnnotationsPDF::HRFAnnotationsPDF(const HRFAnnotationsPDF& pi_rObj)
    : HMDAnnotations(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HFCPtr<HMDMetaDataContainer> HRFAnnotationsPDF::Clone() const
    {
    return HFCPtr<HMDMetaDataContainer>(new HRFAnnotationsPDF(*this));
    }

//-----------------------------------------------------------------------------
// Public
// Get an annotation
//-----------------------------------------------------------------------------
const HMDAnnotationInfo* HRFAnnotationsPDF::GetAnnotation(double pi_PosX,
                                                                 double pi_PosY) const
    {
    const HMDAnnotationInfo* pAnnotationInfo = 0;

    if (m_pAnnotations.size() > 0)
        {
        //Use the same CS that is used by all annotations of this container to optimize
        //the search
        HFCPtr<HVE2DVector> pSelectZone;

        ((HRFAnnotationInfoPDF*)m_pAnnotations[0])->GetSelectionZone(pSelectZone);

        HGF2DLocation TestPoint(pi_PosX,
                                pi_PosY,
                                pSelectZone->GetCoordSys());

        for (uint32_t AnnotInd = 0; AnnotInd < m_pAnnotations.size(); AnnotInd++)
            {
            if (((HRFAnnotationInfoPDF*)m_pAnnotations[AnnotInd])->IsPointOver(TestPoint) == true)
                {
                pAnnotationInfo = m_pAnnotations[AnnotInd];
                }
            }
        }

    return pAnnotationInfo;
    }

//-----------------------------------------------------------------------------
// Public
// Return a collection of points for each basic 2d vector element.
//-----------------------------------------------------------------------------
const HRFAnnotationInfoPDF::PointCollections& HRFAnnotationInfoPDF::GetSelectZonePoints() const
    {
    if (m_SelectZonePointCollections.size() == 0)
        {
        double MinDistance = 0.0;

        if (m_p2DSelectionZone->IsCompatibleWith(HVE2DDisjointedComplexLinear::CLASS_ID))
            {
            const HVE2DComplexLinear::LinearList& rLinears(((HFCPtr<HVE2DDisjointedComplexLinear>&)m_p2DSelectionZone)->
                                                           GetLinearList());

            HVE2DComplexLinear::LinearList::const_iterator LinearIter(rLinears.begin());
            HVE2DComplexLinear::LinearList::const_iterator LinearIterEnd(rLinears.end());
            HAutoPtr<HGF2DLocationCollection>              pPointCollection;

            while (LinearIter != LinearIterEnd)
                {
                pPointCollection = new HGF2DLocationCollection();
                (*LinearIter)->Drop(pPointCollection.get(),
                                    MinDistance);
                m_SelectZonePointCollections.push_back(pPointCollection);
                pPointCollection.release();
                LinearIter++;
                }
            }
        else if (m_p2DSelectionZone->IsCompatibleWith(HVE2DLinear::CLASS_ID))
            {
            HAutoPtr<HGF2DLocationCollection> pPointCollection(new HGF2DLocationCollection());

            ((HFCPtr<HVE2DLinear>&)m_p2DSelectionZone)->Drop(pPointCollection,
                                                             MinDistance);
            m_SelectZonePointCollections.push_back(pPointCollection);
            pPointCollection.release();
            }
        else if (m_p2DSelectionZone->IsCompatibleWith(HVE2DShape::CLASS_ID))
            {
            HGF2DLocationCollection PointCollection;

            //Save the current tolerance
            double Tol(m_p2DSelectionZone->GetTolerance());

            //242398 - Set a very small tolerance before requesting the collection of
            //points because the tolerance is used by some shapes to verify that
            //the shape is not empty before returning the point collection.
            m_p2DSelectionZone->SetTolerance(DBL_EPSILON * 2);

            ((HFCPtr<HVE2DShape>&)m_p2DSelectionZone)->Drop(&PointCollection,
                                                            MinDistance);

            m_p2DSelectionZone->SetTolerance(Tol);

            //Some points should be found, but the code doesn't just HASSERT that to ensure
            //that no crash happens if really no point are found.
            HASSERT(PointCollection.size() > 0);

            if (PointCollection.size() > 0)
                {
                HAutoPtr<HGF2DLocationCollection> pPointCollection(new HGF2DLocationCollection());

                HGF2DLocationCollection::iterator PointIter(PointCollection.begin());
                HGF2DLocationCollection::iterator PointIterEnd(PointCollection.end());

                HGF2DLocation FirstShapePoint(*PointIter);

                while (PointIter != PointIterEnd)
                    {
                    pPointCollection->push_back(*PointIter);
                    PointIter++;

                    if ((PointIter != PointIterEnd) && (FirstShapePoint == *PointIter))
                        {
                        pPointCollection->push_back(*PointIter);

                        m_SelectZonePointCollections.push_back(pPointCollection);
                        pPointCollection.release();
                        pPointCollection = new HGF2DLocationCollection();

                        PointIter++;

                        if (PointIter != PointIterEnd)
                            {
                            FirstShapePoint = *PointIter;
                            }
                        }
                    }
                }
            }
        else
            {
            HASSERT(0);
            }
        }

    return m_SelectZonePointCollections;
    }

//-----------------------------------------------------------------------------
// Protected
// Return true if the point passed as parameter is on the annotation's
// selection zone.
//-----------------------------------------------------------------------------
bool HRFAnnotationInfoPDF::IsPointOver(const HGF2DLocation& pi_rTestPoint) const
    {
    bool IsPointOver = false;

    if (m_IsFilled == false)
        {
        IsPointOver = m_p2DSelectionZone->IsPointOnSCS(pi_rTestPoint);
        }
    else
        {
        HGFGraphicObject::Location PointLoc = m_p2DSelectionZone->Locate(pi_rTestPoint);

        IsPointOver = (PointLoc != HGFGraphicObject::S_OUTSIDE);
        }

    return IsPointOver;
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HRFAnnotationInfoPDF::CopyMemberData(const HRFAnnotationInfoPDF& pi_rObj)
    {
    HPMPersistentObject* pCloned2DVector = pi_rObj.m_p2DSelectionZone->Clone();

    HASSERT(pCloned2DVector->IsCompatibleWith(HVE2DVector::CLASS_ID) == true);

    m_p2DSelectionZone = (HVE2DVector*)pCloned2DVector;
    m_IsFilled         = pi_rObj.m_IsFilled;
    m_AnnotationType   = pi_rObj.m_AnnotationType;

    //The following member used for caching is not copied
    //m_SelectZonePointCollections
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFAnnotationInfoPDF::HRFAnnotationInfoPDF(const WString&             pi_rMsg,
                                                  const HFCPtr<HVE2DVector>& pi_rp2DSelectionZone,
                                                  const WString&             pi_rAnnotationType,
                                                  bool                      pi_IsSupported,
                                                  double                    pi_SetTolerance,
                                                  bool                      pi_IsFilled)
    : HMDAnnotationInfo(pi_rMsg, pi_IsSupported, pi_rAnnotationType)
    {
    m_IsFilled         = pi_IsFilled;
    m_p2DSelectionZone = pi_rp2DSelectionZone;
    m_p2DSelectionZone->SetTolerance(pi_SetTolerance);
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFAnnotationInfoPDF::~HRFAnnotationInfoPDF()
    {
    PointCollections::iterator CollectionIter(m_SelectZonePointCollections.begin());
    PointCollections::iterator CollectionIterEnd(m_SelectZonePointCollections.end());

    while (CollectionIter < CollectionIterEnd)
        {
        delete *CollectionIter;
        CollectionIter++;
        }
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HRFAnnotationInfoPDF::HRFAnnotationInfoPDF(const HRFAnnotationInfoPDF& pi_rObj)
    : HMDAnnotationInfo(pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Return the width in image pixel of each linear segment composing the shape
//-----------------------------------------------------------------------------
unsigned short HRFAnnotationInfoPDF::GetLineWidth() const
    {
    return ((unsigned short)m_p2DSelectionZone->GetTolerance()) * 2;
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the point passed as parameter is on the annotation's
// selection zone.
//-----------------------------------------------------------------------------
bool HRFAnnotationInfoPDF::IsFilled() const
    {
    return m_IsFilled;
    }

//-----------------------------------------------------------------------------
// Protected
// Return the coordinate system of the selection zone.
//-----------------------------------------------------------------------------
void HRFAnnotationInfoPDF::GetSelectionZone(HFCPtr<HVE2DVector>& po_rSelectZone) const
    {
    HPRECONDITION(m_p2DSelectionZone != 0);

    po_rSelectZone = m_p2DSelectionZone;
    }