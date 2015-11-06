/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnView.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

#define DGN_CLASSNAME_ViewDefinition "ViewDefinition"
#define DGN_CLASSNAME_CameraViewDefinition "CameraViewDefinition"
#define DGN_CLASSNAME_DrawingViewDefinition "DrawingViewDefinition"
#define DGN_CLASSNAME_SheetViewDefinition "SheetViewDefinition"
#define DGN_CLASSNAME_RedlineViewDefinition "RedlineViewDefinition"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Holds the definition of a view.
//! @ingroup DgnViewGroup
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewDefinition : DictionaryElement
{
    DEFINE_T_SUPER(DictionaryElement);
public:
    //! Holds the data which describes a view definition
    struct Data
    {
    public:
        DgnModelId      m_baseModelId;
        Utf8String      m_descr;
        DgnViewSource   m_source;

        explicit Data(DgnModelId baseModelId=DgnModelId(), DgnViewSource source=DgnViewSource::User, Utf8StringCR descr="")
            {
            Init(baseModelId, source, descr);
            }

        void Init(DgnModelId baseModelId=DgnModelId(), DgnViewSource source=DgnViewSource::User, Utf8StringCR descr="")
            {
            m_baseModelId = baseModelId;
            m_descr = descr;
            m_source = source;
            }

        uint32_t GetMemSize() const { return static_cast<uint32_t>(sizeof(*this) + m_descr.length()); }
    };

    //! Parameters used to construct a ViewDefinition
    struct CreateParams : T_Super::CreateParams
    {
    protected:
        CreateParams(DgnDbR db, DgnClassId classId, Code const& code, Data const& data, DgnElementId id=DgnElementId(), DgnElementId parentId=DgnElementId())
            : T_Super(db, DgnModel::DictionaryId(), classId, code, id, parentId), m_data(data) { }
    public:
        DEFINE_T_SUPER(ViewDefinition::T_Super::CreateParams);

        Data m_data;

        explicit CreateParams(DgnElement::CreateParams const& params, Data const& data=Data()) : T_Super(params), m_data(data) { }

        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Code const& code, DgnClassId classId, Data const& data);
    };
private:
    Data m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    explicit ViewDefinition(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;

    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + m_data.GetMemSize(); }
    virtual Code _GenerateDefaultCode() override { return Code(); }

    virtual ViewControllerPtr _SupplyController() const = 0;

    // ###TODO:
    //  - Is base model valid for this view?
    //  - Can I add a given model to the viewed models list?
    //  - Why is viewed models list in the settings table rather than a property of the view?
public:
    WIPViewId GetViewId() const { return WIPViewId(GetElementId().GetValue()); } //!< This view definition's ID
    Utf8StringCR GetDescr() const { return m_data.m_descr; } //!< This view definition's description
    DgnViewSource GetSource() const { return m_data.m_source; } //!< This view definition's source
    DgnModelId GetBaseModelId() const { return m_data.m_baseModelId; } //!< This view definition's base model ID

    void SetDescr(Utf8StringCR descr) { m_data.m_descr = descr; } //!< Change this view definition's description
    void SetSource(DgnViewSource source) { m_data.m_source = source; } //!< Change this view definition's source
    void SetBaseModelId(DgnModelId modelId) { m_data.m_baseModelId = modelId; } //!< Change the base model ID

    ViewDefinitionCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<ViewDefinition>(*this, status); }
    ViewDefinitionCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<ViewDefinition>(*this, status); }

    enum class FillModels{No=0, Yes=1};
    DGNPLATFORM_EXPORT ViewControllerPtr LoadViewController(FillModels fillModels=FillModels::No) const;
    DGNPLATFORM_EXPORT static ViewControllerPtr LoadViewController(WIPViewId viewId, DgnDbR db, FillModels fillModels=FillModels::No);

    DGNPLATFORM_EXPORT static Code CreateCode(Utf8StringCR name);

    DGNPLATFORM_EXPORT static WIPViewId QueryViewId(Code const& code, DgnDbR db);

    static ViewDefinitionCPtr QueryView(WIPViewId viewId, DgnDbR db) { return db.Elements().Get<ViewDefinition>(viewId); }

    static void AddClassParams(ECSqlClassParams& params); //!< @private
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalViewDefinition : ViewDefinition
{
    DEFINE_T_SUPER(ViewDefinition);
protected:
    explicit PhysicalViewDefinition(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraViewDefinition : PhysicalViewDefinition
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_CameraViewDefinition, PhysicalViewDefinition);
protected:
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
public:
    explicit CameraViewDefinition(CreateParams const& params) : T_Super(params) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_CameraViewDefinition)); }
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewDefinition2d : ViewDefinition
{
    DEFINE_T_SUPER(ViewDefinition);
protected:
    explicit ViewDefinition2d(CreateParams const& params) : T_Super(params) { }
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingViewDefinition : ViewDefinition2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingViewDefinition, ViewDefinition2d);
protected:
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
public:
    explicit DrawingViewDefinition(CreateParams const& params) : T_Super(params) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_DrawingViewDefinition)); }
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SheetViewDefinition : ViewDefinition2d
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_SheetViewDefinition, ViewDefinition2d);
protected:
    DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;
public:
    explicit SheetViewDefinition(CreateParams const& params) : T_Super(params) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SheetViewDefinition)); }
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RedlineViewDefinition : SheetViewDefinition
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_RedlineViewDefinition, SheetViewDefinition);
public:
    explicit RedlineViewDefinition(CreateParams const& params) : T_Super(params) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_RedlineViewDefinition)); }
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for CameraViewDefinition elements
    // @bsiclass                                                      Paul.Connelly   10/15
    //=======================================================================================
    struct CameraViewDef : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_CameraViewDefinition, CameraViewDefinition, CameraViewDef, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };

    //=======================================================================================
    //! The handler for DrawingViewDefinition elements
    // @bsiclass                                                      Paul.Connelly   10/15
    //=======================================================================================
    struct DrawingViewDef : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingViewDefinition, DrawingViewDefinition, DrawingViewDef, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };

    //=======================================================================================
    //! The handler for SheetViewDefinition elements
    // @bsiclass                                                      Paul.Connelly   10/15
    //=======================================================================================
    struct SheetViewDef : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_SheetViewDefinition, SheetViewDefinition, SheetViewDef, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };

    //=======================================================================================
    //! The handler for RedlineViewDefinition elements
    // @bsiclass                                                      Paul.Connelly   10/15
    //=======================================================================================
    struct RedlineViewDef : SheetViewDef
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_RedlineViewDefinition, RedlineViewDefinition, RedlineViewDef, SheetViewDef, DGNPLATFORM_EXPORT);
    };
};

//=======================================================================================
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewControllerOverride : DgnDomain::Handler::Extension
{
    HANDLER_EXTENSION_DECLARE_MEMBERS(ViewControllerOverride, DGNPLATFORM_EXPORT);
public:
    //! @param[in] db The DgnDb for the view
    //! @param[in] view The ViewDefinition
    //! @return an instance of a ViewController for the supplied ViewDefinition, or nullptr if the ViewDefinition is not of interest.
    virtual ViewControllerPtr _SupplyController(ViewDefinitionCR view) = 0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

