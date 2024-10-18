/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define EXTENDED_TYPENAME_BeGuid "BeGuid"
#define EC_PRIMITIVE_TYPENAME_BINARY      "binary"
#define EC_PRIMITIVE_TYPENAME_BOOLEAN     "boolean"
#define EC_PRIMITIVE_TYPENAME_BOOL        "bool"
#define EC_PRIMITIVE_TYPENAME_DATETIME    "dateTime"
#define EC_PRIMITIVE_TYPENAME_DOUBLE      "double"
#define EC_PRIMITIVE_TYPENAME_INTEGER     "int"
#define EC_PRIMITIVE_TYPENAME_LONG        "long"
#define EC_PRIMITIVE_TYPENAME_POINT2D     "point2d"
#define EC_PRIMITIVE_TYPENAME_POINT3D     "point3d"
#define EC_PRIMITIVE_TYPENAME_STRING      "string"
#define EC_PRIMITIVE_TYPENAME_IGEOMETRY   "IGeometry"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ValueHelpers : NonCopyableClass
{
private:
    ValueHelpers() {}
    static BentleyStatus GetEnumDisplayValue(Utf8StringR displayValue, ECN::ECEnumerationCR enumeration,
        std::function<int()> const& getIntEnumId, std::function<Utf8CP()> const& getStrEnumId);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop,
        std::function<int()> const& getIntEnumId, std::function<Utf8CP()> const& getStrEnumId);
public:
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, BeSQLite::DbValue const& dbValue);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, ECN::ECValueCR ecValue);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, RapidJsonValueCR jsonValue);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECEnumerationCR enumeration, BeSQLite::DbValue const& dbValue);

    static BentleyStatus ParsePrimitiveType(PrimitiveType&, Utf8StringCR);

    static DPoint2d GetPoint2dFromSqlValue(BeSQLite::EC::IECSqlValue const&);
    static DPoint2d GetPoint2dFromJson(RapidJsonValueCR);
    static DPoint2d GetPoint2dFromJsonString(Utf8CP);
    static DPoint3d GetPoint3dFromSqlValue(BeSQLite::EC::IECSqlValue const&);
    static DPoint3d GetPoint3dFromJson(RapidJsonValueCR);
    static DPoint3d GetPoint3dFromJsonString(Utf8CP);

    static rapidjson::Document GetPoint2dJson(DPoint2dCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint2dJsonFromString(Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint3dJson(DPoint3dCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint3dJsonFromString(Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);

    static rapidjson::Document GetJsonFromPrimitiveValue(ECN::PrimitiveType, Utf8StringCR, BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromStructValue(ECN::ECStructClassCR, BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromArrayValue(BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromString(ECN::PrimitiveType, Utf8StringCR, Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);
    ECPRESENTATION_EXPORT static rapidjson::Document GetJsonFromECValue(ECN::ECValueCR, Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);

    static ECN::ECValue GetECValueFromSqlValue(ECN::PrimitiveType, Utf8StringCR, BeSQLite::DbValue const&);
    static ECN::ECValue GetECValueFromSqlValue(ECN::PrimitiveType, Utf8StringCR, BeSQLite::EC::IECSqlValue const&);
    static ECN::ECValue GetECValueFromString(ECN::PrimitiveType, Utf8StringCR);
    ECPRESENTATION_EXPORT static ECN::ECValue GetECValueFromJson(ECN::PrimitiveType, Utf8StringCR, RapidJsonValueCR);
    ECPRESENTATION_EXPORT static bvector<ECN::ECValue> GetECValueSetFromJson(ECN::PrimitiveType, Utf8StringCR, RapidJsonValueCR);

    static rapidjson::Document GetECInstanceKeyAsJson(ECInstanceKeyCR, rapidjson::MemoryPoolAllocator<>* = nullptr);
    static Utf8String GetECInstanceKeyAsJsonString(ECInstanceKeyCR);
    static BeSQLite::EC::ECInstanceKey GetECInstanceKeyFromJson(RapidJsonValueCR);
    ECPRESENTATION_EXPORT static BeSQLite::EC::ECInstanceKey GetECInstanceKeyFromJsonString(Utf8CP);

    template<typename TContainer> static rapidjson::Document GetECInstanceKeysAsJson(TContainer const& keys, rapidjson::MemoryPoolAllocator<>* allocator = nullptr)
        {
        rapidjson::Document json(rapidjson::kArrayType);
        for (ECInstanceKeyCR key : keys)
            json.PushBack(GetECInstanceKeyAsJson(key, &json.GetAllocator()), json.GetAllocator());
        return json;
        }
    template<typename TContainer> static Utf8String GetECInstanceKeysAsJsonString(TContainer const& keys)
        {
        Utf8String str("[");
        for (ECInstanceKeyCR key : keys)
            {
            if (str.size() > 1)
                str.append(",");
            str.append(GetECInstanceKeyAsJsonString(key));
            }
        str.append("]");
        return str;
        }
    static bvector<BeSQLite::EC::ECInstanceKey> GetECInstanceKeysFromJson(RapidJsonValueCR);
    ECPRESENTATION_EXPORT static bvector<BeSQLite::EC::ECInstanceKey> GetECInstanceKeysFromJsonString(Utf8CP);

    static ECClassInstanceKey GetECClassInstanceKey(BeSQLite::EC::SchemaManagerCR, ECInstanceKeyCR);

    static Utf8CP PrimitiveTypeAsString(ECN::PrimitiveType);

    static Utf8String PadNumbersInString(Utf8StringCR str);
    static Utf8String GuidToString(BeGuidCR guid);

    static Formatting::Format const* GetPresentationFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystemGroup, std::map<std::pair<Utf8String, UnitSystem>, std::shared_ptr<Formatting::Format>> const& defaultFormats);

    static rapidjson::Document ToRapidJson(BeJsConst json);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
