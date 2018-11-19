/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RevisionComparisonViewController.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/ViewController.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/IAuxCoordSys.h>
#include <DgnPlatform/ViewContext.h>
#include <DgnPlatform/UpdatePlan.h>
#include <DgnPlatform/ViewDefinition.h>
#include <Bentley/BeThread.h>
#include <BeSQLite/RTreeMatch.h>
#include <DgnPlatform/Render.h>

#define BEGIN_REVISION_COMPARISON_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace RevisionComparison {
#define END_REVISION_COMPARISON_NAMESPACE } END_BENTLEY_DGN_NAMESPACE
#define USING_REVISION_COMPARISON_NAMESPACE using namespace BentleyApi::Dgn::RevisionComparison;

BEGIN_REVISION_COMPARISON_NAMESPACE

using DbOpcode = BeSQLite::DbOpcode;

DEFINE_POINTER_SUFFIX_TYPEDEFS(Controller);
DEFINE_REF_COUNTED_PTR(Controller);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Symbology);
DEFINE_POINTER_SUFFIX_TYPEDEFS(ComparisonData);
DEFINE_REF_COUNTED_PTR(ComparisonData);

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct Symbology
{
    using Appearance = Render::FeatureSymbologyOverrides::Appearance;
private:
    struct Overrides
    {
        Appearance  m_appearance[3];

        static constexpr size_t GetIndex(DbOpcode opcode) { return DbOpcode::Insert == opcode ? 0 : (DbOpcode::Update == opcode ? 1 : 2); }

        Appearance& GetAppearance(DbOpcode opcode) { return m_appearance[GetIndex(opcode)]; }
        Appearance const& GetAppearance(DbOpcode opcode) const { return m_appearance[GetIndex(opcode)]; }
        void SetAppearance(DbOpcode opcode, Appearance const& app) { m_appearance[GetIndex(opcode)] = app; }
    };

    Overrides   m_current;
    Overrides   m_target;
    Appearance  m_untouched;
public:
    Symbology() { InitializeDefaults(); }

    Appearance GetCurrentRevisionOverrides(DbOpcode opcode) const { return m_current.GetAppearance(opcode); }
    Appearance GetTargetRevisionOverrides(DbOpcode opcode) const { return m_target.GetAppearance(opcode); }
    Appearance GetUntouchedOverrides() const { return m_untouched; }

    Appearance& GetCurrentRevisionOverrides(DbOpcode opcode) { return m_current.GetAppearance(opcode); }
    Appearance& GetTargetRevisionOverrides(DbOpcode opcode) { return m_target.GetAppearance(opcode); }
    Appearance& GetUntouchedOverrides() { return m_untouched; }

    DGNPLATFORM_EXPORT void InitializeDefaults();
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct State
{
    DbOpcode    m_opcode;
    bool        m_hidden;

    explicit State(DbOpcode opcode=DbOpcode::Insert) : m_opcode(opcode), m_hidden(false) { }

    bool IsInsertion() const { return DbOpcode::Insert == m_opcode; }
    bool IsDeletion() const { return DbOpcode::Delete == m_opcode; }
    bool IsModified() const { return DbOpcode::Update == m_opcode; }
    
    bool IsHidden() const { return m_hidden; }
    void SetHidden(bool value) { m_hidden = value; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct PersistentState : State
{
    DgnElementId    m_elementId;

    PersistentState() = default;
    PersistentState(DgnElementId elementId, DbOpcode opcode) : State(opcode), m_elementId(elementId) { }

    bool IsValid() const { return m_elementId.IsValid(); }

    bool operator<(PersistentState const& rhs) const { return m_elementId < rhs.m_elementId; }
};

//=======================================================================================
// @bsistruct                                                   Diego.Pinate    11/18
//=======================================================================================
struct TransientGraphic
{
    Render::GraphicPtr  m_graphic;
    double              m_minSize;
    double              m_maxSize;
    DgnElementId        m_elementId;

    TransientGraphic() { }
    TransientGraphic(DgnElementId id, Render::GraphicPtr graphic, DRange1d range) : m_elementId(id), m_graphic(graphic), m_minSize(range.low), m_maxSize(range.high) { }

    bool IsValidForSize(double pixelSize) const;
    bool IsValid() const { return m_elementId.IsValid(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct TransientState : State
{
    DgnElementCPtr m_element;
    mutable Render::GraphicPtr          m_graphic;
    mutable bvector<TransientGraphic>   m_graphicCache;
    mutable bool m_drawn;
    double m_minSize, m_maxSize; TransientState() = default;
    TransientState(DgnElementCPtr el, DbOpcode opcode) : State(opcode), m_element(el) { }

    bool IsValid() const { return m_element.IsValid(); }

    TransientGraphic GetFromCache(double pixelSize) const;

    bool operator<(TransientState const& rhs) const { return m_element.get() < rhs.m_element.get(); }

    DRange1d GetRange(double pixelSize, AxisAlignedBox3d const& elementRange) const;

    Render::GraphicP GetGraphic(ViewContextR context, bool wantShowBoth) const;
    Render::GraphicP GetGraphic(ViewContextR context) const;
    DgnElementId GetElementId() const { return m_element.IsValid() ? m_element->GetElementId() : DgnElementId(); }

    bool IsDrawn() const                { return m_drawn; }
    void SetIsDrawn(bool value) const   { m_drawn = value; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct ComparisonData : RefCountedBase
{
private:
    bset<PersistentState>   m_persistent;
    bset<TransientState>    m_transient;
public:
    DGNPLATFORM_EXPORT PersistentState GetPersistentState(DgnElementId elementId) const;
    DGNPLATFORM_EXPORT PersistentState GetAnyPersistentState(DgnElementId elementId) const;
    DGNPLATFORM_EXPORT TransientState GetTransientState(DgnElementId elementId) const;

    bset<PersistentState> const& GetPersistentStates() const { return m_persistent; }
    bset<TransientState> const& GetTransientStates() const { return m_transient; }

    void Clear() { m_persistent.clear(); m_transient.clear(); }
    void Add(DgnElementCPtr el, DbOpcode opcode) { m_transient.insert(TransientState(el, opcode)); }
    void Add(DgnElementId id, DbOpcode opcode) { m_persistent.insert(PersistentState(id, opcode)); }
    bool ContainsElement(DgnElementCP element) const;

    void UpdateTransientDisplay(DgnCategoryIdSet const& categories, DgnModelIdSet const& models);

    DGNPLATFORM_EXPORT void HideElement(DgnElementId id, bool transient, bool hidden = true);
    DGNPLATFORM_EXPORT void ClearHidden();

    DGNPLATFORM_EXPORT StatusInt GetDbOpcode(DgnElementId elementId, DbOpcode& opcode);
};

//=======================================================================================
//! Used to compare two revisions, showing the elements with different symbology
//! overrides depending on their opcode in the change summary
//! e.g. Show a particular element in "Red" for deleted elements,
//! "Green" for inserted, etc.
// @bsistruct                                                   Diego.Pinate    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Controller : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController)
    friend struct SpatialViewDefinition;

    enum Show
    {
        kShowCurrent = 1 << 0,
        kShowTarget = 1 << 1,
        kShowBoth = kShowCurrent | kShowTarget,
    };
protected:
    Symbology           m_symbology;
    ComparisonDataPtr   m_comparisonData;
    Show                m_show;
    bmap<DgnElementId, DbOpcode>    m_persistentOpcodeCache;
    bmap<DgnElementId, DbOpcode>    m_transientOpcodeCache;
    std::function<void(ControllerPtr)>   m_cnmHandler;

    DgnElementId        m_focusedElementId;
    Utf8String          m_labelString;
#ifdef USE_LABEL
    TextStringPtr       m_label;
#endif
    
    DGNPLATFORM_EXPORT void _DrawDecorations(DecorateContextR context) override;
    void _OnViewOpened (Dgn::DgnViewportR) override;

    DGNPLATFORM_EXPORT void _AddFeatureOverrides(Render::FeatureSymbologyOverrides& overrides) const override;
    DGNPLATFORM_EXPORT BentleyStatus _CreateScene(SceneContextR context) override;
    DGNPLATFORM_EXPORT Render::GraphicPtr _StrokeGeometry(ViewContextR, GeometrySourceCR, double) override;

    void _OnCategoryChange(bool singleEnable) override;
    void _ChangeModelDisplay(DgnModelId, bool onOff) override;
public:
    static ControllerPtr Create(SpatialViewDefinition const& view, ComparisonDataR data, Show show=kShowBoth, SymbologyCR symb=Symbology())
        {
        return new Controller(view, data, show, symb);
        }

    void SetShow(Show show) { m_show = show; SetFeatureOverridesDirty(); }
    void SetSymbology(SymbologyCR symb) { m_symbology = symb; SetFeatureOverridesDirty(); }
    bool WantShowBoth() const { return WantShowCurrent() && WantShowTarget(); }
    bool WantShowCurrent() const { return 0 != (m_show & kShowCurrent); }
    bool WantShowTarget() const { return 0 != (m_show & kShowTarget); }
    bool WantShowOnlyCurrent() const { return kShowCurrent == m_show; }
    bool WantShowOnlyTarget() const { return kShowTarget == m_show; }
    DGNPLATFORM_EXPORT void SetItemsDisplayHandler(std::function<void(ControllerPtr)> handler);
    std::function<void(ControllerPtr)> GetHandler() { return m_cnmHandler; }

    void UpdateTransientGraphicDisplay();

    DGNPLATFORM_EXPORT void SetModelDisplay(DgnModelIdSet& modelIds, bool visible);
    DGNPLATFORM_EXPORT void SetCategoryDisplay(DgnCategoryIdSet& categories, bool visible);
    DGNPLATFORM_EXPORT void SetVersionLabel(Utf8String label);
    DGNPLATFORM_EXPORT void SetFocusedElementId(DgnElementId elementId) { m_focusedElementId = elementId; SetFeatureOverridesDirty(); }
    DGNPLATFORM_EXPORT Controller(SpatialViewDefinition const& view, ComparisonDataR data, Show flags, SymbologyCR symb=Symbology());
};

END_REVISION_COMPARISON_NAMESPACE

