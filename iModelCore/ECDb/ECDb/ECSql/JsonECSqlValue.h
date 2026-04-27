/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlField.h"
#include <BeRapidJson/BeRapidJson.h>
#include <Bentley/ByteStream.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct JsonECSqlValue final : public IECSqlValue, IECSqlValueIterable
    {
    private:
        struct ArrayIteratorState final : IECSqlValueIterable::IIteratorState
            {
        private:
            JsonECSqlValue const& m_value;
            mutable rapidjson::Value::ConstValueIterator m_jsonIterator;
            mutable int m_jsonIteratorIndex = -1;

            ArrayIteratorState(ArrayIteratorState const& rhs) : m_value(rhs.m_value), m_jsonIterator(rhs.m_jsonIterator), m_jsonIteratorIndex(rhs.m_jsonIteratorIndex) {}

            std::unique_ptr<IIteratorState> _Copy() const override {return std::unique_ptr<IIteratorState>(new ArrayIteratorState(*this)); }
            void _MoveToNext(bool onInitializingIterator) const override;
            bool _IsAtEnd() const override { return GetJson().IsNull() || m_jsonIterator == GetJson().End(); }
            IECSqlValue const& _GetCurrent() const override;

            rapidjson::Value const& GetJson() const { return m_value.m_json; }

        public:
            explicit ArrayIteratorState(JsonECSqlValue const& val) : IIteratorState(), m_value(val) {}
            };

        struct StructIteratorState final : IECSqlValueIterable::IIteratorState
            {
        private:
            JsonECSqlValue const& m_value;
            mutable ECN::ECPropertyIterable::const_iterator m_memberPropIterator;
            ECN::ECPropertyIterable::const_iterator m_memberPropEndIterator;

            StructIteratorState(StructIteratorState const& rhs) : m_value(rhs.m_value), m_memberPropIterator(rhs.m_memberPropIterator), m_memberPropEndIterator(rhs.m_memberPropEndIterator) {}

            std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new StructIteratorState(*this)); }
            void _MoveToNext(bool onInitializingIterator) const override;
            bool _IsAtEnd() const override { return m_memberPropIterator == m_memberPropEndIterator; }
            IECSqlValue const& _GetCurrent() const override;

            rapidjson::Value const& GetJson() const { return m_value.m_json; }

        public:
            StructIteratorState(JsonECSqlValue const& val, ECN::ECPropertyIterableCR structMemberPropertyIterable);
            };

        static rapidjson::Value const* s_nullJson;

        ECDbCR m_ecdb;
        rapidjson::Value const& m_json;
        ECSqlColumnInfo m_columnInfo;
        //for prims
        mutable ByteStream m_blobCache;
        mutable std::vector<std::unique_ptr<JsonECSqlValue>> m_arrayElementCache;
        mutable std::map<Utf8CP, std::unique_ptr<JsonECSqlValue>, CompareIUtf8Ascii> m_structMemberCache;

        ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
        bool _IsNull() const override;

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

        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override;

        int _GetArrayLength() const override;
        IECSqlValueIterable const& _GetArrayIterable() const override;

        const_iterator _CreateIterator() const override;


        IECSqlValue const& CreateStructMemberValue(ECN::ECPropertyCR memberProp) const;

        bool CanCallGetFor(ECN::PrimitiveType requestedType) const;

        static Utf8CP GetPrimitiveGetMethodName(ECN::PrimitiveType getMethodType);
        static rapidjson::Value const& GetNullJson();

    public:
        JsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo);
        ~JsonECSqlValue() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
