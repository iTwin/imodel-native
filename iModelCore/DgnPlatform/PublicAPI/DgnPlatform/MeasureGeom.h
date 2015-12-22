/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MeasureGeom.h $
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
struct MeasureGeomCollector : RefCountedBase, IGeometryProcessor
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

IFacetOptionsPtr    m_facetOptions;
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

virtual DrawPurpose _GetProcessPurpose() const override {return DrawPurpose::Measure;}
virtual IFacetOptionsP _GetFacetOptionsP() override;

virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR) const override {return UnhandledPreference::Auto;}
virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR) const override {return UnhandledPreference::Auto;}
virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR) const override {return UnhandledPreference::Auto;}

virtual bool _ProcessCurveVector (CurveVectorCR, bool isFilled, SimplifyGraphic const&) override;
virtual bool _ProcessSolidPrimitive (ISolidPrimitiveCR, SimplifyGraphic const&) override;
virtual bool _ProcessSurface (MSBsplineSurfaceCR, SimplifyGraphic const&) override;
virtual bool _ProcessPolyface (PolyfaceQueryCR, bool isFilled, SimplifyGraphic const&) override;
virtual bool _ProcessBody (ISolidKernelEntityCR, SimplifyGraphic const&) override;

virtual void _OutputGraphics (ViewContextR context) override;

void AccumulateVolumeSums (double volumeB, double areaB, double closureErrorB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateAreaSums (double areaB, double perimeterB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (double lengthB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (DMatrix4dCR products);

bool DoAccumulateLengths (CurveVectorCR, SimplifyGraphic const& graphic);
bool DoAccumulateLengths (ISolidKernelEntityCR, SimplifyGraphic const& graphic);

bool DoAccumulateAreas (CurveVectorCR, SimplifyGraphic const& graphic);
bool DoAccumulateAreas (ISolidPrimitiveCR, SimplifyGraphic const& graphic);
bool DoAccumulateAreas (MSBsplineSurfaceCR, SimplifyGraphic const& graphic);
bool DoAccumulateAreas (PolyfaceQueryCR, SimplifyGraphic const& graphic);
bool DoAccumulateAreas (ISolidKernelEntityCR, SimplifyGraphic const& graphic);

bool DoAccumulateVolumes (ISolidPrimitiveCR, SimplifyGraphic const& graphic);
bool DoAccumulateVolumes (PolyfaceQueryCR, SimplifyGraphic const& graphic);
bool DoAccumulateVolumes (ISolidKernelEntityCR, SimplifyGraphic const& graphic);

void GetOutputTransform (TransformR transform, SimplifyGraphic const& graphic);
bool GetPreFlattenTransform (TransformR flattenTransform, SimplifyGraphic const& graphic);

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
DGNPLATFORM_EXPORT BentleyStatus Process (GeometrySourceCR);

//! Create new instance of a measure geometry collector.
DGNPLATFORM_EXPORT static MeasureGeomCollectorPtr Create (OperationType opType);

}; // MeasureGeomCollector

END_BENTLEY_DGN_NAMESPACE

