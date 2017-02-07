/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayECSqlField.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include "IECSqlPrimitiveValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayECSqlField : public ECSqlField, public IECSqlArrayValue
    {
private:
    struct ArrayElementValue : public IECSqlValue, IECSqlPrimitiveValue
        {
        private:
            // unused - ECDbCR m_ecdb;
            ECN::ECValue m_value;
            ECSqlColumnInfo m_columnInfo;

        private:
            //IECSqlValue
            bool _IsNull() const override { return m_value.IsNull(); }
            ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
            IECSqlPrimitiveValue const& _GetPrimitive() const override { return *this; }
            IECSqlStructValue const& _GetStruct() const override;
            IECSqlArrayValue const& _GetArray() const override;

            //IECSqlPrimitiveValue
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

            bool CanRead(ECN::PrimitiveType requestedType) const;

        public:
            explicit ArrayElementValue(ECDbCR ecdb) /*:  unued - m_ecdb(ecdb) */ {}
            void Init(ECSqlColumnInfoCR parentColumnInfo);

            BentleyStatus SetValue(ECN::IECInstanceCR instance, uint32_t arrayIndex, DateTime::Info const& dateTimeMetadata);
            void Reset() { m_value.Clear(); }
        };

private:
    ECN::ECClassCR m_primitiveArraySystemClass;
    int m_sqliteColumnIndex;
    ECN::IECInstancePtr m_arrayValueECInstance;
    ECN::ArrayInfo m_arrayInfo;
    DateTime::Info m_datetimeMetadata;

    ECN::IECInstancePtr m_emptyArrayValueECInstance;
    ECN::ArrayInfo m_emptyArrayInfo;

    mutable int m_currentArrayIndex;
    mutable ArrayElementValue m_arrayElement;

    //IECSqlValue
    bool _IsNull() const override { return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex); }
    IECSqlPrimitiveValue const& _GetPrimitive() const override;
    IECSqlStructValue const& _GetStruct() const override;
    IECSqlArrayValue const& _GetArray() const override { return *this; }

    //IECSqlArrayValue
    void _MoveNext(bool onInitializingIterator) const override;
    bool _IsAtEnd() const override { return m_currentArrayIndex >= _GetArrayLength(); }
    IECSqlValue const* _GetCurrent() const override { BeAssert(m_currentArrayIndex >= 0 && m_currentArrayIndex < _GetArrayLength()); return &m_arrayElement; }
    int _GetArrayLength() const override { return m_arrayInfo.GetCount(); }

    //ECSqlField
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

    void DoReset() const;
    ECN::IECInstanceCP GetArrayValueECInstance() const { return m_arrayValueECInstance.get(); }

public:
    PrimitiveArrayECSqlField(ECSqlStatementBase&, ECSqlColumnInfo const&, int sqliteColumnIndex, ECN::ECClassCR primitiveArraySystemClass);
    ~PrimitiveArrayECSqlField() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
