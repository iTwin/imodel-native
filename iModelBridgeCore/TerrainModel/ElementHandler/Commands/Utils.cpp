/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/Utils.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "StdAfx.h"

#pragma unmanaged

//#include    <nullcontext.h>
//#include    <SimplifyViewDrawGeom.h>
//#include    <msdim.fdf>
//#include    <midim.fdf>
//#include    <polyline3d.fdf>
//#include    <adim.h>
//#include    <dimUtilities.h>
//#include    <dimUtilities.fdf>
//#include    <idtm.h>
//#include    <bcDTMClass.h>
//#include    <DTMElementHandlerManager.h>
//#include    <DTMDataRef.h>
//#include    <DTMElementSubHandler.h>
//#include    <dimapps.ids>

#include "dtmcommandstool.h"
#include    "commandsdefs.h"

#include    "SelectDTMElemTool.h"

#define ADP_APPEND      (-1)
#define USE_UORS        (true)
#define CHECK_REF_FOR_ACTIVATED (true)
#ifndef TEXTID_GenericDistanceProp
#define TEXTID_GenericDistanceProp  (4)
#endif

extern dtmcommandsInfo *dtmcommandsInfoP;
extern void EngageWordlib (void);
extern void DisengageWordlib (void);
extern void EngageDialogFilter (void);
extern void DisengageDialogFilter (void);

enum IntersectionType : long
    {
    INTERSECTION_TYPE_None      = 0,
    INTERSECTION_TYPE_TinPoint  = 1,
    INTERSECTION_TYPE_TinLine   = 2,
    INTERSECTION_TYPE_Triangle  = 3,
    };


