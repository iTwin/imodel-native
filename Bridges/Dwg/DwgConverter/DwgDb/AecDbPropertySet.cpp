/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"
#include    <Bentley/bset.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Architecture/DbObject/AECDbPropertySet.h>
#include    <Teigha/Architecture/DbObject/AECDbPropertySetDef.h>
#include    <Teigha/Architecture/Properties/AECPropertyExtensionBase.h>
#endif

USING_NAMESPACE_DWGDB


#ifdef DWGTOOLKIT_OpenDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AddArrayToJson (rapidjson::Document& json, rapidjson::Value& jsKey, AECVariant const& variant)
    {
    auto odArray = variant.GetArray ();
    if (odArray.empty())
        return  false;

    auto& allocator = json.GetAllocator ();
    rapidjson::Value jsArray(rapidjson::kArrayType);

    for (auto iter = odArray.begin(); iter != odArray.end(); ++iter)
        {
        switch (iter->GetType())
            {
            case AECVariant::eInt32:
            case AECVariant::eUInt32:
                jsArray.PushBack (iter->GetInt32(), allocator);
                break;
            case AECVariant::eReal:
                jsArray.PushBack (iter->GetDouble(), allocator);
                break;
            case AECVariant::eBool:
                jsArray.PushBack (iter->GetBool(), allocator);
                break;
            case AECVariant::eString:
                {
                Utf8String utf8str(reinterpret_cast<WCharCP>(iter->GetString().c_str()));
                jsArray.PushBack (rapidjson::Value(utf8str.c_str(), allocator), allocator);
                break;
                }
            case AECVariant::eVariantArray:
                // WIP - need test case to support nested array
                BeAssert (false && "Nested array not supported yet!");
                break;
            }
        }
    if (jsArray.Empty())
        return  false;

    json.AddMember (jsKey, jsArray, allocator);
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AddVariantToJson (rapidjson::Document& json, OdString const& name, AECVariant const& variant)
    {
    auto& allocator = json.GetAllocator ();
    Utf8String  utf8Name(reinterpret_cast<WCharCP>(name.c_str()));
    rapidjson::Value jsKey(utf8Name.c_str(), allocator);

    switch (variant.GetType())
        {
        case AECVariant::eInt32:
        case AECVariant::eUInt32:
            json.AddMember (jsKey, (int)variant.GetInt32(), allocator);
            break;
        case AECVariant::eReal:
            json.AddMember (jsKey, variant.GetDouble(), allocator);
            break;
        case AECVariant::eBool:
            json.AddMember (jsKey, variant.GetBool(), allocator);
            break;
        case AECVariant::eString:
            {
            Utf8String utf8str(reinterpret_cast<WCharCP>(variant.GetString().c_str()));
            json.AddMember (jsKey, rapidjson::Value(utf8str.c_str(), allocator), allocator);
            break;
            }
        case AECVariant::eVariantArray:
            AddArrayToJson (json, jsKey, variant);
            break;
        default:
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static DwgDbStatus ParsePSetById (rapidjson::Document& json, AECPropertyExtensionBase& propExtractor, OdDbObject* entity, OdDbObjectId const& aecpsetId)
    {
    AECDbPropertySetPtr propertySet = aecpsetId.openObject();
    if (propertySet.isNull())
        return  DwgDbStatus::InvalidInput;

    AECDbPropertySetDefPtr propsetDef = propertySet->GetPropertySetDef().openObject();
    if (propsetDef.isNull())
        return  DwgDbStatus::UnknownError;

    auto count = propsetDef->GetPropertyDefCount();
    for (OdUInt32 i = 0; i < count; i++)
        {
        const AECPropertyDefSubPtr def = propsetDef->GetPropertyDefByPosition(i);
        if (def.isNull())
            continue;

        auto name = def->GetName();

        AECVariant  var;
        if (def->IsAutomatic())
            var = propExtractor.GetAutomaticProperty(entity, def->GetName(), def->GetDataFormat());
        else
            var = propExtractor.GetProperty(entity, propsetDef->objectId(), def->GetIndex(), def->GetDataFormat());

        AddVariantToJson (json, name, var);
        }

    return json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus UtilsLib::ParseAecDbPropertySet (rapidjson::Document& json, DwgDbObjectId entityId, DwgDbObjectIdCP aecpsetId)
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
    if (json.GetType() != rapidjson::kObjectType)
        return  status;

#ifdef DWGTOOLKIT_OpenDwg
    auto entity = entityId.openObject ();
    if (entity.isNull())
        return  DwgDbStatus::NotPersistentObject;
    
    OdDbDictionaryPtr extensionDict = AECDbPropertySet::GetAECDictionaryExt(entity).openObject(OdDb::kForRead);
    if (extensionDict.isNull())
        return  DwgDbStatus::InvalidInput;

    AECPropertyExtensionBasePtr propExtractor = AECPropertyExtensionBase::cast (entity);
    if (propExtractor.isNull())
        return  DwgDbStatus::MemoryError;

    // if query for a specific pset, extract properties from that object only
    if (aecpsetId != nullptr)
        return ParsePSetById (json, *propExtractor, entity, *aecpsetId);

    auto iter = extensionDict->newIterator ();
    if (iter.isNull())
        return  DwgDbStatus::MemoryError;

    for (; !iter->done(); iter->next())
        ParsePSetById (json, *propExtractor, entity, iter->objectId());

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    BeAssert (false && "AecDbPropertySet is not implemented for RealDWG");
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus UtilsLib::ParseAecDbPropertySetDef (rapidjson::Document& json, DwgDbObjectId propsetdefId)
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
    json.SetObject ();

#ifdef DWGTOOLKIT_OpenDwg
    AECDbPropertySetDefPtr propsetdef = propsetdefId.openObject();
    if (propsetdef.isNull())
        return  DwgDbStatus::NotPersistentObject;

    auto count = propsetdef->GetPropertyDefCount();

    for (OdUInt32 i = 0; i < count; i++)
        {
        const AECPropertyDefSubPtr def = propsetdef->GetPropertyDefByPosition(i);
        if (def.isNull())
            continue;

        auto name = def->GetName();
        auto var = def->GetDefaultValue();

        AddVariantToJson (json, name, var);
        }

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg
    BeAssert (false && "AecDbPropertySet is not implemented for RealDWG");
#endif
    return  status;
    }

#ifdef DWGTOOLKIT_RealDwg

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
class DwgDbFiler : public AcDbDwgFiler
{
private:
    Acad::ErrorStatus   m_status;
    DwgDbDatabaseR      m_dwgdb;
    bset<DwgDbObjectId> m_collectedIds;

public:
DwgDbFiler (DwgDbDatabaseR dwg) : m_dwgdb(dwg), m_status(Acad::eOk) {}
bset<DwgDbObjectId>&    GetCollectedIds () { return m_collectedIds; }

virtual Acad::ErrorStatus filerStatus() const override { return m_status; }
virtual AcDb::FilerType   filerType() const override { return AcDb::kBagFiler; }
virtual void              setFilerStatus(Acad::ErrorStatus es) override { m_status = es; }
virtual void              resetFilerStatus() override { m_status = Acad::eNoInputFiler; }
virtual Acad::ErrorStatus seek(Adesk::Int64 nOffset, int nMethod) override { return Acad::eNoInputFiler; }
virtual Adesk::Int64      tell() const override { return 0; }

virtual Acad::ErrorStatus readHardOwnershipId(AcDbHardOwnershipId* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeHardOwnershipId(const AcDbHardOwnershipId& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readSoftOwnershipId(AcDbSoftOwnershipId* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeSoftOwnershipId(const AcDbSoftOwnershipId& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readHardPointerId(AcDbHardPointerId* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeHardPointerId(const AcDbHardPointerId& val) override
    { 
    m_collectedIds.insert (val);
    return  m_status;
    }
virtual Acad::ErrorStatus readSoftPointerId(AcDbSoftPointerId* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeSoftPointerId(const AcDbSoftPointerId& val) override
    {
    m_collectedIds.insert (val);
    return  m_status;
    }
virtual Acad::ErrorStatus readInt8(Adesk::Int8* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeInt8(Adesk::Int8 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readString(ACHAR** pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeString(const ACHAR* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readString(AcString& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeString(const AcString& pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readBChunk(ads_binary* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeBChunk(const ads_binary&) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readAcDbHandle(AcDbHandle* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeAcDbHandle(const AcDbHandle& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readInt64(Adesk::Int64* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeInt64(Adesk::Int64 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readInt32(Adesk::Int32* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeInt32(Adesk::Int32 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readInt16(Adesk::Int16* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeInt16(Adesk::Int16 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readUInt64(Adesk::UInt64* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeUInt64(Adesk::UInt64 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readUInt32(Adesk::UInt32* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeUInt32(Adesk::UInt32 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readUInt16(Adesk::UInt16* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeUInt16(Adesk::UInt16 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readUInt8(Adesk::UInt8* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeUInt8(Adesk::UInt8 val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readBoolean(Adesk::Boolean* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeBoolean(Adesk::Boolean val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readBool(bool* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeBool(bool val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readDouble(double* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeDouble(double val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readPoint2d(AcGePoint2d* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writePoint2d(const AcGePoint2d& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readPoint3d(AcGePoint3d* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writePoint3d(const AcGePoint3d& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readVector2d(AcGeVector2d* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeVector2d(const AcGeVector2d& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readVector3d(AcGeVector3d* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeVector3d(const AcGeVector3d& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readScale3d(AcGeScale3d* pVal) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeScale3d(const AcGeScale3d& val) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus readBytes(void* pDest, Adesk::UIntPtr nBytes) override { return Acad::eNoInputFiler; }
virtual Acad::ErrorStatus writeBytes(const void* pSrc, Adesk::UIntPtr nBytes) override { return Acad::eNoInputFiler; }
};  // DwgDbFiler
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId UtilsLib::GetAecDbPropertySetDef (DwgDbObjectId propsetId)
    {
    DwgDbObjectId   defId;
#ifdef DWGTOOLKIT_OpenDwg

    AECDbPropertySetPtr propertySet = propsetId.openObject ();
    if (!propertySet.isNull())
        defId = propertySet->GetPropertySetDef();

#elif DWGTOOLKIT_RealDwg

    DwgDbObjectPtr propertySet(propsetId, DwgDbOpenMode::ForRead);
    if (propertySet.OpenStatus() == DwgDbStatus::Success)
        {
        DwgString dictionaryName(propertySet->isA()->name());
        if (dictionaryName.EqualsI(L"AecDbPropertySet"))
            {
            DwgDbFiler  filer(*propsetId.GetDatabase());
            propertySet->dwgOutFields (&filer);

            auto ids = filer.GetCollectedIds ();
            for (auto id : ids)
                {
                dictionaryName.Assign (id.objectClass()->name());
                if (dictionaryName.EqualsI(L"AecDbPropertySetDef"))
                    {
                    defId = id;
                    break;
                    }
                }
            }
        }
#endif
    return  defId;
    }
