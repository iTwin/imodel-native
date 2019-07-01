/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <Dwg/DwgDb/DwgDbCommon.h>
#include    <Dwg/DwgDb/DwgGiObjects.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Drawing/Include/DbObjectId.h>
#include    <Teigha/Drawing/Include/DbDatabase.h>
#include    <Teigha/Drawing/Include/DbDictionary.h>
#include    <Teigha/Drawing/Include/DbMaterial.h>
#include    <Teigha/Drawing/Include/DbVisualStyle.h>
#include    <Teigha/Drawing/Include/DbSpatialFilter.h>
#include    <Teigha/Drawing/Include/DbSpatialIndex.h>
#include    <Teigha/Drawing/Include/DbSortentsTable.h>
#include    <Teigha/Drawing/Include/DbObjectIterator.h>
#include    <Teigha/Drawing/Include/DbBackground.h>
#include    <Teigha/Drawing/Include/DbSun.h>

#elif DWGTOOLKIT_RealDwg

#include    <RealDwg/Base/dbid.h>
#include    <RealDwg/Base/dbmain.h>
#include    <RealDwg/Base/dbents.h>
#include    <RealDwg/Base/dbmaterial.h>
#include    <RealDwg/Base/dblayout.h>
#include    <RealDwg/Base/dblead.h>
#include    <RealDwg/Base/dbmleaderstyle.h>
#include    <RealDwg/Base/dbunderlaydef.h>
#include    <RealDwg/Base/dbvisualstyle.h>
#include    <RealDwg/Base/dbxrecrd.h>
#include    <RealDwg/Base/dbspfilt.h>
#include    <RealDwg/Base/dbspindx.h>
#include    <RealDwg/Base/dbsun.h>
#include    <RealDwg/Base/sorttab.h>
#else
    #error  "Must define DWGTOOLKIT!!!"
#endif

#include    <Geom/GeomApi.h>


BEGIN_DWGDB_NAMESPACE

class DwgDbDatabasePtr;
class DwgDbObjectPtr;

/*=================================================================================**//**
* Interface to do bag filer for DxfOut/In on an DwgDbObject
* @bsiclass                                                     Don.Fu          04/16
+===============+===============+===============+===============+===============+======*/
struct IDxfFiler
    {
public:
    // Number of decimal digits printed in ASCII DXF file
    enum class DoublePrecision
        {
        Default     = -1,
        Max         = 16
        };
    
    virtual DwgFilerType    _GetFilerType () const = 0;
    virtual DwgDbDatabaseP  _GetDatabase () const = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, int8_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, int16_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, int32_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, int64_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint8_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint16_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint32_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, uint64_t v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, bool v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, double v, DoublePrecision prec = DoublePrecision::Default) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgStringCR v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgBinaryDataCR v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbHandleCR v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DwgDbObjectIdCR v) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint2dCR v, DoublePrecision prec = DoublePrecision::Default) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DPoint3dCR v, DoublePrecision prec = DoublePrecision::Default) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DVec2dCR v, DoublePrecision prec = DoublePrecision::Default) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, DVec3dCR v, DoublePrecision prec = DoublePrecision::Default) = 0;
    virtual DwgDbStatus     _Write (DxfGroupCode code, double x, double y, double z, DoublePrecision prec = DoublePrecision::Default) = 0;  // scales
    // WIP - implement Read...
    };  // IDxfFiler
DEFINE_NO_NAMESPACE_TYPEDEFS (IDxfFiler)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgDbHandle : public DWGDB_EXTENDCLASS(Handle)
    {
public:
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(Handle))
    DWGDB_ADD_CONSTRUCTORS (Handle)
    DwgDbHandle (DwgDbUInt64 id) : T_Super(id) {}

    DWGDB_EXPORT uint64_t           AsUInt64 () const;
    DWGDB_EXPORT DwgString          AsAscii () const;
    DWGDB_EXPORT bool               IsNull () const;
    DWGDB_EXPORT void               SetNull ();
    DWGDB_EXPORT DwgDbHandleR       operator = (DWGDB_TypeCR(Handle) h);
    };  // DwgDbHandle

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgDbObjectId : public DWGDB_EXTENDCLASS(ObjectId)
    {
public:
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(ObjectId))
    DWGDB_ADD_CONSTRUCTORS (ObjectId)
    DwgDbObjectId (DwgDbObjectIdCR id) : T_Super(id) {}
    DwgDbObjectId (DWGDB_TypeP(Stub) stub) : T_Super(stub) {}

    DWGDB_EXPORT bool               IsNull () const;
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT DwgDbDatabaseP     GetDatabase () const;
    DWGDB_EXPORT DwgDbHandle        GetHandle () const;
    DWGDB_EXPORT DwgString          GetDwgClassName () const;
    DWGDB_EXPORT DWG_TypeP(RxClass) GetDwgClass () const;
    DWGDB_EXPORT bool               IsObjectDerivedFrom (DWG_TypeCP(RxClass) rxClass) const;
    DWGDB_EXPORT uint64_t           ToUInt64 () const;
    DWGDB_EXPORT DwgString          ToAscii () const;
    DWGDB_EXPORT DwgDbObjectP       OpenObject (DwgDbOpenMode mode = DwgDbOpenMode::ForRead, bool openErased = false) const;
    DWGDB_EXPORT DwgDbStatus        OpenObject (DwgDbObjectPtr& obj, DwgDbOpenMode mode = DwgDbOpenMode::ForRead, bool openErased = false) const;
    DWGDB_EXPORT void*              PeekStubPtr () const;
    DWGDB_EXPORT void               SetNull ();
    DWGDB_EXPORT bool               operator == (DwgDbObjectIdCR objectId) const;
    DWGDB_EXPORT bool               operator != (DwgDbObjectIdCR objectId) const;
    DWGDB_EXPORT bool               operator < (DwgDbObjectIdCR id) const;
    DWGDB_EXPORT bool               operator > (DwgDbObjectIdCR id) const;
    DWGDB_EXPORT DwgDbObjectIdR     operator = (DWGDB_TypeP(Stub) objectId);
    DWGDB_EXPORT DwgDbObjectIdR     operator = (DWGDB_TypeCR(ObjectId) objectId);
    };  // DwgDbObjectId

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbObject : public DWGDB_EXTENDCLASS(Object)
    {
public:
    DWGDB_DECLARE_BASECLASS_MEMBERS(Object)

    DWGDB_EXPORT bool               IsAProxy () const;
    DWGDB_EXPORT bool               IsPersistent () const;
    };  // DwgDbObject
DWGDB_DEFINE_OBJECTPTR (Object)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbObjectIterator : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectIteratorPtr           m_objectIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbObjectIterator (OdDbObjectIterator* newIter) : m_objectIterator(newIter) {}

//__PUBLISH_SECTION_END__
#elif DWGTOOLKIT_RealDwg
    AcDbObjectIterator*             m_objectIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbObjectIterator (AcDbObjectIterator* newIter) { m_objectIterator = newIter; }
    DwgDbObjectIterator () : m_objectIterator(nullptr) {}
//__PUBLISH_SECTION_END__
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__

    DWGDB_EXPORT ~DwgDbObjectIterator ();

public:
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT void               Start ();
    DWGDB_EXPORT void               Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT DwgDbEntityP       GetEntity ();
    DWGDB_EXPORT DwgDbObjectId      GetObjectId ();
    };  // DwgDbObjectIterator
typedef RefCountedPtr<DwgDbObjectIterator>  DwgDbObjectIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbObjectIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDictionaryIterator : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbDictionaryIteratorPtr       m_dictionaryIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbDictionaryIterator (OdDbDictionaryIterator* newIter) : m_dictionaryIterator(newIter) {}

//__PUBLISH_SECTION_END__
#elif DWGTOOLKIT_RealDwg
    AcDbDictionaryIterator*         m_dictionaryIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbDictionaryIterator (AcDbDictionaryIterator* newIter) { m_dictionaryIterator = newIter; }
    DwgDbDictionaryIterator () : m_dictionaryIterator(nullptr) {}
//__PUBLISH_SECTION_END__
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__

    DWGDB_EXPORT ~DwgDbDictionaryIterator ();

public:
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT void               Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT bool               SetPosition (DwgDbObjectIdCR id);
    DWGDB_EXPORT DwgDbObjectId      GetObjectId () const;
    DWGDB_EXPORT DwgString          GetName () const;
    };  // DwgDbDictionaryIterator
typedef RefCountedPtr<DwgDbDictionaryIterator>  DwgDbDictionaryIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbDictionaryIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDictionary : public DWGDB_EXTENDCLASS (Dictionary)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Dictionary)

    DWGDB_EXPORT DwgDbDictionaryIteratorPtr GetIterator () const;
    DWGDB_EXPORT DwgDbStatus                GetIdAt (DwgDbObjectIdR outId, DwgStringCR inName) const;
    DWGDB_EXPORT DwgDbStatus                GetNameAt (DwgStringR outName, DwgDbObjectIdCR inId) const;
    DWGDB_EXPORT bool                       Has (DwgStringCR name) const;
    DWGDB_EXPORT bool                       Has (DwgDbObjectIdCR id) const;
    //! Find an entry by name, and change it to a new name
    DWGDB_EXPORT DwgDbStatus                SetName (DwgStringCR oldName, DwgStringCR newName);
    //! Add a new entry to the dictionary
    //! @param[in] name A new entry name to be added
    //! @param[in] object A new entry to be added
    //! @param[out] entryId An optional object ID to be returned upon a successful add
    DWGDB_EXPORT DwgDbStatus                SetAt (DwgStringCR name, DwgDbObjectP object, DwgDbObjectIdP entryId = nullptr);
    DWGDB_EXPORT DwgDbStatus                Remove (DwgStringCR name);
    DWGDB_EXPORT DwgDbStatus                Remove (DwgDbObjectIdCR entryId);
    };  // DwgDbDictionary
DWGDB_DEFINE_OBJECTPTR (Dictionary)

class DwgGiMaterialColor;
class DwgGiMaterialMap;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbMaterial : public DWGDB_EXTENDCLASS (Material)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Material)

    enum ChannelFlags           // == AcGiMaterialTraits::ChannelFlags
        {
        None                    = 0x00000,
        UseDiffuse              = 0x00001,
        UseSpecular             = 0x00002,
        UseReflection           = 0x00004,
        UseOpacity              = 0x00008,
        UseBump                 = 0x00010,
        UseRefraction           = 0x00020,
        UseNormalMap            = 0x00040,
        UseAll                  = 0xfffff
        };

    enum IlluminationModel      // == AcGiMaterialTraits::IlluminationModel
        {
        BlinnShader             = 0,
        MetalShader             = 1,
        };

    enum Mode                   // == AcGiMaterialTraits::Mode
        {
        Realistic               = 0,
        Advanced                = 1,
        };

    enum LuminanceMode          // == AcGiMaterialTraits::LuminanceMode
        {
        SelfIllumination        = 0,
        Luminance               = 1,
        };

    enum NormalMapMethod        // == AcGiMaterialTraits::NormalMapMethod
        {
        TangentSpace
        };

    enum GlobalIlluminationMode // == AcGiMaterialTraits::GlobalIlluminationMode
        {
        GlobalIlluminationNone,
        GlobalIlluminationCast,
        GlobalIlluminationReceive,
        GlobalIlluminationCastAndReceive
        };

    enum FinalGatherMode        // == AcGiMaterialTraits::FinalGatherMode
        {
        FinalGatherNone,
        FinalGatherCast,
        FinalGatherReceive,
        FinalGatherCastAndReceive
        };

    DWGDB_EXPORT DwgString              GetName () const;
    DWGDB_EXPORT DwgString              GetDescription () const;
    DWGDB_EXPORT bool                   IsAnonymous () const;
    DWGDB_EXPORT bool                   IsTwoSided () const;
    DWGDB_EXPORT ChannelFlags           GetChannelFlags () const;
    DWGDB_EXPORT void                   GetAmbient (DwgGiMaterialColor& color) const;
    DWGDB_EXPORT void                   GetBump (DwgGiMaterialMap& map) const;
    DWGDB_EXPORT double                 GetColorBleedScale () const;
    DWGDB_EXPORT void                   GetDiffuse (DwgGiMaterialColor& color, DwgGiMaterialMap& map) const;
    DWGDB_EXPORT FinalGatherMode        GetFinalGather () const;
    DWGDB_EXPORT GlobalIlluminationMode GetGlobalIllumination () const;
    DWGDB_EXPORT IlluminationModel      GetIlluminationModel () const;
    DWGDB_EXPORT double                 GetSelfIllumination () const;
    DWGDB_EXPORT double                 GetIndirectBumpScale () const;
    DWGDB_EXPORT Mode                   GetMode () const;
    DWGDB_EXPORT DwgDbStatus            GetNormalMap (DwgGiMaterialMap& map, NormalMapMethod& method, double& strength) const;
    DWGDB_EXPORT double                 GetOpacity (DwgGiMaterialMap& map) const;
    DWGDB_EXPORT double                 GetReflectanceScale () const;
    DWGDB_EXPORT void                   GetReflection (DwgGiMaterialMap& map) const;
    DWGDB_EXPORT double                 GetReflectivity () const;
    DWGDB_EXPORT double                 GetRefraction (DwgGiMaterialMap& map) const;
    DWGDB_EXPORT DwgDbUnits             GetScaleUnits () const;
    DWGDB_EXPORT double                 GetShininess () const;
    DWGDB_EXPORT double                 GetSpecular (DwgGiMaterialColor& color, DwgGiMaterialMap& map) const;
    DWGDB_EXPORT double                 GetTranslucence () const;
    DWGDB_EXPORT double                 GetTransmittanceScale () const;
    };  // DwgDbMaterial
