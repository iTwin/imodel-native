/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/RevisionComparisonViewController.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/ViewController.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/IAuxCoordSys.h>
#include <DgnPlatform/ViewContext.h>
#include <DgnPlatform/SectionClip.h>
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

    explicit State(DbOpcode opcode=DbOpcode::Insert) : m_opcode(opcode) { }

    bool IsInsertion() const { return DbOpcode::Insert == m_opcode; }
    bool IsDeletion() const { return DbOpcode::Delete == m_opcode; }
    bool IsModified() const { return DbOpcode::Update == m_opcode; }
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
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct TransientState : State
{
    DgnElementCPtr  m_element;

    TransientState() = default;
    TransientState(DgnElementCR el, DbOpcode opcode) : State(opcode), m_element(&el) { }

    bool IsValid() const { return m_element.IsValid(); }

    bool operator<(TransientState const& rhs) const { return m_element.get() < rhs.m_element.get(); }
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
    DGNPLATFORM_EXPORT TransientState GetTransientState(DgnElementId elementId) const;

    bset<PersistentState> const& GetPersistentStates() const { return m_persistent; }
    bset<TransientState> const& GetTransientStates() const { return m_transient; }

    void Clear() { m_persistent.clear(); m_transient.clear(); }
    void Add(DgnElementCR el, DbOpcode opcode) { m_transient.insert(TransientState(el, opcode)); }
    void Add(DgnElementId id, DbOpcode opcode) { m_persistent.insert(PersistentState(id, opcode)); }
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
    ComparisonDataCPtr  m_comparisonData;
    Show                m_show;

    Controller(SpatialViewDefinition const& view, ComparisonDataCR data, Show show, SymbologyCR symb) : T_Super(view), m_symbology(symb), m_comparisonData(&data), m_show(show) { }

    DGNPLATFORM_EXPORT void _AddFeatureOverrides(Render::FeatureSymbologyOverrides& overrides) const override;
    DGNPLATFORM_EXPORT BentleyStatus _CreateScene(RenderContextR context) override;
public:
    static ControllerPtr Create(SpatialViewDefinition const& view, ComparisonDataCR data, Show show=kShowBoth, SymbologyCR symb=Symbology())
        {
        return new Controller(view, data, show, symb);
        }

    void SetShow(Show show) { m_show = show; SetFeatureOverridesDirty(); }
    void SetSymbology(SymbologyCR symb) { m_symbology = symb; SetFeatureOverridesDirty(); }

    bool WantShowCurrent() const { return 0 != (m_show & kShowCurrent); }
    bool WantShowTarget() const { return 0 != (m_show & kShowTarget); }
};

END_REVISION_COMPARISON_NAMESPACE