DMatrix4d GetMatrixWorldToView(int view)
    {
    DMap4d worldToViewMap;
    if (!mdlView_isValidIndex(view))
        {
        BeAssert(0);
        bsiDMap4d_initIdentity (&worldToViewMap);
        }
    else
        mdlView_getHomogeneousMaps(NULL, NULL, &worldToViewMap, NULL, NULL, NULL, NULL, NULL, view);

    return worldToViewMap.M0;
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
static StatusInt PrepareDimensionElement (EditElementHandleR element, double val, int viewNumber, DgnModelRef* dtmModelRef = nullptr)
    {
    MSElement dimElem;
    StatusInt       status;
    DPoint3d        p0 = { 0., 0., 0.};
    DgnModelRef*    dstModelRef;
    // ToDo    bool         CREATE_ASSOCIATION = true;
    MSElement*      NO_SEED_ELEMENT = NULL;
    RotMatrix*      NO_ROTATION_MATRIX = NULL;
//    int             TURN_ON_WITNESS = true;

    status = mdlDim_create (&dimElem, NO_SEED_ELEMENT, NO_ROTATION_MATRIX, DimensionType::Ordinate, viewNumber);
    BeAssert (SUCCESS == status && "mdlDim_create failed");
    if (SUCCESS != status)
        return status;

    if (dtmModelRef == nullptr)
        dstModelRef = ACTIVEMODEL;
    else
        dstModelRef = dtmModelRef;

    element.SetModelRef (dstModelRef);
    element.SetElementDescr (EditElementHandle (&dimElem, dstModelRef).ExtractElementDescr(), true, false, dstModelRef);
    DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*>(&element.GetHandler());
    IDimensionEdit* dimEdit = dynamic_cast <IDimensionEdit*> (&element.GetHandler());

    dimEdit->InsertPoint (element, &p0, NULL, *DimensionStyle::GetActive(), -1);

    dimEdit->SetWitnessVisibility (element, 0, true);

    // mdlDim_insertPoint (&dimElem, &p0, nullptr, CREATE_ASSOCIATION , ADP_APPEND);
    //mdlDim_insertPoint (&dimElem, &p0, NULL, 0, POINT_STD);

    //mdlDim_setExtensionLine (&dimElem ,0, TURN_ON_WITNESS);

    // ToDo mdlDim_ordinateSetStartValueX (&dimElem, &val);
    DimOrdinateOverrides    ordinate;
    memset (&ordinate, 0, sizeof (ordinate));

    mdlDim_getOrdinateOverride (&ordinate, element);
    ordinate.modifiers |= ORDINATE_Override_StartValueX;
    ordinate.startValueX = val;
    mdlDim_setOrdinateOverride (element, &ordinate);
    RotMatrix rotMatrix;
    rotMatrix.InitIdentity();
    dimHandler->SetRotationMatrix (element, rotMatrix);

    dimHandler->SetPoint (element, &p0, nullptr, 0);
    dimHandler->SetTextOffset (element, 0, DPoint2d::From (0.0001, 0.0001));
    dimHandler->ApplyDimensionStyle (element, *DimensionStyle::GetActive(), true);
    if (NULL != dimHandler)
        dimHandler->ValidateElementRange (element, dstModelRef->Is3d());
    return SUCCESS;
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
static DVec2d CalculateAngleVector (double radius, DPoint3dCR intersectionPt, DPoint3dR midPoint, DPoint3dCP translatedPts, int numPoints)
    {
    DPoint3d circle[100];
    DPoint3d pPtsA2[100];
    int out2, out22;
    DVec2d vec;
    double ang;

    for(int i = 0; i < _countof(circle) - 1; ++i)
        {
        ang = ((1. / (double)(_countof(circle))) * i) * PI * 2;
        circle[i].x = intersectionPt.x + (radius * sin (ang));
        circle[i].y = intersectionPt.y + (radius * cos (ang));
        circle[i].z = 0;
        }
    circle[_countof(circle) - 1] = circle[0];

    midPoint = intersectionPt;
    bsiDPoint3dArray_polylineIntersectXY (pPtsA2, NULL, NULL, NULL, NULL, NULL, &out2, &out22,
        _countof(pPtsA2), translatedPts, numPoints, false, circle, _countof(circle), true);
    if (out2 == 2)
        {
        midPoint.x = pPtsA2[1].x + (pPtsA2[0].x - pPtsA2[1].x) / 2;
        midPoint.y = pPtsA2[1].y + (pPtsA2[0].y - pPtsA2[1].y) / 2;
        vec.init (pPtsA2[0].y - pPtsA2[1].y, pPtsA2[0].x - pPtsA2[1].x);
        }
    else
        vec.init (intersectionPt.y - pPtsA2[0].y, intersectionPt.x - pPtsA2[0].x);
    return vec;
    }

struct TMSimplifyViewDrawGeom : SimplifyViewDrawGeom
    {
    WString m_string;
    virtual void _DrawTextString (TextStringCR text, double* zDepth)
        {
        m_string = text.GetString ();
        }
    };

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
struct TextExtractorViewContext : public NullContext
    {
    private: TMSimplifyViewDrawGeom   m_output;

    public: void Init (ViewportP vp)
                {
                m_output.SetViewContext (this);
                SetIViewDraw (m_output);
                if (vp && vp->GetIViewOutput ())
                    _Attach (vp, DrawPurpose::CaptureGeometry);
                }

    public: void Finish (void)
                {
                if (IsAttached ()) // Fence need not have a viewport...
                    _Detach ();
                }
    public: WCharCP GetString ()
                {
                return m_output.m_string.GetWCharCP();
                }
    }; // End TextExtractorViewContext struct

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
static void ConvertToText (WStringR t, double val, ViewportP vp, DgnModelRef* dtmModelRef)
    {
    StatusInt      status;
//    MSElementDescrP edP;
    EditElementHandle  elemHandle;
    bool            /*OWNED = true,*/ /*IS_UNMODIFIED = true,*/ CHECK_RANGE = true, CHECK_SCAN_CRITERIA = true;

    status = PrepareDimensionElement (elemHandle, val, vp->GetViewNumber (), dtmModelRef);
    BeAssert (SUCCESS == status && "PrepareDimensionElement failed");
    
    TextExtractorViewContext context;
    context.Init (vp);
    context.SetToModelRef (elemHandle.GetModelRef ());
    context.VisitElemHandle (elemHandle, !CHECK_RANGE, !CHECK_SCAN_CRITERIA);
    context.Finish ();
    t = context.GetString();
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
bool ConvertToTextBlock (WStringR buf, double elevation, ViewportP vp, DgnModelRef* dtmModelRef)
    {
    ConvertToText (buf, elevation, vp, dtmModelRef);
    return !buf.empty();
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void ConvertToTextBlock (TextBlockPtr& tb, double elevation, ViewportP vp, DgnModelRef* dtmModelRef)
    {
    DgnModelRef*    dstModelRef;
    TextParamWide   tpw;
    short           lineLength;
    DPoint2d        txtScale;
    bool         IS_TEXT_NODE = true;
    DgnModelP       dstDgnCache;
    WString buf;

    memset (&tpw, 0, sizeof (tpw));
    if (!ConvertToTextBlock (buf, elevation, vp, dtmModelRef) || buf.empty () || buf.CompareTo (L"0") == 0)
        { return; }
    dstModelRef = ACTIVEMODEL;
    mdlTextStyle_getTextParamWideFromTCB (&tpw, &txtScale.x, &txtScale.y, &lineLength, !IS_TEXT_NODE);
    dstDgnCache = dstModelRef->GetDgnModelP();
    tb = TextBlock::Create (*dstDgnCache);
    tb->SetType (WORKMODE_DWG == mdlSystem_getWorkmode () ? TEXTBLOCK_TYPE_DwgMText : TEXTBLOCK_TYPE_Dgn);
    
    if (mdlModelRef_getModelFlag (dstModelRef, MODELFLAG_USE_ANNOTATION_SCALE))
        {
        TextBlockPropertiesPtr tbProps = tb->GetProperties ().Clone ();
        tbProps->SetAnnotationScale (mdlModelRef_getEffectiveAnnotationScale (dstModelRef));
        tb->SetProperties (*tbProps);
        }

    RunProperties runProperties (tpw, txtScale, *dstDgnCache);
    tb->SetRunPropertiesForAdd (runProperties);
    tb->AppendText (buf.GetWCharCP ()); // Was AddText
    }

/// <author>Piotr.Slowinski</author>                            <date>10/2011</date>
bool IsSelfSnap (DgnButtonEventCR ev, ElementHandleCR dtm, DPoint3dR point)
    {
    SnapPathCP          snapPath;

    if (DgnButtonEvent::FROM_ElemSnap != ev.GetCoordSource())
        { return false; }
    snapPath = mdlSnap_getCurrent ();
    if (NULL == snapPath)
        { return false; }
    if (dtm.GetModelRef() != snapPath->GetRoot() || dtm.GetElementRef() != snapPath->GetPathElem (0))
        { return false; }
    point = snapPath->GetSnapPoint();
    return true;
    }

/// <author>Piotr.Slowinski</author>                            <date>10/2011</date>
bool IsSelfSnap (DgnButtonEventCR ev, ElementHandleCR dtm, double &elevation)
    {
    DPoint3d p;

    if (!IsSelfSnap(ev, dtm, p))
        { return false; }
    elevation = p.z;
    return true;
    }

/*---------------------------------------------------------------------------------****
* @bsimethod                                    Piotr.Slowinski                 03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool     PrepText (TextBlockPtr& rtb, ElementHandleCR elHandle, DgnButtonEventCR ev)
    {
    DTMDataRefPtr dtmDataRef;
    DTMElementHandlerManager::GetDTMDataRef (dtmDataRef, elHandle);

    if (NULL == dtmDataRef.get())
        return false;
    CRefUnitsConverter converter (elHandle.GetModelRef(), true);

    DMatrix4d w2vMap = GetMatrixWorldToView (ev.GetViewNum());

    DPoint3d    cPt = { 0 };

    dtmDataRef->GetProjectedPointOnDTM (cPt, elHandle, w2vMap, *ev.GetPoint());

    DTMPtr dtm;
    dtmDataRef->GetDTMReferenceDirect (dtm);
    if (!dtm.IsValid())
        return false;

    int      drapeType = INTERSECTION_TYPE_None;
    double   elevation=0.,slope=0.,aspect=0. ;
    DPoint3d triangle[3] ; 

    converter.FullRootUorsToRefMeters (cPt);

    DPoint3d cStoragePt = cPt;

    dtm->GetDTMDraping()->DrapePoint (&elevation, &slope, &aspect, triangle, drapeType, cStoragePt);
    cStoragePt.z = elevation;

    cPt.z = cStoragePt.z;
    // Encode Labels
    DVec3d ptGO;
    mdlModelRef_getGlobalOrigin (elHandle.GetModelRef(), &ptGO);
    cPt -= ptGO;

    if (drapeType == INTERSECTION_TYPE_TinPoint
        ||
        drapeType == INTERSECTION_TYPE_Triangle
        ||
        IsSelfSnap(ev, elHandle, cPt.z))
        {
        ConvertToTextBlock (rtb, cPt.z, ev.GetViewport(), elHandle.GetModelRef());
        BeAssert (rtb.IsValid());
        if (rtb.IsValid())
            rtb->SetForceTextNodeFlag (true);
        }
    else
        {
        rtb = NULL;
        return false;
        }
    return true;
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
void LoadDependable (void)
    {
    static WCharCP  pDimStyle = L"DIMSTYLE";

    if (NULL == mdlSystem_findMdlDesc(pDimStyle))
        mdlSystem_loadMdlProgram (pDimStyle, NULL, NULL);
    }

/*----------------------------------------------------------------------------------***
 * @bsimethod                                                    SunandSandurkar 09/04
 +---------------+---------------+---------------+---------------+---------------+------*/
TextElementJustification ComputeDynamicJustification (DPoint3dP point1P, DPoint3dP point2P, RotMatrixP viewMatrixP)
    {
    int             horJustLeft, verJustTop, noteRotation = 1;
    TextElementJustification just = TextElementJustification::LeftTop;
    double          xDelta, yDelta;
    DPoint3d        pt1 = *point1P, pt2 = *point2P;
    DimensionStylePtr dimStyle = DimensionStyle::GetActive ();

    mdlRMatrix_rotatePoint (&pt1, viewMatrixP);
    mdlRMatrix_rotatePoint (&pt2, viewMatrixP);

    dimStyle->GetIntegerProp (noteRotation, DIMSTYLE_PROP_MLNote_TextRotation_INTEGER);

    if (1 == noteRotation)
        {
        // Find the Horizontal Justification
        xDelta = pt1.y - pt2.y;
        horJustLeft = (xDelta < 0.0);

        // Find the Vertical Justification
        yDelta = pt1.x - pt2.x;
        verJustTop = (yDelta < 0.0);

        // Put Hor and Ver together
        if (horJustLeft)
            just = (verJustTop) ? TextElementJustification::LeftTop : TextElementJustification::LeftBaseline;
        else
            just = (verJustTop) ? TextElementJustification::RightTop : TextElementJustification::RightBaseline;
        }
    else
        {
        // Find the Horizontal Justification
        xDelta = pt1.x - pt2.x;
        horJustLeft = (xDelta < 0.0);

        // Find the Vertical Justification
        yDelta = pt1.y - pt2.y;
        verJustTop = (yDelta > 0.0);

        // Put Hor and Ver together
        if (horJustLeft)
            just = (verJustTop) ? TextElementJustification::LeftTop : TextElementJustification::LeftBaseline;
        else
            just = (verJustTop) ? TextElementJustification::RightTop : TextElementJustification::RightBaseline;
        }

    return just;
    }
/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
inline double PreventMinusZero (double val)
    {
    return (val < 0. && val > -0.000001) ? -val : val;
    }

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
struct LazyStorageToUORTransformProvider
    {
    private:
        Transform       m_transform;
        bool            m_initialized;
        ElementHandleCR    m_eh;

    public:
        LazyStorageToUORTransformProvider (ElementHandleCR eh) : m_eh (eh), m_initialized (false)
            {}

        TransformCR GetTransform (void)
            {
            if (!m_initialized)
                {
                m_initialized = true;
                mdlTMatrix_getIdentity (&m_transform);
                DTMElementHandlerManager::GetStorageToUORMatrix (m_transform, m_eh);
                }
            return m_transform;
            }

    }; // End LazyStorageToUORTransformProvider struct

/// <author>Piotr.Slowinski</author>                            <date>9/2011</date>
struct IntersectionHelper
    {
    protected:

        DPoint3dP   m_activeUORs;
        int         m_numPoints;
        DTMDataRefPtr m_dtmDataRef;

    public:

        DPoint3dCP GetActiveUORs (void) const
            { return m_activeUORs; }

        int GetNumPoints (void) const
            { return m_numPoints; }

        DTMDataRefPtr GetDtmDataRef (void) const
            { return m_dtmDataRef; }

    }; // End IntersectionHelper struct

/// <author>Piotr.Slowinski</author>                            <date>8/2011</date>
struct LabelContoursCollector
    {
    private:

        bvector<ElementHandle>& m_collector;
        double              m_lineAngle;
        RotMatrix           m_rotMatrix;
        DVec2d              m_vec2;
        double              m_angle;
        EditElementHandle   m_dimElm;
        double              m_elmWidth;

    public:

        LabelContoursCollector (DPoint3dCR pt1, DPoint3dCR pt2, bvector<ElementHandle>& collector) : m_collector (collector)
            {
            DVec2d vec, vec2;

            vec.init (pt1.x - pt2.x, pt1.y - pt2.y);
            vec2.init (0, 1);
            m_lineAngle = vec.angleTo (&vec2);
            while (m_lineAngle < 0.)
                m_lineAngle += PI * 2.;
            }

        void PreTraverse (double elevation, int viewNumber)
            {
            StatusInt   result;
            DRange3d    range;
            DisplayHandler* dh;
            TransformCP NO_TRANSFORM = NULL;

            result = PrepareDimensionElement (m_dimElm, PreventMinusZero (elevation), viewNumber);
            BeAssert (SUCCESS == result && "PrepareDimensionElement failed");

            dh = m_dimElm.GetDisplayHandler ();
            BeAssert (NULL != dh && "-");
            result = dh->CalcElementRange (m_dimElm, range, NO_TRANSFORM);
            BeAssert (SUCCESS == result);
            m_elmWidth = range.high.x - range.low.x;
            }

        void OnIntersection (DPoint3dCR intersectionPt, IntersectionHelper &ih)
            {
            DPoint3d midPoint;
            DVec2d angleVec = CalculateAngleVector (m_elmWidth/2., intersectionPt, midPoint, ih.GetActiveUORs(), ih.GetNumPoints());

            m_vec2.init (0, 1);
            m_angle = angleVec.angleTo (&m_vec2);
            if (dtmcommandsInfoP->annotateContoursTxtAlignment == OPTIONBUTTONIDX_FollowLine)
                {
                angleVec.rotate (m_lineAngle);
                if (angleVec.y > 0)
                    { m_angle -= PI; }
                }
            else
                {
                DPoint3d pt = intersectionPt;
                DTMPtr dtm;

                if (SUCCESS == ih.GetDtmDataRef()->GetDTMReferenceDirect (dtm))
                    {
                    double aspect;
                    int drapeType;
                    dtm->GetDTMDraping()->DrapePoint (NULL, NULL, &aspect, NULL, drapeType, pt) ;
                    angleVec.rotate (-aspect * PI / 180.);
                    if (angleVec.y > 0.)
                        { m_angle -= PI; }
                    }
                }
            mdlRMatrix_fromAngle (&m_rotMatrix, m_angle);
            DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*>(&m_dimElm.GetHandler());
            dimHandler->SetViewRotation (m_dimElm, m_rotMatrix);
            dimHandler->SetRotationMatrix (m_dimElm, m_rotMatrix);

            dimHandler->SetPoint (m_dimElm, &midPoint/*intersectionPt*/, nullptr, 0);
            dimHandler->SetTextOffset (m_dimElm, 0, DPoint2d::From (0.0001, 0.0001));
            //ToDo mdlDim_setViewRotation (&m_dimElm, &m_rotMatrix);
            //m_dimElm.dim.InsertPoint (intersectionPt);
            //m_dimElm.dim.GetDimTextP(0)->offset = 0.0001;
            //if (NULL != dimHandler)
            //    dimHandler->ValidateElementRange (m_dimElm, m_dimElm.GetModelRef()->Is3d());
            EditElementHandle dimElm (m_dimElm, true);
            m_collector.push_back (dimElm);
            }

    }; // end LabelContoursCollector struct


struct ContoursIntersectOutput : public SimplifyViewDrawGeom, public IntersectionHelper
    {
    DPoint3d                m_pts[2];
    ElementHandle              m_elem;
    bool                    m_processFirstIntersection;
    bool                    m_sheet;
    LabelContoursCollector& m_textCollector;
    LazyStorageToUORTransformProvider&  m_stu;
    double                  m_factor;
    DPoint3d                m_globalOrigin;
    bool                    m_ignore;

    public: ContoursIntersectOutput
                (
                LabelContoursCollector&             textCollector,
                LazyStorageToUORTransformProvider&  stu
                ) : m_textCollector (textCollector), m_stu (stu), m_processFirstIntersection (false), m_ignore(false)
                {}

   private: ViewportP GetViewport (void)
                 { return GetViewContext()->GetViewport(); }

    public: void ContoursIntersectOutput::SetDTMRef (DTMDataRefPtr value, ElementHandleCR elem)
                {
                m_elem = elem;
                m_dtmDataRef = value;
                m_processFirstIntersection = true;
                }

    private: void OnFirstIntersection (void)
                 {
                 DgnModelRef*    targetModelRef;

                 targetModelRef = GetViewport() ? GetViewport()->GetTargetModel() : ACTIVEMODEL;
                 if (m_sheet = (mdlModelRef_isSheet(targetModelRef) || !mdlModelRef_is3D(targetModelRef)))
                     {
                     DgnModelRef* srcModelRefP;

                     srcModelRefP = m_elem.IsValid() ? m_elem.GetModelRef() : GetViewContext()->GetCurrentModel();
                     m_factor = mdlModelRef_getUorPerMeter(targetModelRef) / mdlModelRef_getUorPerMeter(srcModelRefP);
                     mdlModelRef_getGlobalOrigin (srcModelRefP, &m_globalOrigin);
                     }
                 else
                     {
                     m_factor = 1.;
                     mdlModelRef_getGlobalOrigin (targetModelRef, &m_globalOrigin);
                     }
                 }

    protected: virtual bool _ProcessAsStrokes (bool isCurved) const {return true;}
    protected: virtual StatusInt ContoursIntersectOutput::_ProcessLinearSegments (DPoint3dCP points, size_t numPoints, bool closed, bool filled) override
                   {
                   DPoint3d intersectPtsA[100];
                   int numIntersectPts, totalNumIntersectPts;
                   DMatrix4d mat;

                   GetViewContext()->GetCurrLocalToActiveTrans (mat);
                   m_numPoints = (int)numPoints;
                   m_activeUORs = (DPoint3dP)_alloca (numPoints * sizeof(DPoint3d));
                   mat.multiplyAndRenormalize (m_activeUORs, points, (int)numPoints);

#ifdef DRAWDEBUG
                   GetViewport()->GetIViewDraw()->SetSymbology (0xff0f00, 0xff0000, 1, -1);
                   GetViewport()->GetIViewDraw()->DrawLineString3d(numPoints, translatedPts, NULL);
#endif

                   GetViewContext()->LocalToView (m_activeUORs, m_activeUORs, (int)numPoints);
                   GetViewContext()->LocalToView (m_pts, m_pts, _countof(m_pts));
                   bsiDPoint3dArray_polylineIntersectXY (intersectPtsA, NULL, NULL, NULL, NULL, NULL,
                       &numIntersectPts, &totalNumIntersectPts, _countof(intersectPtsA), m_activeUORs, (int)numPoints, false, m_pts, _countof(m_pts), false);
                   GetViewContext()->ViewToLocal (m_activeUORs, m_activeUORs, (int)numPoints);
                   GetViewContext()->ViewToLocal (m_pts, m_pts, _countof(m_pts));
                   GetViewContext()->ViewToLocal (intersectPtsA, intersectPtsA, numIntersectPts);

                   if (numIntersectPts > 0)
                       {
                       DVec3d originShift;

                       if (m_processFirstIntersection)
                           {
                           OnFirstIntersection ();
                           m_processFirstIntersection = false;
                           }

                       if (m_sheet)
                           {
                           DPoint3d pt = points[0];
                           mdlTMatrix_transformPoint (&pt, &m_stu.GetTransform());
                           originShift = pt -m_globalOrigin;
                           }
                       else
                           { 
                           originShift = m_activeUORs[0] - m_globalOrigin;
                           }

                       m_textCollector.PreTraverse (originShift.z * m_factor, GetViewport()->GetViewNumber());
                       for (DPoint3dCP intersectionPt = &intersectPtsA[0]; intersectionPt < &intersectPtsA[numIntersectPts]; ++intersectionPt)
                           { m_textCollector.OnIntersection (*intersectionPt, *this); }
                       }
                   return SUCCESS;
                   }
    public:
        void Init (ViewContextP context, DPoint3dCR pt1, DPoint3dCR pt2)
            {
            m_pts[0] = pt1;
            m_pts[1] = pt2;
            SetViewContext (context);
            }
    };

struct ContoursIntersectContext : public ViewContext
    {
    ContoursIntersectOutput     m_output;

    public:
        ContoursIntersectContext
            (
            LabelContoursCollector&             textCollector,
            LazyStorageToUORTransformProvider&  stu,
            ViewportP vp, DPoint3dCR pt1, DPoint3dCR pt2
            ) : m_output (textCollector, stu)
            {
            if (vp && vp->GetIViewOutput ())
                _Attach (vp, DrawPurpose::CaptureGeometry);
            m_output.Init (this, pt1, pt2);
            SetIViewDraw (m_output);
            }

        ~ContoursIntersectContext (void)
            {
            if (IsAttached ()) // Fence need not have a viewport...
                _Detach ();
            }

    protected:
        virtual void            _SetupOutputs ()
            {
            // toDo?
            }

        virtual void _OutputElement (ElementHandleCR inEl) override
            {
            DTMDataRefPtr DTMDataRef;
            DPoint3d fencePts[5];
            DRange3d drange;
            double extent;
            ElementHandle symbologyElem;

            DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, inEl);
            m_output.SetDTMRef (DTMDataRef, inEl);
            drange.initFrom (m_output.m_pts, _countof(m_output.m_pts));
            extent = sqrt (drange.extentSquared ());
            drange.extend (extent * 0.2);
            fencePts[0].x = drange.low.x;
            fencePts[0].y = drange.low.y;
            fencePts[0].z = 0;
            fencePts[1].x = drange.high.x;
            fencePts[1].y = drange.low.y;
            fencePts[1].z = 0;
            fencePts[2].x = drange.high.x;
            fencePts[2].y = drange.high.y;
            fencePts[2].z = 0;
            fencePts[3].x = drange.low.x;
            fencePts[3].y = drange.high.y;
            fencePts[3].z = 0;
            fencePts[4].x = drange.low.x;
            fencePts[4].y = drange.low.y;
            fencePts[4].z = 0;

            DTMSubElementContext subElementContext;
            DTMElementHandlerManager::GetElementForSymbology (inEl, symbologyElem, ACTIVEMODEL);
            DTMSubElementIter &iter = *DTMSubElementIter::Create (symbologyElem);
            for (; iter.IsValid (); iter.ToNext ())
                {
                if (DTMElementContoursHandler::GetInstance()->GetSubHandlerId () == iter.GetCurrentId().GetHandlerId ())
                    {
                    RefCountedPtr<DTMElementContoursHandler::DisplayParams> params = DTMElementContoursHandler::DisplayParams::From (iter);
                    if (params->GetIsMajor () || dtmcommandsInfoP->annotateContoursMode == OPTIONBUTTONIDX_AllContours)
                        {
                        if (GetLocalToView ().coff[0][2] == 0)
                            DTMElementDisplayHandler::DrawSubElement (inEl, iter, *this, DTMFenceParams (DTMFenceType::Block, DTMFenceOption::Overlap, fencePts, _countof (fencePts)), subElementContext);
                        else
                            DTMElementDisplayHandler::DrawSubElement (inEl, iter, *this, DTMFenceParams (), subElementContext); //FenceType_Block, FenceOption_Overlap, fencePts, 5));
                        }
                    }
                }
            delete &iter;
            }
    };

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void CollectIntersections (bvector<ElementHandle>& geometryCollector, DPoint3dCR p1, DPoint3dCR p2, ElementHandleCR eh, ViewportP vp)
    {
    bool const PUSH_PARENTS = true;
    bool const CHECK_RANGE = true, CHECK_SCAN_CRITERIA = true;
    LabelContoursCollector collector (p1, p2, geometryCollector);
    LazyStorageToUORTransformProvider lazyStorage (eh);
    ContoursIntersectContext cic (collector, lazyStorage, vp, p1, p2);

    cic.PushModelRef (eh.GetModelRef(), PUSH_PARENTS);
    cic.VisitElemHandle (eh, !CHECK_RANGE, !CHECK_SCAN_CRITERIA);
    }

/// <author>Piotr.Slowinski</author>                            <date>04/2011</date>
bool GetDTMLabelOnToolsettings (MSDialogP &db, int &item)
    {
    MSDialogP toolSettings = mdlDialog_getToolSettings ();
    BeAssert ( toolSettings );
    if ( !toolSettings )
        return false;

    DialogItem *dtmLabel = mdlDialog_itemGetByTypeAndId (toolSettings, RTYPE_Label, LABELID_SelectedTerrainModel, 0);
    BeAssert (dtmLabel);
    if (!dtmLabel)
        return false;

    db = toolSettings;
    item = dtmLabel->itemIndex;
    return true;
    }

/// <author>Piotr.Slowinski</author>                            <date>03/2011</date>
void UpdateToolsettings (ElementHandleCR dtm)
    {
    MSDialogP   toolSettings   = NULL;
    int         labelIndex      = -1;

    if (!GetDTMLabelOnToolsettings (toolSettings, labelIndex))
        return;

    Bentley::WString    name;
    std::wstring        _name;
    if (dtm.IsValid () && SUCCESS != DTMElementHandlerManager::GetName (dtm, name))
        {
        // Now need to get the name from the handle.
        Bentley::WString wname;
        dtm.GetHandler().GetDescription (dtm, wname, 256);
        int i = (int)wname.find (L": ");
        if (i != - 1)
            name = wname.substr (i + 2).c_str ();
        }
    mdlDialog_itemSetLabel (toolSettings, labelIndex, const_cast<wchar_t*>(name.c_str()));
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
static void ViewMonitor (bool started)
    {
    Annotate::SequencedTool *sequencedTool;

    if (started)
        { return; }
    sequencedTool = dynamic_cast <Annotate::SequencedTool*> (DgnTool::GetActivePrimitiveTool());
    BeAssert (NULL != sequencedTool);
    if (NULL == sequencedTool)
        { return; }
    if ( NULL != dynamic_cast <Annotate::SelectDTMElemTool*>(sequencedTool) )
        {
        MSDialogP   toolSettings   = NULL;
        int         labelIndex      = -1;
        wchar_t     msg[1024]       = L"";

        if (!GetDTMLabelOnToolsettings (toolSettings, labelIndex))
            { return; }

        mdlDialog_itemSetLabel (toolSettings, labelIndex, SUCCESS == mdlResource_loadFromStringList (msg, NULL, WMSGLIST_Main, WSTATUS_NotSelected) ? msg : L"<Not Selected>");
        }
    else
        {
        Annotate::DTMHolder *dtmHolder;

        dtmHolder = dynamic_cast <Annotate::DTMHolder*> (DgnTool::GetActivePrimitiveTool());
        BeAssert (NULL != dtmHolder);
        if (NULL  == dtmHolder)
            { return; }
        UpdateToolsettings (dtmHolder->GetDtm());
        }
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void StartViewMonitor (void)
    {
    SystemCallback::SetMonitorViewCommandsFunction (&ViewMonitor);
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void EndViewMonitor (void)
    {
    SystemCallback::SetMonitorViewCommandsFunction (NULL);
    }

/// <summary>
/// Based on wordproc_onDimStyleChange
/// </summary>
/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void    DimStyleChangeSpot (DgnModelRefP modelRef, ElementId styleId, DimensionStyleChangeType type)
    {
    /* ToDo
    if (!mdlModelRef_inMasterFile (modelRef))
    { return; }

    switch (type)
    {
    case DIMSTYLE_TABLE_CHANGE:
    case DIMSTYLE_CHANGE_ACTIVE:
    case DIMSTYLE_CHANGE_SETTINGS:
    {
    MSDialogP pDb = mdlDialog_getToolSettings ();
    if (!pDb)
    return;

    DialogItem  * pDimStyleCombo = mdlDialog_itemGetByTypeAndId (pDb, RTYPE_ComboBox, COMBOBOXID_DimStyle, 0);
    if (!pDimStyleCombo)
    { return; }

    bool REDRAW = true;
    mdlDialog_rItemReloadData (pDimStyleCombo->rawItemP, REDRAW);
    mdlDialog_itemsSynch (pDb);

    break;
    }
    }
    */
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void    DimStyleChangeContours (DgnModelRefP modelRef, ElementId styleId, DimensionStyleChangeType type)
    {
    if (!mdlModelRef_inMasterFile (modelRef))
    { return; }

    switch (type)
    {
    case DIMSTYLE_TABLE_CHANGE:
    case DIMSTYLE_CHANGE_ACTIVE:
    case DIMSTYLE_CHANGE_SETTINGS:
    {
    MSDialogP  toolSettings;
    DialogItem* textItem;
    bool        found[2] = {false, false};
    bool     REDRAW = true;

    toolSettings = mdlDialog_getToolSettings ();
    if (NULL == toolSettings)
    return;

    for (int index = 0; (!found[0] || !found[1]) && NULL != (textItem = mdlDialog_itemGetByTypeAndId (toolSettings, RTYPE_Text, TEXTID_GenericDistanceProp, index)); index = textItem->itemIndex + 1)
    {
    if (textItem->itemArg != DIMSTYLE_PROP_Text_Height_DISTANCE && textItem->itemArg != DIMSTYLE_PROP_Text_Width_DISTANCE)
    { continue; }
    BeAssert (!found[textItem->itemArg == DIMSTYLE_PROP_Text_Height_DISTANCE ? 0 : 1]);
    found [textItem->itemArg == DIMSTYLE_PROP_Text_Height_DISTANCE ? 0 : 1] = true;
    mdlDialog_rItemReloadData (textItem->rawItemP, REDRAW);
    }
    BeAssert (found[0] && found[1]);
    if (found[0] || found[1])
    mdlDialog_itemsSynch (toolSettings);
    break;
    }
    }
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void        StartSequence (UInt64 cmdNum)
    {
    StartViewMonitor ();
    switch (cmdNum)
        {
        case CMD_TERRAINMODEL_LABEL_SPOTS:
            {
            EngageWordlib ();
            SystemCallback::SetDimStyleChangeFunction (&DimStyleChangeSpot);
            break;
            }

        case CMD_TERRAINMODEL_LABEL_CONTOURS:
            {
            EngageDialogFilter ();
            SystemCallback::SetDimStyleChangeFunction (&DimStyleChangeContours);
            break;
            }

        default:
            { BeAssert (0); }
        }
    }

/// <author>Piotr.Slowinski</author>                            <date>09/2011</date>
void        EndSequence (UInt64 cmdNum)
    {
    switch (cmdNum)
        {
        case CMD_TERRAINMODEL_LABEL_SPOTS:
            {
            SystemCallback::SetDimStyleChangeFunction (NULL);
            DisengageWordlib ();
            break;
            }

        case CMD_TERRAINMODEL_LABEL_CONTOURS:
            {
            SystemCallback::SetDimStyleChangeFunction (NULL);
            DisengageDialogFilter ();
            break;
            }

        default:
            { BeAssert (0); }
        }
    EndViewMonitor ();
    }
