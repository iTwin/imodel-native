/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/MeasureGeom.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct MeasureGeomCollector;
typedef RefCountedPtr<MeasureGeomCollector> MeasureGeomCollectorPtr;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MeasureGeomCollector : RefCountedBase, IElementGraphicsProcessor
{
public:

enum OperationType
    {
    AccumulateLengths = 0,
    AccumulateAreas   = 1,
    AccumulateVolumes = 2,
    };

//__PUBLISH_SECTION_END__
struct IGeomProvider
{
//! Allow an application to accumulate measure results by drawing directly to output without creating elements.
virtual void _OutputGraphics (ViewContextR context) = 0;
};

protected:

OperationType       m_opType;
IGeomProvider*      m_geomProvider;

ViewContextP        m_context;
IFacetOptionsPtr    m_facetOptions;
Transform           m_currentTransform;
Transform           m_invCurrTransform;
Transform           m_preFlattenTransform;
bool                m_inFlatten;

double              m_amountSum;    // Accumated area or volume for centroid/moment calculation...
double              m_volumeSum;
double              m_areaSum;
double              m_perimeterSum;
double              m_lengthSum;
double              m_iXY;
double              m_iXZ;
double              m_iYZ;
DPoint3d            m_moment1;
DPoint3d            m_moment2;
double              m_closureError;
int                 m_calculationMode;  // 0=best available, 1=PSD preferred, 2=facets preferred.

mutable DPoint3d    m_spinMoments; // Holds computed result...
mutable DPoint3d    m_centroidSum; // Holds computed result...

virtual bool _ProcessAsFacets (bool isPolyface) const override {return true;} // Resort to facets for anything not specifically handled...

virtual IFacetOptionsP _GetFacetOptionsP () override;

virtual void _AnnounceContext (ViewContextR) override;
virtual void _AnnounceTransform (TransformCP) override;

virtual BentleyStatus _ProcessCurveVector (CurveVectorCR, bool isFilled) override;
virtual BentleyStatus _ProcessSolidPrimitive (ISolidPrimitiveCR) override;
virtual BentleyStatus _ProcessSurface (MSBsplineSurfaceCR) override;
virtual BentleyStatus _ProcessFacets (PolyfaceQueryCR, bool isFilled) override;
virtual BentleyStatus _ProcessBody (ISolidKernelEntityCR) override;

virtual DrawPurpose _GetDrawPurpose () override {return DrawPurpose::Measure;}
virtual void _OutputGraphics (ViewContextR context) override;

void AccumulateVolumeSums (double volumeB, double areaB, double closureErrorB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateAreaSums (double areaB, double perimeterB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (double lengthB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (DMatrix4dCR products);

BentleyStatus DoAccumulateLengths (CurveVectorCR);
BentleyStatus DoAccumulateLengths (ISolidKernelEntityCR);

BentleyStatus DoAccumulateAreas (CurveVectorCR);
BentleyStatus DoAccumulateAreas (ISolidPrimitiveCR);
BentleyStatus DoAccumulateAreas (MSBsplineSurfaceCR);
BentleyStatus DoAccumulateAreas (PolyfaceQueryCR);
BentleyStatus DoAccumulateAreas (ISolidKernelEntityCR);

BentleyStatus DoAccumulateVolumes (ISolidPrimitiveCR);
BentleyStatus DoAccumulateVolumes (PolyfaceQueryCR);
BentleyStatus DoAccumulateVolumes (ISolidKernelEntityCR);

void GetOutputTransform (TransformR transform);
bool GetPreFlattenTransform (TransformR flattenTransform);

BentleyStatus GetOperationStatus ();

MeasureGeomCollector (OperationType);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

//! 
DGNPLATFORM_EXPORT double GetVolume () const;

//! 
DGNPLATFORM_EXPORT double GetArea () const;

//! 
DGNPLATFORM_EXPORT double GetPerimeter () const;

//! 
DGNPLATFORM_EXPORT double GetLength () const;

//! 
DGNPLATFORM_EXPORT double GetClosureError () const;

//! 
DGNPLATFORM_EXPORT DPoint3dCR GetCentroid () const;

//! 
DGNPLATFORM_EXPORT double GetIXY () const;

//! 
DGNPLATFORM_EXPORT double GetIXZ () const;

//! 
DGNPLATFORM_EXPORT double GetIYZ () const;

//! 
DGNPLATFORM_EXPORT DPoint3dCR GetMoments () const;

//! 
DGNPLATFORM_EXPORT static BentleyStatus CalculatePrincipalAxes
(
DPoint3dP   principalMomentsP,      // <=  opt: {Ixx, Iyy, Izz} about principal axes
DPoint3dP   principalDirectionsP,   // <=  opt: vectors along 3 principal axes
DPoint3dCP  i,                      //  => Ixx, Iyy, Izz
double      iXY,                    //  => Ixy
double      iXZ,                    //  => Ixz
double      iYZ                     //  => Iyz
);

//__PUBLISH_SECTION_END__
//! Supply facet options and output transform (only rigid scaling is supported).
DGNPLATFORM_EXPORT void SetResultOptions (IFacetOptionsP options, TransformCP invCurrTrans = NULL);

//! Supply a flattening transform to be applied prior to doing length/area measurement (makes no sense for volume, caller is expected to valid input element/geometry).
DGNPLATFORM_EXPORT void SetPreFlattenTransform (TransformCR preFlattenTrans);

//! Select method of calculation
//! Values are:
//! 0 ==> no preference -- internal when available, parasolids if needed.
//! 1 ==> prefer parasolids
//! 2 ==> prefer faceted
DGNPLATFORM_EXPORT void SetCalculationMethod (int selector);

//! Query the calculation mode.
DGNPLATFORM_EXPORT int GetCalculationMethod () const;

//! Call IGeomProvider::_OutputGraphics instead of visiting an element.
DGNPLATFORM_EXPORT BentleyStatus Process (IGeomProvider&, DgnDbR);
//__PUBLISH_SECTION_START__

//! Visit the supplied element and accumulate the measure information.
DGNPLATFORM_EXPORT BentleyStatus Process (GeometricElementCR);

//! Create new instance of a measure geometry collector.
DGNPLATFORM_EXPORT static MeasureGeomCollectorPtr Create (OperationType opType);

}; // MeasureGeomCollector

END_BENTLEY_DGN_NAMESPACE

