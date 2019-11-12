/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
static bool AddArrayToJson (rapidjson::Document& json, rapidjson::Value& jsKey, rapidjson::MemoryPoolAllocator<>& allocator, AECVariant const& variant)
    {
    auto odArray = variant.GetArray ();
    if (odArray.empty())
        return  false;

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
static bool AddVariantToJson (rapidjson::Document& json, rapidjson::MemoryPoolAllocator<>& allocator, OdString const& name, AECVariant const& variant)
    {
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
            AddArrayToJson (json, jsKey, allocator, variant);
            break;
        default:
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetVariantAsUtf8 (Utf8StringR out, AECVariant const& variant)
    {
    switch (variant.GetType())
        {
        case AECVariant::eInt32:
        case AECVariant::eUInt32:
            out.Sprintf ("%d", variant.GetInt32());
            break;
        case AECVariant::eReal:
            out.Sprintf ("%g", variant.GetDouble());
            break;
        case AECVariant::eBool:
            out.Sprintf ("%s", variant.GetBool() ? "True" : "False");
            break;
        case AECVariant::eString:
            out.Assign (reinterpret_cast<WCharCP>(variant.GetString().c_str()));
            break;
        case AECVariant::eVariantArray:
        default:
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static DwgDbStatus ParsePSetById (rapidjson::Document& json, rapidjson::MemoryPoolAllocator<>& allocator, AECPropertyExtensionBase& propExtractor, OdDbObject* entity, OdDbObjectId const& aecpsetId)
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
        AECPropertyDefSubPtr def = propsetDef->GetPropertyDefByPosition(i);
        if (def.isNull())
            continue;

        auto name = def->GetName();

        // do not format value - need raw value with correct type
        AECVariant  var;
        if (def->IsAutomatic())
            var = propExtractor.GetAutomaticProperty(entity, name);
        else
            var = propExtractor.GetProperty(entity, propsetDef->objectId(), def->GetIndex());

        AddVariantToJson (json, allocator, name, var);
        }

    return json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;
    }

#ifdef FIND_AECPROPERTIES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void FindValuesFrom (T_Utf8StringVectorR values, T_Utf8StringVectorCR names, AECDbPropertySet const& propertySet, bvector<bool>& found, size_t& foundCount)
    {
    for (size_t i = 0; i < names.size(); i++)
        {
        if (names[i].empty())
            {
            found[i] = true;
            foundCount++;
            continue;
            }

        if (!found[i])
            {
            AECPropertySubPtr prop = propertySet.GetPropertyByName (names[i].c_str());
            if (!prop.isNull())
                {
                GetVariantAsUtf8 (values[i], prop->GetValue());

                found[i] = true;
                foundCount++;
                }
            }
        }
    }
#endif  // FIND_AECPROPERTIES

#elif DWGTOOLKIT_RealDwg && VendorVersion > 2019

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AddVariantToJson (rapidjson::Document& json, rapidjson::MemoryPoolAllocator<>& allocator, AcString const& name, ::VARIANT const& variant)
    {
    Utf8String  utf8Name(name.kwszPtr());
    rapidjson::Value jsKey(utf8Name.c_str(), static_cast<rapidjson::SizeType>(utf8Name.size()), allocator);

    switch (variant.vt)
        {
        case VT_BSTR:
            {
            Utf8String utf8str(variant.bstrVal);
            json.AddMember (jsKey, rapidjson::Value(utf8str.c_str(), static_cast<rapidjson::SizeType>(utf8str.size()), allocator).Move(), allocator);
            }
            break;
        case VT_I4:
            json.AddMember (jsKey, (int)variant.lVal, allocator);
            break;
        case VT_I2:
        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
        case VT_I8:
        case VT_UI8:
        case VT_UINT:
        case VT_INT:
            {
            ::VARIANT   varOut;
            if (SUCCEEDED(::VariantChangeType(&varOut, &variant, 0, VT_I4)))
                json.AddMember (jsKey, (int)varOut.intVal, allocator);
            else
                return  false;
            }
            break;
        case VT_R4:
            json.AddMember (jsKey, variant.fltVal, allocator);
            break;
        case VT_R8:
            json.AddMember (jsKey, variant.dblVal, allocator);
            break;
        case VT_BOOL:
            json.AddMember (jsKey, variant.bVal, allocator);
            break;
        case VT_ARRAY:
            {
            ::SAFEARRAY* subArray = variant.parray;
            if (subArray != nullptr)
                {
                ::LONG  startIndex = 0, endIndex = 0;
                ::SafeArrayGetLBound (subArray, 1, &startIndex);
                ::SafeArrayGetUBound (subArray, 1, &endIndex);
                ::LONG count = endIndex - startIndex;
                if (count < 1)
                    return  true;
                // WIP
                }
            else
                {
                return  false;
                }
            }
            break;
        default:
            {
            // convert everything else to a string
            ::VARIANT   stringVar;
            if (SUCCEEDED(::VariantChangeType(&stringVar, &variant, 0, VT_BSTR)))
                {
                Utf8String utf8str(stringVar.bstrVal);
                json.AddMember (jsKey, rapidjson::Value(utf8str.c_str(), allocator).Move(), allocator);
                }
            else
                {
                return  false;
                }
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus CreateAecPsetsIterator (AcDbDictionaryIterator* iter, AcDbObjectId entityId)
    {
    AcDbEntityPointer   hostEnt(entityId, AcDb::kForRead);
    if (hostEnt.openStatus() != Acad::eOk)
        return  ToDwgDbStatus(hostEnt.openStatus());

    AcDbObjectId    aecpsetsId;
    AcDbDictionaryPointer   extDict(hostEnt->extensionDictionary(), AcDb::kForRead);
    if (extDict.openStatus() != Acad::eOk || extDict->getAt(L"AEC_PROPERTY_SETS", aecpsetsId) != Acad::eOk)
        return  DwgDbStatus::UnknownError;

    AcDbDictionaryPointer   aecpsetsDict(hostEnt->extensionDictionary(), AcDb::kForRead);
    if (aecpsetsDict.openStatus() != Acad::eOk)
        return  ToDwgDbStatus(aecpsetsDict.openStatus());

    iter = aecpsetsDict->newIterator ();
    return iter == nullptr ? DwgDbStatus::MemoryError : DwgDbStatus::Success;
    }

#ifdef FIND_AECPROPERTIES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void FindValuesFrom (T_Utf8StringVectorR valuesOut, T_Utf8StringVectorCR namesIn, AcStringArray const& acNamesIn, AcVariantArray const& acValuesIn, bvector<bool>& found, size_t& foundCount)
    {
    for (size_t i = 0; i < namesIn.size(); i++)
        {
        if (namesIn[i].empty())
            {
            found[i] = true;
            foundCount++;
            continue;
            }

        if (!found[i])
            {
            int acIndex = acNamesIn.find (AcString(namesIn[i].c_str()));
            if (acIndex >= 0)
                {
                // convert to string value
                ::VARIANT   stringVar;
                if (SUCCEEDED(::VariantChangeType(&stringVar, &acValuesIn[acIndex], 0, VT_BSTR)))
                    valuesOut[i].Assign (stringVar.bstrVal);
                else
                    valuesOut[i].clear ();
                found[i] = true;
                foundCount++;
                }
            }
        }
    }
#endif  // FIND_AECPROPERTIES

#endif  // DWGTOOLKIT_

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus UtilsLib::ParseAecDbPropertySet (rapidjson::Document& json, rapidjson::MemoryPoolAllocator<>& allocator, DwgDbObjectId entityId, DwgDbObjectIdCP aecpsetId)
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
        return ParsePSetById (json, allocator, *propExtractor, entity, *aecpsetId);

    auto iter = extensionDict->newIterator ();
    if (iter.isNull())
        return  DwgDbStatus::MemoryError;

    for (; !iter->done(); iter->next())
        ParsePSetById (json, allocator, *propExtractor, entity, iter->objectId());

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg && VendorVersion > 2019

    AcStringArray   nameArray;
    AcVariantArray  valueArray;
    AecPropertyDataTypeArray    typeArray;
    AecPropertyUnitTypeArray    unitArray;

    if (aecpsetId != nullptr)
        {
        auto es = ::getAecDbPropertySet(*aecpsetId, nameArray, valueArray, typeArray, unitArray);
        if (es == Acad::eOk)
            {
            auto count = nameArray.length ();
            for (int i = 0; i < count; i++)
                AddVariantToJson (json, allocator, nameArray[i], valueArray[i]);

            status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;
            }
        else
            {
            ToDwgDbStatus(es);
            }
        return  status;
        }

    AcDbDictionaryIterator* iter = nullptr;
    status = ToDwgDbStatus (CreateAecPsetsIterator(iter, entityId));
    if (status != DwgDbStatus::Success)
        return  status;

    for (; !iter->done(); iter->next())
        {
        nameArray.removeAll ();
        valueArray.removeAll ();

        auto es = ::getAecDbPropertySet(iter->objectId(), nameArray, valueArray, typeArray, unitArray);
        if (es == Acad::eOk)
            {
            auto count = nameArray.length ();
            for (int i = 0; i < count; i++)
                AddVariantToJson (json, allocator, nameArray[i], valueArray[i]);
            }
        }

    delete iter;

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;

#endif  // DWGTOOLKIT_
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus UtilsLib::ParseAecDbPropertySetDef (rapidjson::Document& json, rapidjson::MemoryPoolAllocator<>& allocator, DwgDbObjectId propsetdefId)
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

        AddVariantToJson (json, allocator, name, var);
        }

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg && VendorVersion > 2019

    AcStringArray   nameArray;
    AcVariantArray  valueArray;
    AecPropertyDataTypeArray    typeArray;
    AecPropertyUnitTypeArray    unitArray;
    BoolArray   defaultIsUnspecifiedArray, isAutomaticArray;

    auto es = ::getAecDbPropertySetDef(propsetdefId, nameArray, valueArray, typeArray, unitArray, defaultIsUnspecifiedArray, isAutomaticArray);
    if (es == Acad::eOk)
        {
        auto count = nameArray.length ();
        for (int i = 0; i < count; i++)
            AddVariantToJson (json, allocator, nameArray[i], valueArray[i]);
        }

    status = json.ObjectEmpty() ? DwgDbStatus::InvalidInput : DwgDbStatus::Success;
#endif

    return  status;
    }

#ifdef FIND_AECPROPERTIES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus UtilsLib::FindAecDbPropertyValues (T_Utf8StringVectorR values1, T_Utf8StringVectorR values2, T_Utf8StringVectorCR names1, T_Utf8StringVectorCR names2, DwgDbObjectIdCR entityId)
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;

#ifdef DWGTOOLKIT_OpenDwg
    auto entity = entityId.openObject ();
    if (entity.isNull())
        return  DwgDbStatus::NotPersistentObject;
    
    OdDbDictionaryPtr extensionDict = AECDbPropertySet::GetAECDictionaryExt(entity).openObject(OdDb::kForRead);
    if (extensionDict.isNull())
        return  DwgDbStatus::InvalidInput;

    auto iter = extensionDict->newIterator ();
    if (iter.isNull())
        return  DwgDbStatus::MemoryError;

    size_t  nRequested1 = names1.size ();
    size_t  nRequested2 = names2.size ();
    size_t  foundCount1 = 0;
    size_t  foundCount2 = 0;

    bvector<bool>   found1, found2;

    found1.resize (nRequested1, false);
    found2.resize (nRequested2, false);
    values1.resize (nRequested1, "");
    values2.resize (nRequested2, "");

    // exhaust search all AecDbPropertySets for the property value(s)
    for (; !iter->done(); iter->next())
        {
        AECDbPropertySetPtr propertySet = iter->objectId().openObject();
        if (propertySet.isNull())
            continue;

        if (foundCount1 < nRequested1)
            FindValuesFrom (values1, names1, *propertySet, found1, foundCount1);

        if (foundCount2 < nRequested2)
            FindValuesFrom (values2, names2, *propertySet, found2, foundCount2);

        if (foundCount1 == nRequested1 && foundCount2 == nRequested2)
            return  DwgDbStatus::Success;
        }
    // treat the second property list as optional
    status = foundCount1 == 0 ? DwgDbStatus::NotFound : DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg && VendorVersion > 2019

    AcDbDictionaryIterator* iter = nullptr;
    status = ToDwgDbStatus (CreateAecPsetsIterator(iter, entityId));
    if (status != DwgDbStatus::Success)
        return  status;

    size_t  nRequested1 = names1.size ();
    size_t  nRequested2 = names2.size ();
    size_t  foundCount1 = 0;
    size_t  foundCount2 = 0;

    bvector<bool>   found1, found2;

    found1.resize (nRequested1, false);
    found2.resize (nRequested2, false);
    values1.resize (nRequested1, "");
    values2.resize (nRequested2, "");

    AcStringArray   nameArray;
    AcVariantArray  valueArray;
    AecPropertyDataTypeArray    typeArray;
    AecPropertyUnitTypeArray    unitArray;

    // exhaust search all AecDbPropertySets for the property value(s)
    for (; !iter->done(); iter->next())
        {
        nameArray.removeAll ();
        valueArray.removeAll ();

        auto es = ::getAecDbPropertySet(iter->objectId(), nameArray, valueArray, typeArray, unitArray);
        if (es == Acad::eOk)
            {
            if (foundCount1 < nRequested1)
                FindValuesFrom (values1, names1, nameArray, valueArray, found1, foundCount1);

            if (foundCount2 < nRequested2)
                FindValuesFrom (values2, names2, nameArray, valueArray, found2, foundCount2);

            if (foundCount1 == nRequested1 && foundCount2 == nRequested2)
                return  DwgDbStatus::Success;
            }
        }

    // treat the second property list as optional
    status = foundCount1 == 0 ? DwgDbStatus::NotFound : DwgDbStatus::Success;

#endif  // DWGTOOLKIT_

    return  status;
    }
#endif  // FIND_AECPROPERTIES

#ifdef USE_DWGFILER
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

#elif DWGTOOLKIT_RealDwg && VendorVersion > 2019

    return ::getAecDbPropertySetDef(propsetId);
    
#endif
    return  defId;
    }
