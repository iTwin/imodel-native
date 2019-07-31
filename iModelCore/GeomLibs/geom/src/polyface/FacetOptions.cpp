/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/FacetOptions.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>



BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static DPoint3d s_defaultSilhouetteOrigin = DPoint3d::FromXYZ (-1,0,0);
static DVec3d   s_defaultSilhouetteViewVector = DVec3d::From (0,0,0);
static double   s_defaultAngleTolerance = 0.523598775598298873; // 30 degrees
static double   s_defaultAngleToleranceForCurves = 0.174532925199432957; // 10 degrees


static FacetParamMode s_defaultParamMode = FACET_PARAM_01BothAxes;

// Public-to-private dispatch methods for FacetOptions


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetChordTolerance (double chordTolerance) {_SetChordTolerance (chordTolerance);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double IFacetOptions::GetChordTolerance () const {return _GetChordTolerance () * _GetToleranceDistanceScale();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     06/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetMaxFacetWidth (double maxFacetWidth) {_SetMaxFacetWidth (maxFacetWidth);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    MattGooding     06/2012
+--------------------------------------------------------------------------------------*/
double IFacetOptions::GetMaxFacetWidth () const {return _GetMaxFacetWidth () * _GetToleranceDistanceScale (); }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetMaxEdgeLength (double maxEdgeLength) {_SetMaxEdgeLength (maxEdgeLength);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double IFacetOptions::GetMaxEdgeLength () const {return _GetMaxEdgeLength () * _GetToleranceDistanceScale();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetAngleTolerance (double normalAngleTolerance) {_SetAngleTolerance (normalAngleTolerance);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double IFacetOptions::GetAngleTolerance () const {return _GetAngleTolerance ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetMaxPerBezier (int maxPerBezier) {_SetMaxPerBezier (maxPerBezier);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
int IFacetOptions::GetMaxPerBezier () const {return _GetMaxPerBezier ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetMinPerBezier (int minPerBezier) {_SetMinPerBezier (minPerBezier);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
int IFacetOptions::GetMinPerBezier () const {return _GetMinPerBezier ();}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetMaxPerFace (int maxPerFace) {_SetMaxPerFace (maxPerFace);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int IFacetOptions::GetMaxPerFace () const {return _GetMaxPerFace ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetEdgeHiding (bool edgeHiding) {_SetEdgeHiding (edgeHiding);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetEdgeHiding () const {return _GetEdgeHiding ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2017
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetBsplineSurfaceEdgeHiding (int edgeHiding) {_SetBsplineSurfaceEdgeHiding (edgeHiding);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      02/2017
+--------------------------------------------------------------------------------------*/
int IFacetOptions::GetBsplineSurfaceEdgeHiding () const {return _GetBsplineSurfaceEdgeHiding ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetNormalsRequired (bool normalsRequired) {_SetNormalsRequired (normalsRequired);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetNormalsRequired () const {return _GetNormalsRequired ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetParamsRequired (bool paramsRequired) {_SetParamsRequired (paramsRequired);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetParamsRequired () const {return _GetParamsRequired ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetEdgeChainsRequired (bool edgeChainsRequired) {_SetEdgeChainsRequired (edgeChainsRequired);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetEdgeChainsRequired () const {return _GetEdgeChainsRequired ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetCurveParameterMapping (CurveParameterMapping curveParameterMapping) {_SetCurveParameterMapping (curveParameterMapping);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveParameterMapping IFacetOptions::GetCurveParameterMapping () const {return _GetCurveParameterMapping ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetVertexColorsRequired (bool vertexColorsRequired) {_SetVertexColorsRequired (vertexColorsRequired);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetVertexColorsRequired () const {return _GetVertexColorsRequired ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetIgnoreFaceMaterialAttachments (bool ignoreFaceAttachments) {_SetIgnoreFaceMaterialAttachments (ignoreFaceAttachments);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetIgnoreFaceMaterialAttachments () const {return _GetIgnoreFaceMaterialAttachments ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetSilhouetteDirection (DVec3d silhouetteDirection) {_SetSilhouetteDirection (silhouetteDirection);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d IFacetOptions::GetSilhouetteDirection () const {return _GetSilhouetteDirection ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetSilhouetteOrigin (DPoint3d silhouetteOrigin) {_SetSilhouetteOrigin (silhouetteOrigin);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d IFacetOptions::GetSilhouetteOrigin () const {return _GetSilhouetteOrigin ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray,Bentley     01/2018
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::GetIgnoreHiddenBRepEntities () const { return _GetIgnoreHiddenBRepEntities(); }
void IFacetOptions::SetIgnoreHiddenBRepEntities (bool ignoreHiddenBRepEntities) { _SetIgnoreHiddenBRepEntities(ignoreHiddenBRepEntities); }
bool IFacetOptions::GetOmitBRepEdgeChainIds () const { return _GetOmitBRepEdgeChainIds(); }
void IFacetOptions::SetOmitBRepEdgeChainIds (bool omit) { _SetOmitBRepEdgeChainIds(omit); }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IFacetOptionsPtr IFacetOptions::Clone () const {return _Clone(); }


#define ImplementGetSetPair(_propertyType_,_typeName_,_propertyName_) \
_propertyType_ _typeName_::Get##_propertyName_ () const {return _Get##_propertyName_();}\
void _typeName_::Set##_propertyName_ (_propertyType_ value) {return _Set##_propertyName_ (value);}

ImplementGetSetPair(bool,IFacetOptions, CombineFacets)

ImplementGetSetPair(double,IFacetOptions, SilhouetteToleranceDivisor)

ImplementGetSetPair(int,IFacetOptions, SilhouetteType)
ImplementGetSetPair(int,IFacetOptions, CurvedSurfaceMaxPerFace)
ImplementGetSetPair(bool,IFacetOptions, ConvexFacetsRequired)
ImplementGetSetPair(bool,IFacetOptions, SmoothTriangleFlowRequired)
ImplementGetSetPair(bool,IFacetOptions, BSurfSmoothTriangleFlowRequired)
ImplementGetSetPair(bool,IFacetOptions, DoSpatialLaplaceSmoothing)
ImplementGetSetPair(bool,IFacetOptions, HideSmoothEdgesWhenGeneratingNormals)

ImplementGetSetPair(FacetParamMode,IFacetOptions, ParamMode)


ImplementGetSetPair(double,IFacetOptions, CurvatureWeightFactor)
ImplementGetSetPair(double,IFacetOptions, ToleranceDistanceScale)
ImplementGetSetPair(double,IFacetOptions, ParamDistanceScale)

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetDefaults ()
    {
    return _SetDefaults ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IFacetOptions::SetCurveDefaults ()
    {
    return _SetCurveDefaults ();
    }

//! make count larger than (numerator/denominator) but restrict to maxCount
void UpdateToleranceCount (size_t &count, double numerator, double denominator, size_t maxCount)
    {
    numerator = fabs (numerator);
    if (denominator > 0.0 && numerator > count * denominator)
        count = (int)((numerator / denominator) + 0.99999);
    if (count > maxCount)
        count = maxCount;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::FullEllipseStrokeCount (DEllipse3dCR ellipse) const
    {
    DEllipse3d fullEllipse = ellipse;
    static int   s_maxCount = 600;        // In SS3, MeshFuncs::s_maxSteps
    static int   s_minCount = 4;

    fullEllipse.sweep = msGeomConst_2pi;
    size_t numChord = (size_t)ellipse.GetStrokeCount (s_minCount, s_maxCount, GetChordTolerance (), GetAngleTolerance ());
    double a;
    if (0.0 < (a = GetMaxEdgeLength ()))
        UpdateToleranceCount (numChord, fullEllipse.ArcLength (), a, s_maxCount);
    if (numChord <= 4)
        numChord = 4;
    else if (numChord < 6)
        numChord = 6;
    else
        numChord = 4 * ((numChord + 3) / 4);
    return numChord;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::SegmentStrokeCount (DSegment3dCR segment) const
    {
    size_t maxCount = 100;
    size_t numChord = 1;
    double a;
    if (0.0 < (a = GetMaxEdgeLength ()))
        UpdateToleranceCount (numChord, segment.Length (), a, maxCount);
    return numChord;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::DistanceStrokeCount (double distance) const
    {
    size_t maxCount = 100;
    size_t numChord = 1;
    double a;
    if (0.0 < (a = GetMaxEdgeLength ()))
        UpdateToleranceCount (numChord, distance, a, maxCount);
    return numChord;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::DistanceAndTurnStrokeCount (double distance, double turnRadians, double maxCurvature,
double chordTolerance,
double angleTolerance,
double maxEdgeLength
)
    {
    size_t maxCount = 400;
    size_t numChord = 1;
    double radians1 = fabs (distance * maxCurvature);
    if (distance > 0.0)
        turnRadians = DoubleOps::MaxAbs (turnRadians, radians1);
    else
        DoubleOps::SafeDivide (distance, turnRadians, maxCurvature, distance);

    if (0.0 < maxEdgeLength)
        UpdateToleranceCount (numChord, distance, maxEdgeLength, maxCount);
    if (0.0 < angleTolerance)
        UpdateToleranceCount (numChord, turnRadians, angleTolerance, maxCount);
    if (0.0 < chordTolerance)
        {
        double q;
        DoubleOps::SafeDivide (q, fabs (distance - chordTolerance * turnRadians), distance, 0.0);
        // q is a cosine of an angle....
        if (q > 0.2 && q < 1.0)
            UpdateToleranceCount (numChord, turnRadians, 2.0 * acos (q), maxCount);
        }
    return numChord;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::DistanceAndTurnStrokeCount (double distance, double turnRadians, double maxCurvature) const
    {
    return DistanceAndTurnStrokeCount (distance, turnRadians, maxCurvature,
            GetChordTolerance (), GetAngleTolerance (), GetMaxEdgeLength ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::EllipseStrokeCount (DEllipse3dCR ellipse) const
    {
    static double s_fullCircleFraction = 0.999;
    static double s_halfCircleFractionTol = 0.001;
    size_t numFullEllipseChord = FullEllipseStrokeCount (ellipse);
    double fraction = fabs (ellipse.sweep / msGeomConst_2pi);
    size_t chordCount = numFullEllipseChord;
    if (fraction > s_fullCircleFraction)
        {
        chordCount = numFullEllipseChord;
        }
    else if (fabs (fraction - 0.5) <= s_halfCircleFractionTol)
        {
        chordCount = (numFullEllipseChord + 1) / 2;
        }
    else
        chordCount = (int) ceil (fraction * numFullEllipseChord);
    
    return chordCount;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::BezierStrokeCount (DPoint4dCP poles, size_t order, size_t &count) const
    {
    if (order <= 2)
        count = 2;
    else
        count = bsiBezierDPoint4d_estimateEdgeCount (poles, (int)order, GetChordTolerance (), GetAngleTolerance (), GetMaxEdgeLength (), false);

    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IFacetOptions::BezierStrokeCount (bvector<DPoint4d> const &poles, size_t index0, int order, size_t &count) const
    {
    return BezierStrokeCount (&poles[index0], order, count);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::BsplineCurveStrokeCount (MSBsplineCurveCR curve) const
    {
    return curve.GetStrokeCount (GetChordTolerance (), GetAngleTolerance (), GetMaxEdgeLength ());
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t IFacetOptions::LineStringStrokeCount (bvector<DPoint3d> const &points) const
    {
    return PolylineOps::GetStrokeCount (points, *this);
    }

//! Facet control values.


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct FacetOptions : public IFacetOptions
{

    double m_chordTolerance;

    double m_maxFacetWidth;

    double m_maxEdgeLength;

    double m_normalAngleTolerance;

    int m_maxPerBezier;
    
    int m_minPerBezier;
    
    int m_maxPerFace;

    bool m_edgeHiding;

    bool m_normalsRequired;

    bool m_paramsRequired;

    bool m_edgeChainsRequired;

    bool m_vertexColorsRequired;

    bool m_faceSizeRequired;

    bool m_faceIndexRequired;

    DVec3d m_silhouetteDirection;

    DPoint3d m_silhouetteOrigin;

    int m_silhouetteType;

    double m_silhouetteToleranceDivisor;

    int m_curveSurfaceMaxPerFace;

    bool m_combineFacets;

    bool m_convexFacetsRequired;

    int m_bsplineSurfaceEdgeHiding;

    FacetParamMode m_paramMode;

    CurveParameterMapping m_curveParameterMapping;

    double m_paramDistanceScale;

    double m_toleranceDistanceScale;
    
    bool m_smoothTriangleFlowRequired;
    bool m_bSurfSmoothTriangleFlowRequired;
    bool m_doSpatialLaplaceSmoothing;

    bool m_ignoreFaceAttachments;

    double m_curvatureWeightFactor;

    bool m_hideSmoothEdgesWhenGeneratingNormals;

    bool m_ignoreHiddenBRepEntities = false;
    bool m_omitBRepEdgeChainIds = false;

    // Get/Set pair for ChordTolerance.  Simple access to m_chordTolerance
   double _GetChordTolerance () const override
            {return m_chordTolerance; }
   void _SetChordTolerance (double chordTolerance) override
            {m_chordTolerance = chordTolerance;}

    // Get/Set pair for MaxFacetWidth.  Simple access to m_maxFacetWidth
   double _GetMaxFacetWidth () const override
            {return m_maxFacetWidth; }
   void _SetMaxFacetWidth (double maxFacetWidth) override
            {m_maxFacetWidth = maxFacetWidth;}

    // Get/Set pair for MaxEdgeLength.  Simple access to m_maxEdgeLength
   double _GetMaxEdgeLength () const override
            {return m_maxEdgeLength;}
   void _SetMaxEdgeLength (double maxEdgeLength) override
            {m_maxEdgeLength = maxEdgeLength;}

    // Get/Set pair for AngleTolerance.  Simple access to m_normalAngleTolerance
   double _GetAngleTolerance () const override
            {return m_normalAngleTolerance;}
   void _SetAngleTolerance (double normalAngleTolerance) override
            {m_normalAngleTolerance = normalAngleTolerance;}

    // Get/Set pair for MaxPerBezier.  Simple access to m_maxPerBezier
   int _GetMaxPerBezier () const override
            {return m_maxPerBezier;}
   void _SetMaxPerBezier (int maxPerBezier) override
            {m_maxPerBezier = maxPerBezier;}

    // Get/Set pair for MinPerBezier.  Simple access to m_minPerBezier
   int _GetMinPerBezier () const override
            {return m_minPerBezier;}
   void _SetMinPerBezier (int minPerBezier) override
            {m_minPerBezier = minPerBezier;}

    // Get/Set pair for MaxPerFace.  Simple access to m_maxPerFace
   int _GetMaxPerFace () const override
            {return m_maxPerFace;}
   void _SetMaxPerFace (int maxPerFace) override
            {m_maxPerFace = maxPerFace;}

    // Get/Set pair for EdgeHiding.  Simple access to m_edgeHiding
   bool _GetEdgeHiding () const override
            {return m_edgeHiding;}
   void _SetEdgeHiding (bool edgeHiding) override
            {m_edgeHiding = edgeHiding;}

    // Get/Set pair for BsplineSurfaceEdgeHiding.  Simple access to m_edgeHiding
   int _GetBsplineSurfaceEdgeHiding () const override
            {return m_bsplineSurfaceEdgeHiding;}
   void _SetBsplineSurfaceEdgeHiding (int bsplineSurfaceEdgeHiding) override
            {m_bsplineSurfaceEdgeHiding = bsplineSurfaceEdgeHiding;}

    // Get/Set pair for NormalsRequired.  Simple access to m_normalsRequired
   bool _GetNormalsRequired () const override
            {return m_normalsRequired;}
   void _SetNormalsRequired (bool normalsRequired) override
            {m_normalsRequired = normalsRequired;}

    // Get/Set pair for ParamsRequired.  Simple access to m_paramsRequired
   bool _GetParamsRequired () const override
            {return m_paramsRequired;}
   void _SetParamsRequired (bool paramsRequired) override
            {m_paramsRequired = paramsRequired;}

    // Get/Set pair for EdgeChainsRequired.  Simple access to m_edgeChainsRequired
   bool _GetEdgeChainsRequired () const override
            {return m_edgeChainsRequired;}
   void _SetEdgeChainsRequired (bool edgeChainsRequired) override
            {m_edgeChainsRequired = edgeChainsRequired;}

    // Get/Set pair for CurveParameterMapping.  Simple access to m_curveParameterMapping
   CurveParameterMapping _GetCurveParameterMapping () const override
            {return m_curveParameterMapping;}
   void _SetCurveParameterMapping (CurveParameterMapping curveParameterMapping) override
            {m_curveParameterMapping = curveParameterMapping;}


    // Get/Set pair for VertexColorsRequired.  Simple access to m_vertexColorsRequired
   bool _GetVertexColorsRequired () const override
            {return m_vertexColorsRequired;}
   void _SetVertexColorsRequired (bool vertexColorsRequired) override
            {m_vertexColorsRequired = vertexColorsRequired;}

    // Get/Set pair for SilhouetteDirection.  Simple access to m_silhouetteDirection
   DVec3d _GetSilhouetteDirection () const override
            {return m_silhouetteDirection;}
   void _SetSilhouetteDirection (DVec3d silhouetteDirection) override
            {m_silhouetteDirection = silhouetteDirection;}

    // Get/Set pair for SilhouetteOrigin.  Simple access to m_silhouetteOrigin
   DPoint3d _GetSilhouetteOrigin () const override
            {return m_silhouetteOrigin;}
   void _SetSilhouetteOrigin (DPoint3d silhouetteOrigin) override
            {m_silhouetteOrigin = silhouetteOrigin;}
                                                                                         
    // Get/Set pair for SilhouetteType.  Simple access to m_silhouetteType
   int _GetSilhouetteType () const override
            {return m_silhouetteType;}
   void _SetSilhouetteType (int silhouetteType) override
            {m_silhouetteType = silhouetteType;}

    // Get/Set pair for SilhouetteToleranceDivisor.  Simple access to m_silhouetteToleranceDivisor
   double _GetSilhouetteToleranceDivisor () const override
            {return m_silhouetteToleranceDivisor;}
   void _SetSilhouetteToleranceDivisor (double silhouetteToleranceDivisor) override
            {m_silhouetteToleranceDivisor = silhouetteToleranceDivisor;}

    // Get/Set pair for CurvedSurfaceMaxPerFace.  Simple access to m_curveSurfaceMaxPerFace
   int _GetCurvedSurfaceMaxPerFace () const override
            {return m_curveSurfaceMaxPerFace;}
   void _SetCurvedSurfaceMaxPerFace (int curveSurfaceMaxPerFace) override
            {m_curveSurfaceMaxPerFace = curveSurfaceMaxPerFace;}

    // Get/Set pair for CombineFacets.  Simple access to m_combineFacets
   bool _GetCombineFacets () const override
            {return m_combineFacets;}
   void _SetCombineFacets (bool combineFacets) override
            {m_combineFacets = combineFacets;}

    // Get/Set pair for SmoothTriangleFlow.  Simple access to m_SmoothTriangleFlow
   bool _GetSmoothTriangleFlowRequired () const override
            {return m_smoothTriangleFlowRequired;}
   void _SetSmoothTriangleFlowRequired (bool value) override
            {m_smoothTriangleFlowRequired = value;}

    // Get/Set pair for BSurfSmoothFlow.  Simple access to m_BSurfSmoothFlow
   bool _GetBSurfSmoothTriangleFlowRequired () const override
            {return m_bSurfSmoothTriangleFlowRequired;}
   void _SetBSurfSmoothTriangleFlowRequired (bool value) override
            {m_bSurfSmoothTriangleFlowRequired = value;}


    // Get/Set pair for BSurfSmoothFlow.  Simple access to m_BSurfSmoothFlow
   bool _GetDoSpatialLaplaceSmoothing () const override
            {return m_doSpatialLaplaceSmoothing;}
   void _SetDoSpatialLaplaceSmoothing (bool value) override
            {m_doSpatialLaplaceSmoothing = value;}

    // Get/Set pair for SmoothTriangleFlow.  Simple access to m_SmoothTriangleFlow
   double _GetCurvatureWeightFactor () const override
            {return m_curvatureWeightFactor;}
   void _SetCurvatureWeightFactor (double value) override
            {m_curvatureWeightFactor = value;}

    // Get/Set pair for ConvexFacetsRequired.  Simple access to m_convexFacetsRequired
   bool _GetConvexFacetsRequired () const override
            {return m_convexFacetsRequired;}
   void _SetConvexFacetsRequired (bool convexFacetsRequired) override
            {m_convexFacetsRequired = convexFacetsRequired;}

    // Get/Set pair for ParamMode.  Simple access to m_paramMode
   FacetParamMode _GetParamMode () const override
            {return m_paramMode;}
   void _SetParamMode (FacetParamMode paramMode) override
            {m_paramMode = paramMode;}

    // Get/Set pair for ParamDistanceScale.  Simple access to m_paramDistanceScale
   double _GetParamDistanceScale () const override
            {return m_paramDistanceScale;}
   void _SetParamDistanceScale (double paramDistanceScale) override
            {m_paramDistanceScale = paramDistanceScale;}

    // Get/Set pair for ToleranceDistanceScale.  Simple access to m_toleranceDistanceScale
   double _GetToleranceDistanceScale () const override
            {return m_toleranceDistanceScale;}
   void _SetToleranceDistanceScale (double toleranceDistanceScale) override
            {m_toleranceDistanceScale = toleranceDistanceScale;}

    // Get/Set pair for IgnoreFaceMaterialAttachments.  Simple access to m_ignoreFaceAttachments
    bool _GetIgnoreFaceMaterialAttachments () const override {return m_ignoreFaceAttachments;}
    void _SetIgnoreFaceMaterialAttachments (bool ignoreFaceAttachments) override {m_ignoreFaceAttachments = ignoreFaceAttachments;}

    // Get/set pair for HideSmoothEdgesWhenGeneratingNormals Simple acces to m_hideSmoothEdgesWhenGeneratingNormals
    bool _GetHideSmoothEdgesWhenGeneratingNormals () const override { return m_hideSmoothEdgesWhenGeneratingNormals; }
    void _SetHideSmoothEdgesWhenGeneratingNormals (bool hideSmoothEdgesWhenGeneratingNormals)  override { m_hideSmoothEdgesWhenGeneratingNormals = hideSmoothEdgesWhenGeneratingNormals; }

    bool _GetIgnoreHiddenBRepEntities () const override { return m_ignoreHiddenBRepEntities; }
    void _SetIgnoreHiddenBRepEntities (bool ignoreHiddenBRepEntities) override { m_ignoreHiddenBRepEntities = ignoreHiddenBRepEntities; }

    bool _GetOmitBRepEdgeChainIds () const override { return m_omitBRepEdgeChainIds; }
    void _SetOmitBRepEdgeChainIds (bool omitBRepEdgeChainIds) override { m_omitBRepEdgeChainIds = omitBRepEdgeChainIds; }


    IFacetOptionsPtr _Clone () const override { return new FacetOptions (*this); }

void _SetDefaults () override
    {
    m_chordTolerance = 0;
    m_maxFacetWidth = 0.0;
    m_maxEdgeLength = 0;
    m_normalAngleTolerance = s_defaultAngleTolerance;
    m_maxPerBezier = 0;
    m_minPerBezier = 0;
    m_maxPerFace = 3;
    m_edgeHiding = true;
    m_normalsRequired = false;
    m_paramsRequired = false;
    m_edgeChainsRequired = false;
    m_vertexColorsRequired = false;
    m_faceSizeRequired = false;
    m_faceIndexRequired = true;
    m_silhouetteDirection = s_defaultSilhouetteViewVector;
    m_silhouetteOrigin = s_defaultSilhouetteOrigin;
    m_silhouetteType = 0;
    m_silhouetteToleranceDivisor = 0;
    m_curveSurfaceMaxPerFace = 4;
    m_combineFacets = false;
    m_convexFacetsRequired = false;
    m_paramMode = s_defaultParamMode;
    m_curveParameterMapping = CURVE_PARAMETER_MAPPING_CurveKnot;
    m_paramDistanceScale = 1;
    m_toleranceDistanceScale = 1;
    m_smoothTriangleFlowRequired = false;
    m_bSurfSmoothTriangleFlowRequired = false;
    m_doSpatialLaplaceSmoothing = false;
    m_curvatureWeightFactor = 0.0;
    m_ignoreFaceAttachments = false;
    m_hideSmoothEdgesWhenGeneratingNormals = true;
    m_bsplineSurfaceEdgeHiding = 1;
    m_ignoreHiddenBRepEntities = false;
    m_omitBRepEdgeChainIds = false;
    }

void _SetCurveDefaults () override
    {
    SetDefaults ();
    m_normalAngleTolerance = s_defaultAngleToleranceForCurves;
    }

FacetOptions ()
    {
    SetDefaults ();
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IFacetOptionsPtr IFacetOptions::New ()
    {
    return Create ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IFacetOptionsPtr IFacetOptions::Create ()
    {
    IFacetOptionsPtr options =  new FacetOptions ();
    options->SetDefaults ();
    return options;
    }

IFacetOptionsPtr IFacetOptions::CreateForSurfaces
(
double chordTol,
double angleRadians,
double maxEdgeLength,
bool triangulate,
bool normals,
bool params
)
    {
    IFacetOptionsPtr options =  new FacetOptions ();
    options->SetDefaults ();
    options->SetChordTolerance (chordTol);
    options->SetAngleTolerance (angleRadians);
    options->SetMaxEdgeLength (maxEdgeLength);
    if (triangulate)
        options->SetCurvedSurfaceMaxPerFace(3); // <- NOTE: SetMaxPerFace is already 3 by default…
    options->SetNormalsRequired (normals);
    options->SetParamsRequired (params);
    return options;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IFacetOptionsPtr IFacetOptions::CreateForCurves ()
    {
    IFacetOptionsPtr options =  new FacetOptions ();
    options->SetCurveDefaults ();
    return options;
    }

END_BENTLEY_GEOMETRY_NAMESPACE