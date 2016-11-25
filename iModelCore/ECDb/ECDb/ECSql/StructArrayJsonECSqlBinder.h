/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayJsonECSqlBinder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlBinder.h"
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlBindValue : public IECSqlBinder
    {
private:
    ECDbCR m_ecdb;
    ECSqlTypeInfo m_typeInfo;
    Utf8CP m_propertyName;
    bool m_isRoot;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

    virtual void _Clear() = 0;
    virtual ECSqlStatus _BuildJson(Json::Value&) const = 0;

    bool IgnorePropertyNameInJson() const { return m_isRoot || Utf8String::IsNullOrEmpty(m_propertyName); }

protected:
    JsonECSqlBindValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propertyName, bool isRoot) : IECSqlBinder(), m_ecdb(ecdb), m_typeInfo(typeInfo), m_propertyName(propertyName), m_isRoot(isRoot) {}

    ECDbCR GetECDb() const { return m_ecdb; }
    ECSqlTypeInfo const& GetTypeInfo() const { return m_typeInfo; }

public:
    virtual ~JsonECSqlBindValue() {}

    void Clear() { _Clear(); }
    ECSqlStatus BuildJson(Json::Value& json) const;
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2016
//+===============+===============+===============+===============+===============+======
struct PrimitiveJsonECSqlBindValue : JsonECSqlBindValue, public IECSqlPrimitiveBinder
    {
private:
    Json::Value m_value;

    virtual ECSqlStatus _BindNull() override;

    virtual ECSqlStatus _BindBoolean(bool) override;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy) override;
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
    virtual ECSqlStatus _BindDouble(double) override;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy) override;
    virtual ECSqlStatus _BindInt(int) override;
    virtual ECSqlStatus _BindInt64(int64_t) override;
    virtual ECSqlStatus _BindPoint2d(DPoint2dCR) override;
    virtual ECSqlStatus _BindPoint3d(DPoint3dCR) override;
    virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy, int byteCount) override;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override { return *this; }

    virtual ECSqlStatus _BuildJson(Json::Value&) const override;
    virtual void _Clear() override { m_value = Json::Value(Json::nullValue); }

    bool CanBindValue(ECN::PrimitiveType actualType) const { return actualType == GetTypeInfo().GetPrimitiveType(); }

public:
    PrimitiveJsonECSqlBindValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propertyName, bool isRoot) : JsonECSqlBindValue(ecdb, typeInfo, propertyName, isRoot), IECSqlPrimitiveBinder(), m_value(Json::nullValue) {}
    ~PrimitiveJsonECSqlBindValue() {}
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2016
//+===============+===============+===============+===============+===============+======
struct StructJsonECSqlBindValue : JsonECSqlBindValue, public IECSqlStructBinder
    {
private:
    std::map<ECN::ECPropertyId, std::unique_ptr<JsonECSqlBindValue>> m_members;

    virtual ECSqlStatus _BindNull() override;
    virtual IECSqlStructBinder& _BindStruct() override { return *this; }
    virtual IECSqlBinder& _GetMember(Utf8CP structMemberPropertyName) override;
    virtual IECSqlBinder& _GetMember(ECN::ECPropertyId structMemberPropertyId) override;

    virtual ECSqlStatus _BuildJson(Json::Value&) const override;
    virtual void _Clear() override;

public:
    StructJsonECSqlBindValue(ECDbCR, ECSqlTypeInfo const&, Utf8CP propertyName, bool isRoot);
    ~StructJsonECSqlBindValue() {}
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayJsonECSqlBindValue : JsonECSqlBindValue, IECSqlArrayBinder
    {
private:
    mutable std::vector<std::unique_ptr<JsonECSqlBindValue>> m_elements;
    virtual ECSqlStatus _BindNull() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;
    virtual IECSqlBinder& _AddArrayElement() override;

    virtual ECSqlStatus _BuildJson(Json::Value&) const override;
    virtual void _Clear() override { return m_elements.clear(); }

public:
    ArrayJsonECSqlBindValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propertyName, bool isRoot) : JsonECSqlBindValue(ecdb, typeInfo, propertyName, isRoot), IECSqlArrayBinder() {}
    size_t GetLength() const {return m_elements.size(); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlBindValueFactory
    {
private:
    JsonECSqlBindValueFactory();
    ~JsonECSqlBindValueFactory();

public:
    static std::unique_ptr<JsonECSqlBindValue> CreateValue(ECDbCR, ECN::ECPropertyCR, bool isRoot);
    static std::unique_ptr<JsonECSqlBindValue> CreatePrimitiveValue(ECDbCR, ECSqlTypeInfo const&, Utf8CP propertyName, bool isRoot);
    static std::unique_ptr<JsonECSqlBindValue> CreateStructValue(ECDbCR, ECSqlTypeInfo const&, Utf8CP propertyName, bool isRoot);
    static std::unique_ptr<ArrayJsonECSqlBindValue> CreateArrayValue(ECDbCR, ECSqlTypeInfo const&, Utf8CP propertyName, bool isRoot);
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct StructArrayJsonECSqlBinder : public ECSqlBinder, public IECSqlArrayBinder
    {
private:
    int m_sqliteIndex;
    std::unique_ptr<ArrayJsonECSqlBindValue> m_binder;

    virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override { m_sqliteIndex = (int) sqliteParameterIndex; }
    virtual void _OnClearBindings() override { m_binder->Clear(); }
    virtual ECSqlStatus _OnBeforeStep() override;

    virtual IECSqlBinder& _AddArrayElement() override { return m_binder->AddArrayElement(); }

    virtual ECSqlStatus _BindNull() override { return m_binder->BindNull(); }
    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override { return m_binder->BindArray(initialCapacity); }

public:
    StructArrayJsonECSqlBinder(ECSqlStatementBase&, ECSqlTypeInfo const&);
    ~StructArrayJsonECSqlBinder() {}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
