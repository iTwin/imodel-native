/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlField.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyECSqlField : public ECSqlField, public IECSqlStructValue
    {
    friend struct ECSqlFieldFactory;

    private:
        std::unique_ptr<ECSqlField> m_idField;
        std::unique_ptr<ECSqlField> m_relClassIdField;

        NavigationPropertyECSqlField(ECSqlStatementBase& stmt, ECSqlColumnInfo const& colInfo)
            : ECSqlField(stmt, colInfo, false, false), IECSqlStructValue(), m_idField(nullptr), m_relClassIdField(nullptr)
            {}

        virtual bool _IsNull() const override { return m_idField->IsNull() && m_relClassIdField->IsNull(); }
        virtual IECSqlPrimitiveValue const& _GetPrimitive() const override;
        virtual IECSqlStructValue const& _GetStruct() const override { return *this; }
        virtual IECSqlArrayValue const& _GetArray() const override;

        virtual int _GetMemberCount() const override { return 2; }
        virtual IECSqlValue const& _GetValue(int columnIndex) const override;

        void SetMembers(std::unique_ptr<ECSqlField> idField, std::unique_ptr<ECSqlField> relClassIdField);

    public:
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
