/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

    static ECSqlStatus CreatePrimitiveField
    (
    std::unique_ptr<ECSqlField>& field,
    int& sqlColumnIndex,
    ECSqlPrepareContext& ctx, 
    ECSqlColumnInfo&& columnInfo,
    PropertyNameExp const* propertyName,
    PrimitiveType primitiveType
    );

    static ECSqlStatus CreateStructField
    (
    std::unique_ptr<ECSqlField>& field,
    int& sqlColumnIndex,
    ECSqlPrepareContext& ctx, 
    ECSqlColumnInfo&& columnInfo,
    PropertyNameExp const* propertyName
    );

    static ECSqlStatus CreatePrimitiveArrayField
    (
    std::unique_ptr<ECSqlField>& field,
    int& sqlColumnIndex,
    ECSqlPrepareContext& ctx, 
    ECSqlColumnInfo&& columnInfo,
    PropertyNameExp const* propertyName,
    PrimitiveType primitiveType
    );

    static ECSqlStatus CreateStructArrayField
    (
    std::unique_ptr<ECSqlField>& field,
    int& sqlColumnIndex,
    ECSqlPrepareContext& ctx, 
    ECSqlColumnInfo&& ecsqlColumnInfo, 
    PropertyMapCR propertyMap
    );

    static ECSqlStatus CreateStructMemberFields
    (
    std::unique_ptr<ECSqlField>& structField, 
    int& sqlColumnIndex, 
    ECSqlPrepareContext& ctx, 
    PropertyMapToInLineStructCR structPropertyMap,
    ECSqlColumnInfo&& structFieldColumnInfo
    );

    static ECSqlColumnInfo CreateECSqlColumnInfoFromPropertyNameExp (ECSqlPrepareContext const& ctx, PropertyNameExp const& propertyNameExp);
    static ECSqlColumnInfo CreateECSqlColumnInfoFromGeneratedProperty (ECSqlPrepareContext const& ctx, ECPropertyCR generatedProperty);

public:
    //WIP_ECSQL: Impl seems to do a lot more than just creating the reader. Shouldn't go more of this to the preparer?
    static ECSqlStatus CreateField (ECSqlPrepareContext& ctx, DerivedPropertyExp const* derivedProperty, int startColumnIndex);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
