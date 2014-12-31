/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructMappedToColumnsECSqlField.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
struct StructMappedToColumnsECSqlField : public ECSqlField, public IECSqlStructValue
    {
private:
    std::vector<std::unique_ptr<ECSqlField>> m_structFields;

    virtual bool _IsNull () const override;
    virtual IECSqlPrimitiveValue const& _GetPrimitive () const override;
    virtual IECSqlStructValue const& _GetStruct () const override;
    virtual IECSqlArrayValue const& _GetArray () const override;

    virtual int _GetMemberCount () const override;
    virtual IECSqlValue const& _GetValue (int columnIndex) const override;

    bool CanRead (int columnIndex) const;
public:
    StructMappedToColumnsECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo);

    void AppendField (std::unique_ptr<ECSqlField> field);

    virtual Collection const& GetChildren () const override;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE