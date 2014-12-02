/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnMarkupProject.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnProject.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>
#include <DgnPlatform/DgnCore/QueryView.h>

/** @addtogroup DgnMarkupProjectGroup Markup Projects
* A markup is a set of annotations that apply to a DgnProject or to views of that project. Markups include redlines, markups, and punch lists.
*
* @section DgnMarkupProjectGroup_Association Associating a DgnMarkupProject with a DgnProject
*
* A DgnMarkupProject is associated with a DgnProject. When you create a DgnMarkupProject, you must supply a reference to a DgnProject.
* The DgnMarkupProject stores the BeGuid and filename of the project. The project filename is stored relative to the markup
* project file. See DgnMarkupProject::GetAssociation. You can use the stored relative filename
* to find the original project. After re-opening a DgnMarkupProject, you should call the DgnMarkupProject::CheckAssociation
* function to be sure that the given DgnProject is in fact associated with the DgnMarkupProject. This check is based on the project's BeGuid.
* 
* @section DgnMarkupProjectGroup_Redlines Markups and Redlines
*
* A "markup" is an annotation applied to a DgnProject. A markup has text properties but no graphics.
* 
* A "redline" is an annotation applied to a DgnProject or view that has both text and graphics.
* Redline graphics are stored in models within a DgnMarkupProject. There are two types of models that hold redlines, RedlineModel and PhysicalRedlineModel.
* A redline model must be created or opened in order to store redlines.
*
* Markups and both types of redlines have EC properties. These properties can be checked and modified using normal ECDb functions. 
* See RedlineModel::GetRedlineECClass and RedlineModel::GetECInstanceId for redlines. Use the Bentley_Markup:Markup ECClass for markups.
* 
* @section DgnMarkupProjectGroup_PhysicalRedlines Physical vs. non-physical redlines.
* Suppose you want to draw redlines  on top of a map. The extent of the map is so great that no single view will show it very well. 
* You want to be able to zoom in and out and pan around and draw your redlines all over the place. So, you want the redline 
* view to show you a live view of the map, overlaid by a live view of the redline model. In this case, you need a "physical" redline 
* model and view. It's called "physical" because in this case the redline model itself is a (3-D) PhysicalModel, and the associated 
* ViewController is derived from PhysicalViewController. In the normal (non-physical) redline case, the redline view shows you a 
* static image of a view of the DgnProject or some other static image. The redline model in that case is a (2-D) SheetModel, and the 
* associated redline view is a SheetViewController.
*
*/

DGNPLATFORM_REF_COUNTED_PTR(RedlineViewController)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalRedlineViewController)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum DgnMarkupProjectSchemaValues
    {
    MARKUP_PROJECT_CURRENT_VERSION_Major = 0,
    MARKUP_PROJECT_CURRENT_VERSION_Minor = 1,
    MARKUP_PROJECT_CURRENT_VERSION_Sub1  = 0,
    MARKUP_PROJECT_CURRENT_VERSION_Sub2  = 0,

    MARKUP_PROJECT_SUPPORTED_VERSION_Major = MARKUP_PROJECT_CURRENT_VERSION_Major,  // oldest version of the project schema supported by current api
    MARKUP_PROJECT_SUPPORTED_VERSION_Minor = MARKUP_PROJECT_CURRENT_VERSION_Minor,
    MARKUP_PROJECT_SUPPORTED_VERSION_Sub1  = 0,
    MARKUP_PROJECT_SUPPORTED_VERSION_Sub2  = 0,
    };

//! Information about a DgnProject that can be used to save and check on markup-project association. See DgnMarkupProject::GetAssociation
struct DgnProjectAssociationData
    {
    private:
    Utf8String          m_relativeFileName; //! The relative path from the DgnProject to the DgnMarkupProject
    BeGuid              m_guid;             //! The GUID of the DgnProject.
    UInt64              m_lastModifiedTime; //! The last-modification time of the DgnProject

    public:
    DGNPLATFORM_EXPORT DgnProjectAssociationData();             //!< Construct an empty object. See DgnMarkupProject::GetAssociation
    DGNPLATFORM_EXPORT Utf8String GetRelativeFileName() const;  //!< Get the relative path of the DgnProject that is associated with the DgnMarkupProject
    DGNPLATFORM_EXPORT BeGuid GetGuid() const;                  //!< Get the GUID of the DgnProject that is associated with the DgnMarkupProject
    DGNPLATFORM_EXPORT UInt64 GetLastModifiedTime() const;      //!< Get the last modified time of the DgnProject as of the time that it was associated with the DgnMarkupProject
//__PUBLISH_SECTION_END__
    void FromDgnProject (DgnProjectCR, DgnMarkupProject const&);
    void ToPropertiesJson(JsonValueR) const;
    BentleyStatus FromPropertiesJson(JsonValueCR);
//__PUBLISH_SECTION_START__

    //! Reasons why markups may no longer correspond to the contents of the subject DgnProject
    struct CheckResults
        {
        bool NameChanged;       //!< The name of the DgnProject has changed. This is not necessarily a problem.
        bool GuidChanged;       //!< The DgnProject has been republished. It is not clear if the content and geometry of stored markups apply to the new version.
        bool ContentsChanged;   //!< The contents of the DgnProject has changed since the markup project was created. This is not necessarily a problem.
        bool UnitsChanged;      //!< The storage units of the DgnProject have changed. This probably means that physical redlines are now invalid.
        };
    };

//! Information about a view in a DgnProject that can be used to save and check on redline-view association. See RedlineModel::GetAssociation
struct DgnViewAssociationData : DgnProjectAssociationData
    {
    DEFINE_T_SUPER(DgnProjectAssociationData);
    
    private:
    DgnViewId     m_viewId;
    Json::Value   m_viewGeometry;

    public:
    DGNPLATFORM_EXPORT DgnViewAssociationData();

    DGNPLATFORM_EXPORT DgnViewId GetViewId() const; //!< Get the ViewId of the view in the DgnProject that is associated with the RedlineModel

    //! Re-create from serialized JSON string.
    //! @param[in] serializedData   Serialized JSON string.
    //! @return non-zero error status if the serialized data is invalid.
    DGNPLATFORM_EXPORT BentleyStatus FromSerializedJson (Utf8CP serializedData);

//__PUBLISH_SECTION_END__
    BentleyStatus FromPropertiesJson(JsonValueCR);
    void FromDgnProject (DgnProjectCR, ViewControllerCR, DgnMarkupProject const&); // hide superclass version of this function
    void ToPropertiesJson(JsonValueR) const;
    //! Get the origin of the view in the DgnProject that is associated with the RedlineModel
    void GetViewOrigin (DPoint3dR origin, DgnProjectCR);
//__PUBLISH_SECTION_START__

    //! Reasons why markups may no longer correspond to the contents of the subject DgnProject
    struct CheckResults : T_Super::CheckResults
        {
        bool ViewNotFound; //!< The view in the subject DgnProject cannot be found. This redline is probably no longer applicable.

        CheckResults (DgnProjectAssociationData::CheckResults const& projResults) {memcpy (this, &projResults, sizeof(projResults)); ViewNotFound = false;}
        };

    };

