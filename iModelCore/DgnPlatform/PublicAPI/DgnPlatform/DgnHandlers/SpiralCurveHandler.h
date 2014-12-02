/*----------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/SpiralCurveHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include "IManipulator.h"
#include "IGeoCoordReproject.h"
#ifdef WIP_EC_ENABLER
#include "DelegatedElementECEnabler.h"
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup CurveElements
/// @beginGroup

// 2008 Spiral types for Civil transitions
#define SPIRALTYPE_TransitionClothoid    10
#define SPIRALTYPE_TransitionBloss       11
#define SPIRALTYPE_TransitionBiQuadratic 12
#define SPIRALTYPE_TransitionCosine      13
#define SPIRALTYPE_TransitionSine        14

/*=================================================================================**//**
* @bsiclass                        Earlin.Lutz
A Transition Spiral is a specialized curve for Civil Engineering road and rail blend curves.
The primary data for transition curves is bearing and radius at nominal start and end of
the interval of interest.   The curve formulas can be evaluated beyond those endpoints.
A particular stored curve indicates a fractional range that can reach to those extrapolated regions
and can go in the reverse of the nominal portion.

Curves are always evaluated in a local coordinate system.  The nominal start is at the origin,
and stroking logic generates vectors from the nominal start to other points on the curve.
(After evaluating the fractional start and end, the displayed curve does NOT have to start (or even pass through)
the m_origin point.

The fields are:
<list>
<item>startAngle = bearing as the (unbounded) spiral passes through the local system origin.</item>
<item startRadius = radius of curvature at start.  0 indicates straight line (infinite radius)</item>
<item>endAngle = bearing as the (unboudned) spiral passes throught the nominal endpoint</item>
<item>endRadius = radius of curvature at end.  0 indicates straight line (infinite radius)</item>
<item>spiralType = integer type code for the spiral.</item>
<item>spiralCreateMode = integer history indicator.  Not used by any non-GUI processing.</item>
<item>origin = origin of local coordinate system.  The "start" bearing and curvature apply as the unbounded spiral passes through this point</item>
<item>rMatrix = axes of local coordinate system.</item>
<item>fractionA  = fractional coordiante (nominal start and end are 0 and 1) for start of "active" part of spiral.</item>
<item>fractionB  = fractional coordiante (nominal start and end are 0 and 1) for end of "active" part of spiral.</item>
<item>reserved = additional doubles that may be used in the fucture for special spiral types.</item>
<list>

<remark>No extra data (m_reserved) is needed for theses spiral types: clothoid, biquadratic, bloss, sine, cosine.</remark>

+===============+===============+===============+===============+===============+======*/
struct TransitionSpiralData
{
public:

static const int EXTRA_DATA_COUNT = 10;

private:

double      m_stAngle;                      // PRIMARY
double      m_endAngle;                     // PRIMARY
double      m_stRadius;                     // PRIMARY
double      m_endRadius;                    // PRIMARY
int         m_spiralType;                   // PRIMARY
int         m_spiralCreateMode;             // CONSTRUCTION
double      m_fractionA;                    // PRIMARY
double      m_fractionB;                    // PRIMARY

DPoint3d    m_origin;                       // PLACEMENT
RotMatrix   m_rMatrix;                      // PLACEMENT

double      m_reserved[EXTRA_DATA_COUNT];   // CONSTRUCTION

public:

DGNPLATFORM_EXPORT TransitionSpiralData ();

DGNPLATFORM_EXPORT TransitionSpiralData (DSpiral2dBaseCR, TransformCR frame, double fractionA, double fractionB);

DGNPLATFORM_EXPORT void GetMetrics
(
double &bearingRadians0,
double &radius0,
double &bearingRadians1,
double &radius1,
double &fractionA,
double &fractionB
);

DGNPLATFORM_EXPORT void SetMetrics
(
double bearingRadians0,
double radius0,
double bearingRadians1,
double radius1,
double fractionA,
double fractionB
);

// returns start and end spiralFractions for active interval.
DGNPLATFORM_EXPORT void GetInterval (double &spiralFractionA, double &spiralFractionB) const;
DGNPLATFORM_EXPORT void SetInterval (double spiralFractionA, double spiralFractionB);

// Individual angle get/set are in DEGREES
DGNPLATFORM_EXPORT double SetAngle (double newValDegrees, bool bStart);
DGNPLATFORM_EXPORT double GetAngle (bool bStart) const;

DGNPLATFORM_EXPORT double SetRadius (double newVal, bool bStart);
DGNPLATFORM_EXPORT double GetRadius (bool bStart) const;

DGNPLATFORM_EXPORT void SetSpiralType (int type);
DGNPLATFORM_EXPORT int GetSpiralType () const;

DGNPLATFORM_EXPORT void SetCreateMode (int mode);
DGNPLATFORM_EXPORT int GetCreateMode () const;

DGNPLATFORM_EXPORT DPoint3d SetOrigin (DPoint3dCR newPt);
DGNPLATFORM_EXPORT DPoint3d GetOrigin () const;
DGNPLATFORM_EXPORT RotMatrix SetMatrix (RotMatrixCR newMatrix);
DGNPLATFORM_EXPORT RotMatrix GetMatrix () const;
DGNPLATFORM_EXPORT Transform GetTransform ();
DGNPLATFORM_EXPORT void SetExtraData (double *pData, int count);
// Copy up to maxCopy extra data.  actualCount is always the stored data count -- may be great than maxCopy.
DGNPLATFORM_EXPORT void GetExtraData (double *pData, int &actualCount, int maxCopy);

// Map the element parameter (fraction of reference interval) back to the nominal spiral range.
DGNPLATFORM_EXPORT double ElementFractionToSpiralFraction (double fraction) const;
DGNPLATFORM_EXPORT double SpiralFractionToElementFraction (double fraction) const;
DGNPLATFORM_EXPORT double SpiralToElementScale () const;
DGNPLATFORM_EXPORT double ElementToSpiralScale () const;

// Apply local origin and orientation ...
DGNPLATFORM_EXPORT void LocalToWorld (DPoint3dP pXYZ, DVec2dCP pUV, int n) const;
DGNPLATFORM_EXPORT void LocalToWorldAsVector (DVec3dP pXYZ, DVec2dCP pUV, int n) const;

// Allocation and deletion for runtime BentleyGeom virtual objects from a transition spiral...
DGNPLATFORM_EXPORT DSpiral2dBaseP GetDSpiral2d () const;
DGNPLATFORM_EXPORT void DropSpiral (DSpiral2dBaseP spiral) const;

// Other than checking for divide by zero, these are simple reciprocal convserions (curvature = 1/radius).
DGNPLATFORM_EXPORT static double CurvatureFromRadius (double radius);
DGNPLATFORM_EXPORT static double RadiusFromCurvature (double curvature);

// Transition spirals (of several types) have symmetric curvature variations which allows (independent of
// spiral type) a calculation of sweep angle from start radius, end radius, and length:
DGNPLATFORM_EXPORT static double SweepRadiansFromRadiiAndLength (double radius0, double radius1, double length);

// In certain civil engineering handbooks, "degree" is used as a curvature unit.
// Theses are the conversions from "degree" to radius of curvature.
DGNPLATFORM_EXPORT static double DegreeFromRadius (double radius);
DGNPLATFORM_EXPORT static double RadiusFromDegree (double degree);

}; // TransitionSpiralData;

#define XATTRIBUTEID_SpiralHandlerElemID    1   // minor id: application specific

/*=================================================================================**//**
* @bsiclass                                                     Ping.Chen       01/2009
+===============+===============+===============+===============+===============+======*/
struct SpiralXAHandler : XAttributeHandler
{
SpiralXAHandler () {DgnSystemDomain::GetInstance().RegisterXAttributeHandler (GetXAttrId (), *this);}

static XAttributeHandlerId GetXAttrId () {return XAttributeHandlerId (XATTRIBUTEID_SpiralHandlerXAtrID, XATTRIBUTEID_SpiralHandlerElemID);}

}; // SpiralXAHandler

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* @bsiclass                        Chen.Ping                                       11/08
+===============+===============+===============+===============+===============+======*/
struct SpiralCurveHandler : BSplineCurveHandler
{
    DEFINE_T_SUPER(BSplineCurveHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (SpiralCurveHandler, DGNPLATFORM_EXPORT)

protected:

// Handler
DGNPLATFORM_EXPORT virtual StatusInt _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual void _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

public:

static ElementHandlerId GetElemHandlerId () {return ElementHandlerId (XATTRIBUTEID_SpiralHandlerXAtrID, XATTRIBUTEID_SpiralHandlerElemID);}

// WIP_EC_ENABLER
//static ElementECExtension&             CreateElementECExtension();

DGNPLATFORM_EXPORT static BentleyStatus GetData (ElementHandleCR, TransitionSpiralData&);
DGNPLATFORM_EXPORT static BSplineStatus SetData (EditElementHandleR, TransitionSpiralData const&);
DGNPLATFORM_EXPORT static BSplineStatus CreateSpiralCurveElement (EditElementHandleR eeh, ElementHandleCP templateEh, TransitionSpiralData const& data, bool is3d, DgnModelR modelRef);

}; // SpiralCurveHandler

#endif
/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

