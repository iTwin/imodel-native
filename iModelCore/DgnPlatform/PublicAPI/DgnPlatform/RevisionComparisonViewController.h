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

USING_NAMESPACE_BENTLEY_SQLITE

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Maintains the settings for the symbology used to display element comparison
//! This class is used to provide the user with overridable settings for the
//! revision comparison
// @bsistruct                                                   Diego.Pinate    04/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RevisionComparisonSettings
{
public:
    //=======================================================================================
    //! Container for the symbology overrides
    //! Contains overrides for the current loaded revision, the target revision,
    //! and elements that were not affected
    // @bsistruct                                                   Diego.Pinate    04/17
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE ComparisonSymbologyOverrides
    {
    private:
    bmap<DbOpcode,Render::OvrGraphicParams> m_currentRevisionOverrides;
    bmap<DbOpcode,Render::OvrGraphicParams> m_targetRevisionOverrides;
    Render::OvrGraphicParams                m_untouchedOverride;

    public:
    //! Sets the symbology overrides for a particular opcode
    //void     SetOverride (DbOpcode const& opcode, Render::OvrGraphicParamsCR overrides);
    void    GetCurrentRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides);
    void    GetTargetRevisionOverrides(DbOpcode const& opcode, Render::OvrGraphicParamsR overrides);
    void    GetUntouchedOverrides(Render::OvrGraphicParamsR overrides);
    //! Sets the color override for a particular opcode
    //void     SetColor (DbOpcode const& opcode, ColorDef const& color);
    //! Sets the transparency override for a particular opcode
    //void     SetTransparency (DbOpcode const& opcode, Byte transparency);
    //! Initialize hard-coded default values
    void    InitializeDefaults();
    };

private:
    static ComparisonSymbologyOverrides*        s_symbologyOverrides;

public:
    //! Initialize the default overrides
    static ComparisonSymbologyOverrides *       Overrides();

    static void                                 DeleteOverrides();
};


//=======================================================================================
//! Maintains data necessary to show the diffs between the current revision and
//! a target revision.
//! Maintains DgnElementPtr of elements that were inserted/modified
//! Maintains a list of element IDs that were modified in the Db and deleted
// @bsistruct                                                   Diego.Pinate    03/17
//=======================================================================================
struct  EXPORT_VTABLE_ATTRIBUTE RevisionComparisonElementKeeper
{
public:
    //=======================================================================================
    //! Provides a way to pair a type with an Opcode
    //! Useful to know which symbology to use on each DgnElementPtr or Element IDs in Db
    //! Later on it should obtain symbology automatically based on user settings
    // @bsistruct                                                   Diego.Pinate    03/17
    //=======================================================================================
    template<typename T_class>
    struct PairWithState
    {
    T_class         m_data;
    DbOpcode        m_opcode;

    PairWithState() { }
    PairWithState(T_class data, DbOpcode opcode) : m_data(data), m_opcode(opcode) { }

    bool IsInsertion() const    { return m_opcode == DbOpcode::Insert; }
    bool IsDeletion() const     { return m_opcode == DbOpcode::Delete; }
    bool IsModified() const     { return m_opcode == DbOpcode::Update; }

    void    GetOverrideGraphicParams(Render::OvrGraphicParamsR symbologyOverrides) const;
    };

    //=======================================================================================
    //! This will be saved statically to show the "transient" elements in the view with
    //! the correct symbology based on their opcodes, and also show the elements in the db
    //! that need to be re-colored
    // @bsistruct                                                   Diego.Pinate    03/17
    //=======================================================================================
    struct ComparisonData : RefCountedBase
    {
    bvector<PairWithState<DgnElementPtr>>   m_elements;
    bvector<PairWithState<DgnElementId>>    m_elementIds;
    };

    DEFINE_REF_COUNTED_PTR(ComparisonData)

private:
    static ComparisonDataPtr                 s_comparisonData;

public:
    //! Checks if our data contains an element ID with a symbology override
    //! @param[in] id DgnElementId of the element to check
    //! @param[out] pair PairWithState<DgnElementId> object containing the element
    //! @return true if the element is contained
    static bool     ContainsElementId(DgnElementId const& id, PairWithState<DgnElementId>*& pair);
    //! Checks if our data contains an element ptr with the supplied ID
    //! @param[in] id DgnElementId of the element to check
    //! @param[out] pair PairWithState<DgnElementPtr> object containing the element
    //! @return true if the element is contained
    static bool     ContainsElement(DgnElementId const& id, PairWithState<DgnElementPtr>*& pair);
    //! Obtains all the DgnElementPtrs in the comparison data into a vector
    //! @param[out] elements The elements
    static void     CollectTransientElements(bvector<DgnElementPtr> & elements);
    static void     CollectTransientElements(bvector<PairWithState<DgnElementPtr>> & elements);
    

    //! Clears the elements being shown, view controller should default elements in the db to Grey
    DGNPLATFORM_EXPORT static void    ClearComparisonData();
    //! Add an element to be drawn and its status/opcode that will define symbology
    DGNPLATFORM_EXPORT static void    AddElement (DgnElementPtr element, DbOpcode opcode);
    //! Adds an element ID of an object in the Db that must be drawn and its status/opcode
    DGNPLATFORM_EXPORT static void    AddElementId (DgnElementId elementId, DbOpcode opcode);

}; // RevisionComparisonElementKeeper

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

protected:
    unsigned int        m_flags;
    bool                m_visitingTransientElements;

    void _OverrideGraphicParams(Dgn::Render::OvrGraphicParamsR overide, Dgn::GeometrySourceCP source) override;
    void _VisitAllElements(ViewContextR context) override;

public:
    enum Flags
        {
        SHOW_CURRENT    = 1,
        SHOW_TARGET     = 1 << 1,
        SHOW_BOTH       = SHOW_CURRENT | SHOW_TARGET,
        };

    void    _CreateTerrain(TerrainContextR context) override;

    //! Set flags for what's shown in the comparison
    DGNPLATFORM_EXPORT void SetFlags(unsigned int flags) { m_flags = flags; }

    //! Constructors
    DGNPLATFORM_EXPORT RevisionComparisonViewController (SpatialViewDefinition const& view) : T_Super(view), m_flags((unsigned int)Flags::SHOW_BOTH), m_visitingTransientElements(false) { }
    DGNPLATFORM_EXPORT RevisionComparisonViewController (SpatialViewDefinition const& view, unsigned int flags) : T_Super(view), m_flags(flags), m_visitingTransientElements(false) { }
};

END_BENTLEY_DGN_NAMESPACE
