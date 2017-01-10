/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct IdECSqlBinder;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyECSqlBinder : public ECSqlBinder
    {
    friend struct ECSqlBinderFactory;

    private:
        std::unique_ptr<IdECSqlBinder> m_idBinder;
        std::unique_ptr<IdECSqlBinder> m_relECClassIdBinder;

        NavigationPropertyECSqlBinder(ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& ecsqlTypeInfo);
        BentleyStatus Initialize(ECSqlPrepareContext&);

        //only needed at prepare time to set up the binder
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

    public:
        ~NavigationPropertyECSqlBinder() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

