/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyECSqlField final : public ECSqlField, IECSqlValueIterable
    {
    private:
        struct IteratorState final : IECSqlValueIterable::IIteratorState
            {
            private:
                enum class State : uint8_t
                    {
                    New = 0,
                    Id = 1,
                    RelECClassId = 2,
                    End = 3
                    };

                NavigationPropertyECSqlField const& m_field;
                mutable State m_state = State::New;

                IteratorState(IteratorState const& rhs) : m_field(rhs.m_field), m_state(rhs.m_state) {}

                std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
                void _MoveToNext(bool onInitializingIterator) const override;
                bool _IsAtEnd() const override { return m_state == State::End; }
                IECSqlValue const& _GetCurrent() const override;

            public:
                explicit IteratorState(NavigationPropertyECSqlField const& field) : m_field(field) {}
            };

        std::unique_ptr<ECSqlField> m_idField;
        std::unique_ptr<ECSqlField> m_relClassIdField;

        //!For a Navigation Property the main information is the Id. So we consider a nav prop value NULL if
        //!the id is NULL (regardless of what the value of the RelECClassId is)
        bool _IsNull() const override { BeAssert(m_idField != nullptr); return m_idField->IsNull(); }

        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override { return *this; }
        const_iterator _CreateIterator() const override { return const_iterator(std::make_unique<IteratorState>(*this));}

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

        int _GetArrayLength() const override;
        IECSqlValueIterable const& _GetArrayIterable() const override;

        //ECSqlField
        ECSqlStatus _OnAfterReset() override;
        ECSqlStatus _OnAfterStep() override;

    public:
        NavigationPropertyECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& colInfo) : ECSqlField(stmt, colInfo, false, false) {}
        void SetMembers(std::unique_ptr<ECSqlField> idField, std::unique_ptr<ECSqlField> relClassIdField);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
