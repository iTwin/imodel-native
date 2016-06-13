/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/SystemPropertyECSqlBinder.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"
#include "IECSqlPrimitiveBinder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct SystemPropertyECSqlBinder : public ECSqlBinder, public IECSqlPrimitiveBinder
    {
private:
    int m_sqliteIndex;
    ECSqlSystemProperty m_systemProperty;
    RelationshipClassMap const* m_constraints;
    bool m_bindValueIsNull;
    bool m_isNoop;
    bool IsNoop() const { return m_isNoop; }

    virtual void _SetSqliteIndex(int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
    virtual ECSqlStatus _OnBeforeStep() override;
    virtual void _OnClearBindings() override;

    virtual IECSqlPrimitiveBinder& _BindPrimitive() override;
    virtual IECSqlStructBinder& _BindStruct() override;
    virtual IECSqlArrayBinder& _BindArray(uint32_t initialCapacity) override;

    virtual ECSqlStatus _BindNull() override;
    virtual ECSqlStatus _BindBoolean(bool value) override;
    virtual ECSqlStatus _BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata) override;
    virtual ECSqlStatus _BindDouble(double value) override;
    virtual ECSqlStatus _BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override;
    virtual ECSqlStatus _BindInt(int value) override;
    virtual ECSqlStatus _BindInt64(int64_t value) override;
    virtual ECSqlStatus _BindPoint2D (DPoint2dCR value) override;
    virtual ECSqlStatus _BindPoint3D (DPoint3dCR value) override;
    virtual ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override;

    bool IsEnsureConstraints() const { return m_constraints != nullptr; }

    ECSqlStatus FailIfConstraintClassIdViolation(ECN::ECClassId const& constraintClassId) const;

    Utf8CP SystemPropertyToString() const;

public:
    SystemPropertyECSqlBinder(ECSqlStatementBase&, ECSqlTypeInfo const&, PropertyNameExp const&, bool isNoop, bool enforceConstraints);
    ~SystemPropertyECSqlBinder() {}

    //!Only called in a single case where ECSQL constains source/target ECClassId, but it does not map
    //!to a column. In this case the preparer calls this method and all calls to Bind on this binder
    //!will not be routed down to SQLite.
    void SetIsNoop() { m_isNoop = true; }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE