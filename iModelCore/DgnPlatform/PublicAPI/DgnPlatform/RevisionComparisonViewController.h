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

DGNPLATFORM_TYPEDEFS(RevisionComparisonViewController)
DGNPLATFORM_REF_COUNTED_PTR(RevisionComparisonViewController)
DGNPLATFORM_TYPEDEFS(ComparisonData)
DGNPLATFORM_REF_COUNTED_PTR(ComparisonData)

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Container for the symbology overrides
//! Contains overrides for the current loaded revision, the target revision,
//! and elements that were not affected
// @bsistruct                                                   Diego.Pinate    04/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComparisonSymbologyOverrides
{
private:
    // TODO: Change to array instead of map (always 3)
    bmap<DbOpcode,Render::OvrGraphicParams> m_currentRevisionOverrides;
    bmap<DbOpcode,Render::OvrGraphicParams> m_targetRevisionOverrides;
    Render::OvrGraphicParams                m_untouchedOverride;

public:
    DGNPLATFORM_EXPORT ComparisonSymbologyOverrides();

    //! Sets the symbology overrides for a particular opcode
    void    GetCurrentRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides);
    void    GetTargetRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides);
    void    GetUntouchedOverrides(Render::OvrGraphicParamsR overrides);
    void    InitializeDefaults();
}; // ComparisonSymbologyOverrides

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct RevisionComparisonState
{
    DbOpcode    m_opcode;

    explicit RevisionComparisonState(DbOpcode opcode=DbOpcode::Insert) : m_opcode(opcode) { }

    bool IsInsertion() const { return DbOpcode::Insert == m_opcode; }
    bool IsDeletion() const { return DbOpcode::Delete == m_opcode; }
    bool IsModified() const { return DbOpcode::Update == m_opcode; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PersistentState : RevisionComparisonState
{
    DgnElementId    m_elementId;

    PersistentState() = default;
    PersistentState(DgnElementId elementId, DbOpcode opcode) : RevisionComparisonState(opcode), m_elementId(elementId) { }

    bool IsValid() const { return m_elementId.IsValid(); }

    bool operator<(PersistentState const& rhs) const { return m_elementId < rhs.m_elementId; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TransientState : RevisionComparisonState
{
    DgnElementCPtr   m_element;

    TransientState() = default;
    TransientState(DgnElementCPtr el, DbOpcode opcode) : RevisionComparisonState(opcode), m_element(el) { }

    bool IsValid() const { return m_element.IsValid(); }

    bool operator<(TransientState const& rhs) const { return m_element.get() < rhs.m_element.get(); }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ComparisonData : RefCountedBase
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
    void Add(DgnElementCPtr el, DbOpcode opcode) { m_transient.insert(TransientState(el, opcode)); }
    void Add(DgnElementId id, DbOpcode opcode) { m_persistent.insert(PersistentState(id, opcode)); }
    bool ContainsElement(DgnElementCP element) const;

    DGNPLATFORM_EXPORT StatusInt GetDbOpcode(DgnElementId elementId, DbOpcode& opcode);
};

//=======================================================================================
//! Used to compare two revisions, showing the elements with different symbology
//! overrides depending on their opcode in the change summary
//! e.g. Show a particular element in "Red" for deleted elements,
//! "Green" for inserted, etc.
// @bsistruct                                                   Diego.Pinate    03/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RevisionComparisonViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController)
    friend struct SpatialViewDefinition;

    enum Flags
        {
        SHOW_CURRENT    = 1,
        SHOW_TARGET     = 1 << 1,
        SHOW_BOTH       = SHOW_CURRENT | SHOW_TARGET,
        };

protected:
    unsigned int                    m_flags;
    bool                            m_visitingTransientElements;

    ComparisonDataCPtr              m_comparisonData;
    ComparisonSymbologyOverrides    m_symbology;

    Utf8String      m_labelString;
    TextStringPtr   m_label;

    void _OverrideGraphicParams(Dgn::Render::OvrGraphicParamsR overide, Dgn::GeometrySourceCP source) override;
    void _DrawDecorations(DecorateContextR context) override;

public:

    bool WantShowCurrent() const { return 0 != (m_flags & SHOW_CURRENT); }
    bool WantShowTarget() const { return 0 != (m_flags & SHOW_TARGET); }
    bool WantShowBoth() const { return WantShowCurrent() && WantShowTarget(); }

    void                _CreateTerrain(TerrainContextR context) override;
    Render::GraphicPtr  _StrokeGeometry(ViewContextR, GeometrySourceCR, double pixelSize) override;

    //! Set flags for what's shown in the comparison
    DGNPLATFORM_EXPORT void SetFlags(unsigned int flags) { m_flags = flags; }
    DGNPLATFORM_EXPORT void SetSymbology(ComparisonSymbologyOverrides overrides) { m_symbology = overrides; }
    DGNPLATFORM_EXPORT void SetVersionLabel(Utf8String label);

    //! Constructors
    DGNPLATFORM_EXPORT RevisionComparisonViewController(SpatialViewDefinition const& view, ComparisonData const& data, unsigned int flags, ComparisonSymbologyOverrides const & symb=ComparisonSymbologyOverrides()) : T_Super(view), m_symbology(symb), m_comparisonData(&data), m_flags(flags), m_visitingTransientElements(false), m_label(nullptr) { }
};

END_BENTLEY_DGN_NAMESPACE
