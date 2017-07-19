/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgImportInternal.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// only include this before including OpenDWG stuff - RealDWG has an MFC dependency:
#if defined(BENTLEY_WIN32) && defined(DWGTOOLKIT_OpenDwg)
#include    <Windows.h>                 // LOGFONTW etc
#endif

#include <Bentley/Bentley.h>
#include <Bentley/BeNumerical.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/Desktop/FileSystem.h>

#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/LineStyle.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>

#include <Raster/RasterApi.h>

#include <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>
#include <DgnDbSync/Dwg/DwgDb/BasicTypes.h>
#include <DgnDbSync/Dwg/DwgDb/DwgResBuf.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbDatabase.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbObjects.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbEntities.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbSymbolTables.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDrawables.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbHost.h>
#include <DgnDbSync/Dwg/DwgImporter.h>
#include <DgnDbSync/Dwg/DwgSyncInfo.h>
#include <DgnDbSync/Dwg/DwgHelper.h>
#include <DgnDbSync/Dwg/ProtocalExtensions.h>

#include <Logging/BentleyLogging.h>

#define LOG             DwgImportLogging::GetLogger (DwgImportLogging::Namespace::General)
#define LOG_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::General,sev)

#define LOG_LAYER       DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Layer)
#define LOG_LAYER_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Layer,sev)

#define LOG_LINETYPE    DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Linetype)
#define LOG_LINETYPE_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Linetype,sev)

#define LOG_TEXTSTYLE   DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Textstyle)
#define LOG_TEXTSTYLE_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Textstyle,sev)

#define LOG_MATERIAL    DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Material)
#define LOG_MATERIAL_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Material,sev)

#define LOG_MODEL       DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Model)
#define LOG_MODEL_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Model,sev)

#define LOG_ENTITY      DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Entity)
#define LOG_ENTITY_IS_SEVERITY_ENABLED(sev) DwgImportLogging::IsSeverityEnabled (DwgImportLogging::Namespace::Entity,sev)

using namespace BentleyApi::Dgn;
using namespace BentleyApi::BeSQLite;
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_DGNDBSYNC
USING_NAMESPACE_DWGDB

BEGIN_DGNDBSYNC_DWG_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/16
+===============+===============+===============+===============+===============+======*/
struct DwgImportLogging
    {
    enum class Namespace
        {
        General,
        Model,
        Layer,
        Linetype,
        Textstyle,
        Material,
        Entity,
        Dictionary,
        Performance,
        MaxLoggers
        };

    static NativeLogging::ILogger&  GetLogger (Namespace ns);
    static bool                     IsSeverityEnabled (Namespace ns, NativeLogging::SEVERITY);
    static void                     LogPerformance (StopWatch& stopWatch, Utf8CP description, ...);
    };  // DwgImportLogging

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgProgressMeter : public IDwgDbProgressMeter
    {
private:
    DgnProgressMeter*   m_meter;

public:
    DwgProgressMeter (DgnProgressMeter* newMeter) : m_meter(newMeter) { }

    virtual void        _Start (WStringCR displayString = WString()) override { /* do nothing */ }
    virtual void        _Stop () override { /* do nothing */ }
    virtual void        _Progress () override;
    virtual void        _SetLimit (int max) override { /* do nothing */ }
    };  // IDwgDbProgressMeter

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgImportHost : public IDwgDbHost
{
private:
    DwgImporter*            m_importer;
    DwgProgressMeter*       m_progressMeter;
    WString                 m_lastShxFontName;
    WString                 m_registryRootKey;

    bool                    FindFontFile (WStringR outPath, WCharCP fontName, AcadFileType hint);
    bool                    FindXrefFile (WStringR outPath, WCharCP fileName, DwgDbDatabaseP dwg = nullptr);

public:
    virtual DwgDbStatus     _FindFile (WStringR fullpathOut, WCharCP filenameIn, DwgDbDatabaseP dwg = nullptr, AcadFileType hint = AcadFileType::Default) override;
    virtual bool            _GetAlternateFontName (WStringR altFont) const override;
    virtual bool            _GetPassword (WCharCP dwgName, PasswordChoice choice, WCharP password, const size_t bufSize) override;
    virtual WCharCP         _GetRegistryProductRootKey (RootRegistry type) override;
    virtual LCID            _GetRegistryProductLCID () override;
    virtual WCharCP         _Product () const override;
    virtual void            _FatalError (WCharCP format, ...) override;
    virtual void            _Alert (WCharCP message) const override;
    virtual void            _Message (WCharCP message, int numChars) const override;
    virtual void            _DebugPrintf (WCharCP format, ...) const override;

    void                    Initialize (DwgImporter& importer);
    void                    NewProgressMeter ();

    static DwgImportHost&   GetHost ();
};  // DwgImportHost

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct PolylineFactory
    {
private:
    DwgDbPolylineCP         m_polyline;
    size_t                  m_numPoints;
    DPoint3dArray           m_points;
    DPoint2dArray           m_widths;
    DwgDbDoubleArray        m_bulges;
    double                  m_constantWidth;
    bool                    m_hasBulges;
    bool                    m_hasConstantWidth;
    bool                    m_hasWidths;
    double                  m_elevation;
    double                  m_thickness;
    bool                    m_isClosed;

public:
    PolylineFactory ();
    PolylineFactory (DwgDbPolylineCR polyline, bool allowClosed = true);
    PolylineFactory (size_t nPoints, DPoint3dCP points, bool closed);

    // create profile polyline curve
    CurveVectorPtr          CreateCurveVector (bool useElevation);
    // apply thickness to the polyline curve created via above method
    GeometricPrimitivePtr   ApplyThicknessTo (CurveVectorPtr const& plineCurve);
    // Hash this polyline data and add it to output MD5:
    void                    HashAndAppendTo (BentleyApi::MD5& hashOut) const;
    // Transform polyline data
    void                    TransformData (TransformCR transform);

    static bool     IsValidBulgeFactor (double bulge);
    };  // PolylineFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          09/16
