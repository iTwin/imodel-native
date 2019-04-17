/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include <DgnPlatform/LinkElement.h>
#include "ViewDefinition.h"

/** @addtogroup DgnMarkupProjectGroup Markups and Redlines
* A markup is a set of annotations that apply to a DgnDb or to views of that project. Markups include redlines, markups, and punch lists.
*
* @section DgnMarkupProjectGroup_Association Associating a DgnMarkupProject with a DgnDb
*
* A DgnMarkupProject is associated with a DgnDb. When you create a DgnMarkupProject, you must supply a reference to a DgnDb.
* The DgnMarkupProject stores the BeGuid and filename of the project. The project filename is stored relative to the markup
* project file. See DgnMarkupProject::GetAssociation. You can use the stored relative filename
* to find the original project. After re-opening a DgnMarkupProject, you should call the DgnMarkupProject::CheckAssociation
* function to be sure that the given DgnDb is in fact associated with the DgnMarkupProject. This check is based on the project's BeGuid.
* 
* @section DgnMarkupProjectGroup_Redlines Markups and Redlines
*
* A "markup" is an annotation applied to a DgnDb. A markup has text properties but no graphics.
* 
* A "redline" is an annotation applied to a DgnDb or view that has both text and graphics.
* Redline graphics are stored in models within a DgnMarkupProject. There are two types of models that hold redlines, RedlineModel and SpatialRedlineModel.
* A redline model must be created or opened in order to store redlines.
*
* @section DgnMarkupProjectGroup_SpatialRedlines Physical vs. non-physical redlines.
* Suppose you want to draw redlines  on top of a map. The extent of the map is so great that no single view will show it very well. 
* You want to be able to zoom in and out and pan around and draw your redlines at any location in the map. Therefore, you want the redline 
* view to show you a live view of the map, combined with a live view of the redline model. In this case, you need a "physical" redline 
* model and view. It's called "physical" because in this case the redline model itself is a (3-D) SpatialModel, and the associated 
* ViewController is derived from SpatialViewController. In the normal (non-physical) redline case, the redline view shows you a 
* static image of a view of the DgnDb or some other static image. The redline model in that case is a (2-D) SheetModel, and the 
* associated redline view is a SheetViewController.
*/

#define MARKUP_SCHEMA_NAME "Markup"
#define MARKUP_SCHEMA_PATH L"ECSchemas/Dgn/Markup.ecschema.xml"
#define MARKUP_SCHEMA(name) MARKUP_SCHEMA_NAME "." name

#define MARKUP_CLASSNAME_MarkupExternalLink         "MarkupExternalLink"
#define MARKUP_CLASSNAME_MarkupExternalLinkGroup    "MarkupExternalLinkGroup"
#define MARKUP_CLASSNAME_Redline                    "Redline"
#define MARKUP_CLASSNAME_RedlineModel               "RedlineModel"
#define MARKUP_CLASSNAME_RedlineViewDefinition      "RedlineViewDefinition"
#define MARKUP_CLASSNAME_SpatialRedlineModel        "SpatialRedlineModel"

#define MARKUP_AUTHORITY_Redline                    MARKUP_SCHEMA_NAME ":" MARKUP_CLASSNAME_Redline

DGNPLATFORM_REF_COUNTED_PTR(RedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(MarkupExternalLink)
DGNPLATFORM_TYPEDEFS(MarkupExternalLink)
DGNPLATFORM_REF_COUNTED_PTR(MarkupExternalLinkGroup)
DGNPLATFORM_TYPEDEFS(MarkupExternalLinkGroup)

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct RedlineElementHandler; struct RedlineViewDef;}

namespace dgn_ModelHandler {struct Redline;}

enum DgnMarkupProjectSchemaValues
    {
    MARKUP_DGNDB_CURRENT_VERSION_Major = 0,
    MARKUP_DGNDB_CURRENT_VERSION_Minor = 1,
    MARKUP_DGNDB_CURRENT_VERSION_Sub1  = 0,
    MARKUP_DGNDB_CURRENT_VERSION_Sub2  = 0,

    MARKUP_DGNDB_SUPPORTED_VERSION_Major = MARKUP_DGNDB_CURRENT_VERSION_Major,  // oldest version of the project schema supported by current api
    MARKUP_DGNDB_SUPPORTED_VERSION_Minor = MARKUP_DGNDB_CURRENT_VERSION_Minor,
    MARKUP_DGNDB_SUPPORTED_VERSION_Sub1  = 0,
    MARKUP_DGNDB_SUPPORTED_VERSION_Sub2  = 0,
    };

//=======================================================================================
//! The DgnDomain for the markup schema.
// @bsiclass                                            Ramanujam.Raman      04/16
//=======================================================================================
struct MarkupDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(MarkupDomain, DGNPLATFORM_EXPORT)

    private:
        WCharCP _GetSchemaRelativePath() const override { return MARKUP_SCHEMA_PATH; }
        void _OnSchemaImported(DgnDbR) const override;
    public:
        MarkupDomain();
    };

//=======================================================================================
//! Represents annotations that mark up (a picture of) something.
//! A Redline may be (but does not have to be) associated with a view of a BIM. 
//! You can create and store a LinkElement to record such an association if you wish.
// @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Redline : Document
    {
    DGNELEMENT_DECLARE_MEMBERS(MARKUP_CLASSNAME_Redline, Document);
    friend struct dgn_ElementHandler::RedlineElementHandler;

    Redline(CreateParams const& params) : T_Super(params) {}
public:
    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetClassId(MARKUP_SCHEMA_NAME, MARKUP_CLASSNAME_Redline)); }

    //! Create a Redline document element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    //! @param createStatus Optional. If not null, non-zero error status is returned in \a createStatus if creation fails
    //! @param model    The model where the Redline is listed. @see DgnMarkupProject::GetRedlineListModel
    //! @param name     The name of the redline.
    //! @return A new, non-persistent Redline element or an invalid handle if the element cannot be created.
    DGNPLATFORM_EXPORT static RedlinePtr Create(DgnDbStatus* createStatus, DocumentListModelCR model, Utf8StringCR name);

    static DgnCode CreateCode(DocumentListModelCR scope, Utf8StringCR name) {return CodeSpec::CreateCode(MARKUP_AUTHORITY_Redline, scope, name);}
    };

//=======================================================================================
//! Holds a raster image of what is being marked up, plus 2d graphics and other annotations.
//! The units of a RedlineModel are in meters. Normally, a RedlineModel will be about
//! the size (in meters) of the screen.
// @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RedlineModel : GraphicalModel2d
    {
    DGNMODEL_DECLARE_MEMBERS(MARKUP_CLASSNAME_RedlineModel, GraphicalModel2d);

    friend struct dgn_ModelHandler::Redline;

    explicit RedlineModel(CreateParams const& params): T_Super(params) {}
public:
    struct ImageDef
    {
        DgnTextureId m_textureId; //!< ID of a DgnTexture holding the image data
        DPoint2d m_origin; //!< lower-left corner of the image in meters
        DPoint2d m_size; //!< width and height of the image in meters

        ImageDef(DgnTextureId textureId, DPoint2dCR origin, DPoint2dCR size) : m_textureId(textureId), m_origin(origin), m_size(size) { }
        ImageDef() : ImageDef(DgnTextureId(), DPoint2d::FromZero(), DPoint2d::FromZero()) { }

        BE_JSON_NAME(textureId);
        BE_JSON_NAME(originX);
        BE_JSON_NAME(originY);
        BE_JSON_NAME(sizeX);
        BE_JSON_NAME(sizeY);

        void FromJson(JsonValueCR);
        Json::Value ToJson() const;

        bool IsValid() const { return m_textureId.IsValid(); }
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ImageDef);
private:
    ImageDef m_imageDef;

    DGNPLATFORM_EXPORT void _OnSaveJsonProperties() override;
    DGNPLATFORM_EXPORT void _OnLoadedJsonProperties() override;
public:
    BE_JSON_NAME(imageDef);

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetClassId(MARKUP_SCHEMA_NAME, MARKUP_CLASSNAME_RedlineModel)); }

    //! Create a RedlineModel that is to contain the graphics for the specified Redline.  @note It is the caller's responsibility to call Insert on the returned model in order to make it persistent.
    //! @param createStatus Optional. If not null, non-zero error status is returned in \a createStatus if creation fails
    //! @param[in] doc      The Redline element
    //! @return a new, non-persisetnt RedlineModel object or an invalid handle if \a doc is invalid
    DGNPLATFORM_EXPORT static RedlineModelPtr Create(DgnDbStatus* createStatus, Redline& doc); 

    //! Save an image to display in this redline model.
    //! @param texture the persistent DgnTexture element holding the image data
    //! @param origin the coordinates (in meters) of the lower left corner of the image
    //! @param size   the size of the image (in meters)
    DGNPLATFORM_EXPORT void StoreImage(DgnTextureCR texture, DPoint2dCR origin, DVec2dCR size);

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;

    ImageDefCR GetImageDef() const { return m_imageDef; }
    };

//=======================================================================================
//! Defines a view of a RedlineModel
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RedlineViewDefinition : ViewDefinition2d
    {
    DGNELEMENT_DECLARE_MEMBERS(MARKUP_CLASSNAME_RedlineViewDefinition, ViewDefinition2d);
    friend struct dgn_ElementHandler::RedlineViewDef;

    protected:
        DGNPLATFORM_EXPORT ViewControllerPtr _SupplyController() const override;

        //! Construct a RedlineViewDefinition from the supplied params prior to loading it
        explicit RedlineViewDefinition(CreateParams const& params) : T_Super(params) {}

        //! Construct a new RedlineViewDefinition prior to inserting it
        RedlineViewDefinition(DefinitionModelR model, Utf8StringCR name, DgnModelId baseModelId, CategorySelectorR categories, DisplayStyle2dR displayStyle) : 
                T_Super(model, name, QueryClassId(model.GetDgnDb()), baseModelId, categories, displayStyle) {}


        //! Look up the ECClass ID used for RedlineViewDefinitions in the specified DgnDb
        static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetClassId(MARKUP_SCHEMA_NAME, MARKUP_CLASSNAME_RedlineViewDefinition)); }

        void _SetExtents(DVec3dCR delta) override {;} // don't allow zooming - see TFS#735477

    public:
        //! Create a new redline view definition element, prior to inserting it.  @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
        //! @param createStatus Optional. If not null, non-zero error status is returned in \a createStatus if creation fails
        //! @param model     The redline model to view
        //! @param viewSize  The width and height of the view in meters.
        //! @return a new non-persistent, redline view definition element or null if the model is invalid.
        DGNPLATFORM_EXPORT static RedlineViewDefinitionPtr Create(DgnDbStatus* createStatus, RedlineModelR model, DVec2dCR viewSize);
    };

//=======================================================================================
//! Displays a RedlineViewDefinition
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct RedlineViewController : ViewController2d
{
    DEFINE_T_SUPER (ViewController2d);

#if !defined (DOCUMENTATION_GENERATOR)
    friend struct DgnMarkupProject;

protected:
    Render::MaterialPtr m_backgroundMaterial;

    Render::MaterialP LoadBackgroundMaterial(ViewContextR context);
    RedlineModel::ImageDef GetImageDef() const;

    void _DrawView(ViewContextR) override;

public:
    DGNPLATFORM_EXPORT static ViewController* Create(DgnDbStatus* openStatus, RedlineViewDefinitionR rdlViewDef);
    DGNPLATFORM_EXPORT RedlineViewController(RedlineViewDefinition const& rdlViewDef);
    DGNPLATFORM_EXPORT ~RedlineViewController();
    
    DGNPLATFORM_EXPORT void OnClose(RedlineModel& targetModel);
    DGNPLATFORM_EXPORT void OnOpen(RedlineModel& targetModel);
#endif // DOCUMENTATION_GENERATOR
};

