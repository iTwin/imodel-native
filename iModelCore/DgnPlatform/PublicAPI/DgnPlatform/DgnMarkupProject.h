/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnMarkupProject.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include <DgnPlatform/LinkElement.h>
#include <DgnPlatform/QueryView.h>

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
#define MARKUP_SCHEMA_PATH L"ECSchemas/Dgn/Markup.01.00.ecschema.xml"
#define MARKUP_SCHEMA(name) MARKUP_SCHEMA_NAME "." name

#define MARKUP_CLASSNAME_MarkupExternalLink         "MarkupExternalLink"
#define MARKUP_CLASSNAME_MarkupExternalLinkGroup    "MarkupExternalLinkGroup"
#define MARKUP_CLASSNAME_RedlineModel               "RedlineModel"
#define MARKUP_CLASSNAME_SpatialRedlineModel        "SpatialRedlineModel"

DGNPLATFORM_REF_COUNTED_PTR(RedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(SpatialRedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(MarkupExternalLink)
DGNPLATFORM_TYPEDEFS(MarkupExternalLink)
DGNPLATFORM_REF_COUNTED_PTR(MarkupExternalLinkGroup)
DGNPLATFORM_TYPEDEFS(MarkupExternalLinkGroup)

BEGIN_BENTLEY_DGN_NAMESPACE

struct RedlineModelHandler;
struct SpatialRedlineModelHandler;

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
    public:
        MarkupDomain();
    };

//=======================================================================================
//! Information about a DgnDb that can be used to save and check on markup-project association. See DgnMarkupProject::GetAssociation
//=======================================================================================
struct DgnProjectAssociationData
    {
private:
    Utf8String  m_relativeFileName; //! The relative path from the DgnDb to the DgnMarkupProject
    BeSQLite::BeGuid m_guid;             //! The GUID of the DgnDb.
    uint64_t    m_lastModifiedTime; //! The last-modification time of the DgnDb

public:
    DGNPLATFORM_EXPORT DgnProjectAssociationData();             //!< Construct an empty object. See DgnMarkupProject::GetAssociation
    DGNPLATFORM_EXPORT Utf8String GetRelativeFileName() const;  //!< Get the relative path of the DgnDb that is associated with the DgnMarkupProject
    DGNPLATFORM_EXPORT BeSQLite::BeGuid GetGuid() const;                  //!< Get the GUID of the DgnDb that is associated with the DgnMarkupProject
    DGNPLATFORM_EXPORT uint64_t GetLastModifiedTime() const;      //!< Get the last modified time of the DgnDb as of the time that it was associated with the DgnMarkupProject
//__PUBLISH_SECTION_END__
    void FromDgnProject(DgnDbCR, DgnMarkupProject const&);
    void ToPropertiesJson(JsonValueR) const;
    BentleyStatus FromPropertiesJson(JsonValueCR);
//__PUBLISH_SECTION_START__

    //! Reasons why markups may no longer correspond to the contents of the subject DgnDb
    struct CheckResults
        {
        bool NameChanged;       //!< The name of the DgnDb has changed. This is not necessarily a problem.
        bool GuidChanged;       //!< The DgnDb has been republished. It is not clear if the content and geometry of stored markups apply to the new version.
        bool ContentsChanged;   //!< The contents of the DgnDb has changed since the markup project was created. This is not necessarily a problem.
        bool UnitsChanged;      //!< The storage units of the DgnDb have changed. This probably means that physical redlines are now invalid.
        };
    };

//=======================================================================================
//! Information about a view in a DgnDb that can be used to save and check on redline-view association. See RedlineModel::GetAssociation
//=======================================================================================
struct DgnViewAssociationData : DgnProjectAssociationData
    {
    DEFINE_T_SUPER(DgnProjectAssociationData);
    
    private:
    DgnViewId     m_viewId;
    Json::Value   m_viewGeometry;

    public:
    DGNPLATFORM_EXPORT DgnViewAssociationData();

    DGNPLATFORM_EXPORT DgnViewId GetViewId() const; //!< Get the ViewId of the view in the DgnDb that is associated with the RedlineModel

    //! Re-create from serialized JSON string.
    //! @param[in] serializedData   Serialized JSON string.
    //! @return non-zero error status if the serialized data is invalid.
    DGNPLATFORM_EXPORT BentleyStatus FromSerializedJson(Utf8CP serializedData);

//__PUBLISH_SECTION_END__
    BentleyStatus FromPropertiesJson(JsonValueCR);
    void FromDgnProject(DgnDbCR, ViewControllerCR, DgnMarkupProject const&); // hide superclass version of this function
    void ToPropertiesJson(JsonValueR) const;
    //! Get the origin of the view in the DgnDb that is associated with the RedlineModel
    void GetViewOrigin(DPoint3dR origin, DgnDbCR);
//__PUBLISH_SECTION_START__

    //! Reasons why markups may no longer correspond to the contents of the subject DgnDb
    struct CheckResults : T_Super::CheckResults
        {
        bool ViewNotFound; //!< The view in the subject DgnDb cannot be found. This redline is probably no longer applicable.

        CheckResults(DgnProjectAssociationData::CheckResults const& projResults) {memcpy(this, &projResults, sizeof(projResults)); ViewNotFound = false;}
        };
    };

//=======================================================================================
//! Holds "redline" graphics and other annotations, plus a raster image.
//! 
//! A RedlineModel stores an image which is displayed as a backdrop to the redline graphics. 
//! See RedlineModel::StoreImageData.
//! 
//! A RedlineModel can be associated with a particular view of the subject DgnDb.
//! See RedlineModel::SetAssociation and RedlineModel::GetAssociation.
//! 
//! A RedlineModel does not have to be associated with a view of a DgnDb. The stored image can be acquired from some external source.
//! 
//! See DgnMarkupProject::CreateRedlineModel, DgnMarkupProject::OpenRedlineModel, and DgnMarkupProject::FindClosestRedlineModel.
// @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RedlineModel : SheetModel
    {
    DGNMODEL_DECLARE_MEMBERS("RedlineModel", SheetModel);

public:
    //! Describes the format, size, and display location of the static image to be displayed as the background of a redline model.
    struct ImageDef
        {
        int                 m_format;       //!< QV_BGRA_FORMAT or QV_RGBA_FORMAT
        Point2d             m_sizeInPixels; //!< How many pixels per row (x) and per column (y) there are in the image.
        DPoint2d            m_origin;       //!< where to display the image on the sheet (sheet coordinates)
        DVec2d              m_size;         //!< how big the image is on the sheet (sheet coordinates)
        bool                m_topDown;      //!< Is the image top-down? Else, bottom-up.

        DGNPLATFORM_EXPORT ImageDef();
        void ToPropertiesJson(JsonValueR) const;
        BentleyStatus FromPropertiesJson(JsonValueCR);
        size_t GetPitch() const;            //!< Get the "pitch" or number of bytes per row. This is just m_size.x * GetSizeofPixelInBytes().
        size_t GetSizeInBytes() const;      //!< Get the total size of the image in bytes. This is just m_size.x*m_size.y*GetSizeofPixelInBytes().
        size_t GetSizeofPixelInBytes() const; //!< Get the number of bytes per pixel. That will be 4 if the image has alpha data or 3 if not.
        bool HasAlpha() const;              //!< Check if the image has alpha data
        bool GetIsTopDown() const;          //!< Is the image top-down? Else, bottom-up.
        Render::Image::Format GetRenderImageFormat() const; //!< Get the format of the image
        };

private:
    ImageDef            m_imageDef;
    Render::GraphicBuilderPtr  m_tileGraphic;
    DgnViewAssociationData m_assoc;

    friend struct DgnMarkupProject;
    friend struct RedlineModelHandler;
    friend struct RedlineViewController;

protected:
    void _WriteJsonProperties(Json::Value&) const override;
    void _ReadJsonProperties(Json::Value const&) override;

    static RedlineModelPtr Create(DgnMarkupProjectR markupProject, Utf8CP name, DgnModelId templateModel);

public:
    explicit RedlineModel(CreateParams const& params): T_Super(params) {}
    Render::GraphicBuilderPtr GetImageGraphic(ViewContextR);
    BentleyStatus LoadImageData(ImageDef& def, bvector<uint8_t>& imageData);
    DGNPLATFORM_EXPORT static BentleyStatus LoadImageData(ImageDef& def, bvector<uint8_t>& imageData, DgnDbCR, DgnModelId);
    
    DgnViewId GetFirstView();

public:

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;

    //! Save an image as the backdrop for this redline model.
    //! @param imageData       the image data
    //! @param isTopDown        If true, the RGB image in imageData is assumed to start at the upper left. Else, it is assumed to start from the lower left and go up.
    //! @param fitToX           If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!                         If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    //! @param compressImageProperty If true, the image data is compressed before being stored in the database. 
    DGNPLATFORM_EXPORT void StoreImageData(Render::ImageCR imageData, bool isTopDown, bool fitToX, bool compressImageProperty=true);

    //! Save an image as the backdrop for this redline model.
    //! @param source the image source 
    //! @param fitToX If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!               If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    DGNPLATFORM_EXPORT void StoreImageData(Render::ImageSourceCR source, bool fitToX);

/** @name Association to DgnDb */
/** @{ */
    //! Create or update an association between this redline model and the specified view in the specified DgnDb.
    //! @param dgnProject   The DgnDb that is being redlined
    //! @param projectView  The view in the DgnDb that is being redlined
    //! @see GetAssociation
    DGNPLATFORM_EXPORT void SetAssociation(DgnDbR dgnProject, ViewControllerCR projectView);

    //! Check that this markup project was associated with the specified DgnDb as of the last call to SetAssociation.
    //! @param assocData    The association data passed to SetAssociation
    //! @see SetAssociation
    DGNPLATFORM_EXPORT void GetAssociation(DgnViewAssociationData& assocData) const;

    //! Check that this redline model is still valid for the target view.
    //! @param subjectProject   The project that was redlined.
    //! @return A list of what's changed in the project since this markup project was created.
    //! @see DgnMarkupProjectGroup_Association
    DGNPLATFORM_EXPORT DgnViewAssociationData::CheckResults CheckAssociation(DgnDbR subjectProject);
/** @} */
    };

//=======================================================================================
//! Displays a RedlineModel
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct RedlineViewController : SheetViewController
{
    DEFINE_T_SUPER (SheetViewController);

#if !defined (DOCUMENTATION_GENERATOR)
    friend struct DgnMarkupProject;

private:
    bool m_enableViewManipulation;
    bool m_drawBorder;

protected:
    virtual void _DrawView(ViewContextR) override;
    virtual void _SetOrigin(DPoint3dCR viewOrg) override;
    virtual void _SetDelta(DVec3dCR viewDelta) override;
    virtual void _SetRotation(RotMatrixCR viewRot) override;

public:
    DGNPLATFORM_EXPORT static ViewController* Create(DgnDbStatus* openStatus, DgnDbR project, DgnViewId id);
    DGNPLATFORM_EXPORT RedlineViewController(RedlineModel&, DgnViewId id = DgnViewId());
    DGNPLATFORM_EXPORT ~RedlineViewController();
    
    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] rdlModel the redline model to display
    //! @param[in] templateView Identifies redline template view.
    //! @param[out] insertStatus  Optional. Set to non-zero status if insert fails. DgnDbStatus::NotOpen if the markup project is not open, 
    DGNPLATFORM_EXPORT static RedlineViewControllerPtr InsertView(DgnDbStatus* insertStatus, RedlineModelR rdlModel, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);
    DGNPLATFORM_EXPORT void OnClose(RedlineModel& targetModel);
    DGNPLATFORM_EXPORT void OnOpen(RedlineModel& targetModel);

    //! Query if sheet border should be displayed
    DGNPLATFORM_EXPORT bool GetDrawBorder() const;

    //! Specify if sheet border should be display
    //! @param b    if true, display the border
    DGNPLATFORM_EXPORT void SetDrawBorder(bool b);
#endif // DOCUMENTATION_GENERATOR
};

//=======================================================================================
//! Displays a SpatialRedlineModel in conjunction with the display of another view controller.
//!@remarks
//! SpatialRedlineViewController is a normal physical view in most respects, and its SpatialRedlineModel is its target model.
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
struct SpatialRedlineViewController : SpatialViewController
{
    DEFINE_T_SUPER (SpatialViewController);

#if !defined (DOCUMENTATION_GENERATOR)
    friend struct DgnMarkupProject;
#endif

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    SpatialViewController& m_subjectView;
    bvector<SpatialRedlineModelP> m_otherRdlsInView;

    bool                    m_targetModelIsInSubjectView;

    virtual void _SaveToSettings(JsonValueR) const override;
    virtual void _RestoreFromSettings(JsonValueCR) override;
    virtual void _DrawView(ViewContextR) override;
    virtual DPoint3d _GetOrigin() const override;
    virtual DVec3d _GetDelta() const override;
    virtual RotMatrix _GetRotation() const override;
    virtual void _SetOrigin(DPoint3dCR org) override;
    virtual void _SetDelta(DVec3dCR delta) override;
    virtual void _SetRotation(RotMatrixCR rot) override;
    virtual GeometricModelP _GetTargetModel() const override;
    virtual void _AdjustAspectRatio(double , bool expandView) override;
    virtual DPoint3d _GetTargetPoint() const override;
    virtual bool _Allow3dManipulations() const override;
    // WIP_MERGE_John_Patterns - virtual double _GetPatternZOffset (ViewContextR, ElementHandleCR) const override;
    virtual AxisAlignedBox3d _GetViewedExtents() const override;
    virtual ColorDef _GetBackgroundColor() const override;
    virtual bool _IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const override {return true;} // Always project snap to ACS plane...
    virtual bool _IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const override {return true;} // Always orient AccuDraw to ACS plane...
    virtual void _OnViewOpened(DgnViewportR vp) override;

    //  Override and forward the methods that trigger a query.
    virtual void _OnCategoryChange(bool singleEnabled) override;
    virtual void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;

    //virtual ScanRange _ShowTxnSummary(TxnSummaryCR summary) override; -- we don't need to override this, because the subject view will never have changed elements that must be displayed
    virtual void _OnAttachedToViewport(DgnViewportR) override;
    virtual FitComplete _ComputeFitRange(FitContextR) override;

#ifdef WIP_SpatialRedlineViewController
    // QueryView
    virtual bool _IsInSet (int nVal, BeSQLite::DbValue const*) const override;
    virtual bool _WantElementLoadStart (ViewportR viewport, double currentTime, double lastQueryTime, uint32_t maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum) override;
    virtual Utf8String _GetRTreeMatchSql (ViewportR viewport) override;
    virtual int32_t _GetMaxElementFactor() override;
    virtual double _GetMinimumSizePixels (DrawPurpose updateType) override;
    virtual uint64_t _GetMaxElementMemory () override;
    // END QueryView
#endif
    // END QueryView

    void SynchWithSubjectViewController();
#endif // DOCUMENTATION_GENERATOR

public:
    //! Create a SpatialRedlineViewController
    //! @param model    The SpatialRedlineModel to view
    //! @param subjectView The view of the underlying physical coordinate space to overlay
    //! @param physicalRedlineViewId The view id
    //! @param otherRdlsToView Optional. Other SpatialRedlineModels to show in the view.
    DGNPLATFORM_EXPORT SpatialRedlineViewController (SpatialRedlineModel& model, SpatialViewController& subjectView, DgnViewId physicalRedlineViewId, bvector<SpatialRedlineModelP> const& otherRdlsToView);

    DGNPLATFORM_EXPORT ~SpatialRedlineViewController();

    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] model the physical redline model to display
    //! @param[in] subjectView the subject view to display underneath the physical redline model
    DGNPLATFORM_EXPORT static SpatialRedlineViewControllerPtr InsertView(SpatialRedlineModel& model, SpatialViewController& subjectView);
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
    friend struct SpatialRedlineModelHandler;

protected:
    static SpatialRedlineModelPtr Create(DgnMarkupProjectR markupProject, Utf8CP name, SpatialModelCR subjectViewTargetModel);

public:
    explicit SpatialRedlineModel(CreateParams const& params) : T_Super(params) {}

    DgnViewId GetFirstView();

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
    //! @param[in] guid         The BeProjectGuid to store in the newly created DgnDb. If invalid (the default), a new BeProjectGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateDgnMarkupProjectParams(DgnDbR dgnProject, BeSQLite::BeGuid guid=BeSQLite::BeGuid()) : CreateDgnDbParams(guid), m_dgnDb(dgnProject), m_overwriteExisting(false) {;}

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
    BeSQLite::DbResult ConvertToMarkupProject(BeFileNameCR fileName, CreateDgnDbParams const& params);

    BentleyStatus QueryPropertyAsJson(JsonValueR json, DgnMarkupProjectProperty::ProjectProperty const& propSpec, uint64_t id=0) const;
    void SavePropertyFromJson(DgnMarkupProjectProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id = 0);
    DgnDbStatus ImportMarkupSchema();

public:
    BentleyStatus CheckIsOpen();

    DgnViewId GetFirstViewOf(DgnModelId);

    BentleyStatus QueryPropertyAsJson(JsonValueR json, RedlineModelProperty::ProjectProperty const&, uint64_t id=0) const;
    void SavePropertyFromJson(RedlineModelProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id=0) const;

    BentleyStatus QueryPropertyAsJson(JsonValueR json, DgnModelCR, RedlineModelProperty::ProjectProperty const& propSpec, uint64_t id=0) const;
    void SavePropertyFromJson(DgnModelCR, RedlineModelProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id=0);

    DgnProjectAssociationData::CheckResults CheckAssociation(DgnDbR subjectProject, DgnProjectAssociationData const&);
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

    //! Query if this project has been initialized for physical redlining.
    //! @see CreateDgnMarkupProjectParams::SetSpatialRedlining
    DGNPLATFORM_EXPORT bool IsSpatialRedlineProject() const;

/** @name Association to DgnDb */
/** @{ */
    //! Create or update an association between this markup project and the specified DgnDb.
    //! @see GetAssociation
    DGNPLATFORM_EXPORT void SetAssociation(DgnDbR dgnProject);

    //! Check that this markup project is associated with the specified DgnDb
    //! @param assocData    The association data passed to SetAssociation
    //! @see SetAssociation
    DGNPLATFORM_EXPORT void GetAssociation(DgnProjectAssociationData& assocData) const;

    //! Check that this markup project is still valid for the subject project
    //! @param subjectProject   The project that is being redlined.
    //! @return A list of what's changed in the project since this markup project was created.
    //! @see DgnMarkupProjectGroup_Association
    DGNPLATFORM_EXPORT DgnProjectAssociationData::CheckResults CheckAssociation(DgnDbR subjectProject);
/** @} */

/** @name Redline Models */
/** @{ */
    //! Create a redline model of that model.
    //! @param name             A unique identifier for the redline model.
    //! @param templateModel    Optional. Identifies the model in this DgnMarkupProject to be used a template for the redline model. Must be a sheet model.
    //! @param createStatus     Optional. Set to non-zero status if the creation fails.
    //! @see OpenRedlineModel, RedlineModel::StoreImageData
    DGNPLATFORM_EXPORT RedlineModelP CreateRedlineModel(DgnDbStatus* createStatus, Utf8CP name, DgnModelId templateModel);

    //! Create a view of the redline model that is as similar as possible to the specified DgnDb view.
    //! @param redlineModel     The redline model returned by CreateRedlineModel
    //! @param redlineTemplateView Optional. Identifies a view in this DgnMarkupProject to be used a template for the redline model view.
    //! @param projectViewRect  The shape of the view that is being redlined. This is used only to get the aspect ratio, so that the redline view can be shaped and aligned to match the original DgnDb as closely as possible.
    //! @param imageViewRect    The area within the view where the background image should be displayed.
    //! @param createStatus     Optional. Set to non-zero status if the creation of the redline view fails.
    DGNPLATFORM_EXPORT DgnViewId CreateRedlineModelView(DgnDbStatus* createStatus, RedlineModelR redlineModel, DgnViewId redlineTemplateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);

    //! Find the redline model that is associated with the specified DgnDb view and whose origin is closest to specified point.
    //! @param[in]  viewController            The view to match
    //! @return If successful, the ID of the redline model that was based on the specified view and is closest to its origin, plus the distance of the 
    //! redlined view from the view's origin. If there is no redline view based on the specified view, then an invalid ID is returned.
    DGNPLATFORM_EXPORT bpair<DgnModelId,double> FindClosestRedlineModel(ViewControllerCR viewController);

    //! Open an existing redline model. The model is also filled.
    //! @param modelId  Identifies the redline model to open
    //! @param openStatus     Optional. Set to non-zero status if the model could not be found or could not be opened.
    //! @return a pointer to the open model or nullptr if the model could not be opened.
    //! @see CreateRedlineModel
    DGNPLATFORM_EXPORT RedlineModelP OpenRedlineModel(DgnDbStatus* openStatus, DgnModelId modelId);

    //! Empty an existing redline model. This function may be called after viewing a redline model. It releases memory held by the redline model.
    //! @param modelId  Identifies the redline model to empty
    //! @see OpenRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptyRedlineModel(DgnModelId modelId);
/** @} */

/** @name SpatialRedline Models */
/** @{ */
    //! Create a physical redline model.
    //! @param name                     A unique identifier for the physical redline model.
    //! @param subjectViewTargetModel   The target model of the view of the subject DgnDb. The SpatialRedlineModel's units are set to match the units of subjectViewTargetModel.
    //! @param createStatus     Optional. Set to non-zero status if the creation fails.
    //! @return a pointer to the new model
    //! @see OpenSpatialRedlineModel, @ref DgnMarkupProjectGroup_SpatialRedlines
    DGNPLATFORM_EXPORT SpatialRedlineModelP CreateSpatialRedlineModel(DgnDbStatus* createStatus, Utf8CP name, SpatialModelCR subjectViewTargetModel);

#if defined (NEEDS_WORK_VIEW_HANDLER_REFACTOR)
    //! Create a view of the physical redline model
    //! @param redlineModel     The physical redline model returned by CreateSpatialRedlineModel
    //! @param dgnView          The view of the subject DgnDb
    //! @return the ID of the new redline view
    DGNPLATFORM_EXPORT DgnViewId CreateSpatialRedlineModelView(SpatialRedlineModelR redlineModel, SpatialViewControllerR dgnView);
#endif

    //! Open an existing physical redline model.
    //! @param modelId  Identifies the physical redline model to open
    //! @param openStatus     Optional. Set to non-zero status if the model could not be found or could not be opened.
    //! @return a pointer to the open model or nullptr if the model could not be opened.
    //! @see CreateSpatialRedlineModel
    DGNPLATFORM_EXPORT SpatialRedlineModelP OpenSpatialRedlineModel(DgnDbStatus* openStatus, DgnModelId modelId);

    //! Empty an existing physical redline model. This function may be called after viewing a model. It releases memory held by the model.
    //! @param modelId  Identifies the physical redline model to empty
    //! @see OpenSpatialRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptySpatialRedlineModel(DgnModelId modelId);
/** @} */

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

    static void AddClassParams(ECSqlClassParamsR params);
    Dgn::DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);

protected:
    DGNPLATFORM_EXPORT virtual void _CopyFrom(Dgn::DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement) override;
    DGNPLATFORM_EXPORT virtual Dgn::DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, Dgn::ECSqlClassParams const& selectParams) override;

public:
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
    virtual DgnElementCP _ToGroupElement() const override { return this; }
    virtual Dgn::IElementGroupCP _ToIElementGroup() const override { return this; }

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
struct EXPORT_VTABLE_ATTRIBUTE MarkupExternalLinkHandler : Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLink, MarkupExternalLink, MarkupExternalLinkHandler, Element, DGNPLATFORM_EXPORT)
    virtual void _GetClassParams(ECSqlClassParamsR params) override { T_Super::_GetClassParams(params); MarkupExternalLink::AddClassParams(params); }
};

//! The handler for MarkupExternalLinkGroup elements
struct EXPORT_VTABLE_ATTRIBUTE MarkupExternalLinkGroupHandler : Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MARKUP_CLASSNAME_MarkupExternalLinkGroup, MarkupExternalLinkGroup, MarkupExternalLinkGroupHandler, Element, DGNPLATFORM_EXPORT)
};
}

namespace dgn_ModelHandler
{
//! The ModelHandler for RedlineModel.
struct Redline : Sheet
{
    MODELHANDLER_DECLARE_MEMBERS("RedlineModel", RedlineModel, Redline, Sheet, )
};

//! The ModelHandler for SpatialRedlineModel.
struct SpatialRedline : Spatial
{
    MODELHANDLER_DECLARE_MEMBERS("SpatialRedlineModel", SpatialRedlineModel, SpatialRedline, Spatial, )
};
}

END_BENTLEY_DGN_NAMESPACE
