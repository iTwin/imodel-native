/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                05/2017
+===============+===============+===============+===============+===============+======*/
struct ValueHelpers : NonCopyableClass
{
private:
    ValueHelpers() {}
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop,
        std::function<int()> getIntEnumId, std::function<Utf8CP()> getStrEnumId);
public:
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, BeSQLite::DbValue const& dbValue);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, ECN::ECValueCR ecValue);
    static BentleyStatus GetEnumPropertyDisplayValue(Utf8StringR displayValue, ECN::ECPropertyCR prop, RapidJsonValueCR jsonValue);

    static DPoint2d GetPoint2dFromSqlValue(BeSQLite::EC::IECSqlValue const&);
    static DPoint2d GetPoint2dFromJson(JsonValueCR);
    static DPoint2d GetPoint2dFromJson(RapidJsonValueCR);
    static DPoint2d GetPoint2dFromJsonString(Utf8CP);
    static DPoint3d GetPoint3dFromSqlValue(BeSQLite::EC::IECSqlValue const&);
    static DPoint3d GetPoint3dFromJson(JsonValueCR);
    static DPoint3d GetPoint3dFromJson(RapidJsonValueCR);
    static DPoint3d GetPoint3dFromJsonString(Utf8CP);

    static rapidjson::Document GetPoint2dJson(DPoint2dCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint2dJsonFromString(Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint3dJson(DPoint3dCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetPoint3dJsonFromString(Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);

    static rapidjson::Document GetJsonFromPrimitiveValue(ECN::PrimitiveType, BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromStructValue(ECN::ECStructClassCR, BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromArrayValue(BeSQLite::EC::IECSqlValue const&, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromString(ECN::PrimitiveType, Utf8StringCR, rapidjson::MemoryPoolAllocator<>*);
    static rapidjson::Document GetJsonFromECValue(ECN::ECValueCR, rapidjson::MemoryPoolAllocator<>*);

    static ECN::ECValue GetECValueFromSqlValue(ECN::PrimitiveType, BeSQLite::DbValue const&);
    static ECN::ECValue GetECValueFromSqlValue(ECN::PrimitiveType, BeSQLite::EC::IECSqlValue const&);
    static ECN::ECValue GetECValueFromString(ECN::PrimitiveType, Utf8StringCR);
    static ECN::ECValue GetECValueFromJson(ECN::PrimitiveType, RapidJsonValueCR);
    static ECN::ECValue GetECValueFromJson(ECN::ECPropertyCR, JsonValueCR);

    static BeSQLite::EC::ECInstanceKey GetECInstanceKeyFromJson(RapidJsonValueCR);
    static BeSQLite::EC::ECInstanceKey GetECInstanceKeyFromSerializedJson(Utf8CP);
    static bvector<BeSQLite::EC::ECInstanceKey> GetECInstanceKeysFromJson(RapidJsonValueCR);
    static bvector<BeSQLite::EC::ECInstanceKey> GetECInstanceKeysFromSerializedJson(Utf8CP);

    static Utf8String GetJsonAsString(RapidJsonValueCR);
    static Utf8String GetDbValueAsString(RapidJsonValueCR);

    static Utf8String GetECValueTypeName(ECN::ECValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
