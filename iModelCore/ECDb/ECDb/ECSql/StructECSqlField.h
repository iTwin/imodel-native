/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructECSqlField.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Affan.Khan      07/2013
//+===============+===============+===============+===============+===============+======
struct StructECSqlField : public ECSqlField, public IECSqlStructValue
    {
    friend struct ECSqlFieldFactory;

    private:
        std::vector<std::unique_ptr<ECSqlField>> m_structFields;

        StructECSqlField(ECSqlStatementBase& stmt, ECSqlColumnInfo const& colInfo) : ECSqlField(stmt, colInfo, false, false) {}
        //Before calling this, the child field must be complete. You must not add child fields to the child fields afterwards
        //Otherwise the flags m_needsInit and m_needsReset might become wrong
        void AppendField(std::unique_ptr<ECSqlField> field);

        bool _IsNull() const override;
        IECSqlPrimitiveValue const& _GetPrimitive() const override;
        IECSqlStructValue const& _GetStruct() const override { return *this; }
        IECSqlArrayValue const& _GetArray() const override;

        int _GetMemberCount() const override { return static_cast<int>(m_structFields.size()); }
        IECSqlValue const& _GetValue(int columnIndex) const override;
        Collection const& _GetChildren() const override { return m_structFields; }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE