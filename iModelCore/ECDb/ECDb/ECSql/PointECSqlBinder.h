/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PointECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct PointECSqlBinder : public ECSqlBinder
    {
    private:
        bool m_isPoint3d;
        int m_xSqliteIndex;
        int m_ySqliteIndex;
        int m_zSqliteIndex;

        virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;

        virtual ECSqlStatus _BindNull() override;
        virtual ECSqlStatus _BindBoolean(bool value) override;
        virtual ECSqlStatus _BindBlob(const void* value, int binarySize, IECSqlBinder::MakeCopy) override;
        virtual ECSqlStatus _BindZeroBlob(int blobSize) override;
        virtual ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const&) override;
        virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const&) override;
        virtual ECSqlStatus _BindDouble(double value) override;
        virtual ECSqlStatus _BindInt(int value) override;
        virtual ECSqlStatus _BindInt64(int64_t value) override;
        virtual ECSqlStatus _BindPoint2d(DPoint2dCR) override;
        virtual ECSqlStatus _BindPoint3d(DPoint3dCR) override;
        virtual ECSqlStatus _BindText(Utf8CP stringValue, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

        virtual IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override;
        virtual IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

        virtual IECSqlBinder& _AddArrayElement() override;

        bool IsPoint3d() const { return m_isPoint3d; }
    public:
        PointECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo, bool isPoint3d)
            : ECSqlBinder(ecsqlStatement, typeInfo, isPoint3d ? 3 : 2, false, false), m_isPoint3d(isPoint3d), m_xSqliteIndex(-1), m_ySqliteIndex(-1), m_zSqliteIndex(-1)
            {}

        ~PointECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE