/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayECSqlField.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include <rapidjson/BeRapidJson.h>
#include <Bentley/ByteStream.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlValue : public IECSqlValue
    {
private:
    ECDbCR m_ecdb;
    rapidjson::Value const& m_json;
    ECSqlColumnInfo m_columnInfo;

    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
    bool _IsNull() const override;

    IECSqlPrimitiveValue const& _GetPrimitive() const override;
    IECSqlStructValue const& _GetStruct() const override;
    IECSqlArrayValue const& _GetArray() const override;

protected:
    JsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo) : IECSqlValue(), m_ecdb(ecdb), m_json(json), m_columnInfo(columnInfo) {}
    rapidjson::Value const& GetJson() const { return m_json; }

    ECDbCR GetECDb() const { return m_ecdb; }

public:
    virtual ~JsonECSqlValue() {}
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct PrimitiveJsonECSqlValue : public JsonECSqlValue, public IECSqlPrimitiveValue
    {
private:
    DateTime::Info m_datetimeMetadata;
    mutable ByteStream m_blobCache;

    IECSqlPrimitiveValue const& _GetPrimitive() const override { return *this; }

    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;

    bool CanCallGetFor(ECN::PrimitiveType requestedType) const;
public:
    PrimitiveJsonECSqlValue(ECDbCR, rapidjson::Value const&, ECSqlColumnInfo const&, DateTime::Info const&);
    ~PrimitiveJsonECSqlValue() {}
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct StructJsonECSqlValue : public JsonECSqlValue, public IECSqlStructValue
    {
private:
    std::vector<std::unique_ptr<JsonECSqlValue>> m_members;

    bool _IsNull() const override;
    IECSqlStructValue const& _GetStruct() const override { return *this; }

    int _GetMemberCount() const override { return (int) m_members.size(); }
    IECSqlValue const& _GetValue(int columnIndex) const override;

public:
    StructJsonECSqlValue(ECDbCR, rapidjson::Value const&, ECSqlColumnInfo const&, ECN::ECStructClassCR);
    ~StructJsonECSqlValue() {}
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayJsonECSqlValue : public JsonECSqlValue, public IECSqlArrayValue
    {
private:
    mutable rapidjson::Value::ConstValueIterator m_jsonIterator;
    mutable int m_jsonIteratorIndex;
    mutable std::unique_ptr<JsonECSqlValue> m_currentElement;
    DateTime::Info m_primitiveArrayDatetimeMetadata;
    ECN::ECStructClassCP m_structArrayElementType;

    IECSqlArrayValue const& _GetArray() const override { return *this; }

    //IECSqlArrayValue
    void _MoveNext(bool onInitializingIterator) const override;
    bool _IsAtEnd() const override;
    IECSqlValue const* _GetCurrent() const override;
    int _GetArrayLength() const override { return (int) GetJson().Size(); }

public:
    ArrayJsonECSqlValue(ECDbCR, rapidjson::Value const&, ECSqlColumnInfo const&);
    ~ArrayJsonECSqlValue() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlValueFactory
    {
private:
    JsonECSqlValueFactory();
    ~JsonECSqlValueFactory();

public:
    static std::unique_ptr<JsonECSqlValue> CreateValue(ECDbCR, rapidjson::Value const&, ECSqlColumnInfo const&);
    static std::unique_ptr<JsonECSqlValue> CreateArrayElementValue(ECDbCR, rapidjson::Value const&, ECSqlColumnInfo const&, DateTime::Info const&, ECN::ECStructClassCP);
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      02/2016
//+===============+===============+===============+===============+===============+======
struct StructArrayECSqlField : public ECSqlField
    {
private:
    int m_sqliteColumnIndex;
    mutable rapidjson::Document m_json;
    mutable std::unique_ptr<ArrayJsonECSqlValue> m_value;

    //IECSqlValue
    bool _IsNull() const override { return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex); }
    IECSqlPrimitiveValue const& _GetPrimitive() const override;
    IECSqlStructValue const& _GetStruct() const override;
    IECSqlArrayValue const& _GetArray() const override { return *m_value; }

    //ECSqlField
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

    void DoReset() const;

public:
    StructArrayECSqlField(ECSqlStatementBase&, ECSqlColumnInfo const&, int sqliteColumnIndex);
    ~StructArrayECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE

