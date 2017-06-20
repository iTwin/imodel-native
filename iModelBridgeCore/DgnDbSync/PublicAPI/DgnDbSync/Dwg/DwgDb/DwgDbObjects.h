/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/DwgDbObjects.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/DbObjectId.h>
#include    <Teigha/Core/Include/DbDatabase.h>
#include    <Teigha/Core/Include/DbDictionary.h>
#include    <Teigha/Core/Include/DbMaterial.h>
#include    <Teigha/Core/Include/DbVisualStyle.h>
#include    <Teigha/Core/Include/DbSpatialFilter.h>
#include    <Teigha/Core/Include/DbSpatialIndex.h>
#include    <Teigha/Core/Include/DbSortentsTable.h>
#include    <Teigha/Core/Include/DbObjectIterator.h>

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
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(Handle))
public:
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
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(ObjectId))
public:
    DWGDB_ADD_CONSTRUCTORS (ObjectId)
    DwgDbObjectId (DwgDbObjectIdCR id) : T_Super(id) {}
    DwgDbObjectId (DWGDB_TypeP(Stub) stub) : T_Super(stub) {}

    DWGDB_EXPORT bool               IsNull () const;
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT DwgDbDatabaseP     GetDatabase () const;
    DWGDB_EXPORT DwgDbHandle        GetHandle () const;
    DWGDB_EXPORT DwgString          GetClassName () const;
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
    DWGDB_DECLARE_COMMON_MEMBERS(Object)

public:
    DWGDB_EXPORT bool               IsAProxy () const;
    DWGDB_EXPORT bool               IsPersistent () const;
    };  // DwgDbObject
DWGDB_DEFINE_OBJECTPTR (Object)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbObjectIterator
    {
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectIteratorPtr           m_objectIterator;

public:
    DwgDbObjectIterator (OdDbObjectIterator* newIter) : m_objectIterator(newIter) {}

#elif DWGTOOLKIT_RealDwg
    AcDbObjectIterator*             m_objectIterator;

public:
    DwgDbObjectIterator (AcDbObjectIterator* newIter) { m_objectIterator = newIter; }
    DwgDbObjectIterator () : m_objectIterator(nullptr) {}
#endif  // DWGTOOLKIT_

    DWGDB_EXPORT ~DwgDbObjectIterator ();

public:
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT void               Start ();
    DWGDB_EXPORT void               Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT DwgDbEntityP       GetEntity ();
    DWGDB_EXPORT DwgDbObjectId      GetObjectId ();
    };  // DwgDbObjectIterator

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDictionaryIterator
    {
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbDictionaryIteratorPtr       m_dictionaryIterator;

public:
    DwgDbDictionaryIterator (OdDbDictionaryIterator* newIter) : m_dictionaryIterator(newIter) {}

#elif DWGTOOLKIT_RealDwg
    AcDbDictionaryIterator*         m_dictionaryIterator;

public:
    DwgDbDictionaryIterator (AcDbDictionaryIterator* newIter) { m_dictionaryIterator = newIter; }
    DwgDbDictionaryIterator () : m_dictionaryIterator(nullptr) {}
#endif  // DWGTOOLKIT_

    DWGDB_EXPORT ~DwgDbDictionaryIterator ();

public:
    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT void               Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT bool               SetPosition (DwgDbObjectIdCR id);
    DWGDB_EXPORT DwgDbObjectId      GetObjectId () const;
    DWGDB_EXPORT DwgString          GetName () const;
    };  // DwgDbDictionaryIterator

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbDictionary : public DWGDB_EXTENDCLASS (Dictionary)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Dictionary)

public:
    DWGDB_EXPORT DwgDbDictionaryIterator    GetIterator () const;
    DWGDB_EXPORT DwgDbStatus                GetIdAt (DwgDbObjectIdR outId, DwgStringCR inName) const;
    DWGDB_EXPORT DwgDbStatus                GetNameAt (DwgStringR outName, DwgDbObjectIdCR inId) const;
    DWGDB_EXPORT bool                       Has (DwgStringCR name) const;
    DWGDB_EXPORT bool                       Has (DwgDbObjectIdCR id) const;
    };  // DwgDbDictionary