DWGDB_DEFINE_OBJECTPTR (Material)

class DwgGiSkyParameters;
class DwgGiShadowParameters;
struct DwgGiDrawable;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSun : public DWGDB_EXTENDCLASS(Sun)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Sun)

    DWGDB_EXPORT bool           IsOn () const;
    DWGDB_EXPORT bool           IsDayLightSavingOn () const;
    DWGDB_EXPORT double         GetAltitude () const;
    DWGDB_EXPORT double         GetAzimuth () const;
    DWGDB_EXPORT DwgDbDateCR    GetDateTime () const;
    DWGDB_EXPORT double         GetIntensity () const;
    DWGDB_EXPORT DwgCmColorCR   GetSunColor () const;
    DWGDB_EXPORT RgbFactor      GetSunColorPhotometric (double scale);
    DWGDB_EXPORT DVec3d         GetSunDirection () const;
    DWGDB_EXPORT DwgDbStatus    GetSkyParameters (DwgGiSkyParameters& params) const;
    DWGDB_EXPORT DwgDbStatus    GetShadowParameters (DwgGiShadowParameters& params) const;
    DWGDB_EXPORT RefCountedPtr<DwgGiDrawable>   GetDrawable ();
    };  // DwgDbSun
DWGDB_DEFINE_OBJECTPTR (Sun)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSkyBackground : public DWGDB_EXTENDCLASS(SkyBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SkyBackground)

    DWGDB_EXPORT DwgDbObjectId  GetSunId () const;
    };  // DwgDbSkyBackground
DWGDB_DEFINE_OBJECTPTR (SkyBackground)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbGradientBackground : public DWGDB_EXTENDCLASS(GradientBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(GradientBackground)

    DWGDB_EXPORT DwgCmEntityColor   GetColorTop () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorMiddle () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorBottom () const;
    DWGDB_EXPORT double             GetHorizon () const;
    DWGDB_EXPORT double             GetHeight () const;
    DWGDB_EXPORT double             GetRotation () const;
    };  // DwgDbGradientBackground
DWGDB_DEFINE_OBJECTPTR (GradientBackground)
                
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbGroundPlaneBackground : public DWGDB_EXTENDCLASS(GroundPlaneBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(GroundPlaneBackground)

    DWGDB_EXPORT DwgCmEntityColor   GetColorSkyZenith () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorSkyHorizon () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorUndergroundHorizon () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorUndergroundAzimuth () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorGroundPlaneNear () const;
    DWGDB_EXPORT DwgCmEntityColor   GetColorGroundPlaneFar () const;
    };  // DwgDbGroundPlaneBackground
DWGDB_DEFINE_OBJECTPTR (GroundPlaneBackground)
                
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbIBLBackground : public DWGDB_EXTENDCLASS(IBLBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(IBLBackground)

    //! Is the Image Based Light(IBL) enabled?  When false, the background will be default black transparent.
    DWGDB_EXPORT bool       IsEnabled () const;
    //! Is the IBL image displayed in the background?
    DWGDB_EXPORT bool       IsImageDisplayed () const;
    //! Get the image used for the IBL
    DWGDB_EXPORT DwgString  GetIBLImageName () const;
    DWGDB_EXPORT double     GetRotation () const;
    //! The background to be displayed when IsImageDisplayed is false.
    DWGDB_EXPORT DwgDbObjectId  GetSecondaryBackground () const;
    };  // DwgDbIBLBackground
DWGDB_DEFINE_OBJECTPTR (IBLBackground)
                
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbImageBackground : public DWGDB_EXTENDCLASS(ImageBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(ImageBackground)

    DWGDB_EXPORT DwgString      GetImageFileName () const;
    DWGDB_EXPORT bool           IsToFitScreen () const;
    DWGDB_EXPORT bool           IsToMaintainAspectRatio () const;
    DWGDB_EXPORT bool           IsToUseTiling () const;
    DWGDB_EXPORT DPoint2d       GetOffset () const;
    DWGDB_EXPORT DPoint2d       GetScale () const;
    };  // DwgDbImageBackground
DWGDB_DEFINE_OBJECTPTR (ImageBackground)
                
/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSolidBackground : public DWGDB_EXTENDCLASS(SolidBackground)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SolidBackground)

    //! The solor color for the background.
    DWGDB_EXPORT DwgCmEntityColor   GetColorSolid () const;
    };  // DwgDbSolidBackground
DWGDB_DEFINE_OBJECTPTR (SolidBackground)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbVisualStyle : public DWGDB_EXTENDCLASS(VisualStyle)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(VisualStyle)

    DWGDB_EXPORT DwgString              GetName () const;
    DWGDB_EXPORT DwgString              GetDescription () const;
    DWGDB_EXPORT DwgGiVisualStyle::RenderType   GetType () const;
    //! Gets a property of the visual style.  
    //! @param prop A DwgGiVisualStyleProperties::Property to get from the visual style.
    //! @param op A DwgGiVisualStyleOperations::Operation to get currently in effect for this property. If nullptr, nothing is returned.
    //! @return The DwgGiVariant value of the property if successful; otherwise, an AcGiVariant of type DwgGiVariant::Undefined.
    DWGDB_EXPORT DwgGiVariantCR     GetTrait (DwgGiVisualStyleProperties::Property prop, DwgGiVisualStyleOperations::Operation* op = nullptr) const;
    //! Gets a property flag from the visual style, for properties which are bitfield enums.
    //! @param prop A bitfield enum DwgGiVisualStyleProperties::Property to get from the visual style.
    //! @param flags A bitfield flag enum property to get from the visual style.
    DWGDB_EXPORT bool               GetTraitFlag (DwgGiVisualStyleProperties::Property prop, DwgDbUInt32 flags) const;
    };  // DwgDbVisualStyle
DWGDB_DEFINE_OBJECTPTR (VisualStyle)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayout : public DWGDB_EXTENDCLASS(Layout)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Layout)

    enum PaperOrientation
        {
        RotateNone      = DWGDB_Type(PlotSettings::PlotRotation::k0degrees),
        RotateBy90      = DWGDB_Type(PlotSettings::PlotRotation::k90degrees),
        RotateBy180     = DWGDB_Type(PlotSettings::PlotRotation::k180degrees),
        RotateBy270     = DWGDB_Type(PlotSettings::PlotRotation::k270degrees),
        };

    enum PlotBy
        {
        Display         = DWGDB_Type(PlotSettings::PlotType::kDisplay),
        Extents         = DWGDB_Type(PlotSettings::PlotType::kExtents),
        Limits          = DWGDB_Type(PlotSettings::PlotType::kLimits),
        View            = DWGDB_Type(PlotSettings::PlotType::kView),
        Window          = DWGDB_Type(PlotSettings::PlotType::kWindow),
        Layout          = DWGDB_Type(PlotSettings::PlotType::kLayout),
        };

    DWGDB_EXPORT DwgString              GetName () const;
    DWGDB_EXPORT DRange3d               GetExtents () const;
    DWGDB_EXPORT DRange2d               GetLimits () const;
    DWGDB_EXPORT int32_t                GetTabOrder () const;
    DWGDB_EXPORT bool                   IsAnnoAllVisible() const;
    DWGDB_EXPORT uint32_t               GetViewports (DwgDbObjectIdArrayR vportIds) const;
    DWGDB_EXPORT DwgDbObjectId          GetBlockTableRecordId () const;
    DWGDB_EXPORT DwgDbUnits             GetPlotPaperUnits () const;
    DWGDB_EXPORT DwgDbStatus            GetPlotPaperSize (DPoint2dR origin) const;
    DWGDB_EXPORT DwgDbStatus            GetPlotPaperMargins (double& left, double& bottom, double& right, double& top) const;
    //! Get the plot origin in mm, an offset applied to the printable origin set by a user via the Page Setup Manager in ACAD.
    DWGDB_EXPORT DwgDbStatus            GetPlotOrigin (DPoint2dR) const;
    DWGDB_EXPORT bool                   IsStandardScale () const;
    DWGDB_EXPORT double                 GetStandardScale () const;
    DWGDB_EXPORT double                 GetCustomScale () const;
    DWGDB_EXPORT DwgDbStatus            GetCustomScale (double& numerator, double& denominator) const;
    DWGDB_EXPORT PaperOrientation       GetPaperOrientation () const;
    DWGDB_EXPORT PlotBy                 GetPlotBy () const;
    //! Get the printable origin of the paper set up for the layout, always in mm.
    //! @note   DXF group codes 148 & 149 are stored with inverted sign. The origin returned by this method is negated, such that the coordinate readout is consistent in ACAD.
    DWGDB_EXPORT DwgDbStatus            GetPaperImageOrigin (DPoint2dR origin) const;
    };  // DwgDbLayout
DWGDB_DEFINE_OBJECTPTR (Layout)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayoutManager : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbLayoutManagerPtr    m_layoutManager;
#elif DWGTOOLKIT_RealDwg
    AcDbLayoutManager*      m_layoutManager;
public:
    DwgDbLayoutManager () : m_layoutManager(nullptr) {}
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__
public:
    //! Constructors
    DWGDB_EXPORT DwgDbLayoutManager (DWGDB_TypeP(LayoutManager) manager);
    
    DWGDB_EXPORT bool   IsValid () const;
    DWGDB_EXPORT int    CountLayouts (DwgDbDatabaseP dwg) const;
    DWGDB_EXPORT DwgDbObjectId  FindLayoutByName (DwgStringCR name, DwgDbDatabaseCP dwg) const;
    DWGDB_EXPORT DwgDbObjectId  FindActiveLayout (DwgDbDatabaseCP dwg) const;
    DWGDB_EXPORT DwgDbObjectId  GetActiveLayoutBlock (DwgDbDatabaseCP dwg) const;
    //! Switch active layout to layoutId.
    //! @note Make sure the layout block is closed before calling this method as otherwise this call fails in RealDWG.
    DWGDB_EXPORT DwgDbStatus    ActivateLayout (DwgDbObjectId layoutId);
    DWGDB_EXPORT DwgDbStatus    CreateLayout (DwgDbObjectIdR layoutId, DwgDbObjectIdR blockId, DwgStringCR name, DwgDbDatabaseP);
    DWGDB_EXPORT DwgDbStatus    DeleteLayout (DwgStringCR name, DwgDbDatabaseP dwg);
    DWGDB_EXPORT DwgDbStatus    RenameLayout (DwgStringCR oldName, DwgStringCR newName, DwgDbDatabaseP dwg);
    };  // DwgDbLayoutManager
typedef RefCountedPtr<DwgDbLayoutManager> DwgDbLayoutManagerPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbLayoutManager)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSpatialFilter : public DWGDB_EXTENDCLASS(SpatialFilter)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SpatialFilter)

    DWGDB_EXPORT DwgDbStatus        GetDefinition (DPoint2dArrayR points, DVec3dR normal, double& elevation, double& frontClip, double& backClip, bool& enabled) const;
    DWGDB_EXPORT DwgDbStatus        SetDefinition (DPoint2dArrayCR points, DVec3dCR normal, double elevation, double frontClip, double backClip, bool enabled);
    DWGDB_EXPORT TransformR         GetClipTransform (TransformR t) const;
    DWGDB_EXPORT TransformR         GetBlockTransform (TransformR t) const;
    DWGDB_EXPORT DwgDbStatus        GetVolume (DPoint3dR from, DPoint3dR to, DVec3dR up, DVec2dR viewField) const;
    DWGDB_EXPORT bool               IsInverted () const;
    DWGDB_EXPORT DwgDbStatus        SetInverted (bool inverted);
    //! Check entity: true if the whole entity is outside of the clipper; false if inside or intersecting the clipper:
    DWGDB_EXPORT bool               IsEntityFilteredOut (DwgDbEntityCR entity) const;
    };  // DwgDbSpatialFilter
DWGDB_DEFINE_OBJECTPTR (SpatialFilter)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbFilteredBlockIterator : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbFilteredBlockIteratorPtr     m_filteredBlockIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbFilteredBlockIterator (OdDbFilteredBlockIterator* newIter) : m_filteredBlockIterator(newIter) {}

//__PUBLISH_SECTION_END__
#elif DWGTOOLKIT_RealDwg
    AcDbFilteredBlockIterator*       m_filteredBlockIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbFilteredBlockIterator (AcDbFilteredBlockIterator* newIter) { m_filteredBlockIterator = newIter; }
    DwgDbFilteredBlockIterator () : m_filteredBlockIterator(nullptr) {}
//__PUBLISH_SECTION_END__
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__

    DWGDB_EXPORT ~DwgDbFilteredBlockIterator ();

    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT DwgDbStatus        Start ();
    DWGDB_EXPORT DwgDbObjectId      Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT DwgDbStatus        Seek (DwgDbObjectId id);
    DWGDB_EXPORT DwgDbObjectId      GetEntityId () const;
    };  // DwgDbFilteredBlockIterator
typedef RefCountedPtr<DwgDbFilteredBlockIterator>  DwgDbFilteredBlockIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbFilteredBlockIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSpatialIndex : public DWGDB_EXTENDCLASS(SpatialIndex)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SpatialIndex)

    DWGDB_EXPORT DwgDbFilteredBlockIteratorPtr  NewIterator (DwgDbSpatialFilterCP filter) const;
    };  // DwgDbSpatialIndex
DWGDB_DEFINE_OBJECTPTR (SpatialIndex)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSortentsTable : public DWGDB_EXTENDCLASS(SortentsTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SortentsTable)

    DWGDB_EXPORT DwgDbStatus        GetFullDrawOrder (DwgDbObjectIdArrayR sortedOut, bool checkSORTENTS=false) const;
    };  // DwgDbSortentsTable
DWGDB_DEFINE_OBJECTPTR (SortentsTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbXrecord : public DWGDB_EXTENDCLASS(Xrecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Xrecord)

    DWGDB_EXPORT DwgResBufIterator  GetRbChain (DwgDbDatabaseP = nullptr, DwgDbStatus* status = nullptr) const;
    };  // DwgDbXrecord
DWGDB_DEFINE_OBJECTPTR (Xrecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/18
+===============+===============+===============+===============+===============+======*/
class DwgDbXrefGraphNode : public DWGDB_EXTENDCLASS(XrefGraphNode)
    {
public:
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(XrefGraphNode))

    DWGDB_EXPORT DwgDbXrefGraphNode (WCharCP name = nullptr, DwgDbObjectIdCR id = DwgDbObjectId(), DwgDbDatabaseP dwg = nullptr, DwgDbXrefStatus status = DwgDbXrefStatus::Resolved);

    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetIncomingNode (int index) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetOutgoingNode (int index) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetCycleIn (int index) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetCycleOut (int index) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetNextCycleNode () const;
    DWGDB_EXPORT int                    GetNumIncoming () const;
    DWGDB_EXPORT int                    GetNumOutgoing () const;
    DWGDB_EXPORT int                    GetNumCycleIn () const;
    DWGDB_EXPORT int                    GetNumCycleOut () const;
    DWGDB_EXPORT DwgDbXrefStatus        GetXrefStatus () const;
    DWGDB_EXPORT DwgString      GetName () const;
    DWGDB_EXPORT DwgDbObjectId  GetBlockId () const;
    DWGDB_EXPORT DwgDbDatabaseP GetDatabase () const;
    DWGDB_EXPORT bool           IsCycleNode () const;
    DWGDB_EXPORT bool           IsNested () const;
    DWGDB_EXPORT void           SetName (DwgStringCR name);
    DWGDB_EXPORT void           SetBlockId (DwgDbObjectIdCR id);
    DWGDB_EXPORT void           SetDatabase (DwgDbDatabaseP dwg);
    };  // DwgDbXrefGraphNode
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbXrefGraphNode)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/18
+===============+===============+===============+===============+===============+======*/
class DwgDbXrefGraph : public DWGDB_EXTENDCLASS(XrefGraph)
    {
//__PUBLISH_SECTION_END__
private:
    // Only effective for RealDWG if xref's are resolved by acdbResolveCurrentXRefs, but database's created this way are NOT ref counted!
    DWGDB_EXPORT static DwgDbStatus     Build (DwgDbXrefGraph& graphOut, DwgDbDatabaseP hostDwg, bool includeGhosts = false);
//__PUBLISH_SECTION_START__
public:
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(XrefGraph))

    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetXrefNode (DwgStringCR name) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetXrefNode (DwgDbObjectIdCR blockId) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetXrefNode (DwgDbDatabaseP dwg) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetXrefNode (int index) const;
    DWGDB_EXPORT DwgDbXrefGraphNodeP    GetHostDwg () const;
    DWGDB_EXPORT bool   IsEmpty () const;
    DWGDB_EXPORT size_t GetNodeCount () const;
    DWGDB_EXPORT bool   MarkUnresolvedTrees ();
    DWGDB_EXPORT bool   FindCycles (DwgDbXrefGraphNodeP start = nullptr);
    DWGDB_EXPORT void   Reset ();
    };  // DwgDbXrefGraph
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbXrefGraph)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbGroupIterator : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbGroupIteratorPtr       m_groupIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbGroupIterator (OdDbGroupIterator* newIter) : m_groupIterator(newIter) {}

