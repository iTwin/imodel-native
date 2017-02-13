/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlFieldFactory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        ECSqlFieldFactory();
        ~ECSqlFieldFactory();

        static ECSqlStatus CreatePrimitiveField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&, ECN::PrimitiveType);
        static ECSqlStatus CreateStructField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&, ECN::ECStructClassCR);
        static ECSqlStatus CreateNavigationPropertyField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&);
        static ECSqlStatus CreateArrayField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&);
        static ECSqlStatus CreateStructMemberFields(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECN::ECStructClassCR, ECSqlColumnInfo const&);
        static ECSqlStatus CreateChildField(std::unique_ptr<ECSqlField>& childField, ECSqlPrepareContext&, int& sqlColumnIndex, ECSqlColumnInfo const& parentFieldColumnInfo, ECN::ECPropertyCR childProperty);

        static ECSqlColumnInfo CreateECSqlColumnInfoFromPropertyNameExp(ECSqlPrepareContext const&, PropertyNameExp const&);
        static ECSqlColumnInfo CreateECSqlColumnInfoFromGeneratedProperty(ECSqlPrepareContext const&, ECN::ECPropertyCR generatedProperty);

    public:
        static ECSqlStatus CreateField(ECSqlPrepareContext&, DerivedPropertyExp const* derivedProperty, int startColumnIndex);


    };

END_BENTLEY_SQLITE_EC_NAMESPACE
