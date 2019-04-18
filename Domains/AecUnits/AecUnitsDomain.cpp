/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "AecUnitsInternal.h"


BEGIN_BENTLEY_NAMESPACE

namespace AecUnits
	{

	DOMAIN_DEFINE_MEMBERS(AecUnitsDomain)

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	AecUnitsDomain::AecUnitsDomain() : DgnDomain(BENTLEY_AEC_UNITS_SCHEMA_NAME, "Aec Units Schema", 1)
		{
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void AecUnitsDomain::_OnSchemaImported(Dgn::DgnDbR dgndb) const
		{
		Dgn::DgnSubCategory::Appearance defaultApperance;
		defaultApperance.SetInvisible(false);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::CodeSpecId  AecUnitsDomain::QueryAecUnitsCodeSpecId(Dgn::DgnDbCR dgndb)
		{
		Dgn::CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BENTLEY_AEC_UNITS_AUTHORITY);
		BeAssert(codeSpecId.IsValid());
		return codeSpecId;
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	Dgn::DgnCode AecUnitsDomain::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value)
		{
		return Dgn::CodeSpec::CreateCode(dgndb, BENTLEY_AEC_UNITS_AUTHORITY, value);
		}

	//---------------------------------------------------------------------------------------
	// @bsimethod                                   Bentley.Systems
	//---------------------------------------------------------------------------------------
	void AecUnitsDomain::_OnDgnDbOpened(Dgn::DgnDbR db) const
		{
		}


    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    ECN::ECUnitCP    AecUnitsUtilities::GetUnitCPFromProperty(Dgn::DgnElementCR element, Utf8StringCR propertyName)
        {
        ECN::ECClassCP elementClass = element.GetElementClass();

        if (nullptr == elementClass)
            return nullptr;

        ECN::ECPropertyP prop = elementClass->GetPropertyP(propertyName);

        if (nullptr == prop)
            return nullptr;

        ECN::KindOfQuantityCP propUnit = prop->GetKindOfQuantity();

        if (nullptr == propUnit)
            return nullptr;

        return propUnit->GetPersistenceUnit();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   AecUnitsUtilities::GetDoublePropertyUsingUnitString(Dgn::DgnElementCR element, Utf8StringCR propertyName, Utf8StringCR unitString, double& value)
        {
        ECN::ECUnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        // Excepting unitString to be a fully qualified Unit name, {SchemaName}:{UnitName}.
        Utf8String unitSchemaName;
        Utf8String unitName;
        ECN::ECClass::ParseClassName(unitSchemaName, unitName, unitString);

        ECN::ECUnitCP u1 = element.GetDgnDb().Schemas().GetUnit(unitSchemaName, unitName);

        ECN::ECValue propVal;

        if (Dgn::DgnDbStatus::Success != element.GetPropertyValue(propVal, propertyName.c_str()))
            return BentleyStatus::ERROR;

        if (propVal.IsNull())
            return BentleyStatus::ERROR;

        double storedValue = propVal.GetDouble();

        Units::UnitsProblemCode code = u->Convert(value, storedValue, u1);

        if (Units::UnitsProblemCode::NoProblem != code)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;

        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   AecUnitsUtilities::SetDoublePropertyUsingUnitString(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR unitString, double value)
        {
        ECN::ECUnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        // Excepting unitString to be a fully qualified Unit name, {SchemaName}:{UnitName}.
        Utf8String unitSchemaName;
        Utf8String unitName;
        ECN::ECClass::ParseClassName(unitSchemaName, unitName, unitString);

        ECN::ECUnitCP u1 = element.GetDgnDb().Schemas().GetUnit(unitSchemaName, unitName);

        double converted;
        Units::UnitsProblemCode code = u1->Convert(converted, value, u);

        if (Units::UnitsProblemCode::NoProblem != code)
            return BentleyStatus::ERROR;

        ECN::ECValue doubleValue;

        doubleValue.SetDouble(converted);

        Dgn::DgnDbStatus status = element.SetPropertyValue(propertyName.c_str(), doubleValue);

        if (Dgn::DgnDbStatus::Success != status)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }

    } // End AecUnits namespace

END_BENTLEY_NAMESPACE
