/*--------------------------------------------------------------------------------------+
|
|     $Source: AecUnitsDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

    Units::UnitCP    AecUnitsUtilities::GetUnitCPFromProperty(Dgn::DgnElementCR element, Utf8StringCR propertyName)
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

        Formatting::FormatUnitSetCR unit = propUnit->GetPersistenceUnit();

        Units::UnitCP u = unit.GetUnit();

        return u;
        }


    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   AecUnitsUtilities::SetDoublePropertyFromStringWithUnits(Dgn::DgnElementR element, Utf8StringCR propertyName, Utf8StringCR propertyValueString)
        {

        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Utf8String numbers = "-1234567890.+";

        int pos = propertyValueString.find_first_not_of(numbers, 0);

        Utf8String valueString = propertyValueString.substr(0, pos);
        Utf8String unitSuffix = propertyValueString.substr(pos, propertyValueString.length());

        unitSuffix = unitSuffix.Trim();

        Units::UnitCP u1 = Units::UnitRegistry::Instance().LookupUnit(unitSuffix.c_str());

        double value = atof(valueString.c_str());

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

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Bentley.Systems
    //---------------------------------------------------------------------------------------

    BentleyStatus   AecUnitsUtilities::GetDoublePropertyUsingUnitString(Dgn::DgnElementCR element, Utf8StringCR propertyName, Utf8StringCR unitString, double& value)
        {
        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Units::UnitCP u1 = Units::UnitRegistry::Instance().LookupUnit(unitString.c_str());

        ECN::ECValue propVal;

        if (Dgn::DgnDbStatus::Success != element.GetPropertyValue(propVal, propertyName.c_str()))
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
        Units::UnitCP u = GetUnitCPFromProperty(element, propertyName);

        if (nullptr == u)
            return BentleyStatus::ERROR;

        Units::UnitCP u1 = Units::UnitRegistry::Instance().LookupUnit(unitString.c_str());

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