//=======================================================================================
//! Holds "redline" graphics and other annotations, plus a raster image.
//! 
//! A RedlineModel stores an image which is displayed as a backdrop to the redline graphics. 
//! See RedlineModel::StoreImageData.
//! 
//! A RedlineModel can be associated with a particular view of the subject DgnProject.
//! See RedlineModel::SetAssociation and RedlineModel::GetAssociation.
//! 
//! A RedlineModel does not have to be associated with a view of a DgnProject. The stored image can be acquired from some external source.
//! 
//! See DgnMarkupProject::CreateRedlineModel, DgnMarkupProject::OpenRedlineModel, and DgnMarkupProject::FindClosestRedlineModel.
//! @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct RedlineModel : SheetModel
    {
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__

    //! Describes the sheet, include its origina and size.
    // *** WIP_SHEETMODEL - reinstate SheetDef?? Copy more fields from SheetDef??
    struct RDLSheetDef
        {
        DPoint2d            m_origin;       //! Lower left corner of sheet border (in sheet model coordinates)
        DVec2d              m_size;         //! width and height of sheet border (in sheet model coordinates)

        RDLSheetDef();
        void ToPropertiesJson(JsonValueR) const;
        BentleyStatus FromPropertiesJson(JsonValueCR);
        };
    
public:
    //! Describes the format, size, and display location of the static image to be displayed as the background of a redline model.
    struct ImageDef
        {
        int                 m_format;       //! QV_BGRA_FORMAT or QV_RGBA_FORMAT
        Point2d             m_sizeInPixels; //! How many pixels per row (x) and per column (y) there are in the image.
        DPoint2d            m_origin;       //! where to display the image on the sheet (sheet coordinates)
        DVec2d              m_size;         //! how big the image is on the sheet (sheet coordinates)

        DGNPLATFORM_EXPORT ImageDef();
        void ToPropertiesJson(JsonValueR) const;
        BentleyStatus FromPropertiesJson(JsonValueCR);
        size_t GetPitch() const;            //!< Get the "pitch" or number of bytes per row. This is just m_size.x * GetSizeofPixelInBytes().
        size_t GetSizeInBytes() const;      //!< Get the total size of the image in bytes. This is just m_size.x*m_size.y*GetSizeofPixelInBytes().
        size_t GetSizeofPixelInBytes() const; //!< Get the number of bytes per pixel. That will be 4 if the image has alpha data or 3 if not.
        };

private:
    DgnMarkupProject*   m_project;
    RDLSheetDef         m_sheetDef;
    ImageDef            m_imageDef;
    bvector<intptr_t>   m_tileIds;
    bvector<DPoint3d>   m_tileOrigins;
    int                 m_tilesX;

    friend struct DgnMarkupProject;

protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::Redline;}
    virtual bool _Is3d() const override {return false;}

    static RedlineModelP CreateModel (DgnMarkupProjectR markupProject, Utf8CP name, DgnModelId templateModel);

    void SetSheetDef (RDLSheetDef const&);
    BentleyStatus StoreSheetDef() const;
    BentleyStatus LoadSheetDef();

    void DefineImageTexturesForRow (ImageDef const& imageDef, UInt8 const* rowStart, DPoint3dCR rowOrigin, Point2dCR tileDims, UInt32 nTilesAcross);

public:
    RedlineModel (DgnProjectR, DgnModelId modelId, Utf8CP name);

    static RedlineModelP Create (DgnMarkupProjectR, Utf8CP name, DgnModelId templateModel);
    static RedlineModelP Open   (DgnMarkupProjectR, DgnModelId);

    BentleyStatus DrawImage (ViewContextR, DPoint3dCR, DVec3dCR, bool drawBorder);
    BentleyStatus LoadImageData (ImageDef& def, bvector<UInt8>& imageData);
    DGNPLATFORM_EXPORT static BentleyStatus LoadImageData (ImageDef& def, bvector<UInt8>& imageData, DgnProjectCR, DgnModelId);
    void DefineImageTextures (ImageDef const& imageDef, bvector<UInt8> const& imageData);

    void LoadImageDataAndDefineTexture();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    DGNPLATFORM_EXPORT uintptr_t GetBackDropTextureId() const;

    //! Get the ECClass of the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT ECN::ECClassP GetECClass () const;

    //! Get the ID if the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceId GetECInstanceId () const;

    //! Get the ID of the view <em>in the DgnMarkupProject</em> associated with this redline model
    DGNPLATFORM_EXPORT DgnViewId GetViewId () const;

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;

    //! Save an image as the backdrop for this redline model.
    //! @param imageData       the image data
    //! @param imageInfo        information about the image, including its width and format
    //! @param fitToX           If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!                         If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    //! @param compressImageProperty If true, the image data is compressed before being stored in the database. 
    DGNPLATFORM_EXPORT void StoreImageData (bvector<UInt8> const& imageData, ImageUtilities::RgbImageInfo const& imageInfo, bool fitToX, bool compressImageProperty=true);

    //! Save an image as the backdrop for this redline model.
    //! @param jpegData         The image data in JPEG format.
    //! @param jpegDataSize     The size of the image data
    //! @param imageInfoIn      Information about the format of the image. Note that the width and format members are ignored as they are already encoded in the JPEG data.
    //! @param fitToX           If true, the image is stretched to fit the width of the sheet, and the image height is computed from it so as to preserve its original aspect ratio. 
    //!                         If false, the image is stretched to fit the height of the sheet, and the image width is computed.
    DGNPLATFORM_EXPORT void StoreImageDataFromJPEG (UInt8 const* jpegData, size_t jpegDataSize, ImageUtilities::RgbImageInfo const& imageInfoIn, bool fitToX);

/** @name Association to DgnProject */
/** @{ */
    //! Create or update an association between this redline model and the specified view in the specified DgnProject.
    //! @param dgnProject   The DgnProject that is being redlined
    //! @param projectView  The view in the DgnProject that is being redlined
    //! @see GetAssociation
    DGNPLATFORM_EXPORT void SetAssociation (DgnProjectR dgnProject, ViewControllerCR projectView);

    //! Check that this markup project was associated with the specified DgnProject as of the last call to SetAssociation.
    //! @param assocData    The association data passed to SetAssociation
    //! @see SetAssociation
    DGNPLATFORM_EXPORT void GetAssociation (DgnViewAssociationData& assocData) const;

    //! Check that this redline model is still valid for the target view.
    //! @param subjectProject   The project that was redlined.
    //! @return A list of what's changed in the project since this markup project was created.
    //! @see DgnMarkupProjectGroup_Association
    DGNPLATFORM_EXPORT DgnViewAssociationData::CheckResults CheckAssociation (DgnProjectR subjectProject);
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
    static Utf8CP GetViewSubType() {return "dgn_RedlineView";} // *** DO NOT CHANGE *** This is persistent data.

private:
    bool m_enableViewManipulation;
    bool m_drawBorder;

protected:
    virtual Utf8CP _GetViewSubType() const override {return GetViewSubType();} // *** DO NOT CHANGE *** This is persistent data.
    virtual void _DrawView (ViewContextR) override;
    virtual void _SetOrigin (DPoint3dCR viewOrg) override;
    virtual void _SetDelta (DVec3dCR viewDelta) override;
    virtual void _SetRotation (RotMatrixCR viewRot) override;

public:
    DGNPLATFORM_EXPORT static ViewController* Create(DgnProjectR project, DgnViewId id);
    DGNPLATFORM_EXPORT RedlineViewController (RedlineModel&, DgnViewId);
    DGNPLATFORM_EXPORT ~RedlineViewController();
    
    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] rdlModel the redline model to display
    DGNPLATFORM_EXPORT static RedlineViewControllerPtr InsertView (RedlineModelR rdlModel, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);
    DGNPLATFORM_EXPORT void OnClose (RedlineModel& targetModel);
    DGNPLATFORM_EXPORT void OnOpen (RedlineModel& targetModel);

    //! Query if sheet border should be displayed
    DGNPLATFORM_EXPORT bool GetDrawBorder() const;

    //! Specify if sheet border should be display
    //! @param b    if true, display the border
    DGNPLATFORM_EXPORT void SetDrawBorder (bool b);
#endif // DOCUMENTATION_GENERATOR
};

//=======================================================================================
//! Displays a PhysicalRedlineModel on top of another view.
//! A PhysicalRedlineViewController is always paired with a PhysicalViewController. 
//! PhysicalRedlineViewController implements the Decorator pattern. http://en.wikipedia.org/wiki/Decorator_pattern
//! PhysicalRedlineViewController overrides _DrawView to draw the PhysicalRedlineModel and then forwards the call to the subject PhysicalViewController.
//! PhysicalRedlineViewController overrides the various get and set methods on ViewController to keep itself and the subject PhysicalViewController in synch.
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

    static Utf8CP GetViewSubType() {return "dgn_PhysicalRedlineView";} // *** DO NOT CHANGE *** This is persistent data.

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    PhysicalViewController& m_subjectView;
    bool                    m_targetModelIsInSubjectView;

    virtual Utf8CP _GetViewSubType() const override {return GetViewSubType();} // *** DO NOT CHANGE *** This is persistent data.
    virtual void _SaveToSettings(JsonValueR) const override;
    virtual void _RestoreFromSettings (JsonValueCR) override;
    virtual void _DrawView (ViewContextR) override;
    virtual DPoint3d _GetOrigin () const override;
    virtual DVec3d _GetDelta () const override;
    virtual RotMatrix _GetRotation() const override;
    virtual void _SetOrigin (DPoint3dCR org) override;
    virtual void _SetDelta (DVec3dCR delta) override;
    virtual void _SetRotation (RotMatrixCR rot) override;
    virtual DgnModelP _GetTargetModel() const override;
    virtual DgnProjectR _GetDgnProject() const override;
    virtual void _AdjustAspectRatio (double , bool expandView) override;
    virtual DPoint3d _GetTargetPoint () const override;
    virtual bool _Allow3dManipulations () const override;
    virtual DgnViewType _GetViewType() const override;
    virtual DRange3d _GetProjectExtents() const override;
    virtual IAuxCoordSysP _GetAuxCoordinateSystem () const override;
    virtual ViewFlagsR _GetViewFlagsR () override;
    virtual RgbColorDef _GetBackgroundColor () const override;
    virtual BitMaskCR  _GetLevelDisplayMask () const override;
    virtual ClipVectorPtr _GetClipVector() const override {return NULL;}
    virtual bool _IsSnapAdjustmentRequired (ViewportR vp, bool snapLockEnabled) const override {return true;} // Always project snap to ACS plane...
    virtual bool _IsContextRotationRequired (ViewportR vp, bool contextLockEnabled) const override {return true;} // Always orient AccuDraw to ACS plane...
    virtual void _OnViewOpened (ViewportR vp) override;

    //  Override and forward the methods that trigger a query.
    virtual void _OnHealUpdate (ViewportR viewport, ViewContextR context, bool fullHeal) override;
    virtual void _OnFullUpdate (ViewportR viewport, ViewContextR context, FullUpdateInfo& info) override;
    virtual void _OnDynamicUpdate (ViewportR viewport, ViewContextR context, DynamicUpdateInfo& info) override;
    virtual void _OnLevelChange(bool singleEnabled) override;
    virtual void _ChangeModelDisplay (DgnModelId modelId, bool onOff) override;

    virtual bool _DrawOverlayDecorations(IndexedViewportR viewport) override;
    virtual bool _DrawZBufferedDecorations(IndexedViewportR viewport) override;
    virtual void _DrawBackgroundGraphics(ViewContextR context) override;
    virtual void _DrawZBufferedGraphics(ViewContextR context) override;

    virtual void _DrawElement(ViewContextR, ElementHandleCR) override;
    virtual void _DrawElementFiltered(ViewContextR, ElementHandleCR, DPoint3dCP pts, double size)  override;

    #ifdef WIP_RDL_QUERYVIEWS
    // QueryViewController
    virtual ScanRange _ShowTxnSummary(TxnSummaryCR summary) override;
    virtual bool _WantElementLoadStart (ViewportR viewport, double currentTime, double lastQueryTime, UInt32 maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum) override;
    virtual void _DrawView (ViewContextR context) override;
    virtual UInt32 _GetMaxElementsToLoad () override;
    virtual BeSQLite::DbResult _Load() override;
    virtual Utf8String _GetRTreeMatchSql (ViewportR viewport) override;
    virtual bool _OnComputeFitRange (DRange3dR range, ViewportR viewport, FitViewParamsR params) override;
    virtual Int32 _GetMaxElementFactor() override;
    virtual double _GetMinimumSizePixels (DrawPurpose updateType) override;
    virtual UInt64 _GetMaxElementMemory () override;
    // END QueryViewController
    #endif

    void SynchWithSubjectViewController();
#endif // DOCUMENTATION_GENERATOR

public:
    DGNPLATFORM_EXPORT PhysicalRedlineViewController (PhysicalRedlineModel& model, PhysicalViewController& subjectView, DgnViewId physicalRedlineViewId = DgnViewId());

    DGNPLATFORM_EXPORT ~PhysicalRedlineViewController ();
    //! Create a new redline view in the database
    //! @return The newly created view controller
    //! @param[in] model the physical redline model to display
    //! @param[in] subjectView the subject view to display underneath the physical redline model
    DGNPLATFORM_EXPORT static PhysicalRedlineViewControllerPtr InsertView (PhysicalRedlineModel& model, PhysicalViewController& subjectView);
};

//=======================================================================================
//! Holds "redline" graphics and other annotations for a physical view of a subject DgnProject. 
//! This type of redline model is displayed simultaneously with view of the subject project by PhysicalRedlineViewController.
//! The subject view is live, not a static image.
//! A PhysicalRedlineModel has the same units and coordinate system as the target model of the view of the subject project.
//! Note that the DgnMarkupProject that holds a PhysicalRedlineModel must have the same StorageUnits as the subject project. See CreateDgnMarkupProjectParams.
//! @see @ref DgnMarkupProjectGroup_PhysicalRedlines
//! @bsiclass                                                    Sam.Wilson      05/13
//=======================================================================================
struct PhysicalRedlineModel : PhysicalModel
    {
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__

private:
    DgnMarkupProject*   m_project;

    friend struct DgnMarkupProject;

protected:
    virtual DgnModelType _GetModelType() const override {return DgnModelType::PhysicalRedline;}
    virtual bool _Is3d() const override {return true;}

    static PhysicalRedlineModelP CreateModel (DgnMarkupProjectR markupProject, Utf8CP name, PhysicalModelCR subjectViewTargetModel);

public:
    PhysicalRedlineModel (DgnProjectR, DgnModelId modelId, Utf8CP name);

    static PhysicalRedlineModelP Open   (DgnMarkupProjectR, DgnModelId);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Get the ECClass of the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT ECN::ECClassP GetECClass () const;

    //! Get the ID if the ECInstance that holds the properties of this model.
    DGNPLATFORM_EXPORT BeSQLite::EC::ECInstanceId GetECInstanceId () const;

    //! Get the ID of the view <em>in the DgnMarkupProject</em> associated with this redline model
    DGNPLATFORM_EXPORT DgnViewId GetViewId () const;

    //! Get the DgnMarkupProject that contains this redline model
    DGNPLATFORM_EXPORT DgnMarkupProject* GetDgnMarkupProject() const;
    };

//=======================================================================================
//! Supplies the parameters necessary to create new DgnMarkupProjects.
// @bsiclass
//=======================================================================================
struct CreateDgnMarkupProjectParams : CreateProjectParams
{
private:
    DgnProjectR     m_dgnProject;
    bool            m_overwriteExisting;
    bool            m_physicalRedlining;

public:
    //! ctor
    //! @param[in] dgnProject   The DgnProject which is the target of this markup.
    //! @param[in] guid         The BeProjectGuid to store in the newly created DgnProject. If invalid (the default), a new BeProjectGuid is created.
    //! The new BeProjectGuid can be obtained via GetGuid.
    CreateDgnMarkupProjectParams (DgnProjectR dgnProject, BeProjectGuid guid=BeProjectGuid()) : CreateProjectParams(guid), m_dgnProject(dgnProject), m_overwriteExisting(false) {;}

    //! Get the subject DgnProject
    DgnProjectR GetSubjectDgnProject() const {return m_dgnProject;}

    //! Specify whether to overwrite an existing file or not. The default is to fail if a file by the supplied name already exists.
    void SetOverwriteExisting (bool val) {m_overwriteExisting = val;}
    bool GetOverwriteExisting () const {return m_overwriteExisting;}

    //! Specify if this markup project is to contain physical redline models. @see @ref DgnMarkupProjectGroup_PhysicalRedlines
    void SetPhysicalRedlining (bool val) {m_physicalRedlining = val;}
    bool GetPhysicalRedlining () const {return m_physicalRedlining;}
};

//=======================================================================================
//! A DgnMarkupProject project is a type of DgnProject that holds markups of various kinds, including redlines and punch lists. 
//! @ingroup DgnMarkupProjectGroup
// @bsiclass
//=======================================================================================
struct DgnMarkupProject : DgnProject
{
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(DgnProject)

private:
    DgnMarkupProject () {}
    virtual ~DgnMarkupProject() {}
    DgnFileStatus ConvertToMarkupProject (BeFileNameCR fileName, CreateProjectParams const& params);

    BentleyStatus ImportMarkupEcschema ();
    BentleyStatus QueryPropertyAsJson (JsonValueR json, DgnMarkupProjectProperty::ProjectProperty const& propSpec, UInt64 id=0) const;
    void SavePropertyFromJson (DgnMarkupProjectProperty::ProjectProperty const& propSpec, JsonValueCR json, UInt64 id=0);

    void CreateModelECProperties (DgnModelId modelId, Utf8CP modelName);
    DgnFileStatus LoadMarkupDgnFile(Db::OpenMode);

public:
    ECN::IECInstancePtr CreateRedlineInstance ();

    BentleyStatus CheckIsOpen();

    DgnModelId CreateModelPhaseI (Utf8CP name, DgnModelType);
    
    BeSQLite::EC::ECInstanceId GetECInstanceId (DgnModelCR) const;

    BentleyStatus QueryPropertyAsJson (JsonValueR json, RedlineModelProperty::ProjectProperty const&, UInt64 id=0) const;
    void SavePropertyFromJson (RedlineModelProperty::ProjectProperty const& propSpec, JsonValueCR json, UInt64 id=0) const;