DWGDB_DEFINE_OBJECTPTR (Dictionary)

class DwgGiMaterialColor;
class DwgGiMaterialMap;

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/16
+===============+===============+===============+===============+===============+======*/
class DwgDbMaterial : public DWGDB_EXTENDCLASS (Material)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Material)

public:
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

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbVisualStyle : public DWGDB_EXTENDCLASS(VisualStyle)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(VisualStyle)

public:
    DWGDB_EXPORT DwgString              GetName () const;
    DWGDB_EXPORT DwgString              GetDescription () const;
    DWGDB_EXPORT DwgDbVisualStyleType   GetType () const;
    };  // DwgDbVisualStyle
DWGDB_DEFINE_OBJECTPTR (VisualStyle)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayout : public DWGDB_EXTENDCLASS(Layout)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(Layout)

public:
    enum PaperOrientation
        {
        RotateNone      = DWGDB_Type(PlotSettings::PlotRotation::k0degrees),
        RotateBy90      = DWGDB_Type(PlotSettings::PlotRotation::k90degrees),
        RotateBy180     = DWGDB_Type(PlotSettings::PlotRotation::k180degrees),
        RotateBy270     = DWGDB_Type(PlotSettings::PlotRotation::k270degrees),
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
    DWGDB_EXPORT DwgDbStatus            GetPlotOrigin (DPoint2dR) const;
    DWGDB_EXPORT bool                   IsStandardScale () const;
    DWGDB_EXPORT double                 GetStandardScale () const;
    DWGDB_EXPORT double                 GetCustomScale () const;
    DWGDB_EXPORT DwgDbStatus            GetCustomScale (double& numerator, double& denominator) const;
    DWGDB_EXPORT PaperOrientation       GetPaperOrientation () const;
    };  // DwgDbLayout
DWGDB_DEFINE_OBJECTPTR (Layout)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSpatialFilter : public DWGDB_EXTENDCLASS(SpatialFilter)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(SpatialFilter)

public:
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
class DwgDbFilteredBlockIterator
    {
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbFilteredBlockIteratorPtr     m_filteredBlockIterator;

public:
    DwgDbFilteredBlockIterator (OdDbFilteredBlockIterator* newIter) : m_filteredBlockIterator(newIter) {}

#elif DWGTOOLKIT_RealDwg
    AcDbFilteredBlockIterator*       m_filteredBlockIterator;

public:
    DwgDbFilteredBlockIterator (AcDbFilteredBlockIterator* newIter) { m_filteredBlockIterator = newIter; }
    DwgDbFilteredBlockIterator () : m_filteredBlockIterator(nullptr) {}
#endif  // DWGTOOLKIT_

    DWGDB_EXPORT ~DwgDbFilteredBlockIterator ();

    DWGDB_EXPORT bool               IsValid () const;
    DWGDB_EXPORT DwgDbStatus        Start ();
    DWGDB_EXPORT DwgDbObjectId      Next ();
    DWGDB_EXPORT bool               Done () const;
    DWGDB_EXPORT DwgDbStatus        Seek (DwgDbObjectId id);
    DWGDB_EXPORT DwgDbObjectId      GetEntityId () const;
    };  // DwgDbFilteredBlockIterator

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSpatialIndex : public DWGDB_EXTENDCLASS(SpatialIndex)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(SpatialIndex)

public:
    DWGDB_EXPORT DwgDbFilteredBlockIterator  NewIterator (DwgDbSpatialFilterCP filter) const;
    };  // DwgDbSpatialIndex
DWGDB_DEFINE_OBJECTPTR (SpatialIndex)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSortentsTable : public DWGDB_EXTENDCLASS(SortentsTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(SortentsTable)

public:
    DWGDB_EXPORT DwgDbStatus        GetFullDrawOrder (DwgDbObjectIdArrayR sortedOut, bool checkSORTENTS=false) const;
    };  // DwgDbSortentsTable
DWGDB_DEFINE_OBJECTPTR (SortentsTable)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