#if !defined (DOCUMENTATION_GENERATOR)
namespace dgn_ElementHandler {
struct RedlineViewDef : ViewElementHandler::View2d
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_RedlineViewDefinition, RedlineViewDefinition, RedlineViewDef, ViewElementHandler::View2d, DGNPLATFORM_EXPORT);
    };
struct RedlineElementHandler : Document
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_Redline, Redline, RedlineElementHandler, Document, DGNPLATFORM_EXPORT);
    };
};
#endif // DOCUMENTATION_GENERATOR

//=======================================================================================
//! Displays a SpatialRedlineModel in conjunction with the display of another view controller.
//!@remarks
//! SpatialRedlineViewController is a normal Orthographic 3D view in most respects, and its SpatialRedlineModel is its target model.
//! 
//! SpatialRedlineViewController is unusual in that it also tries work in sync with another view controller.
//! This other controller is called the "subject view controller." It must be be supplied in the SpatialRedlineViewController constructor.
//! A SpatialRedlineViewController actually has no view parameters of its own. Instead, it adopts the view parameters of the subject view controller on the fly. 
//! SpatialRedlineViewController overrides _DrawView to draw its own SpatialRedlineModel. It then forwards the draw request to the subject view controller.
//! A SpatialRedlineViewController handles most viewing-related queries by applying them to itself and then forwarding them to the subject view controller, so that
//! the two view controllers are always in sync.
//! 
//! <h4>Locating and Editing</h4>
//! Locates and edits are normally directed to the SpatialRedlineModel, since that is the normal target model of a SpatialRedlineViewController.
//! If an app wants to re-direct locates and/or edits to the target of the subject view controller instead, it should SpatialRedlineViewController::SetTargetModel
//! in order to change the SpatialRedlineViewController's target model.
//!
//! @see @ref DgnMarkupProjectGroup_SpatialRedlines
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct SpatialRedlineViewController : OrthographicViewController
{
    DEFINE_T_SUPER (OrthographicViewController);

#if !defined (DOCUMENTATION_GENERATOR)
    friend struct DgnMarkupProject;
#endif

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    SpatialViewController& m_subjectView;
    bvector<SpatialRedlineModelP> m_otherRdlsInView;
    bool m_targetModelIsInSubjectView;

    void _DrawView(ViewContextR) override;
    bool _Allow3dManipulations() const override;
    // WIP_MERGE_John_Patterns - double _GetPatternZOffset (ViewContextR, ElementHandleCR) const override;
    bool _IsSnapAdjustmentRequired(bool snapLockEnabled) const override {return true;} // Always project snap to ACS plane...
    bool _IsContextRotationRequired(bool contextLockEnabled) const override {return true;} // Always orient AccuDraw to ACS plane...

    //  Override and forward the methods that trigger a query.
    void _OnCategoryChange(bool singleEnabled) override;
    void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;
    void _SetViewedModels(DgnModelIdSet const&) override;

    void SynchWithSubjectViewController();
#endif // DOCUMENTATION_GENERATOR

public:
    //! Create a SpatialRedlineViewController
    //! @param model    The SpatialRedlineModel to view
    //! @param subjectView The view of the underlying physical coordinate space to overlay
    //! @param physicalRedlineViewDef The redline view definition
    //! @param otherRdlsToView Optional. Other SpatialRedlineModels to show in the view.
    DGNPLATFORM_EXPORT SpatialRedlineViewController (SpatialRedlineModel& model, SpatialViewController& subjectView, OrthographicViewDefinition& physicalRedlineViewDef, bvector<SpatialRedlineModelP> const& otherRdlsToView);

    DGNPLATFORM_EXPORT ~SpatialRedlineViewController();

    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] model the physical redline model to display
    //! @param[in] subjectView the subject view to display underneath the physical redline model
    DGNPLATFORM_EXPORT static SpatialRedlineViewControllerPtr InsertView(SpatialRedlineModel& model, OrthographicViewController& subjectView);
};

//=======================================================================================
//! Holds "redline" graphics and other annotations for a physical view of a subject DgnDb. 
//! This type of redline model is displayed simultaneously with view of the subject project by SpatialRedlineViewController.
//! The subject view is live, not a static image.
//! A SpatialRedlineModel has the same units and coordinate system as the target model of the view of the subject project.
//! Note that the DgnMarkupProject that holds a SpatialRedlineModel must have the same StorageUnits as the subject project. See CreateDgnMarkupProjectParams.
//! @see @ref DgnMarkupProjectGroup_SpatialRedlines
// @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct SpatialRedlineModel : SpatialModel
    {
    DGNMODEL_DECLARE_MEMBERS("SpatialRedlineModel", SpatialModel);
private:

    friend struct DgnMarkupProject;

protected:
    static SpatialRedlineModelPtr Create(DgnMarkupProjectR markupProject, Utf8CP name, SpatialModelCR subjectViewTargetModel);

public:
    explicit SpatialRedlineModel(CreateParams const& params) : T_Super(params) {}

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;
    };

//=======================================================================================
//! Supplies the parameters necessary to create new DgnMarkupProjects.
//! @ingroup DgnMarkupProjectGroup
// @bsiclass
//=======================================================================================
struct CreateDgnMarkupProjectParams : CreateDgnDbParams
{
private:
    DgnDbR          m_dgnDb;
    bool            m_overwriteExisting;
    bool            m_spatialRedlining;

public:
    //! ctor
    //! @param[in] dgnProject   The DgnDb which is the target of this markup.
    //! @param[in] guid         The BeProjectGuid to store in the newly created DgnDb. If not supplied, a new BeSQLite::BeGuid value is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateDgnMarkupProjectParams(DgnDbR dgnProject, BeSQLite::BeGuid guid=BeSQLite::BeGuid(true)) : CreateDgnDbParams(guid), m_dgnDb(dgnProject), m_overwriteExisting(false) {;}

    //! Get the subject DgnDb
    DgnDbR GetSubjectDgnProject() const {return m_dgnDb;}

    //! Specify whether to overwrite an existing file or not. The default is to fail if a file by the supplied name already exists.
    void SetOverwriteExisting(bool val) {m_overwriteExisting = val;}
    bool GetOverwriteExisting() const {return m_overwriteExisting;}

    //! Specify if this markup project is to contain physical redline models. @see @ref DgnMarkupProjectGroup_SpatialRedlines
    void SetSpatialRedlining(bool val) {m_spatialRedlining = val;}
    bool GetSpatialRedlining() const {return m_spatialRedlining;}
};

//=======================================================================================
//! A DgnMarkupProject project is a type of DgnDb that holds markups of various kinds, including redlines and punch lists. 
//! @ingroup DgnMarkupProjectGroup
// @bsiclass
//=======================================================================================
struct DgnMarkupProject : DgnDb
{
private:
    DECLARE_KEY_METHOD

    DEFINE_T_SUPER(DgnDb)

private:
    DgnMarkupProject() {}
    virtual ~DgnMarkupProject() {}
    BeSQLite::DbResult InitializeNewMarkupProject(BeFileNameCR fileName, CreateDgnMarkupProjectParams const& params);

public:
    BentleyStatus CheckIsOpen();

public:
    //! Compute the default filename for a DgnMarkupProject, based on the associated DgnDb
    //! @param[out] markupProjectName   The computed name of the DgnMarkupProject
    //! @param[in]  dgnProjectName      The name of the DgnDb
    DGNPLATFORM_EXPORT static void ComputeDgnProjectFileName(BeFileNameR markupProjectName, BeFileNameCR dgnProjectName);

