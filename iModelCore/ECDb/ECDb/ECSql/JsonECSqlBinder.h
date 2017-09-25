/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include <json/json.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//======================================================================================
//! Helper API to bind values from EC JSON members to ECSQL parameters
// @bsiclass                                                  09/2017
//+===============+===============+===============+===============+===============+======
struct JsonECSqlBinder final
    {
    private:
        JsonECSqlBinder() = delete;
        ~JsonECSqlBinder() = delete;

        static ECSqlStatus BindValue(IECSqlBinder& binder, JsonValueCR memberJson, ECN::ECPropertyCR);
        static ECSqlStatus BindPrimitiveValue(IECSqlBinder&, JsonValueCR primJson, ECN::PrimitiveType);
        static ECSqlStatus BindStructValue(IECSqlBinder&, JsonValueCR structJson, ECN::ECStructClassCR);
        static ECSqlStatus BindArrayValue(IECSqlBinder&, JsonValueCR arrayJson, ECN::ArrayECPropertyCR);
        static ECSqlStatus BindNavigationValue(IECSqlBinder&, JsonValueCR navJson, ECN::NavigationECPropertyCR, ECN::IECClassLocater&);

        static ECSqlStatus BindValue(IECSqlBinder& binder, RapidJsonValueCR memberJson, ECN::ECPropertyCR);
        static ECSqlStatus BindPrimitiveValue(IECSqlBinder&, RapidJsonValueCR primJson, ECN::PrimitiveType);
        static ECSqlStatus BindStructValue(IECSqlBinder&, RapidJsonValueCR structJson, ECN::ECStructClassCR);
        static ECSqlStatus BindArrayValue(IECSqlBinder&, RapidJsonValueCR arrayJson, ECN::ArrayECPropertyCR);
        static ECSqlStatus BindNavigationValue(IECSqlBinder&, RapidJsonValueCR navJson, ECN::NavigationECPropertyCR, ECN::IECClassLocater&);

    public:
        //! Binds the value of the specified EC JSON member to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to
        //! @param[in] memberJson EC JSON member whose value is to be bound to @p binder
        //! @param[in] prop ECProperty of the parameter expression
        //! @param[in] classLocater Class locater needed to look up ECClasses or ECClassIds from EC JSON class names (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindValue(IECSqlBinder& binder, JsonValueCR memberJson, ECN::ECPropertyCR prop, ECN::IECClassLocater& classLocater);

        //! Binds the value of the specified EC JSON member to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to
        //! @param[in] memberJson EC JSON member whose value is to be bound to @p binder
        //! @param[in] prop ECProperty of the parameter expression
        //! @param[in] classLocater Class locater needed to look up ECClasses or ECClassIds from EC JSON class names (e.g. via ECDb::GetClassLocater).
        //! @return ECSqlStatus
        ECDB_EXPORT static ECSqlStatus BindValue(IECSqlBinder& binder, RapidJsonValueCR memberJson, ECN::ECPropertyCR prop, ECN::IECClassLocater& classLocater);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE