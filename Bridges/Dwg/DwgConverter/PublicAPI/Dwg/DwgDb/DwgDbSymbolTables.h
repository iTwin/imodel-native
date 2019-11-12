/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <Dwg/DwgDb/DwgDbCommon.h>
#include    <Dwg/DwgDb/BasicTypes.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Drawing/Include/Tables.h>

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
struct DwgDbSymbolTableIterator : public RefCountedBase
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

public:
    DWGDB_EXPORT ~DwgDbSymbolTableIterator ();

    //! Check if the iterator is valid
    DWGDB_EXPORT bool                   IsValid () const { return nullptr != m_symbolTableIterator; }
    //! Set the iterator at begining of the table entries
    DWGDB_EXPORT void                   Start () { if (nullptr != m_symbolTableIterator) m_symbolTableIterator->start(); }
    //! Move the iterator to next table entry
    DWGDB_EXPORT void                   Step () { if (nullptr != m_symbolTableIterator) m_symbolTableIterator->step(); }
    //! Check if the iterator has reach the last table entry
    //! @return     True, has reached end of the table; false, otherwise
    DWGDB_EXPORT bool                   Done () const { return nullptr != m_symbolTableIterator ? m_symbolTableIterator->done() : true; }
    //! @return     The object ID of current table entry
    DWGDB_EXPORT DwgDbObjectId          GetRecordId () const;
    //! Include or exclude hidden layers in the iterator.
    //! @note   Valid only for layer tables, and using RealDWG toolkit.  Default to skip hidden layers.
    //! param[in] skip  True to exclude hidden layers from the layer table iterator(default); false to include hidden layers.
    DWGDB_EXPORT void   SetSkipHiddenLayers (bool skip);
    //! Are hidden layers excluded in the layer table iterator?
    DWGDB_EXPORT bool   GetSkipHiddenLayers () const;
    //! Include or exclude reconciled layers in the iterator.
    //! @note   Valid only for layer tables, and using RealDWG toolkit.  Default to skip reconciled layers.
    //! param[in] skip  True to exclude reconciled layers from the layer table iterator(default); false to include hidden layers.
    DWGDB_EXPORT void   SetSkipReconciledLayers (bool skip);
    //! Are reconciled layers excluded in the layer table iterator?
    DWGDB_EXPORT bool   GetSkipReconciledLayers () const;
    };  // DwgDbSymbolTableIterator
typedef RefCountedPtr<DwgDbSymbolTableIterator>     DwgDbSymbolTableIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbSymbolTableIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DwgDbBlockChildIterator : public RefCountedBase
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
    
public:
    DWGDB_EXPORT ~DwgDbBlockChildIterator ();

    DWGDB_EXPORT bool                   IsValid () const { return nullptr != m_blockChildIterator; }
    DWGDB_EXPORT void                   Start () { if (nullptr != m_blockChildIterator) m_blockChildIterator->start(); }
    DWGDB_EXPORT void                   Step () { if (nullptr != m_blockChildIterator) m_blockChildIterator->step(); }
    DWGDB_EXPORT bool                   Done () const { return nullptr != m_blockChildIterator ? m_blockChildIterator->done() : true; }
    DWGDB_EXPORT DwgDbObjectId          GetEntityId () const;
    };  // DwgDbBlockChildIterator
typedef RefCountedPtr<DwgDbBlockChildIterator>  DwgDbBlockChildIteratorPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbBlockChildIterator)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
#define DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()   \
    DWGDB_EXPORT DwgString  GetName () const;       \
    DWGDB_EXPORT bool       IsDependent () const;   \
    DWGDB_EXPORT bool       IsResolved () const;    \
    DWGDB_EXPORT DwgDbStatus SetName (DwgStringCR);

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSymbolTableRecord : public DWGDB_EXTENDCLASS(SymbolTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SymbolTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()
    };  // DwgDbSymbolTableRecord
DWGDB_DEFINE_OBJECTPTR (SymbolTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
#define DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(__name__)     \
    DWGDB_EXPORT DwgDbSymbolTableIteratorPtr NewIterator(bool atBeginning = true, bool skipDeleted = true) const;  \
    DWGDB_EXPORT DwgDbObjectId  GetByName (WStringCR name, bool getErased = false) const;   \
    DWGDB_EXPORT DwgDbObjectId  GetByName (DwgStringCR name, bool getErased = false) const;   \
    DWGDB_EXPORT bool Has (WStringCR name) const;  \
    DWGDB_EXPORT bool Has (DwgStringCR name) const;  \
    DWGDB_EXPORT bool Has (DwgDbObjectIdCR id) const;  \
    DWGDB_EXPORT DwgDbObjectId  Add (DwgDb##__name__##RecordP record);

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbSymbolTable : public DWGDB_EXTENDCLASS(SymbolTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(SymbolTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(SymbolTable)
    };  // DwgDbSymbolTable
DWGDB_DEFINE_OBJECTPTR (SymbolTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbBlockTableRecord : public DWGDB_EXTENDCLASS(BlockTableRecord)
    {
//__PUBLISH_SECTION_END__
private:
    // Only effective for RealDWG if xref's are resolved by acdbResolveCurrentXRefs, but the resulting database's are NOT ref counted!
    DWGDB_EXPORT DwgDbDatabaseP             GetXrefDatabase (bool includeUnresolved = false) const;
//__PUBLISH_SECTION_START__
public:
    DWGDB_DECLARE_COMMON_MEMBERS(BlockTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()

    DWGDB_EXPORT DwgString                  GetComments () const;
    DWGDB_EXPORT bool                       IsLayout () const;
    DWGDB_EXPORT DwgDbObjectId              GetLayoutId () const;
    DWGDB_EXPORT bool                       IsModelspace () const;
    DWGDB_EXPORT bool                       IsPaperspace () const;
    DWGDB_EXPORT bool                       IsExternalReference () const;
    DWGDB_EXPORT bool                       IsOverlayReference () const;
    DWGDB_EXPORT bool                       IsAnonymous () const;
    DWGDB_EXPORT bool                       HasAttributeDefinitions () const;
    DWGDB_EXPORT DwgDbBlockChildIteratorPtr GetBlockChildIterator () const;
    DWGDB_EXPORT DwgString                  GetPath () const;
    DWGDB_EXPORT DPoint3d                   GetBase () const;
    DWGDB_EXPORT DwgDbUnits                 GetINSUNITS () const;
    DWGDB_EXPORT DwgDbXrefStatus            GetXrefStatus () const;
    DWGDB_EXPORT DwgDbStatus                GetBlockReferenceIds (DwgDbObjectIdArrayR ids, bool noNested = true, bool validate = false);
    DWGDB_EXPORT DwgDbStatus                OpenSpatialIndex (DwgDbSpatialIndexPtr& indexOut, DwgDbOpenMode mode) const;
    DWGDB_EXPORT DwgDbStatus                OpenSortentsTable (DwgDbSortentsTablePtr& sortentsOut, DwgDbOpenMode mode);
    DWGDB_EXPORT DwgDbObjectId              AppendEntity (DwgDbEntityR entity);
    DWGDB_EXPORT DwgDbStatus                ComputeRange (DRange3dR out);
    };  // DwgDbBlockTableRecord
DWGDB_DEFINE_OBJECTPTR (BlockTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbBlockTable : public DWGDB_EXTENDCLASS(BlockTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(BlockTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(BlockTable)

    DWGDB_EXPORT DwgDbObjectId              GetModelspaceId () const;
    DWGDB_EXPORT DwgDbObjectId              GetPaperspaceId () const;
    };  // DwgDbBlockTable
DWGDB_DEFINE_OBJECTPTR (BlockTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayerTableRecord : public DWGDB_EXTENDCLASS(LayerTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(LayerTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()

    DWGDB_EXPORT DwgString                  GetDescription () const;
    DWGDB_EXPORT bool                       IsOff () const;
    DWGDB_EXPORT bool                       IsFrozen () const;
    DWGDB_EXPORT bool                       IsLocked () const;
    DWGDB_EXPORT bool                       IsPlottable () const;
    DWGDB_EXPORT bool                       IsHidden () const;
    DWGDB_EXPORT bool                       IsReconciled () const;
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
    DWGDB_EXPORT DwgDbStatus                SetIsFrozen (bool);
    DWGDB_EXPORT DwgDbStatus                SetIsOff (bool);
    DWGDB_EXPORT DwgDbStatus                SetIsHidden (bool);
    DWGDB_EXPORT DwgDbStatus                SetIsLocked (bool);
    DWGDB_EXPORT DwgDbStatus                SetIsPlottable (bool);
    DWGDB_EXPORT DwgDbStatus                SetIsReconciled (bool);
    DWGDB_EXPORT DwgDbStatus                SetDescription (DwgStringCR);
    };  // DwgDbLayerTableRecord
DWGDB_DEFINE_OBJECTPTR (LayerTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLayerTable : public DWGDB_EXTENDCLASS(LayerTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(LayerTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(LayerTable)

    DWGDB_EXPORT bool           HasUnreconciledLayers () const;
    DWGDB_EXPORT DwgDbStatus    GetUnreconciledLayers (DwgDbObjectIdArrayR layers) const;
    };  // DwgDbLayerTable
DWGDB_DEFINE_OBJECTPTR (LayerTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbViewportTableRecord : public DWGDB_EXTENDCLASS(ViewportTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(ViewportTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()

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
    DWGDB_EXPORT DwgDbObjectId              GetSunId () const;
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
    DWGDB_EXPORT double                     GetBrightness () const;
    DWGDB_EXPORT DwgCmColor                 GetAmbientLightColor () const;
    };  // DwgDbViewportTableRecord
DWGDB_DEFINE_OBJECTPTR (ViewportTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbViewportTable : public DWGDB_EXTENDCLASS(ViewportTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(ViewportTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(ViewportTable)
    };  // DwgDbViewportTable
DWGDB_DEFINE_OBJECTPTR (ViewportTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbTextStyleTableRecord : public DWGDB_EXTENDCLASS(TextStyleTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(TextStyleTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()

    DWGDB_EXPORT DwgDbStatus                GetFontInfo (DwgFontInfoR info) const;
    DWGDB_EXPORT WString                    GetFileName () const;
    DWGDB_EXPORT WString                    GetBigFontFileName () const;
    DWGDB_EXPORT double                     GetTextSize () const;
    DWGDB_EXPORT double                     GetWidthFactor () const;
    DWGDB_EXPORT double                     GetObliqueAngle () const;
    DWGDB_EXPORT bool                       IsVertical () const;
    DWGDB_EXPORT bool                       IsShapeFile () const;
    DWGDB_EXPORT bool                       IsActiveTextStyle () const;
    };  // DwgDbTextStyleTableRecord
DWGDB_DEFINE_OBJECTPTR (TextStyleTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbTextStyleTable : public DWGDB_EXTENDCLASS(TextStyleTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(TextStyleTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(TextStyleTable)
    };  // DwgDbTextStyleTable
DWGDB_DEFINE_OBJECTPTR (TextStyleTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbLinetypeTableRecord : public DWGDB_EXTENDCLASS(LinetypeTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(LinetypeTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()

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
public:
    DWGDB_DECLARE_COMMON_MEMBERS(LinetypeTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(LinetypeTable)
    };  // DwgDbLineTypeTable
DWGDB_DEFINE_OBJECTPTR (LinetypeTable)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbRegAppTableRecord : public DWGDB_EXTENDCLASS(RegAppTableRecord)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(RegAppTableRecord)
    DWGDB_DECLARE_SYMBOLTABLERECORD_MEMBERS()
    };  // DwgDbRegAppTableRecord
DWGDB_DEFINE_OBJECTPTR (RegAppTableRecord)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbRegAppTable : public DWGDB_EXTENDCLASS(RegAppTable)
    {
public:
    DWGDB_DECLARE_COMMON_MEMBERS(RegAppTable)
    DWGDB_DECLARE_SYMBOLTABLE_MEMBERS(RegAppTable)
    };  // DwgDbRegAppTable
DWGDB_DEFINE_OBJECTPTR (RegAppTable)

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