    //! Open and existing DgnMarkupProject file.
    //! @param[out] status BE_SQLITE_OK if the DgnMarkupProject file was successfully opened or a non-zero error code if the project could not be opened or is not a markup project. May be NULL. 
    //! @param[in] filename The name of the file from which the DgnMarkupProject is to be opened. Must be a valid filename.
    //! @param[in] openParams Parameters for opening the database file
    //! will never access any element data, you can skip that step. However, generally it is preferable to load the DgnFile when opening the project.
    //! @return a reference counted pointer to the opened DgnMarkupProject. Its IsValid() method will be false if the open failed for any reason.
    DGNPLATFORM_EXPORT static DgnMarkupProjectPtr OpenDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, OpenParams const& openParams);

    //! Create and open a new DgnMarkupProject file.
    //! @param[out] status \a BE_SQLITE_OK if the DgnMarkupProject file was successfully created or a non-zero error code if the project could not be created. May be NULL. 
    //! @param[in] filename The name of the file for the new DgnMarkupProject. The directory must be writable.
    //! @param[in] params Parameters that control aspects of the newly created DgnMarkupProject
    //! @return a reference counted pointer to the newly created DgnMarkupProject. Its IsValid() method will be false if the open failed for any reason.
    DGNPLATFORM_EXPORT static DgnMarkupProjectPtr CreateDgnDb(BeSQLite::DbResult* status, BeFileNameCR filename, CreateDgnMarkupProjectParams const& params);

    //! Get the model where Redline elements are normally stored
    DGNPLATFORM_EXPORT DocumentListModelPtr GetRedlineListModel();

    //! Query if this project has been initialized for physical redlining.
    //! @see CreateDgnMarkupProjectParams::SetSpatialRedlining
    DGNPLATFORM_EXPORT bool IsSpatialRedlineProject() const;

};

namespace dgn_ElementHandler { struct MarkupExternalLinkHandler; struct MarkupExternalLinkGroupHandler; }

//=======================================================================================
//! Captures a link to an element in the external DgnDb referenced from the markup DgnDb
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE MarkupExternalLink : LinkElement, ILinkElementBase<MarkupExternalLink>
{
    DGNELEMENT_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLink, LinkElement)
    friend struct dgn_ElementHandler::MarkupExternalLinkHandler;

public:
    //! Parameters used to construct a MarkupExternalLink
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(MarkupExternalLink::T_Super::CreateParams);

    DgnElementId m_linkedElementId;

    explicit CreateParams(Dgn::DgnElement::CreateParams const& params, DgnElementId linkedElementId = DgnElementId()) : T_Super(params), m_linkedElementId(linkedElementId) {}

    //! Constructor
    //! @param[in] linkModel Model that should contain the link
    //! @param[in] linkedElementId Id of the linked element in the external file
    DGNPLATFORM_EXPORT explicit CreateParams(LinkModelR linkModel, DgnElementId linkedElementId = DgnElementId());
    };

private:
    DgnElementId m_linkedElementId;

protected:
    DGNPLATFORM_EXPORT void _CopyFrom(Dgn::DgnElementCR source) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;