+===============+===============+===============+===============+===============+======*/
struct  AttributeFactory
    {
private:
    DwgImporter::ElementImportResults&  m_outputResults;
    DwgImporter::ElementImportInputs&   m_inputContext;
    DwgImporter&                        m_importer;
    DgnElementR                         m_hostElement;
    IECInstancePtr                      m_ecInstance;
    size_t                              m_propertyCount;
    size_t                              m_adhocCount;

    bool            AddPropertyOrAdhocFromAttribute (DwgDbAttributeCR attrib);
    ECObjectsStatus AddConstantProperty (DwgDbAttributeDefinitionCR attrdef);
    BentleyStatus   CreateECInstance (DwgDbObjectIdCR blockId);
    BentleyStatus   ProcessVariableAttributes (DwgDbObjectIterator& attribIter);
    BentleyStatus   ProcessConstantAttributes (DwgDbObjectIdCR blockId, DwgDbObjectIdCR blockRefId, TransformCR toBlockRef);
    size_t          GetPropertyCount () { return m_propertyCount; }
    size_t          GetAdhocCount () { return m_adhocCount; }
    size_t          GetTotalCount () { return m_propertyCount + m_adhocCount; }
    
public:
    explicit AttributeFactory (DwgImporter& importer, DgnElementR hostElement, DwgImporter::ElementImportResults& reults, DwgImporter::ElementImportInputs& inputs);

    // create DgnElements, along with its ElementAspect's, from DWG attributes
    BentleyStatus   CreateElements (DwgDbBlockReferenceCR blockReference);
    }; // AttributeFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/16
+===============+===============+===============+===============+===============+======*/
struct  ViewportFactory
    {
private:
    // DWG common viewport data
    DPoint3d            m_center;
    DPoint3d            m_target;
    DVec3d              m_viewDirection;
    bool                m_hasCamera;
    bool                m_isFrontClipped;
    bool                m_isBackClipped;
    double              m_frontClipDistance;
    double              m_backClipDistance;
    double              m_height;
    double              m_width;
    double              m_lensLength;
    double              m_viewTwistAngle;
    bool                m_isGridOn;
    bool                m_isUcsIconOn;
    DwgDbObjectId       m_clipEntityId;
    DwgDbObjectId       m_backgroundId;
    DwgDbObjectId       m_visualStyleId;
    double              m_customScale;
    DwgDbObjectCP       m_inputViewport;
    // BIM members
    ColorDef            m_backgroundColor;
    bool                m_isPrivate;
    Transform           m_transform;
    CategorySelectorPtr m_categories;
    DwgImporter&        m_importer;

    void ComputeSpatialView (SpatialViewDefinitionR dgnView);
    void ComputeSheetView (SheetViewDefinitionR dgnView);
    void ComputeSpatialDisplayStyle (DisplayStyle3dR displayStyle);
    void ComputeSheetDisplayStyle (DisplayStyleR displayStyle);
    bool ComputeViewAttachment (Placement2dR placement);
    bool ComposeLayoutTransform (TransformR trans, DwgDbObjectIdCR blockId);
    void TransformDataToBim ();
    void AddSpatialCategories (DgnDbR dgndb, Utf8StringCR viewName);
    void ApplyViewportClipping (SpatialViewDefinitionR dgnView, double frontClip, double backClip);
    bool ComputeClipperTransformation (TransformR toClipper, RotMatrixCR viewRotation);

public:
    // constructor for a modelspace viewport
    ViewportFactory (DwgImporter& importer, DwgDbViewportTableRecordCR viewportRecord);
    // constructor for the overall layout viewport and a paperspace viewport entity
    ViewportFactory (DwgImporter& importer, DwgDbViewportCR viewportEntity, DwgDbLayoutCP layout = nullptr);

    // create a camera or orthoganal view for a modelspace viewport or a viewport entity
    DgnViewId       CreateSpatialView (DgnModelId modelId, Utf8StringCR proposedName);
    // create a sheet view for the overall layout viewport
    DgnViewId       CreateSheetView (DgnModelId sheetId, Utf8StringCR proposedName);
    // create a view attachment element in a sheet model for a viewport entity in a paperspace
    DgnElementPtr   CreateViewAttachment (DgnModelCR sheetModel, DgnViewId viewId);
    // Corresponding Update methods
    BentleyStatus   UpdateSpatialView (DgnViewId viewId);
    DgnElementPtr   UpdateViewAttachment (DgnElementId attachId, DgnViewId viewId);

    bool    ValidateViewName (Utf8StringR viewNameInOut, DgnViewId& viewIdOut);
    void    SetBackgroundColor (ColorDefCR color) { m_backgroundColor = color; }
    void    SetViewSourcePrivate (bool viewSourcePrivate) { m_isPrivate = viewSourcePrivate; }
    };  // ViewportFactory

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/17
+===============+===============+===============+===============+===============+======*/
struct LayoutFactory
    {
    bool                m_isValid;
    DwgImporter&        m_importer;
    DwgDbLayoutPtr      m_layout;
public:
    LayoutFactory (DwgImporter& importer, DwgDbObjectId layoutId);
    bool            IsValid () const { return m_isValid; }
    double          GetUserScale () const;
    BentleyStatus   CalculateSheetSize (DPoint2dR sheetSize) const;
    };  // LayoutFactory

END_DGNDBSYNC_DWG_NAMESPACE
