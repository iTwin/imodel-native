/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructECSqlField.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct StructECSqlField final : public ECSqlField, IECSqlValueIterable
    {
    private:
        struct IteratorState final : IECSqlValueIterable::IIteratorState
            {
        private:
            mutable std::map<Utf8CP, std::unique_ptr<ECSqlField>, CompareIUtf8Ascii>::const_iterator m_it;
            std::map<Utf8CP, std::unique_ptr<ECSqlField>, CompareIUtf8Ascii>::const_iterator m_endIt;

            IteratorState(IteratorState const& rhs) : m_it(rhs.m_it), m_endIt(rhs.m_endIt) {}

            std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
            void _MoveToNext(bool onInitializingIterator) const override 
                { 
                if (!onInitializingIterator) 
                    ++m_it; 
                }

            bool _IsAtEnd() const override { return m_it == m_endIt; }
            IECSqlValue const& _GetCurrent() const override { return *m_it->second; }

            public:
                explicit IteratorState(std::map<Utf8CP, std::unique_ptr<ECSqlField>, CompareIUtf8Ascii> const& memberFields) : IIteratorState(), m_it(memberFields.begin()), m_endIt(memberFields.end()) {}
            };

        std::map<Utf8CP, std::unique_ptr<ECSqlField>, CompareIUtf8Ascii> m_structMemberFields;

        bool _IsNull() const override;

        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override { return *this; }
        const_iterator _CreateIterator() const override { return IECSqlValueIterable::const_iterator(std::make_unique<IteratorState>(m_structMemberFields)); }

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
        StructECSqlField(ECSqlSelectPreparedStatement& stmt, ECSqlColumnInfo const& colInfo) : ECSqlField(stmt, colInfo, false, false) {}
        //Before calling this, the child field must be complete. You must not add child fields to the child fields afterwards
        //Otherwise the flags m_needsInit and m_needsReset might become wrong
        void AppendField(std::unique_ptr<ECSqlField> field);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE