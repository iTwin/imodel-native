/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PickContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// __BENTLEY_INTERNAL_ONLY__

#include    "Locate.h"
#include    <DgnPlatform/DgnCore/SimplifyViewDrawGeom.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum TestLStylePhase
    {
    TEST_LSTYLE_None        = 0,
    TEST_LSTYLE_Component   = 1,
    TEST_LSTYLE_BaseGeom    = 2,
    };

/*=================================================================================**//**
* Output to determine if element should be accepted for fence processing..
* @bsiclass                                                     Brien.Bastings  09/04
+===============+===============+===============+===============+===============+======*/
struct PickOutput : public SimplifyViewDrawGeom, IPickGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
private:

    bool                        m_doneSearching;
    bool                        m_unusableLStyleHit;
    bool                        m_doLocateSilhouettes;
    bool                        m_doLocateInteriors;
    TestLStylePhase             m_testingLStyle;

    IViewOutputP                m_viewOutput;
    GeomDetail                  m_currGeomDetail;
    HitListP                    m_hitList;
    HitPriority                 m_hitPriorityOverride;

    DPoint3d                    m_pickPointWorld;
    DPoint4d                    m_pickPointView;
    double                      m_pickAperture;
    double                      m_pickApertureSquared;

    LocateOptions               m_options;

    bool                PointWithinTolerance (DPoint4dCR testPt);
    bool                TestPoint (DPoint3dCR localPt, HitPriority priority);
    bool                TestPointArray (size_t numPts, DPoint3dCP localPts, HitPriority priority);
    bool                TestCurveVector (CurveVectorCR, HitPriority);
    bool                TestCurveVectorInterior (CurveVectorCR, HitPriority priority);
    bool                TestIndexedPolyEdge (DPoint3dCP verts, DPoint4dCP hVertsP, int closeVertexId, int segmentVertexId, DPoint3dR pickPt, HitPriority);
    bool                TestQvElem (QvElem* qvElem, HitPriority);
    static int          LocateQvElemCheckStop (CallbackArgP);
    void                AddSurfaceHit (DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority);

protected:

    virtual bool        _DoClipping () const override {return m_inSymbolDraw;} // Only need clip for symbols...
    virtual bool        _DoSymbolGeometry () const override {return true;}

    virtual StatusInt   _ProcessCurvePrimitive (ICurvePrimitiveCR, bool closed, bool filled) override;
    virtual StatusInt   _ProcessCurveVector (CurveVectorCR, bool isFilled) override;
    virtual StatusInt   _ProcessSolidPrimitive (ISolidPrimitiveCR) override;
    virtual StatusInt   _ProcessSurface (MSBsplineSurfaceCR) override;
    virtual StatusInt   _ProcessFacetSet (PolyfaceQueryCR, bool filled) override;
    virtual StatusInt   _ProcessBody (ISolidKernelEntityCR) override;

    virtual void        _PushTransClip (TransformCP trans, ClipPlaneSetCP clip) override;
    virtual void        _PopTransClip () override;
    virtual void        _SetDrawViewFlags (ViewFlagsCP flags) override;

    virtual void        _DrawTextString (TextStringCR, double* zDepth) override;
    virtual bool        _DrawSprite (ISpriteP sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) override;
    virtual void        _DrawPointCloud (IPointCloudDrawParams* drawParams) override;
    virtual void        _DrawQvElem (QvElem* qvElem, int subElemIndex) override;

public:
    PickOutput ();
    DGNPLATFORM_EXPORT ~PickOutput ();

    void                Init (ViewContextP, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP, LocateOptions const& options);

    double              GetPickAperture () {return m_pickAperture;}
    HitListP            GetHitList () {return m_hitList;}
    bool                GetDoneSearching () {return m_doneSearching;}
    void                SetTestLStylePhase (TestLStylePhase phase) {m_testingLStyle = phase; if (TEST_LSTYLE_None == phase) m_unusableLStyleHit = false;}
    DPoint3dP           GetProjectedPickPointView (DPoint3dR pPoint);
    void                SetupViewOutput (IViewOutputP output) {m_viewOutput = output;}
    void                InitStrokeForCache () {m_doLocateSilhouettes = false;}
    bool                GetLocateSilhouettes () {return m_doLocateSilhouettes;}
    bool*               GetLocateInteriors () {return &m_doLocateInteriors;}

    // IPickGeom
    virtual DPoint4dCR  _GetPickPointView() const override {return m_pickPointView;}
    virtual DPoint3dCR  _GetPickPointWorld() const override {return m_pickPointWorld;}
    virtual GeomDetailR _GetGeomDetail() override {return m_currGeomDetail;}
    virtual bool        _IsPointVisible(DPoint3dCP screenPt) override;
    virtual void        _SetHitPriorityOverride(HitPriority priority) override {m_hitPriorityOverride = priority;}
    virtual void        _AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority) override;
    virtual bool        _IsSnap() const;
    virtual DRay3d      _GetBoresite() const;

}; // PickOutput

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct PickContext : public ViewContext
{
    DEFINE_T_SUPER(ViewContext)
private:
    PickOutput          m_output;
    StopLocateTest*     m_stopTester;
    LocateOptions       m_options;

    virtual void            _SetupOutputs () override;
    virtual bool            _CheckStop() override;
    virtual StatusInt       _VisitDgnModel (DgnModelP inDgnModel) override;
    virtual void            _OutputElement (GeometricElementCR) override;
    virtual void            _OnPreDrawTransient() override;
    virtual void            _DrawAreaPattern (ViewContext::ClipStencil& boundary) override;
    virtual void            _DrawSymbol (IDisplaySymbolP, TransformCP, ClipPlaneSetP, bool ignoreColor, bool ignoreWeight) override;
    virtual void            _DeleteSymbol (IDisplaySymbolP) override {}
    virtual ILineStyleCP    _GetCurrLineStyle (LineStyleSymbP*) override;
    virtual void            _DrawStyledLineString2d (int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false) override;
    virtual void            _DrawStyledLineString3d (int nPts, DPoint3dCP pts,  DPoint3dCP range, bool closed = false) override;
    virtual void            _DrawStyledArc2d (DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) override;
    virtual void            _DrawStyledArc3d (DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) override;
    virtual void            _DrawStyledBSplineCurve3d (MSBsplineCurveCR) override;
    virtual void            _DrawStyledBSplineCurve2d (MSBsplineCurveCR, double zDepth) override;
    virtual QvElemP         _DrawCached (IStrokeForCache&) override;
    virtual IPickGeomP      _GetIPickGeom () override {return &m_output;}

    void                    InitNpcSubRect (DPoint3dCR pickPointWorld, double pickAperture, DgnViewportR viewport);
    void                    InitSearch (DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP);

public:
    DGNPLATFORM_EXPORT PickContext (LocateOptions const& options, StopLocateTest* stopTester=NULL);

    DGNPLATFORM_EXPORT bool PickElements (DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureDevice, HitListP hitList);
    DGNPLATFORM_EXPORT TestHitStatus TestHit (HitDetailCR, DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList);

    DGNPLATFORM_EXPORT static void InitBoresite (DRay3dR boresite, DPoint3dCR spacePoint, DMatrix4dCR worldToLocal);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

