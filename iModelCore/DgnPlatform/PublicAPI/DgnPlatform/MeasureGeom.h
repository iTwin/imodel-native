/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MeasureGeom.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/SimplifyGraphic.h>

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
protected:

OperationType           m_opType;
GeometricPrimitivePtr   m_geomPrimitive;
Transform               m_geomTransform;

IFacetOptionsPtr        m_facetOptions;
Transform               m_invCurrTransform;
Transform               m_preFlattenTransform;
bool                    m_inFlatten;

double                  m_amountSum; // Accumated area or volume for centroid/moment calculation...
double                  m_volumeSum;
double                  m_areaSum;
double                  m_perimeterSum;
double                  m_lengthSum;
double                  m_iXY;
double                  m_iXZ;
double                  m_iYZ;
DPoint3d                m_moment1;
DPoint3d                m_moment2;
double                  m_closureError;
int                     m_calculationMode; // 0=best available, 1=solid kernel preferred, 2=facets preferred.

mutable DPoint3d        m_spinMoments; // Holds computed result...
mutable DPoint3d        m_centroidSum; // Holds computed result...

DrawPurpose _GetProcessPurpose() const override {return DrawPurpose::Measure;}
IFacetOptionsP _GetFacetOptionsP() override;

UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Auto;}
UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Auto;}
UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Auto;}
UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override {return UnhandledPreference::Box;}

bool _ProcessCurveVector (CurveVectorCR, bool isFilled, SimplifyGraphic&) override;
bool _ProcessSolidPrimitive (ISolidPrimitiveCR, SimplifyGraphic&) override;
bool _ProcessSurface (MSBsplineSurfaceCR, SimplifyGraphic&) override;
bool _ProcessPolyface (PolyfaceQueryCR, bool isFilled, SimplifyGraphic&) override;
bool _ProcessBody (IBRepEntityCR, SimplifyGraphic&) override;

void _OutputGraphics (ViewContextR context) override;

void AccumulateVolumeSums (double volumeB, double areaB, double closureErrorB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateAreaSums (double areaB, double perimeterB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (double lengthB, DPoint3dCR centroidB, DPoint3dCR momentB2, double iXYB, double iXZB, double iYZB);
void AccumulateLengthSums (DMatrix4dCR products);

bool DoAccumulateLengths (CurveVectorCR, SimplifyGraphic& graphic);
bool DoAccumulateLengths (IBRepEntityCR, SimplifyGraphic& graphic);

bool DoAccumulateAreas (CurveVectorCR, SimplifyGraphic& graphic);
bool DoAccumulateAreas (ISolidPrimitiveCR, SimplifyGraphic& graphic);
bool DoAccumulateAreas (MSBsplineSurfaceCR, SimplifyGraphic& graphic);
bool DoAccumulateAreas (PolyfaceQueryCR, SimplifyGraphic& graphic);
bool DoAccumulateAreas (IBRepEntityCR, SimplifyGraphic& graphic);

bool DoAccumulateVolumes (ISolidPrimitiveCR, SimplifyGraphic& graphic);
bool DoAccumulateVolumes (PolyfaceQueryCR, SimplifyGraphic& graphic);
bool DoAccumulateVolumes (IBRepEntityCR, SimplifyGraphic& graphic);

void GetOutputTransform (TransformR transform, SimplifyGraphic const& graphic);
bool GetPreFlattenTransform (TransformR flattenTransform, SimplifyGraphic const& graphic);

BentleyStatus GetOperationStatus ();

MeasureGeomCollector (OperationType);

//__PUBLISH_SECTION_START__
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
DGNPLATFORM_EXPORT void SetResultOptions (IFacetOptionsP options, TransformCP invCurrTrans = nullptr);

//! Supply a flattening transform to be applied prior to doing length/area measurement (makes no sense for volume, caller is expected to validate input element/geometry).
DGNPLATFORM_EXPORT void SetPreFlattenTransform (TransformCR preFlattenTrans);

//! Select method of calculation
//! Values are:
//! 0 ==> no preference -- internal when available, solid kernel if needed.
//! 1 ==> prefer solid kernel
//! 2 ==> prefer faceted
DGNPLATFORM_EXPORT void SetCalculationMethod (int selector);

//! Query the calculation mode.
DGNPLATFORM_EXPORT int GetCalculationMethod () const;

//__PUBLISH_SECTION_START__
//! Visit the supplied geometric primitive and accumulate the measure information.
DGNPLATFORM_EXPORT BentleyStatus Process (GeometricPrimitiveCR, DgnDbR, TransformCP transform = nullptr);

//! Visit the supplied element and accumulate the measure information.
DGNPLATFORM_EXPORT BentleyStatus Process (GeometrySourceCR);

//! Create new instance of a measure geometry collector.
DGNPLATFORM_EXPORT static MeasureGeomCollectorPtr Create (OperationType opType);

}; // MeasureGeomCollector

END_BENTLEY_DGN_NAMESPACE

