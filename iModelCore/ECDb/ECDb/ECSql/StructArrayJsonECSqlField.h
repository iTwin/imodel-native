/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayJsonECSqlField.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlValue : public IECSqlValue
    {
private:
    ECDbCR m_ecdb;
    ECSqlColumnInfo m_columnInfo;
    Json::Value const& m_json;

    virtual ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
    virtual bool _IsNull() const override;

    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual IECSqlArrayValue const& _GetArray() const override;

protected:
    JsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop) : IECSqlValue(), m_ecdb(ecdb), m_json(json)
        {
        m_columnInfo = ECSqlColumnInfo::CreateChild(parentColumnInfo, prop);
        }

    Json::Value const& GetJson() const { return m_json; }
    ECDbCR GetECDb() const { return m_ecdb; }
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct PrimitiveJsonECSqlValue : JsonECSqlValue, public IECSqlPrimitiveValue
    {
private:
    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override { return *this; }

    virtual void const* _GetBinary(int* binarySize) const override;
    virtual bool _GetBoolean() const override;
    virtual uint64_t _GetDateTimeJulianDaysHns(DateTime::Info& metadata) const override;
    virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    virtual double _GetDouble() const override;
    virtual int _GetInt() const override;
    virtual int64_t _GetInt64() const override;
    virtual Utf8CP _GetText() const override;
    virtual DPoint2d _GetPoint2D() const override;
    virtual DPoint3d _GetPoint3D() const override;
    virtual IGeometryPtr _GetGeometry() const override;
    virtual void const* _GetGeometryBlob(int* blobSize) const override;

public:
    PrimitiveJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop) : JsonECSqlValue(ecdb, json, parentColumnInfo, prop), IECSqlPrimitiveValue() {}
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct StructJsonECSqlValue : JsonECSqlValue, public IECSqlStructValue
    {
private:
    std::vector<std::unique_ptr<JsonECSqlValue>> m_members;

    virtual IECSqlStructValue const& _GetStruct() const override { return *this; }

    virtual int _GetMemberCount() const override { return (int) m_members.size(); }
    virtual IECSqlValue const& _GetValue(int columnIndex) const override;

public:
    StructJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop);
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayJsonECSqlValue : JsonECSqlValue, public IECSqlArrayValue
    {
private:
    mutable Json::ValueConstIterator m_jsonIterator;
    mutable std::unique_ptr<JsonECSqlValue> m_currentElement;

    virtual IECSqlArrayValue const& _GetArray() const override { return *this; }

    //IECSqlArrayValue
    virtual void _MoveNext(bool onInitializingIterator) const override;
    virtual bool _IsAtEnd() const override;
    virtual IECSqlValue const* _GetCurrent() const override;
    virtual int _GetArrayLength() const override { return (int) GetJson().size(); }

public:
    ArrayJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct JsonECSqlFactory
    {
private:
    JsonECSqlFactory();
    ~JsonECSqlFactory();

public:
    static std::unique_ptr<JsonECSqlValue> CreateValue(ECDbCR, Json::Value const& json, ECSqlColumnInfo const& columnInfo);
    };

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      02/2016
//+===============+===============+===============+===============+===============+======
struct StructArrayJsonECSqlField : public ECSqlField, public IECSqlArrayValue
    {
private:
    int m_sqliteColumnIndex;
    mutable Json::Value m_json;
    mutable ArrayJsonECSqlValue m_value;

    //IECSqlValue
    virtual bool _IsNull() const override;
    virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
    virtual IECSqlStructValue const& _GetStruct() const override;
    virtual IECSqlArrayValue const& _GetArray() const override { return *this; }

    //IECSqlArrayValue
    virtual void _MoveNext(bool onInitializingIterator) const override;
    virtual bool _IsAtEnd() const override;
    virtual IECSqlValue const* _GetCurrent() const override {return m_value.Get}
    virtual int _GetArrayLength() const override { return m_value.GetArrayLength(); }

    //ECSqlField
    virtual ECSqlStatus _Reset() override;
    virtual ECSqlStatus _Init() override;

    void DoReset() const;

public:
    StructArrayJsonECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int sqliteColumnIndex);
    ~StructArrayJsonECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE

