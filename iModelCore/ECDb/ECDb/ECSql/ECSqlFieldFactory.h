/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <Bentley/NonCopyableClass.h>

#include "ECSqlPrepareContext.h"
#include "PropertyNameExp.h"
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlFieldFactory
    {
private:
    //static class
    ECSqlFieldFactory ();
    ~ECSqlFieldFactory ();

    static ECSqlStatus CreatePrimitiveField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo&&, ECN::PrimitiveType);
    static ECSqlStatus CreateStructField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo&&, PropertyNameExp const*);
    static ECSqlStatus CreatePrimitiveArrayField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo&&, ECN::PrimitiveType);
    static ECSqlStatus CreateStructArrayField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo&&);
    static ECSqlStatus CreateStructMemberFields(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, StructPropertyMap const&, ECSqlColumnInfo&&);

    static ECSqlColumnInfo CreateECSqlColumnInfoFromPropertyNameExp (ECSqlPrepareContext const& ctx, PropertyNameExp const& propertyNameExp);
    static ECSqlColumnInfo CreateECSqlColumnInfoFromGeneratedProperty (ECSqlPrepareContext const& ctx, ECN::ECPropertyCR generatedProperty);

public:
    static ECSqlStatus CreateField (ECSqlPrepareContext& ctx, DerivedPropertyExp const* derivedProperty, int startColumnIndex);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
