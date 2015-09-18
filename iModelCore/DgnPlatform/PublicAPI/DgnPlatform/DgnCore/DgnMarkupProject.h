/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnMarkupProject.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include <DgnPlatform/DgnCore/QueryView.h>

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
* Redline graphics are stored in models within a DgnMarkupProject. There are two types of models that hold redlines, RedlineModel and PhysicalRedlineModel.
* A redline model must be created or opened in order to store redlines.
*
* @section DgnMarkupProjectGroup_PhysicalRedlines Physical vs. non-physical redlines.
* Suppose you want to draw redlines  on top of a map. The extent of the map is so great that no single view will show it very well. 
* You want to be able to zoom in and out and pan around and draw your redlines at any location in the map. Therefore, you want the redline 
* view to show you a live view of the map, combined with a live view of the redline model. In this case, you need a "physical" redline 
* model and view. It's called "physical" because in this case the redline model itself is a (3-D) PhysicalModel, and the associated 
* ViewController is derived from PhysicalViewController. In the normal (non-physical) redline case, the redline view shows you a 
* static image of a view of the DgnDb or some other static image. The redline model in that case is a (2-D) SheetModel, and the 
* associated redline view is a SheetViewController.
*/

#ifdef WIP_REDLINE_ECINSTANCE
* Markups and both types of redlines have EC properties. These properties can be checked and modified using normal ECDb functions. 
* See RedlineModel::GetRedlineECClass and RedlineModel::GetECInstanceId for redlines. Use the Bentley_Markup:Markup ECClass for markups.
#endif

DGNPLATFORM_REF_COUNTED_PTR(RedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalRedlineModel)
DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalRedlineViewController)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct RedlineModelHandler;
struct PhysicalRedlineModelHandler;

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

//! Information about a DgnDb that can be used to save and check on markup-project association. See DgnMarkupProject::GetAssociation
struct DgnProjectAssociationData
    {
private:
    Utf8String  m_relativeFileName; //! The relative path from the DgnDb to the DgnMarkupProject
    BeGuid      m_guid;             //! The GUID of the DgnDb.
    uint64_t    m_lastModifiedTime; //! The last-modification time of the DgnDb

public:
    DGNPLATFORM_EXPORT DgnProjectAssociationData();             //!< Construct an empty object. See DgnMarkupProject::GetAssociation
    DGNPLATFORM_EXPORT Utf8String GetRelativeFileName() const;  //!< Get the relative path of the DgnDb that is associated with the DgnMarkupProject
    DGNPLATFORM_EXPORT BeGuid GetGuid() const;                  //!< Get the GUID of the DgnDb that is associated with the DgnMarkupProject
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

//! Information about a view in a DgnDb that can be used to save and check on redline-view association. See RedlineModel::GetAssociation
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
    DEFINE_T_SUPER(SheetModel)

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
        bool GetIsTopDown() const;          //!< Is the image top-down? Else, bottom-up.
        };

private:
    ImageDef            m_imageDef;
    bvector<intptr_t>   m_tileIds;
    bvector<DPoint3d>   m_tileOrigins;
    int                 m_tilesX;
    DgnViewAssociationData m_assoc;

    friend struct DgnMarkupProject;
    friend struct RedlineModelHandler;

protected:
    DgnModelType _GetModelType() const override {return DgnModelType::Sheet;}
    void _ToPropertiesJson(Json::Value&) const override;
    void _FromPropertiesJson(Json::Value const&) override;

    void DefineImageTexturesForRow(ImageDef const& imageDef, uint8_t const* rowStart, DPoint3dCR rowOrigin, Point2dCR tileDims, uint32_t nTilesAcross);

    static RedlineModelPtr Create(DgnMarkupProjectR markupProject, Utf8CP name, DgnModelId templateModel);

public:
    explicit RedlineModel(CreateParams const& params): T_Super(params) {m_tilesX = 0;}
    BentleyStatus DrawImage(ViewContextR, DPoint3dCR, DVec3dCR, bool drawBorder);
    BentleyStatus LoadImageData(ImageDef& def, bvector<uint8_t>& imageData);
    DGNPLATFORM_EXPORT static BentleyStatus LoadImageData(ImageDef& def, bvector<uint8_t>& imageData, DgnDbCR, DgnModelId);
    void DefineImageTextures(ImageDef const& imageDef, bvector<uint8_t> const& imageData);

    void LoadImageDataAndDefineTexture();

    DgnViewId GetFirstView();

public:

    DGNPLATFORM_EXPORT uintptr_t GetBackDropTextureId() const;

#ifdef WIP_REDLINE_ECINSTANCE
    //! Get the ECClass of the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT ECN::ECClassCP GetECClass() const;

    //! Get the ID if the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceId GetECInstanceId() const;
#endif

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;

    //! Save an image as the backdrop for this redline model.
    //! @param imageData       the image data
    //! @param imageInfo        information about the image, including its width and format
    //! @param fitToX           If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!                         If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    //! @param compressImageProperty If true, the image data is compressed before being stored in the database. 
    DGNPLATFORM_EXPORT void StoreImageData(bvector<uint8_t> const& imageData, ImageUtilities::RgbImageInfo const& imageInfo, bool fitToX, bool compressImageProperty=true);

    //! Save an image as the backdrop for this redline model.
    //! @param jpegData         The image data in JPEG format.
    //! @param jpegDataSize     The size of the image data
    //! @param imageInfoIn      Information about the format of the image. Note that the width and format members are ignored as they are already encoded in the JPEG data.
    //! @param fitToX           If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!                         If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    DGNPLATFORM_EXPORT void StoreImageDataFromJPEG (uint8_t const* jpegData, size_t jpegDataSize, ImageUtilities::RgbImageInfo const& imageInfoIn, bool fitToX);

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
//! @ingroup DgnViewGroup
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
    DGNPLATFORM_EXPORT static ViewController* Create(DgnDbR project, DgnViewId id);
    DGNPLATFORM_EXPORT RedlineViewController(RedlineModel&, DgnViewId id = DgnViewId());
    DGNPLATFORM_EXPORT ~RedlineViewController();
    
    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] rdlModel the redline model to display
    DGNPLATFORM_EXPORT static RedlineViewControllerPtr InsertView(RedlineModelR rdlModel, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);
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
//! Displays a PhysicalRedlineModel in conjunction with the display of another view controller.
//!@remarks
//! PhysicalRedlineViewController is a normal physical view in most respects, and its PhysicalRedlineModel is its target model.
//! 
//! PhysicalRedlineViewController is unusual in that it also tries work in sync with another view controller.
//! This other controller is called the "subject view controller." It must be be supplied in the PhysicalRedlineViewController constructor.
//! A PhysicalRedlineViewController actually has no view parameters of its own. Instead, it adopts the view parameters of the subject view controller on the fly. 
//! PhysicalRedlineViewController overrides _DrawView to draw its own PhysicalRedlineModel. It then forwards the draw request to the subject view controller.
//! A PhysicalRedlineViewController handles most viewing-related queries by applying them to itself and then forwarding them to the subject view controller, so that
//! the two view controllers are always in sync.
//! 
//! <h4>Locating and Editing</h4>
//! Locates and edits are normally directed to the PhysicalRedlineModel, since that is the normal target model of a PhysicalRedlineViewController.
//! If an app wants to re-direct locates and/or edits to the target of the subject view controller instead, it should PhysicalRedlineViewController::SetTargetModel
//! in order to change the PhysicalRedlineViewController's target model.
//!
//! @see @ref DgnMarkupProjectGroup_PhysicalRedlines
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct PhysicalRedlineViewController : PhysicalViewController
{
    DEFINE_T_SUPER (PhysicalViewController);

#if !defined (DOCUMENTATION_GENERATOR)
    friend struct DgnMarkupProject;
#endif

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    PhysicalViewController& m_subjectView;
    bvector<PhysicalRedlineModelP> m_otherRdlsInView;

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
    virtual DgnModelP _GetTargetModel() const override;
    virtual DgnDbR _GetDgnDb() const override;
    virtual void _AdjustAspectRatio(double , bool expandView) override;
    virtual DPoint3d _GetTargetPoint() const override;
    virtual bool _Allow3dManipulations() const override;
    virtual AxisAlignedBox3d _GetViewedExtents() const override;
    virtual IAuxCoordSysP _GetAuxCoordinateSystem() const override;
    virtual ColorDef _GetBackgroundColor() const override;
    virtual ClipVectorPtr _GetClipVector() const override {return NULL;}
    virtual bool _IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const override {return true;} // Always project snap to ACS plane...
    virtual bool _IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const override {return true;} // Always orient AccuDraw to ACS plane...
    virtual void _OnViewOpened(DgnViewportR vp) override;

    //  Override and forward the methods that trigger a query.
    virtual void _OnHealUpdate(DgnViewportR viewport, ViewContextR context, bool fullHeal) override;
    virtual void _OnFullUpdate(DgnViewportR viewport, ViewContextR context, FullUpdateInfo& info) override;
    virtual void _OnDynamicUpdate(DgnViewportR viewport, ViewContextR context, DynamicUpdateInfo& info) override;
    virtual void _OnCategoryChange(bool singleEnabled) override;
    virtual void _ChangeModelDisplay(DgnModelId modelId, bool onOff) override;

    virtual bool _DrawOverlayDecorations(IndexedViewportR viewport) override;
    virtual bool _DrawZBufferedDecorations(IndexedViewportR viewport) override;
    virtual void _DrawBackgroundGraphics(ViewContextR context) override;
    virtual void _DrawZBufferedGraphics(ViewContextR context) override;

    virtual void _DrawElement(ViewContextR, GeometricElementCR) override;
    virtual void _DrawElementFiltered(ViewContextR, GeometricElementCR, DPoint3dCP pts, double size)  override;

#ifdef WIP_PhysicalRedlineViewController
    // QueryViewController
    virtual bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    virtual void _DrawView(ViewContextR context) override;
    virtual uint32_t _GetMaxElementsToLoad() override;
    virtual BeSQLite::DbResult _Load() override;
    virtual Utf8String _GetRTreeMatchSql(DgnViewportR viewport) override;
    virtual int32_t _GetMaxElementFactor() override;
    virtual double _GetMinimumSizePixels(DrawPurpose updateType) override;
    virtual uint64_t _GetMaxElementMemory() override;
#endif
    // END QueryViewController

    void SynchWithSubjectViewController();
#endif // DOCUMENTATION_GENERATOR

public:
    //! Create a PhysicalRedlineViewController
    //! @param model    The PhysicalRedlineModel to view
    //! @param subjectView The view of the underlying physical coordinate space to overlay
    //! @param physicalRedlineViewId The view id
    //! @param otherRdlsToView Optional. Other PhysicalRedlineModels to show in the view.
    DGNPLATFORM_EXPORT PhysicalRedlineViewController (PhysicalRedlineModel& model, PhysicalViewController& subjectView, DgnViewId physicalRedlineViewId, bvector<PhysicalRedlineModelP> const& otherRdlsToView);

    DGNPLATFORM_EXPORT ~PhysicalRedlineViewController();

    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] model the physical redline model to display
    //! @param[in] subjectView the subject view to display underneath the physical redline model
    DGNPLATFORM_EXPORT static PhysicalRedlineViewControllerPtr InsertView(PhysicalRedlineModel& model, PhysicalViewController& subjectView);
};

//=======================================================================================
//! Holds "redline" graphics and other annotations for a physical view of a subject DgnDb. 
//! This type of redline model is displayed simultaneously with view of the subject project by PhysicalRedlineViewController.
//! The subject view is live, not a static image.
//! A PhysicalRedlineModel has the same units and coordinate system as the target model of the view of the subject project.
//! Note that the DgnMarkupProject that holds a PhysicalRedlineModel must have the same StorageUnits as the subject project. See CreateDgnMarkupProjectParams.
//! @see @ref DgnMarkupProjectGroup_PhysicalRedlines
// @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct PhysicalRedlineModel : PhysicalModel
    {
private:
    DEFINE_T_SUPER(PhysicalModel)

    friend struct DgnMarkupProject;
    friend struct PhysicalRedlineModelHandler;

protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Physical;}

    static PhysicalRedlineModelPtr Create(DgnMarkupProjectR markupProject, Utf8CP name, PhysicalModelCR subjectViewTargetModel);

public:
    explicit PhysicalRedlineModel(CreateParams const& params) : T_Super(params) {}

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
    bool            m_physicalRedlining;

public:
    //! ctor
    //! @param[in] dgnProject   The DgnDb which is the target of this markup.
    //! @param[in] guid         The BeProjectGuid to store in the newly created DgnDb. If invalid (the default), a new BeProjectGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateDgnMarkupProjectParams(DgnDbR dgnProject, BeDbGuid guid=BeDbGuid()) : CreateDgnDbParams(guid), m_dgnDb(dgnProject), m_overwriteExisting(false) {;}

    //! Get the subject DgnDb
    DgnDbR GetSubjectDgnProject() const {return m_dgnDb;}

    //! Specify whether to overwrite an existing file or not. The default is to fail if a file by the supplied name already exists.
    void SetOverwriteExisting(bool val) {m_overwriteExisting = val;}
    bool GetOverwriteExisting() const {return m_overwriteExisting;}

    //! Specify if this markup project is to contain physical redline models. @see @ref DgnMarkupProjectGroup_PhysicalRedlines
    void SetPhysicalRedlining(bool val) {m_physicalRedlining = val;}
    bool GetPhysicalRedlining() const {return m_physicalRedlining;}
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

    BentleyStatus ImportMarkupEcschema();
    BentleyStatus QueryPropertyAsJson(JsonValueR json, DgnMarkupProjectProperty::ProjectProperty const& propSpec, uint64_t id=0) const;
    void SavePropertyFromJson(DgnMarkupProjectProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id=0);

public:
    BentleyStatus CheckIsOpen();

    DgnViewId GetFirstViewOf(DgnModelId);

#ifdef WIP_REDLINE_ECINSTANCE
    ECN::IECInstancePtr CreateRedlineInstance();
    BeSQLite::EC::ECInstanceId GetECInstanceId(DgnModelCR) const;
#endif

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
    //! @see CreateDgnMarkupProjectParams::SetPhysicalRedlining
    DGNPLATFORM_EXPORT bool IsPhysicalRedlineProject() const;

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
    //! @see OpenRedlineModel, RedlineModel::StoreImageData
    //! @note: This function also creates an ECInstance associated with the new redline model. The instance is from the DgnMarkupSchema:Redline class.
    //! After calling this function, the caller should then finish the job of defining the properties of the DgnMarkupSchema:Redline instance.
    DGNPLATFORM_EXPORT RedlineModelP CreateRedlineModel(Utf8CP name, DgnModelId templateModel);

    //! Create a view of the redline model that is as similar as possible to the specified DgnDb view.
    //! @param redlineModel     The redline model returned by CreateRedlineModel
    //! @param redlineTemplateView Optional. Identifies a view in this DgnMarkupProject to be used a template for the redline model view.
    //! @param projectViewRect  The shape of the view that is being redlined. This is used only to get the aspect ratio, so that the redline view can be shaped and aligned to match the original DgnDb as closely as possible.
    //! @param imageViewRect    The area within the view where the background image should be displayed.
    DGNPLATFORM_EXPORT DgnViewId CreateRedlineModelView(RedlineModelR redlineModel, DgnViewId redlineTemplateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);

    //! Find the redline model that is associated with the specified DgnDb view and whose origin is closest to specified point.
    //! @param[in]  viewController            The view to match
    //! @return If successful, the ID of the redline model that was based on the specified view and is closest to its origin, plus the distance of the 
    //! redlined view from the view's origin. If there is no redline view based on the specified view, then an invalid ID is returned.
    DGNPLATFORM_EXPORT bpair<DgnModelId,double> FindClosestRedlineModel(ViewControllerCR viewController);

    //! Open an existing redline model.
    //! @param modelId  Identifies the redline model to open
    //! @see CreateRedlineModel
    DGNPLATFORM_EXPORT RedlineModelP OpenRedlineModel(DgnModelId modelId);

    //! Empty an existing redline model. This function may be called after viewing a redline model. It releases memory held by the redline model.
    //! @param modelId  Identifies the redline model to empty
    //! @see OpenRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptyRedlineModel(DgnModelId modelId);
/** @} */

/** @name PhysicalRedline Models */
/** @{ */
    //! Create a physical redline model.
    //! @param name                     A unique identifier for the physical redline model.
    //! @param subjectViewTargetModel   The target model of the view of the subject DgnDb. The PhysicalRedlineModel's units are set to match the units of subjectViewTargetModel.
    //! @return a pointer to the new model
    //! @see OpenPhysicalRedlineModel, @ref DgnMarkupProjectGroup_PhysicalRedlines
    DGNPLATFORM_EXPORT PhysicalRedlineModelP CreatePhysicalRedlineModel(Utf8CP name, PhysicalModelCR subjectViewTargetModel);

#if defined (NEEDS_WORK_VIEW_HANDLER_REFACTOR)
    //! Create a view of the physical redline model
    //! @param redlineModel     The physical redline model returned by CreatePhysicalRedlineModel
    //! @param dgnView          The view of the subject DgnDb
    //! @return the ID of the new redline view
    DGNPLATFORM_EXPORT DgnViewId CreatePhysicalRedlineModelView(PhysicalRedlineModelR redlineModel, PhysicalViewControllerR dgnView);
#endif

    //! Open an existing physical redline model.
    //! @param modelId  Identifies the physical redline model to open
    //! @see CreatePhysicalRedlineModel
    DGNPLATFORM_EXPORT PhysicalRedlineModelP OpenPhysicalRedlineModel(DgnModelId modelId);

    //! Empty an existing physical redline model. This function may be called after viewing a model. It releases memory held by the model.
    //! @param modelId  Identifies the physical redline model to empty
    //! @see OpenPhysicalRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptyPhysicalRedlineModel(DgnModelId modelId);
/** @} */

};

namespace dgn_ModelHandler
{
    //! The ModelHandler for RedlineModel.
    struct Redline : Sheet
    {
        MODELHANDLER_DECLARE_MEMBERS("RedlineModel", RedlineModel, Redline, Sheet,)
    };

    //! The ModelHandler for PhysicalRedlineModel.
    struct PhysicalRedline : Physical
    {
        MODELHANDLER_DECLARE_MEMBERS("PhysicalRedlineModel", PhysicalRedlineModel, PhysicalRedline, Physical,)
    };
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