public:
    BE_JSON_NAME(linkedElementId)

    //! Constructor
    explicit MarkupExternalLink(CreateParams const& params) : T_Super(params), m_linkedElementId(params.m_linkedElementId) {}

    //! Create a MarkupExternalLink
    static MarkupExternalLinkPtr Create(CreateParams const& params) { return new MarkupExternalLink(params); }

    //! Insert the MarkupExternalLink in the DgnDb
    DGNPLATFORM_EXPORT MarkupExternalLinkCPtr Insert();

    //! Update the persistent state of the MarkupExternalLink in the DgnDb from this modified copy of it. 
    DGNPLATFORM_EXPORT MarkupExternalLinkCPtr Update();

    //! Set the linked element id
    void SetLinkedElementId(DgnElementId linkedElementId) { m_linkedElementId = linkedElementId; }

    //! Get the linked element id
    DgnElementId GetLinkedElementId() const { return m_linkedElementId; }

    //! Get the schema name for the MarkupExternalLink class
    //! @note This is a static method that always returns the schema name of the MarkupExternalLink class - it does @em not return the schema of a specific instance.
    static Utf8CP MyECSchemaName() { return MARKUP_SCHEMA_NAME; }

};

//=======================================================================================
// Group of markup external links for assignment of activities to multiple reference elements
//=======================================================================================
struct MarkupExternalLinkGroup : LinkElement, ILinkElementBase<MarkupExternalLinkGroup>, IElementGroupOf<MarkupExternalLink>
{
    DGNELEMENT_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLinkGroup, LinkElement)
    friend struct dgn_ElementHandler::MarkupExternalLinkGroupHandler;

public:
    //! Parameters used to construct a MarkupExternalLinkGroup
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(MarkupExternalLinkGroup::T_Super::CreateParams);

        explicit CreateParams(Dgn::DgnElement::CreateParams const& params) : T_Super(params) {}

        //! Constructor
        //! @param[in] linkModel Model that should contain the link group
        DGNPLATFORM_EXPORT explicit CreateParams(LinkModelR linkModel);
        };

private:
    DgnElementCP _ToGroupElement() const override { return this; }
    Dgn::IElementGroupCP _ToIElementGroup() const override { return this; }

public:
    explicit MarkupExternalLinkGroup(CreateParams const& params) : T_Super(params) {}

    static MarkupExternalLinkGroupPtr Create(CreateParams const& params) { return new MarkupExternalLinkGroup(params); }

    //! Insert the MarkupExternalLinkGroup in the DgnDb
    DGNPLATFORM_EXPORT MarkupExternalLinkGroupCPtr Insert();

    //! Update the persistent state of the MarkupExternalLinkGroup in the DgnDb from this modified copy of it. 
    DGNPLATFORM_EXPORT MarkupExternalLinkGroupCPtr Update();

    //! Get the schema name for the MarkupExternalLinkGroup class
    //! @note This is a static method that always returns the schema name of the MarkupExternalLinkGroup class - it does @em not return the schema of a specific instance.
    static Utf8CP MyECSchemaName() { return MARKUP_SCHEMA_NAME; }
};


namespace dgn_ElementHandler
{
//! The handler for MarkupExternalLink elements
struct EXPORT_VTABLE_ATTRIBUTE MarkupExternalLinkHandler : InformationContent
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLink, MarkupExternalLink, MarkupExternalLinkHandler, InformationContent, DGNPLATFORM_EXPORT)
    DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
};


//! The handler for MarkupExternalLinkGroup elements
struct EXPORT_VTABLE_ATTRIBUTE MarkupExternalLinkGroupHandler : InformationContent
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLinkGroup, MarkupExternalLinkGroup, MarkupExternalLinkGroupHandler, InformationContent, DGNPLATFORM_EXPORT)
};
}

namespace dgn_ModelHandler
{
//! The ModelHandler for RedlineModel.
struct Redline : Geometric2d
{
    MODELHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_RedlineModel, RedlineModel, Redline, Geometric2d, )
};

//! The ModelHandler for SpatialRedlineModel.
struct SpatialRedline : Spatial
{
    MODELHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_SpatialRedlineModel, SpatialRedlineModel, SpatialRedline, Spatial, )
};

}

END_BENTLEY_DGN_NAMESPACE
