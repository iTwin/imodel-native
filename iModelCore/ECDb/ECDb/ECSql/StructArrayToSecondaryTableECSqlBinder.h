/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayToSecondaryTableECSqlBinder.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlBinder.h"
#include "ECSqlParameterValue.h"
#include <ECDb/IECSqlValue.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      01/2014
//+===============+===============+===============+===============+===============+======
struct StructArrayToSecondaryTableECSqlBinder : public ECSqlBinder, public IECSqlArrayBinder
    {
private:
    std::unique_ptr<ArrayECSqlParameterValue> m_value;

    virtual void _SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteParameterIndex) override;
    virtual void _OnClearBindings () override;
    virtual ECSqlStatus _OnBeforeStep () override;

    virtual IECSqlBinder& _AddArrayElement () override;

    virtual ECSqlStatus _BindNull () override;
    virtual IECSqlPrimitiveBinder& _BindPrimitive () override;
    virtual IECSqlStructBinder& _BindStruct () override;
    virtual IECSqlArrayBinder& _BindArray (uint32_t initialCapacity) override;

public:
    StructArrayToSecondaryTableECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo);
    ArrayECSqlParameterValue& GetParameterValue () { return *m_value; }
    ~StructArrayToSecondaryTableECSqlBinder () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
