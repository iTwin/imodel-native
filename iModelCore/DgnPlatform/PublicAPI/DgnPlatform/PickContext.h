/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include  <DgnPlatform/SimplifyGraphic.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum class TestLStylePhase
{
    None        = 0,
    Component   = 1,
    BaseGeom    = 2
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
enum class TestHitStatus
{
    NotOn       = 0,
    IsOn        = 1,
    Aborted     = 2
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/10
//=======================================================================================
struct StopLocateTest
{
    virtual ~StopLocateTest() {}
    virtual bool _CheckStopLocate() = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/14
+===============+===============+===============+===============+===============+======*/
struct LocateOptions
{
private:
    bool m_disableDgnDbFilter = false;
    bool m_allowTransients = false;
    uint32_t m_maxHits;
    HitSource m_hitSource;

public:
    LocateOptions()
        {
        m_maxHits = 20;
        m_hitSource = HitSource::DataPoint;
        }

    LocateOptions(HitSource hitSource, uint32_t maxHits)
        {
        m_hitSource = hitSource;
        m_maxHits = maxHits;
        }

    void SetDisableDgnDbFilter(bool disableDgnDbFilter) {m_disableDgnDbFilter = disableDgnDbFilter;}
    void SetAllowTransients(bool allowTransients) {m_allowTransients = allowTransients;}
    void SetMaxHits(uint32_t maxHits) {m_maxHits = maxHits;}
    void SetHitSource(HitSource hitSource) {m_hitSource = hitSource;}

    bool GetDisableDgnDbFilter() const {return m_disableDgnDbFilter;}
    bool GetAllowTransients() const {return m_allowTransients;}
    uint32_t GetMaxHits() const {return m_maxHits;}
    HitSource GetHitSource() const {return m_hitSource;}
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct PickContext : ViewContext, IPickGeom, IGeometryProcessor
{
    DEFINE_T_SUPER(ViewContext)
    friend struct SheetAttachmentPicker;

private:
    bool m_doneSearching;
    bool m_unusableLStyleHit;
    TestLStylePhase m_testingLStyle;
    GeomDetail m_currGeomDetail;
    HitListP m_hitList;
    HitPriority m_hitPriorityOverride;
    DPoint3d m_pickPointWorld;
    DPoint4d m_pickPointView;
    double m_pickAperture;
    double m_pickApertureSquared;
    uint32_t m_overflowCount;
    LocateOptions m_options;
    StopLocateTest* m_stopTester;
    DgnViewportP m_sheetViewport = nullptr;
    DMap4dCP m_sheetMap = nullptr; // from attachement world to sheet world.
    GeometrySourceCP m_currentGeomSource;
    IElemTopologyCPtr m_currElemTopo;
    HitDescriptionPtr m_hitDescription;

    void SetPickAperture(double val) {m_pickAperture=val; m_pickApertureSquared=val*val;}
    IPickGeomP _GetIPickGeom () override {return this;}
    StatusInt _OutputGeometry(GeometrySourceCR) override;
    void _OnNewGeometry() override;
    bool _CheckStop() override;
    StatusInt _VisitDgnModel(GeometricModelR inDgnModel) override;
    void _DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params, bool doCook) override;
    void _DrawStyledCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curve, Render::GeometryParamsR params, bool doCook) override;
    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override {_GetGeomDetail().Init(); SimplifyGraphic* graphic = new SimplifyGraphic(params, *this, *this); return graphic;}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, DgnDbR db, TransformCR tf, ClipVectorCP) override {return new SimplifyGraphic::Base(db);}
    DPoint4d ConvertLocalToView(DPoint3dCR localPt, SimplifyGraphic const& graphic) const;
    DPoint3d ConvertViewToLocal(DPoint4dCR viewPt, SimplifyGraphic const& graphic) const;

    bool TestPoint(DPoint3dCR localPt, HitPriority priority, SimplifyGraphic const&);
    bool TestPointArray(size_t numPts, DPoint3dCP localPts, HitPriority priority, SimplifyGraphic const&);
    bool TestArcCenterAndPointString(ICurvePrimitiveCR primitive, SimplifyGraphic const&);
    bool TestCurveVector(CurveVectorCR, HitPriority, SimplifyGraphic const&);
    bool TestCurveVectorInterior(CurveVectorCR, HitPriority priority, SimplifyGraphic const&);

    void InitNpcSubRect(DPoint3dCR pickPointWorld, double pickAperture, DgnViewportCR viewport);
    void InitSearch(DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP);
    bool PointWithinTolerance(DPoint4dCR testPt);
    DRay3d GetBoresite(SimplifyGraphic const&) const;
    void AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority, SimplifyGraphic const&);
    void AddSurfaceHit(DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority, SimplifyGraphic const&);
    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
    UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
    bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic&) override;
    bool _ProcessCurveVector(CurveVectorCR, bool filled, SimplifyGraphic&) override;
    bool _ProcessSolidPrimitive(ISolidPrimitiveCR, SimplifyGraphic&) override;
    bool _ProcessSurface(MSBsplineSurfaceCR, SimplifyGraphic&) override;
    bool _ProcessPolyface(PolyfaceQueryCR, bool filled, SimplifyGraphic&) override;
    bool _ProcessBody(IBRepEntityCR, SimplifyGraphic&) override;
    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;
    bool _DoPatternStroke(PatternParamsCR, SimplifyGraphic&) const override {return true;} // locate/snap to pattern and line style geometry...
    bool _DoLineStyleStroke(Render::LineStyleSymbCR lsSymb, IFacetOptionsPtr& options, SimplifyGraphic& graphic) const override {return TestLStylePhase::Component == m_testingLStyle;}

public:
    double GetPickAperture() {return m_pickAperture;}
    HitListP GetHitList() {return m_hitList;}
    bool GetDoneSearching() {return m_doneSearching;}
    void SetTestLStylePhase(TestLStylePhase phase) {m_testingLStyle = phase; if (TestLStylePhase::None == phase) m_unusableLStyleHit = false;}
    DPoint3dP GetProjectedPickPointView(DPoint3dR pPoint);

    bool _IsSnap() const override;
    DPoint4dCR _GetPickPointView() const override {return m_pickPointView;}
    DPoint3dCR _GetPickPointWorld() const override {return m_pickPointWorld;}
    DRay3d _GetBoresite(TransformCR localToWorld) const override;
    void _SetHitPriorityOverride(HitPriority priority) override {m_hitPriorityOverride = priority;}
    GeomDetailR _GetGeomDetail() override {return m_currGeomDetail;}
    void _AddHit(HitDetailR) override;
    IElemTopologyCP _GetElemTopology() const override {return (m_currElemTopo.IsValid() ? m_currElemTopo.get() : nullptr);}
    void _SetElemTopology(IElemTopologyCP topo) override {m_currElemTopo = topo;}
    virtual bool _ProcessSheetAttachment(Sheet::Attachment::ViewportR);
    void SetHitDescription(HitDescription* descr) {m_hitDescription = descr;}
    HitDescriptionPtr GetHitDescription() const {return m_hitDescription;}

    DGNVIEW_EXPORT PickContext(LocateOptions const& options, StopLocateTest* stopTester=nullptr);
    DGNVIEW_EXPORT bool PickElements(DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureDevice, HitListP hitList);
    DGNVIEW_EXPORT TestHitStatus TestHit(HitDetailCR, DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList);
    DGNVIEW_EXPORT static void InitBoresite(DRay3dR boresite, DPoint3dCR localPoint, DMatrix4dCR viewToLocal);
    DGNVIEW_EXPORT static void InitBoresite(DRay3dR boresite, DPoint3dCR worldPoint, DgnViewportCR vp, TransformCP localToWorld=nullptr);
    DGNVIEW_EXPORT static void InitBoresite(DRay3dR boresite, DgnButtonEventCR ev, TransformCP localToWorld=nullptr);

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   02/17
    //=======================================================================================
    struct ActiveDescription 
    {
        PickContextR m_context;
        ActiveDescription(PickContextR context, Utf8StringCR descr) : m_context(context) {context.SetHitDescription(new HitDescription(descr));}
        ~ActiveDescription() {m_context.SetHitDescription(nullptr);}
    };
};

END_BENTLEY_DGN_NAMESPACE
                                                                                                                                                                      