    BentleyStatus QueryPropertyAsJson (JsonValueR json, DgnModelCR, RedlineModelProperty::ProjectProperty const& propSpec, UInt64 id=0) const;
    void SavePropertyFromJson (DgnModelCR, RedlineModelProperty::ProjectProperty const& propSpec, JsonValueCR json, UInt64 id=0);

    DgnProjectAssociationData::CheckResults CheckAssociation (DgnProjectR subjectProject, DgnProjectAssociationData const&);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Compute the default filename for a DgnMarkupProject, based on the associated DgnProject
    //! @param[out] markupProjectName   The computed name of the DgnMarkupProject
    //! @param[in]  dgnProjectName      The name of the DgnProject
    DGNPLATFORM_EXPORT static void ComputeDgnProjectFileName (BeFileNameR markupProjectName, BeFileNameCR dgnProjectName);

    //! Open and existing DgnMarkupProject file.
    //! @param[out] status DGNFILE_STATUS_Success if the DgnMarkupProject file was successfully opened or a non-zero error code if the project could not be opened or is not a markup project. May be NULL. Common errors include:
    //! - DGNOPEN_STATUS_FileNotFound - \a filename was not found
    //! - DGNPROJECT_ERROR_NotDgnMarkupProject - \a filename identifies a file that is not a DgnMarkupProject file
    //! @param[in] filename The name of the file from which the DgnMarkupProject is to be opened. Must be a valid filename.
    //! @param[in] openParams Parameters for opening the database file
    //! @param[in] loadDgnFile After the DgnMarkupProject database is opened, this method will also load the DgnFile for the project. If you know you
    //! will never access any element data, you can skip that step. However, generally it is preferable to load the DgnFile when opening the project.
    //! @return a reference counted pointer to the opened DgnMarkupProject. Its IsValid() method will be false if the open failed for any reason.
    DGNPLATFORM_EXPORT static DgnMarkupProjectPtr OpenProject (DgnFileStatus* status, BeFileNameCR filename, OpenParams const& openParams, bool loadDgnFile=true);

    //! Create and open a new DgnMarkupProject file.
    //! @param[out] status \a DGNFILE_STATUS_Success if the DgnMarkupProject file was successfully created or a non-zero error code if the project could not be created. May be NULL. Some common errors are:
    //! - DGNFILE_ERROR_BadArg - a required argument was not supplied. For example, CreateDgnMarkupProjectParams::m_seedDb is a required argument.
    //! - DGNPATHNAME_ERROR_FileNotFound - The seed file was not found
    //! - DGNPATHNAME_ERROR_AccessViolation - The seed file cannot be read.
    //! - DGNPATHNAME_ERROR_CantDeleteFile - CreateDgnMarkupProjectParams::m_overwriteExisting is true but the existing file cannot be deleted
    //! - Or \a status may be set to one of the DGNOPEN_STATUS_... values if there is a problem opening the new file.
    //! @param[in] filename The name of the file for the new DgnMarkupProject. The directory must be writable.
    //! @param[in] params Parameters that control aspects of the newly created DgnMarkupProject
    //! @return a reference counted pointer to the newly created DgnMarkupProject. Its IsValid() method will be false if the open failed for any reason.
    DGNPLATFORM_EXPORT static DgnMarkupProjectPtr CreateProject (DgnFileStatus* status, BeFileNameCR filename, CreateDgnMarkupProjectParams const& params);

    //! Query if this project has been initialized for physical redlining.
    //! @see CreateDgnMarkupProjectParams::SetPhysicalRedlining
    DGNPLATFORM_EXPORT bool IsPhysicalRedlineProject() const;

/** @name Association to DgnProject */
/** @{ */
    //! Create or update an association between this markup project and the specified DgnProject.
    //! @see GetAssociation
    DGNPLATFORM_EXPORT void SetAssociation (DgnProjectR dgnProject);

    //! Check that this markup project is associated with the specified DgnProject
    //! @param assocData    The association data passed to SetAssociation
    //! @see SetAssociation
    DGNPLATFORM_EXPORT void GetAssociation (DgnProjectAssociationData& assocData) const;

    //! Check that this markup project is still valid for the subject project
    //! @param subjectProject   The project that is being redlined.
    //! @return A list of what's changed in the project since this markup project was created.
    //! @see DgnMarkupProjectGroup_Association
    DGNPLATFORM_EXPORT DgnProjectAssociationData::CheckResults CheckAssociation (DgnProjectR subjectProject);
//** @} */

/** @name Redline Models */
/** @{ */
    //! Create a redline model of that model.
    //! @param name             A unique identifier for the redline model.
    //! @param templateModel    Optional. Identifies the model in this DgnMarkupProject to be used a template for the redline model. Must be a sheet model.
    //! @see OpenRedlineModel, RedlineModel::StoreImageData
    //! @note: This function also creates an ECInstance associated with the new redline model. The instance is from the DgnMarkupSchema:Redline class.
    //! After calling this function, the caller should then finish the job of defining the properties of the DgnMarkupSchema:Redline instance.
    DGNPLATFORM_EXPORT RedlineModelP CreateRedlineModel (Utf8CP name, DgnModelId templateModel);

    //! Create a view of the redline model that is as similar as possible to the specified DgnProject view.
    //! @param redlineModel     The redline model returned by CreateRedlineModel
    //! @param redlineTemplateView Optional. Identifies a view in this DgnMarkupProject to be used a template for the redline model view.
    //! @param projectViewRect  The shape of the view that is being redlined. This is used only to get the aspect ratio, so that the redline view can be shaped and aligned to match the original DgnProject as closely as possible.
    //! @param imageViewRect    The area within the view where the background image should be displayed.
    DGNPLATFORM_EXPORT DgnViewId CreateRedlineModelView (RedlineModelR redlineModel, DgnViewId redlineTemplateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect);

    //! Find the redline model that is associated with the specified DgnProject view and whose origin is closest to specified point.
    //! @param[in]  viewController            The view to match
    //! @return If successful, the ID of the redline model that was based on the specified view and is closest to its origin, plus the distance of the 
    //! redlined view from the view's origin. If there is no redline view based on the specified view, then an invalid ID is returned.
    DGNPLATFORM_EXPORT bpair<DgnModelId,double> FindClosestRedlineModel (ViewControllerCR viewController);

    //! Open an existing redline model.
    //! @param modelId  Identifies the redline model to open
    //! @see CreateRedlineModel
    DGNPLATFORM_EXPORT RedlineModelP OpenRedlineModel (DgnModelId modelId);

    //! Empty an existing redline model. This function may be called after viewing a redline model. It releases memory held by the redline model.
    //! @param modelId  Identifies the redline model to empty
    //! @see OpenRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptyRedlineModel (DgnModelId modelId);

    //! Convenience function to look up the DgnMarkupSchema:Redline ECClass.
    DGNPLATFORM_EXPORT ECN::ECClassP GetRedlineECClass();
//** @} */

/** @name PhysicalRedline Models */
/** @{ */
    //! Create a physical redline model.
    //! @param name                     A unique identifier for the physical redline model.
    //! @param subjectViewTargetModel   The target model of the view of the subject DgnProject. The PhysicalRedlineModel's units are set to match the units of subjectViewTargetModel.
    //! @return a pointer to the new model
    //! @see OpenPhysicalRedlineModel, @ref DgnMarkupProjectGroup_PhysicalRedlines
    DGNPLATFORM_EXPORT PhysicalRedlineModelP CreatePhysicalRedlineModel (Utf8CP name, PhysicalModelCR subjectViewTargetModel);

    //! Create a view of the physical redline model
    //! @param redlineModel     The physical redline model returned by CreatePhysicalRedlineModel
    //! @param dgnView          The view of the subject DgnProject
    //! @return the ID of the new redline view
    DGNPLATFORM_EXPORT DgnViewId CreatePhysicalRedlineModelView (PhysicalRedlineModelR redlineModel, PhysicalViewControllerR dgnView);

    //! Open an existing physical redline model.
    //! @param modelId  Identifies the physical redline model to open
    //! @see CreatePhysicalRedlineModel
    DGNPLATFORM_EXPORT PhysicalRedlineModelP OpenPhysicalRedlineModel (DgnModelId modelId);

    //! Empty an existing physical redline model. This function may be called after viewing a model. It releases memory held by the model.
    //! @param modelId  Identifies the physical redline model to empty
    //! @see OpenPhysicalRedlineModel
    DGNPLATFORM_EXPORT BentleyStatus EmptyPhysicalRedlineModel (DgnModelId modelId);
//** @} */

};

END_BENTLEY_DGNPLATFORM_NAMESPACE