//__PUBLISH_SECTION_END__
#elif DWGTOOLKIT_RealDwg
    AcDbGroupIterator*         m_groupIterator;
//__PUBLISH_SECTION_START__

public:
    DwgDbGroupIterator (AcDbGroupIterator* newIter) { m_groupIterator = newIter; }
    DwgDbGroupIterator () : m_groupIterator(nullptr) {}
//__PUBLISH_SECTION_END__
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__

    DWGDB_EXPORT ~DwgDbGroupIterator ();

public:
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT void               Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT DwgDbObjectId      GetObjectId () const;
    };  // DwgDbGroupIterator
typedef RefCountedPtr<DwgDbGroupIterator>  DwgDbGroupIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbGroupIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/18
+===============+===============+===============+===============+===============+======*/
class DwgDbGroup : public DWGDB_EXTENDCLASS(Group)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(Group)

    DWGDB_EXPORT  DwgDbGroupIteratorPtr GetIterator ();
    DWGDB_EXPORT  DwgDbStatus   Clear ();
    DWGDB_EXPORT  DwgString     GetName () const;
    DWGDB_EXPORT  DwgString     GetDescription () const;
    DWGDB_EXPORT  bool          IsAnonymous () const;
    DWGDB_EXPORT  bool          IsSelectable () const;
    DWGDB_EXPORT  bool          IsNotAccessible () const;
    DWGDB_EXPORT  bool          Has (DwgDbEntityCP entity) const;
    DWGDB_EXPORT  size_t        GetAllEntityIds (DwgDbObjectIdArrayR ids) const;
    DWGDB_EXPORT  size_t        GetNumEntities () const;
    DWGDB_EXPORT  DwgDbStatus   GetIndex (DwgDbObjectIdCR id, size_t& out) const;
    DWGDB_EXPORT  DwgDbStatus   Append (DwgDbObjectIdCR id);
    DWGDB_EXPORT  DwgDbStatus   Append (DwgDbObjectIdArrayCR ids);
    DWGDB_EXPORT  DwgDbStatus   Prepend (DwgDbObjectIdCR id);
    DWGDB_EXPORT  DwgDbStatus   Prepend (DwgDbObjectIdArrayCR ids);
    DWGDB_EXPORT  DwgDbStatus   InsertAt (size_t at, DwgDbObjectIdCR id);
    DWGDB_EXPORT  DwgDbStatus   InsertAt (size_t at, DwgDbObjectIdArrayCR ids);
    DWGDB_EXPORT  DwgDbStatus   Remove (DwgDbObjectIdCR id);
    DWGDB_EXPORT  DwgDbStatus   Remove (DwgDbObjectIdArrayCR ids);
    DWGDB_EXPORT  DwgDbStatus   RemoveAt (size_t at);
    DWGDB_EXPORT  DwgDbStatus   RemoveAt (size_t at, DwgDbObjectIdArrayCR ids);
    DWGDB_EXPORT  DwgDbStatus   Replace (DwgDbObjectIdCR oldId, DwgDbObjectIdCR newId);
    DWGDB_EXPORT  DwgDbStatus   Reverse ();
    DWGDB_EXPORT  DwgDbStatus   SetName (DwgStringCR name);
    DWGDB_EXPORT  DwgDbStatus   SetDescription (DwgStringCR descr);
    DWGDB_EXPORT  DwgDbStatus   SetColor (DwgCmColorCR color);
    DWGDB_EXPORT  DwgDbStatus   SetColorIndex (uint16_t color);
    DWGDB_EXPORT  DwgDbStatus   SetLayer (DwgDbObjectIdCR layerId);
    DWGDB_EXPORT  DwgDbStatus   SetLinetype (DwgDbObjectIdCR linetypeId);
    DWGDB_EXPORT  DwgDbStatus   SetLinetypeScale (double scale);
    DWGDB_EXPORT  DwgDbStatus   SetMaterial (DwgDbObjectIdCR materialId);
    DWGDB_EXPORT  DwgDbStatus   SetSelectable (bool selectable);
    DWGDB_EXPORT  DwgDbStatus   SetVisibility (DwgDbVisibility visibility);
    DWGDB_EXPORT  DwgDbStatus   Transfer (size_t from, size_t to, size_t num);
    };  // DwgDbGroup
DWGDB_DEFINE_OBJECTPTR (Group)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
