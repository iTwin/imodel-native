/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/DwgDbSymbolTables.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>
#include    <DgnDbSync/Dwg/DwgDb/BasicTypes.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/Tables.h>

#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/dbsymtb.h>

#else
    #error  "Must define DWGTOOLKIT!!!"
#endif  // DWGTOOLKIT_

#include    <Geom/GeomApi.h>


BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgFontInfo
    {
    WString         m_typeface;
    WString         m_shxFontName;
    WString         m_bigFontName;
    bool            m_bold;
    bool            m_italic;
    int32_t         m_charset;
    int32_t         m_pitchAndFamily;

    DwgFontInfo () : m_bold(false), m_italic(false), m_charset(0), m_pitchAndFamily(0) { }

    WStringCR   GetTypeFace () const { return m_typeface; }
    WStringCR   GetShxFontName () const { return m_shxFontName; }
    WStringCR   GetBigFontName () const { return m_bigFontName; }
    bool        IsBold () const { return m_bold; }
    bool        IsItalic () const { return m_italic; }
    int32_t     GetCharacterSet () const { return m_charset; }
    int32_t     GetPitchAndCharFamily () const { return m_pitchAndFamily; }
    void        Set (WCharCP tface, WCharCP shx, WCharCP bigf, bool bold, bool it, int32_t charset, int32_t pnf)
        {
        m_typeface = tface;
        m_shxFontName = shx;
        m_bigFontName = bigf;
        m_bold = bold;
        m_italic = it;
        m_charset = charset;
        m_pitchAndFamily = pnf;
        }
    };  // DwgFontInfo
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgFontInfo)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgDbSymbolTableIterator
    {
private:
    DwgDbSymbolTableIterator& operator = (DwgDbSymbolTableIterator& iter);

#ifdef DWGTOOLKIT_OpenDwg
    OdDbSymbolTableIteratorPtr          m_symbolTableIterator;

public:
    DwgDbSymbolTableIterator (OdDbSymbolTableIterator* newIter) : m_symbolTableIterator(newIter) {}
    DwgDbSymbolTableIterator () : m_symbolTableIterator() {}
#elif DWGTOOLKIT_RealDwg
    AcDbSymbolTableIterator*            m_symbolTableIterator;

public:
    EXPORT DwgDbSymbolTableIterator (AcDbSymbolTableIterator* newIter) : m_symbolTableIterator(newIter) {}
    DwgDbSymbolTableIterator () : m_symbolTableIterator(nullptr) {}
#endif  // DWGTOOLKIT_

    DWGDB_EXPORT ~DwgDbSymbolTableIterator ();

    DWGDB_EXPORT bool                   IsValid () const { return nullptr != m_symbolTableIterator; }
    DWGDB_EXPORT void                   Start () { if (nullptr != m_symbolTableIterator) m_symbolTableIterator->start(); }
    DWGDB_EXPORT void                   Step () { if (nullptr != m_symbolTableIterator) m_symbolTableIterator->step(); }
    DWGDB_EXPORT bool                   Done () const { return nullptr != m_symbolTableIterator ? m_symbolTableIterator->done() : true; }
    DWGDB_EXPORT DwgDbObjectId          GetRecordId () const;
    };  // DwgDbSymbolTableIterator

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgDbBlockChildIterator
    {
private:
#ifdef DWGTOOLKIT_OpenDwg
    OdDbObjectIteratorPtr               m_blockChildIterator;

public:
    DwgDbBlockChildIterator (OdDbObjectIterator* newIter) : m_blockChildIterator(newIter) {}
    DwgDbBlockChildIterator () : m_blockChildIterator() {}
#elif DWGTOOLKIT_RealDwg
    AcDbBlockTableRecordIterator*       m_blockChildIterator;

public:
    DwgDbBlockChildIterator (AcDbBlockTableRecordIterator* newIter) : m_blockChildIterator(newIter) {}
    DwgDbBlockChildIterator () : m_blockChildIterator(nullptr) {}
#endif  // DWGTOOLKIT_
    
    DWGDB_EXPORT ~DwgDbBlockChildIterator ();

    DWGDB_EXPORT bool                   IsValid () const { return nullptr != m_blockChildIterator; }
    DWGDB_EXPORT void                   Start () { if (nullptr != m_blockChildIterator) m_blockChildIterator->start(); }
    DWGDB_EXPORT void                   Step () { if (nullptr != m_blockChildIterator) m_blockChildIterator->step(); }
    DWGDB_EXPORT bool                   Done () const { return nullptr != m_blockChildIterator ? m_blockChildIterator->done() : true; }
    DWGDB_EXPORT DwgDbObjectId          GetEntityId () const;
    };  // DwgDbBlockChildIterator

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSymbolTableRecord : public DWGDB_EXTENDCLASS(SymbolTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(SymbolTableRecord)

public:
    DWGDB_EXPORT DwgString              GetName () const;
    };  // DwgDbSymbolTableRecord
DWGDB_DEFINE_OBJECTPTR (SymbolTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSymbolTable : public DWGDB_EXTENDCLASS(SymbolTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(SymbolTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    DWGDB_EXPORT DwgDbStatus                GetByName (DwgDbObjectIdR outId, WStringCR name, bool getErased = false) const;
    };  // DwgDbSymbolTable
DWGDB_DEFINE_OBJECTPTR (SymbolTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbBlockTableRecord : public DWGDB_EXTENDCLASS(BlockTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(BlockTableRecord)

public:
    DWGDB_EXPORT DwgString                  GetName () const;
    DWGDB_EXPORT DwgString                  GetComments () const;
    DWGDB_EXPORT bool                       IsLayout () const;
    DWGDB_EXPORT DwgDbObjectId              GetLayoutId () const;
    DWGDB_EXPORT bool                       IsModelspace () const;
    DWGDB_EXPORT bool                       IsPaperspace () const;
    DWGDB_EXPORT bool                       IsExternalReference () const;
    DWGDB_EXPORT bool                       IsOverlayReference () const;
    DWGDB_EXPORT bool                       IsAnonymous () const;
    DWGDB_EXPORT bool                       HasAttributeDefinitions () const;
    DWGDB_EXPORT DwgDbBlockChildIterator    GetBlockChildIterator () const;
    DWGDB_EXPORT DwgString                  GetPath () const;
    DWGDB_EXPORT DPoint3d                   GetBase () const;
    DWGDB_EXPORT DwgDbUnits                 GetINSUNITS () const;
    DWGDB_EXPORT DwgDbDatabaseP             GetXrefDatabase (bool includeUnresolved = false) const;
    DWGDB_EXPORT DwgDbStatus                GetBlockReferenceIds (DwgDbObjectIdArrayR ids, bool noNested = true, bool validate = false);
    DWGDB_EXPORT DwgDbStatus                OpenSpatialIndex (DwgDbSpatialIndexPtr& indexOut, DwgDbOpenMode mode) const;
    DWGDB_EXPORT DwgDbStatus                OpenSortentsTable (DwgDbSortentsTablePtr& sortentsOut, DwgDbOpenMode mode);
    };  // DwgDbBlockTableRecord
DWGDB_DEFINE_OBJECTPTR (BlockTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbBlockTable : public DWGDB_EXTENDCLASS(BlockTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(BlockTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    DWGDB_EXPORT DwgDbObjectId              GetModelspaceId () const;
    DWGDB_EXPORT DwgDbObjectId              GetPaperspaceId () const;
    };  // DwgDbBlockTable
DWGDB_DEFINE_OBJECTPTR (BlockTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayerTableRecord : public DWGDB_EXTENDCLASS(LayerTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(LayerTableRecord)

public:
    DWGDB_EXPORT DwgString                  GetName () const;
    DWGDB_EXPORT DwgString                  GetDescription () const;
    DWGDB_EXPORT bool                       IsOff () const;
    DWGDB_EXPORT bool                       IsFrozen () const;
    DWGDB_EXPORT bool                       IsLocked () const;
    DWGDB_EXPORT bool                       IsPlottable () const;
    DWGDB_EXPORT DwgDbObjectId              GetLinetypeId () const;
    DWGDB_EXPORT DwgDbObjectId              GetLinetypeId (bool& isOverridden, DwgDbObjectIdCR viewportId) const;
    DWGDB_EXPORT DwgDbObjectId              GetMaterialId () const;
    DWGDB_EXPORT DwgCmEntityColor           GetColor () const;
    DWGDB_EXPORT DwgCmEntityColor           GetColor (bool& isOverridden, DwgDbObjectIdCR viewportId) const;
    DWGDB_EXPORT DwgDbLineWeight            GetLineweight () const;
    DWGDB_EXPORT DwgDbLineWeight            GetLineweight (bool& isOverridden, DwgDbObjectIdCR viewportId) const;
    DWGDB_EXPORT DwgTransparency            GetTransparency () const;
    DWGDB_EXPORT DwgTransparency            GetTransparency (bool& isOverridden, DwgDbObjectIdCR viewportId) const;
    DWGDB_EXPORT bool                       HasOverrides (DwgDbObjectIdCR viewportId) const;
    };  // DwgDbLayerTableRecord
DWGDB_DEFINE_OBJECTPTR (LayerTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayerTable : public DWGDB_EXTENDCLASS(LayerTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(LayerTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    };  // DwgDbLayerTable
DWGDB_DEFINE_OBJECTPTR (LayerTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbViewportTableRecord : public DWGDB_EXTENDCLASS(ViewportTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(ViewportTableRecord)

public:
    DWGDB_EXPORT DwgString                  GetName () const;
    DWGDB_EXPORT bool                       IsGridEnabled () const;
    DWGDB_EXPORT bool                       IsUcsIconEnabled () const;
    DWGDB_EXPORT bool                       IsFrontClipEnabled () const;
    DWGDB_EXPORT bool                       IsFrontClipAtEye () const;
    DWGDB_EXPORT double                     GetFrontClipDistance () const;
    DWGDB_EXPORT bool                       IsBackClipEnabled () const;
    DWGDB_EXPORT double                     GetBackClipDistance () const;
    DWGDB_EXPORT bool                       IsPerspectiveEnabled () const;
    DWGDB_EXPORT bool                       IsDefaultLightingOn () const;
    DWGDB_EXPORT bool                       GetFillMode () const;
    DWGDB_EXPORT DwgDbObjectId              GetBackground () const;
    DWGDB_EXPORT DwgDbObjectId              GetVisualStyle () const;
    DWGDB_EXPORT DVec3d                     GetViewDirection () const;
    DWGDB_EXPORT DwgDbStatus                GetUcs (DPoint3dR origin, DVec3dR xAxis, DVec3d yAxis) const;
    DWGDB_EXPORT bool                       IsUcsSavedWithViewport () const;
    DWGDB_EXPORT double                     GetHeight () const;
    DWGDB_EXPORT double                     GetWidth () const;
    DWGDB_EXPORT double                     GetLensLength () const;
    DWGDB_EXPORT double                     GetViewTwist () const;
    DWGDB_EXPORT double                     GetUcsElevation () const;
    DWGDB_EXPORT DPoint2d                   GetCenterPoint () const;
    DWGDB_EXPORT DPoint3d                   GetTargetPoint () const;
    DWGDB_EXPORT DPoint2d                   GetLowerLeftCorner () const;
    DWGDB_EXPORT DPoint2d                   GetUpperRightCorner () const;
    DWGDB_EXPORT DPoint2d                   GetGridIncrements () const;
    DWGDB_EXPORT DPoint2d                   GetSnapIncrements () const;
    DWGDB_EXPORT DPoint2d                   GetSnapBase () const;
    DWGDB_EXPORT double                     GetSnapAngle () const;
    DWGDB_EXPORT SnapIsoPair                GetSnapPair () const;
    DWGDB_EXPORT bool                       IsSnapEnabled () const;
    DWGDB_EXPORT bool                       IsIsometricSnapEnabled () const;
    DWGDB_EXPORT int16_t                    GetGridMajor () const;
    };  // DwgDbViewportTableRecord
DWGDB_DEFINE_OBJECTPTR (ViewportTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbViewportTable : public DWGDB_EXTENDCLASS(ViewportTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(ViewportTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    };  // DwgDbViewportTable
DWGDB_DEFINE_OBJECTPTR (ViewportTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbTextStyleTableRecord : public DWGDB_EXTENDCLASS(TextStyleTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(TextStyleTableRecord)

public:
    DWGDB_EXPORT DwgDbStatus                GetFontInfo (DwgFontInfoR info) const;
    DWGDB_EXPORT WString                    GetFileName () const;
    DWGDB_EXPORT WString                    GetBigFontFileName () const;
    DWGDB_EXPORT double                     GetTextSize () const;
    DWGDB_EXPORT double                     GetWidthFactor () const;
    DWGDB_EXPORT double                     GetObliqueAngle () const;
    DWGDB_EXPORT bool                       IsVertical () const;
    DWGDB_EXPORT DwgString                  GetName () const;
    DWGDB_EXPORT bool                       IsShapeFile () const;
    DWGDB_EXPORT bool                       IsActiveTextStyle () const;
    };  // DwgDbTextStyleTableRecord
DWGDB_DEFINE_OBJECTPTR (TextStyleTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbTextStyleTable : public DWGDB_EXTENDCLASS(TextStyleTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(TextStyleTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    };  // DwgDbTextStyleTable
DWGDB_DEFINE_OBJECTPTR (TextStyleTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLinetypeTableRecord : public DWGDB_EXTENDCLASS(LinetypeTableRecord)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(LinetypeTableRecord)

public:
    DWGDB_EXPORT DwgString                  GetName () const;
    DWGDB_EXPORT DwgDbStatus                GetComments (DwgStringR comments) const;
    DWGDB_EXPORT uint32_t                   GetNumberOfDashes () const;
    DWGDB_EXPORT double                     GetPatternLength () const;
    DWGDB_EXPORT double                     GetDashLengthAt (uint32_t index) const;
    DWGDB_EXPORT uint32_t                   GetShapeNumberAt (uint32_t index) const;
    DWGDB_EXPORT DVec2d                     GetShapeOffsetAt (uint32_t index) const;
    DWGDB_EXPORT double                     GetShapeScaleAt (uint32_t index) const;
    DWGDB_EXPORT double                     GetShapeRotationAt (uint32_t index) const;
    DWGDB_EXPORT DwgDbStatus                GetTextAt (DwgStringR text, uint32_t index) const;
    DWGDB_EXPORT DwgDbObjectId              GetShapeStyleAt (uint32_t index) const;
    DWGDB_EXPORT bool                       IsShapeUcsOrientedAt (uint32_t index) const;
    DWGDB_EXPORT bool                       IsShapeUprightAt (uint32_t index) const;
    DWGDB_EXPORT bool                       IsScaledToFit () const;
    DWGDB_EXPORT bool                       IsByLayer() const;
    DWGDB_EXPORT bool                       IsByBlock() const;
    DWGDB_EXPORT bool                       IsContinuous () const;
    };  // DwgDbLineTypeTableRecord
DWGDB_DEFINE_OBJECTPTR (LinetypeTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLinetypeTable : public DWGDB_EXTENDCLASS(LinetypeTable)
    {
    DWGDB_DECLARE_COMMON_MEMBERS(LinetypeTable)

public:
    DWGDB_EXPORT DwgDbSymbolTableIterator   NewIterator(bool atBeginning = true, bool skipDeleted = true) const;
    };  // DwgDbLineTypeTable
DWGDB_DEFINE_OBJECTPTR (LinetypeTable)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
