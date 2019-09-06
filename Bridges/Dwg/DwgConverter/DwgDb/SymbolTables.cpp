/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

// define common DbObject methods
DWGDB_OBJECT_DEFINE_MEMBERS2(SymbolTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(SymbolTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(BlockTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(BlockTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(LayerTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(LayerTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(LinetypeTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(LinetypeTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(ViewportTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(ViewportTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(TextStyleTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(TextStyleTableRecord)
DWGDB_OBJECT_DEFINE_MEMBERS2(RegAppTable)
DWGDB_OBJECT_DEFINE_MEMBERS2(RegAppTableRecord)

// define common methods for symbol tables
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(SymbolTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(BlockTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(LayerTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(LinetypeTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(ViewportTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(TextStyleTable)
DWGDB_DEFINE_SYMBOLTABLE_MEMBERS(RegAppTable)

// define common methods for symbol table records
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(SymbolTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(BlockTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(LayerTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(LinetypeTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(ViewportTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(TextStyleTableRecord)
DWGDB_DEFINE_SYMBOLTABLERECORD_MEMBERS(RegAppTableRecord)


// unresolved toolkit's symbols
#if defined(DWGTOOLKIT_RealDwg)
#if VendorVersion >= 2018
AcDbSymbolTable::AcDbSymbolTable () { BeAssert(false && "No vtable class instantiated!"); }
AcDbSymbolTableRecord::AcDbSymbolTableRecord () { BeAssert(false && "No vtable class instantiated!"); }
Acad::ErrorStatus AcDbBlockTableRecord::assumeOwnershipOf (const AcDbObjectIdArray& entitiesToMove) { BeAssert(false && "Symbol AcDbBlockTableRecord::assumeOwnershipOf unresolved in RealDWG!"); return Acad::eNotApplicable; }
#endif
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgDbSymbolTableIterator::GetRecordId () const
    {
    if (nullptr != m_symbolTableIterator)
        {
#ifdef DWGTOOLKIT_OpenDwg
        return  DwgDbObjectId(m_symbolTableIterator->getRecordId());
#elif DWGTOOLKIT_RealDwg
        AcDbObjectId    id;
        if (Acad::eOk == m_symbolTableIterator->getRecordId(id))
            return DwgDbObjectId(id);
#endif
        }
    return  DwgDbObjectId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgDbSymbolTableIterator::GetSkipHiddenLayers () const
    {
#ifdef DWGTOOLKIT_RealDwg
    AcDbLayerTableIterator* layertableIter = dynamic_cast<AcDbLayerTableIterator*> (m_symbolTableIterator);
    if (nullptr != layertableIter)
        return layertableIter->getSkipHidden ();
#endif
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgDbSymbolTableIterator::SetSkipHiddenLayers (bool skip)
    {
#ifdef DWGTOOLKIT_RealDwg
    AcDbLayerTableIterator* layertableIter = dynamic_cast<AcDbLayerTableIterator*> (m_symbolTableIterator);
    if (nullptr != layertableIter)
        layertableIter->setSkipHidden (skip);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgDbSymbolTableIterator::GetSkipReconciledLayers () const
    {
#ifdef DWGTOOLKIT_RealDwg
    AcDbLayerTableIterator* layertableIter = dynamic_cast<AcDbLayerTableIterator*> (m_symbolTableIterator);
    if (nullptr != layertableIter)
        return layertableIter->getSkipReconciled ();
#endif
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgDbSymbolTableIterator::SetSkipReconciledLayers (bool skip)
    {
#ifdef DWGTOOLKIT_RealDwg
    AcDbLayerTableIterator* layertableIter = dynamic_cast<AcDbLayerTableIterator*> (m_symbolTableIterator);
    if (nullptr != layertableIter)
        layertableIter->setSkipReconciled (skip);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbSymbolTableIterator::~DwgDbSymbolTableIterator ()
    {
#if DWGTOOLKIT_RealDwg
    if (nullptr != m_symbolTableIterator)
        {
        delete m_symbolTableIterator;
        m_symbolTableIterator = nullptr;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgDbBlockChildIterator::GetEntityId () const
    {
    if (nullptr != m_blockChildIterator)
        {
#ifdef DWGTOOLKIT_OpenDwg
        return  DwgDbObjectId(m_blockChildIterator->objectId());
#elif DWGTOOLKIT_RealDwg
        AcDbObjectId    id;
        if (Acad::eOk == m_blockChildIterator->getEntityId(id))
            return DwgDbObjectId(id);
#endif
        }
    return  DwgDbObjectId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbBlockChildIterator::~DwgDbBlockChildIterator ()
    {
#if DWGTOOLKIT_RealDwg
    if (nullptr != m_blockChildIterator)
        {
        delete m_blockChildIterator;
        m_blockChildIterator = nullptr;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbBlockChildIteratorPtr  DwgDbBlockTableRecord::GetBlockChildIterator () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return new DwgDbBlockChildIterator (this->newIterator(true, true, false).get());
#elif DWGTOOLKIT_RealDwg
    AcDbBlockTableRecordIterator*   newIterator = nullptr;
    if (Acad::eOk == this->newIterator(newIterator, true, true))
        return new DwgDbBlockChildIterator (newIterator);
    return  nullptr;
#endif    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgDbBlockTable::GetModelspaceId () const
    {
    return  DWGDB_CALLSDKMETHOD(T_Super::getModelSpaceId(), acdbSymUtil()->blockModelSpaceId(T_Super::database()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgDbBlockTable::GetPaperspaceId () const
    {
    return  DWGDB_CALLSDKMETHOD(T_Super::getPaperSpaceId(), acdbSymUtil()->blockPaperSpaceId(T_Super::database()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbBlockTableRecord::IsLayout () const
    { 
    if (T_Super::isLayout())
        return  true;

    DwgDbObjectId       blockId = T_Super::objectId ();
    DwgDbDatabasePtr    dwg = T_Super::database ();
    if (dwg.IsNull() || !blockId.isValid())
        return  false;

#ifdef DWGTOOLKIT_OpenDwg
    if (dwg->getModelSpaceId() == blockId || dwg->getPaperSpaceId() == blockId)
        return  true;
#elif DWGTOOLKIT_RealDwg
    if (acdbSymUtil()->blockModelSpaceId(dwg.get()) == blockId || acdbSymUtil()->blockPaperSpaceId(dwg.get()) == blockId)
        return  true;
#endif
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbBlockTableRecord::IsModelspace () const
    {
    // the modelspace model should always be a database resident
    DwgDbDatabasePtr    dwg = T_Super::database ();
    if (!dwg.IsNull())
        return  dwg->GetModelspaceId() == T_Super::objectId();
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbBlockTableRecord::IsPaperspace () const
    {
    DwgDbDatabasePtr    dwg = T_Super::database ();
    if (!dwg.IsNull())
        return  dwg->GetPaperspaceId() == T_Super::objectId();
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbBlockTableRecord::GetPath () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return T_Super::pathName();
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    path = nullptr;
    if (Acad::eOk == T_Super::pathName(path) && nullptr != path && path[0] != 0)
        return  path;
    return  nullptr;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgString       DwgDbBlockTableRecord::GetComments () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    return T_Super::comments();
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    comments = nullptr;
    if (Acad::eOk == T_Super::comments(comments) && nullptr != comments && comments[0] != 0)
        return  comments;
    return  DwgString();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbUnits      DwgDbBlockTableRecord::GetINSUNITS () const
    {
    // return sys var INSUNITS if this is a layout block (blockInsertUnits seems always returns us Undefined):
    DWGDB_SDKENUM_DB(UnitsValue)    insunits = DWGDB_SDKENUM_DB(UnitsValue)::kUnitsUndefined;
    if (T_Super::isLayout())
        {
        DwgDbDatabasePtr    dwg = T_Super::database ();
        if (!dwg.IsNull())
            insunits = DWGDB_CALLSDKMETHOD(dwg->getINSUNITS, dwg->insunits) ();

        return DWGDB_CASTFROMENUM_DB(Units) (insunits);
        }

    return DWGDB_CASTFROMENUM_DB(Units) (T_Super::blockInsertUnits());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockTableRecord::GetBlockReferenceIds (DwgDbObjectIdArrayR ids, bool noNested, bool validate)
    {
    DWGDB_Type(ObjectIdArray)   idArray;
    DwgDbStatus                 status = DwgDbStatus::Success;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getBlockReferenceIds (idArray, noNested, validate);
    if (idArray.isEmpty())
        status = DwgDbStatus::UnknownError;
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getBlockReferenceIds(idArray, noNested, validate));
#endif

    if (DwgDbStatus::Success == status)
        {
        for (int i = 0; i < (int)idArray.length(); i++)
            ids.push_back (DwgDbObjectId(idArray.at(i)));
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   DwgDbBlockTableRecord::AppendEntity (DwgDbEntityR entity)
    {
    DwgDbObjectId   entityId;
#ifdef DWGTOOLKIT_OpenDwg
    entityId = T_Super::appendOdDbEntity (&entity);
#elif DWGTOOLKIT_RealDwg
    AcDbEntity* acEntity = AcDbEntity::cast (&entity);
    if (Acad::eOk != T_Super::appendAcDbEntity(entityId, acEntity))
        entityId.SetNull();
#endif
    return  entityId;
    }

DwgDbObjectId   DwgDbBlockTableRecord::GetLayoutId () const     { return T_Super::getLayoutId(); }
bool            DwgDbBlockTableRecord::IsExternalReference () const { return T_Super::isFromExternalReference(); }
bool            DwgDbBlockTableRecord::IsOverlayReference () const { return T_Super::isFromOverlayReference(); }
bool            DwgDbBlockTableRecord::IsAnonymous () const     { return T_Super::isAnonymous(); }
bool            DwgDbBlockTableRecord::HasAttributeDefinitions () const { return T_Super::hasAttributeDefinitions(); }
DPoint3d        DwgDbBlockTableRecord::GetBase () const         { return Util::DPoint3dFrom(T_Super::origin()); }
DwgDbDatabaseP  DwgDbBlockTableRecord::GetXrefDatabase (bool unresolve) const { return static_cast<DwgDbDatabaseP>(T_Super::xrefDatabase(unresolve)); }
DwgDbXrefStatus DwgDbBlockTableRecord::GetXrefStatus () const { return DWGDB_UPWARDCAST(XrefStatus)(T_Super::xrefStatus()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLayerTable::GetUnreconciledLayers (DwgDbObjectIdArrayR layers) const
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
#ifdef DWGTOOLKIT_OpenDwg
    BeAssert (false && "Teigha does not support getUnreconciledLayers!");
#elif DWGTOOLKIT_RealDwg
    AcDbObjectIdArray   ids;
    Acad::ErrorStatus   es = T_Super::getUnreconciledLayers (ids);
    if (Acad::eOk == es)
        {
        for (int i = 0; i < ids.length(); i++)
            layers.push_back (ids.at(i));
        }
    status = ToDwgDbStatus(es);
#endif
    return  ToDwgDbStatus(status);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgDbLayerTable::HasUnreconciledLayers () const
    {
    bool unreconciled = false;
#ifdef DWGTOOLKIT_OpenDwg
    BeAssert (false && "Teigha does not support hasUnreconciledLayers!");
#elif DWGTOOLKIT_RealDwg
    unreconciled = T_Super::hasUnreconciledLayers ();
#endif
    return  unreconciled;
    }
DwgString       DwgDbLayerTableRecord::GetDescription () const  { return T_Super::description(); }
bool            DwgDbLayerTableRecord::IsOff () const           { return T_Super::isOff(); }
bool            DwgDbLayerTableRecord::IsFrozen () const        { return T_Super::isFrozen(); }
bool            DwgDbLayerTableRecord::IsLocked () const        { return T_Super::isLocked(); }
bool            DwgDbLayerTableRecord::IsPlottable () const     { return T_Super::isPlottable(); }
bool            DwgDbLayerTableRecord::IsHidden () const        { return T_Super::isHidden(); }
bool            DwgDbLayerTableRecord::IsReconciled () const    { return T_Super::isReconciled(); }
DwgDbObjectId   DwgDbLayerTableRecord::GetLinetypeId () const   { return T_Super::linetypeObjectId(); }
DwgDbObjectId   DwgDbLayerTableRecord::GetLinetypeId (bool& ovrd, DwgDbObjectIdCR vpId) const { return DWGDB_CALLSDKMETHOD(T_Super::linetypeObjectId(vpId,&ovrd),T_Super::linetypeObjectId(vpId,ovrd)); }
DwgDbObjectId   DwgDbLayerTableRecord::GetMaterialId () const   { return T_Super::materialId(); }
DwgCmEntityColor DwgDbLayerTableRecord::GetColor () const       { return DWGDB_CALLSDKMETHOD(T_Super::color().entityColor, T_Super::entityColor)(); }
DwgCmEntityColor DwgDbLayerTableRecord::GetColor (bool& ovrd, DwgDbObjectIdCR vpId) const { return DWGDB_CALLSDKMETHOD(T_Super::color(vpId,&ovrd),T_Super::color(vpId,ovrd)).entityColor(); }
DwgDbLineWeight DwgDbLayerTableRecord::GetLineweight () const   { return DWGDB_CASTFROMENUM_DB(LineWeight)(T_Super::lineWeight()); }
DwgDbLineWeight DwgDbLayerTableRecord::GetLineweight (bool& ovrd, DwgDbObjectIdCR vpId) const   { return DWGDB_CASTFROMENUM_DB(LineWeight)(DWGDB_CALLSDKMETHOD(T_Super::lineWeight(vpId, &ovrd),T_Super::lineWeight(vpId, ovrd))); }
DwgTransparency DwgDbLayerTableRecord::GetTransparency () const { return T_Super::transparency(); }
DwgTransparency DwgDbLayerTableRecord::GetTransparency (bool& ovrd, DwgDbObjectIdCR vpId) const { return DWGDB_CALLSDKMETHOD(T_Super::transparency(vpId, &ovrd),T_Super::transparency(vpId, ovrd)); }
bool            DwgDbLayerTableRecord::HasOverrides (DwgDbObjectIdCR vpId) const { return T_Super::hasOverrides(vpId); }
DwgDbStatus     DwgDbLayerTableRecord::SetIsFrozen (bool b) { DwgDbStatus e=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setIsFrozen(b), e=ToDwgDbStatus(T_Super::setIsFrozen(b))); return e; }
DwgDbStatus     DwgDbLayerTableRecord::SetIsOff (bool b) { T_Super::setIsOff(b); return DwgDbStatus::Success; }
DwgDbStatus     DwgDbLayerTableRecord::SetIsHidden (bool b) { DwgDbStatus e=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setIsHidden(b), e=ToDwgDbStatus(T_Super::setIsHidden(b))); return e; }
DwgDbStatus     DwgDbLayerTableRecord::SetIsLocked (bool b) { T_Super::setIsLocked(b); return DwgDbStatus::Success; }
DwgDbStatus     DwgDbLayerTableRecord::SetIsPlottable (bool b) { DwgDbStatus e=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setIsPlottable(b), e=ToDwgDbStatus(T_Super::setIsPlottable(b))); return e; }
DwgDbStatus     DwgDbLayerTableRecord::SetIsReconciled (bool b) { DwgDbStatus e; DWGDB_CALLSDKMETHOD(e=DwgDbStatus::NotSupported, e=ToDwgDbStatus(T_Super::setIsReconciled(b))); return e; }
DwgDbStatus     DwgDbLayerTableRecord::SetDescription (DwgStringCR s) { DwgDbStatus e=DwgDbStatus::Success; DWGDB_CALLSDKMETHOD(T_Super::setDescription(s), e=ToDwgDbStatus(T_Super::setDescription(s))); return e; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockTableRecord::OpenSpatialIndex (DwgDbSpatialIndexPtr& indexOut, DwgDbOpenMode mode) const
    {
    DwgDbStatus status = DwgDbStatus::ObjectNotOpenYet;

#ifdef DWGTOOLKIT_OpenDwg
    OdDbIndexPtr    index = OdDbIndexFilterManager::getIndex (this, OdDbSpatialIndex::desc(), FromDwgDbOpenMode(mode));
    if (!index.isNull() && !(indexOut = DwgDbSpatialIndex::Cast(index.get())).isNull())
        status = DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    AcDbIndex*          index = nullptr;
    Acad::ErrorStatus   es = AcDbIndexFilterManager::getIndex (this, AcDbSpatialIndex::desc(), FromDwgDbOpenMode(mode), index);

    if (Acad::eOk == es && nullptr != index)
        {
        DwgDbSpatialIndexP  spatialIndex = DwgDbSpatialIndex::Cast (index);
        if (nullptr == spatialIndex || Acad::eOk != (es = indexOut.acquire(spatialIndex)))
            {
            index->close ();
            if (Acad::eOk == es)
                es = Acad::eNullObjectPointer;
            }
        }
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockTableRecord::OpenSortentsTable (DwgDbSortentsTablePtr& sortentsOut, DwgDbOpenMode mode)
    {
    DwgDbStatus status = DwgDbStatus::ObjectNotOpenYet;

#ifdef DWGTOOLKIT_OpenDwg
    sortentsOut = T_Super::getSortentsTable (false);
    if (!sortentsOut.IsNull())
        status = DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    AcDbSortentsTable*  acSortents = nullptr;
    Acad::ErrorStatus   es = T_Super::getSortentsTable (acSortents, FromDwgDbOpenMode(mode), false);

    if (Acad::eOk == es && nullptr != acSortents)
        {
        DwgDbSortentsTableP sortents = DwgDbSortentsTable::Cast (acSortents);
        if (nullptr == sortents || Acad::eOk != (es = sortentsOut.acquire(sortents)))
            {
            acSortents->close ();
            if (Acad::eOk == es)
                es = Acad::eNullObjectPointer;
            }
        }
    status = ToDwgDbStatus (es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbBlockTableRecord::ComputeRange (DRange3dR out)
    {
    DwgDbStatus status = DwgDbStatus::ObjectNotOpenYet;
#ifdef DWGTOOLKIT_OpenDwg
    OdGeExtents3d blockExtents;
    status = ToDwgDbStatus (T_Super::getGeomExtents(blockExtents));

#elif DWGTOOLKIT_RealDwg
    AcDbExtents blockExtents;
    status = ToDwgDbStatus (blockExtents.addBlockExt(this));
#endif

    if (status == DwgDbStatus::Success)
        out = Util::DRange3dFrom (blockExtents);
    return  status;
    }

bool           DwgDbViewportTableRecord::IsGridEnabled () const { return T_Super::gridEnabled(); }
bool           DwgDbViewportTableRecord::IsUcsIconEnabled () const { return T_Super::iconEnabled(); }
bool           DwgDbViewportTableRecord::IsFrontClipEnabled () const { return T_Super::frontClipEnabled(); }
bool           DwgDbViewportTableRecord::IsFrontClipAtEye () const { return T_Super::frontClipAtEye(); }
double         DwgDbViewportTableRecord::GetFrontClipDistance () const { return T_Super::frontClipDistance(); }
bool           DwgDbViewportTableRecord::IsBackClipEnabled () const { return T_Super::backClipEnabled(); }
double         DwgDbViewportTableRecord::GetBackClipDistance () const { return T_Super::backClipDistance(); }
bool           DwgDbViewportTableRecord::IsPerspectiveEnabled () const { return T_Super::perspectiveEnabled(); }
bool           DwgDbViewportTableRecord::IsDefaultLightingOn () const { return T_Super::isDefaultLightingOn(); }
DwgDbObjectId  DwgDbViewportTableRecord::GetBackground () const { return T_Super::background(); }
DwgDbObjectId  DwgDbViewportTableRecord::GetVisualStyle () const { return T_Super::visualStyle(); }
DVec3d         DwgDbViewportTableRecord::GetViewDirection () const { return Util::DVec3dFrom(T_Super::viewDirection()); }
double         DwgDbViewportTableRecord::GetHeight () const { return T_Super::height(); }
double         DwgDbViewportTableRecord::GetWidth () const { return T_Super::width(); }
double         DwgDbViewportTableRecord::GetLensLength () const { return T_Super::lensLength(); }
double         DwgDbViewportTableRecord::GetViewTwist () const { return T_Super::viewTwist(); }
double         DwgDbViewportTableRecord::GetUcsElevation () const { return T_Super::elevation(); }
DPoint2d       DwgDbViewportTableRecord::GetCenterPoint () const { return Util::DPoint2dFrom(T_Super::centerPoint()); }
DPoint3d       DwgDbViewportTableRecord::GetTargetPoint () const { return Util::DPoint3dFrom(T_Super::target()); }
DPoint2d       DwgDbViewportTableRecord::GetLowerLeftCorner () const { return Util::DPoint2dFrom(T_Super::lowerLeftCorner()); }
DPoint2d       DwgDbViewportTableRecord::GetUpperRightCorner () const { return Util::DPoint2dFrom(T_Super::upperRightCorner()); }
bool           DwgDbViewportTableRecord::IsUcsSavedWithViewport () const { return T_Super::isUcsSavedWithViewport(); }
DPoint2d       DwgDbViewportTableRecord::GetGridIncrements () const { return Util::DPoint2dFrom(T_Super::gridIncrements()); }
DPoint2d       DwgDbViewportTableRecord::GetSnapIncrements () const { return Util::DPoint2dFrom(T_Super::snapIncrements()); }
DPoint2d       DwgDbViewportTableRecord::GetSnapBase () const { return Util::DPoint2dFrom(T_Super::snapBase()); }
double         DwgDbViewportTableRecord::GetSnapAngle () const { return T_Super::snapAngle(); }
SnapIsoPair    DwgDbViewportTableRecord::GetSnapPair () const { return static_cast<SnapIsoPair>(T_Super::snapPair()); }
bool           DwgDbViewportTableRecord::IsSnapEnabled () const { return T_Super::snapEnabled(); }
bool           DwgDbViewportTableRecord::IsIsometricSnapEnabled () const { return T_Super::isometricSnapEnabled(); }
int16_t        DwgDbViewportTableRecord::GetGridMajor () const { return T_Super::gridMajor(); }
double         DwgDbViewportTableRecord::GetBrightness () const { return T_Super::brightness(); }
DwgCmColor     DwgDbViewportTableRecord::GetAmbientLightColor () const { return static_cast<DwgCmColor>(T_Super::ambientLightColor()); }
DwgDbObjectId  DwgDbViewportTableRecord::GetSunId () const { return T_Super::sunId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbViewportTableRecord::GetUcs (DPoint3dR origin, DVec3dR xAxis, DVec3d yAxis) const
    { 
    DWGGE_Type(Point3d)     geOrigin;
    DWGGE_Type(Vector3d)    xDir, yDir;
    DwgDbStatus             status = DwgDbStatus::UnknownError;

#ifdef DWGTOOLKIT_OpenDwg
    T_Super::getUcs (geOrigin, xDir, yDir);
#elif DWGTOOLKIT_RealDwg
    status = ToDwgDbStatus (T_Super::getUcs(geOrigin, xDir, yDir));
#endif

    if (DwgDbStatus::Success == status)
        {
        origin = Util::DPoint3dFrom (geOrigin);
        xAxis = Util::DVec3dFrom (xDir);
        yAxis = Util::DVec3dFrom (yDir);
        }
    else
        {
        origin.Zero ();
        xAxis.Init (0., 0., 1.0);
        yAxis.Init (0., 0., 1.0);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbTextStyleTableRecord::GetFontInfo (DwgFontInfo& info) const
    {
    DwgDbBool   bold = false, italic = false;
    int         charset = 0, pitchNFamily = 0;
#ifdef DWGTOOLKIT_OpenDwg
    OdString    typeface;

    T_Super::font (typeface, bold, italic, charset, pitchNFamily);

#elif DWGTOOLKIT_RealDwg
    ACHAR*      typeface = nullptr;
#if VendorVersion <= 2016
    Acad::ErrorStatus   es = T_Super::font (typeface, bold, italic, charset, pitchNFamily);
#else   // >= 2017
    Charset     acCharset = Charset::kAnsiCharset;

    Autodesk::AutoCAD::PAL::FontUtils::FontPitch    pitch;
    Autodesk::AutoCAD::PAL::FontUtils::FontFamily   family;

    Acad::ErrorStatus   es = T_Super::font (typeface, bold, italic, acCharset, pitch, family);
#endif  // 2016
#endif  // DWGTOOLKIT_

    WString     shx = this->GetFileName ();
    WString     bigfont = this->GetBigFontFileName ();

#ifdef DWGTOOLKIT_OpenDwg

    const OdChar*   odChars = typeface.c_str ();
    info.Set (reinterpret_cast<WCharCP>(odChars), shx.c_str(), bigfont.c_str(), bold, italic, charset, pitchNFamily);
    return  DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg

    if (Acad::eOk == es)
        {
#if VendorVersion >= 2017
        charset = acCharset;
        pitchNFamily = static_cast<int>(pitch) | static_cast<int>(family);
#endif

        info.Set (typeface, shx.c_str(), bigfont.c_str(), Adesk::kTrue == bold, Adesk::kTrue == italic, charset, pitchNFamily);
        }

    acutDelString (typeface);
    return  ToDwgDbStatus(es);
#endif  // DWGTOOLKIT_
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DwgDbTextStyleTableRecord::GetFileName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    const OdChar*   odChars = T_Super::fileName().c_str ();
    return  WString(reinterpret_cast<WCharCP>(odChars));
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    name = nullptr;
    if (Acad::eOk == T_Super::fileName(name))
        return  name;
    return  WString();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString         DwgDbTextStyleTableRecord::GetBigFontFileName () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    const OdChar*   odChars = T_Super::bigFontFileName().c_str ();
    return  WString(reinterpret_cast<WCharCP>(odChars));
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    name = nullptr;
    if (Acad::eOk == T_Super::bigFontFileName(name))
        return  name;
    return  WString();
#endif
    }

double          DwgDbTextStyleTableRecord::GetTextSize () const { return T_Super::textSize(); }
double          DwgDbTextStyleTableRecord::GetWidthFactor () const { return T_Super::xScale(); }
double          DwgDbTextStyleTableRecord::GetObliqueAngle () const { return T_Super::obliquingAngle(); }
bool            DwgDbTextStyleTableRecord::IsVertical () const { return DWGDB_IsTrue(T_Super::isVertical()); }
bool            DwgDbTextStyleTableRecord::IsShapeFile () const { return DWGDB_IsTrue(T_Super::isShapeFile()); }
bool            DwgDbTextStyleTableRecord::IsActiveTextStyle () const { return T_Super::objectId() == DWGDB_CALLSDKMETHOD(T_Super::database()->getTEXTSTYLE(), T_Super::database()->textstyle()); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLinetypeTableRecord::GetComments (DwgStringR comments) const
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
#ifdef DWGTOOLKIT_OpenDwg
    comments = T_Super::comments ();
    if (!comments.IsEmpty())
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    str = nullptr;
    status = ToDwgDbStatus (T_Super::comments(str));
    if (DwgDbStatus::Success == status && nullptr != str)
        comments.assign (str);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          1/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgDbLinetypeTableRecord::GetTextAt (DwgStringR text, uint32_t index) const
    {
    DwgDbStatus status = DwgDbStatus::UnknownError;
#ifdef DWGTOOLKIT_OpenDwg
    text = T_Super::textAt (index);
    if (!text.IsEmpty())
        status = DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    const ACHAR*    str = nullptr;
    status = ToDwgDbStatus (T_Super::textAt(index, str));
    if (DwgDbStatus::Success == status && nullptr != str)
        text.assign (str);
#endif
    return  status;
    }
uint32_t        DwgDbLinetypeTableRecord::GetNumberOfDashes () const { return T_Super::numDashes(); }
double          DwgDbLinetypeTableRecord::GetPatternLength () const { return T_Super::patternLength(); }
double          DwgDbLinetypeTableRecord::GetDashLengthAt (uint32_t index) const { return T_Super::dashLengthAt(index); }
uint32_t        DwgDbLinetypeTableRecord::GetShapeNumberAt (uint32_t index) const { return T_Super::shapeNumberAt(index); }
DVec2d          DwgDbLinetypeTableRecord::GetShapeOffsetAt (uint32_t index) const { return Util::DVec2dFrom(T_Super::shapeOffsetAt(index)); }
double          DwgDbLinetypeTableRecord::GetShapeScaleAt (uint32_t index) const { return T_Super::shapeScaleAt(index); }
double          DwgDbLinetypeTableRecord::GetShapeRotationAt (uint32_t index) const { return T_Super::shapeRotationAt(index); }
DwgDbObjectId   DwgDbLinetypeTableRecord::GetShapeStyleAt (uint32_t index) const { return T_Super::shapeStyleAt(index); }
bool            DwgDbLinetypeTableRecord::IsShapeUcsOrientedAt (uint32_t index) const { return T_Super::shapeIsUcsOrientedAt(index); }
bool            DwgDbLinetypeTableRecord::IsShapeUprightAt (uint32_t index) const { return T_Super::shapeIsUprightAt(index); }
bool            DwgDbLinetypeTableRecord::IsScaledToFit () const { return T_Super::isScaledToFit(); }
bool            DwgDbLinetypeTableRecord::IsByLayer () const { return nullptr != T_Super::database() && T_Super::objectId() == DWGDB_CALLSDKMETHOD(T_Super::database()->getLinetypeByLayerId(), T_Super::database()->byLayerLinetype()); }
bool            DwgDbLinetypeTableRecord::IsByBlock () const { return nullptr != T_Super::database() && T_Super::objectId() == DWGDB_CALLSDKMETHOD(T_Super::database()->getLinetypeByBlockId(), T_Super::database()->byBlockLinetype()); }
bool            DwgDbLinetypeTableRecord::IsContinuous () const { return nullptr != T_Super::database() && T_Super::objectId() == DWGDB_CALLSDKMETHOD(T_Super::database()->getLinetypeContinuousId(), T_Super::database()->continuousLinetype()); }
