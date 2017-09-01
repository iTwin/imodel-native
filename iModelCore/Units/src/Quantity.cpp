/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Quantity.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <complex>      // for std::abs

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
static almost_equal(const T x, const T y, int ulp)
    {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
        // unless the result is subnormal            
        || std::abs(x - y) < std::numeric_limits<T>::min();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
//QuantityCP Quantity::Create (double magnitude, Utf8CP unitName)
//    {
//    auto unit = UnitRegistry::Instance().LookupUnit(unitName);
//    if (nullptr == unit)
//        return nullptr;
//
//    return new Quantity (magnitude, unit);
//    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                             David Fox-Rabinovitz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
//QuantityCP Quantity::Create (double magnitude, UnitCP unit)
//    {
//    return Quantity (magnitude, unit);
//    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity::Quantity (double magnitude, UnitCR unit)
    {
    m_unit = &unit;
    m_magnitude = magnitude;
    m_tolerance = 1000;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity::Quantity(QuantityCR rhs)
    {
    m_unit = rhs.m_unit;
    m_magnitude = rhs.m_magnitude;
    m_tolerance = rhs.m_tolerance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
//BentleyStatus Quantity::ConvertTo(Utf8CP unitName, double& value) const
//    {
//    auto newUnit = UnitRegistry::Instance().LookupUnit(unitName);
//    if (nullptr == newUnit)
//        return ERROR;
//
//    if (m_unit == newUnit)
//        {
//        value = m_magnitude;
//        return SUCCESS;
//        }
//
//    value = m_unit->Convert(m_magnitude, newUnit);
//    if (value == 0.0)
//        return ERROR;
//
//    return SUCCESS;
//    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                               David.Fox-Rabinovitz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Quantity::ConvertTo(UnitCP unit, double& value) const
    {
    if (m_unit == unit)
        {
        value = m_magnitude;
        return SUCCESS;
        }

    m_unit->Convert(value, m_magnitude, unit);
    if (value == 0.0)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                               David.Fox-Rabinovitz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
UnitsProblemCode Quantity::GetConvertedMagnitude(double& value, UnitCP unit) const
    {
    if (m_unit == unit)
        {
        value = m_magnitude;
        return UnitsProblemCode::NoProblem;
        }
    return m_unit->Convert(value, m_magnitude, unit);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
//QuantityCP Quantity::ConvertTo(Utf8CP unitName) const
//    {
//    double newValue;
//    UnitCP unit = UnitRegistry::Instance().LookupUnit(unitName);
//    if (nullptr == unit || SUCCESS != ConvertTo(unit, newValue))
//        return nullptr;
//
//    return  Quantity(newValue, unit);
//    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                            David.Fox-Rabinovitz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::ConvertTo(UnitCP unit) const
    {
    double newValue;
    if (nullptr == unit)
        return *this;  // returning  the current Quantity (without conversion)

    UnitsProblemCode prob = GetConvertedMagnitude(newValue, unit);
    if(UnitsProblemCode::NoProblem != prob)
        return Quantity();     // when impossible to convert - return invalid quantity

    return Quantity(newValue, *unit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostEqual (QuantityCR rhs) const
    {
    if (IsNullQuantity() || rhs.IsNullQuantity())
        {
        return false;
        }
    if (m_unit == rhs.m_unit && almost_equal(m_magnitude, rhs.m_magnitude, m_tolerance))
        return true;
    
    if (m_unit == rhs.m_unit)
        return false;

    double temp;
    if (SUCCESS != ConvertTo(rhs.m_unit, temp))
        return false;

    bool ae = almost_equal(temp, rhs.m_magnitude, m_tolerance);
    return ae; //almost_equal(temp, rhs.m_magnitude, m_tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostLessThan (QuantityCR rhs) const   // SignificantlyLess(
    {
    if (AlmostEqual(rhs))
        return false;

    double convertedValue;
    if (SUCCESS != rhs.ConvertTo(m_unit, convertedValue))
        return false;

    return m_magnitude < convertedValue;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostGreaterThan (QuantityCR rhs) const
    {
    if (this->AlmostEqual(rhs))
        return false;

    double convertedValue;
    if (SUCCESS != rhs.ConvertTo(m_unit, convertedValue))
        return false;

    return m_magnitude > convertedValue;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostGreaterThanOrEqual(QuantityCR rhs) const
    {
    return this->AlmostEqual(rhs) || this->AlmostGreaterThan(rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostLessThanOrEqual (QuantityCR rhs) const
    {
    return this->AlmostEqual(rhs) || this->AlmostLessThan(rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::Multiply(QuantityCR rhs) const
    {
    double newValue = m_magnitude * rhs.m_magnitude;

    auto newUnit = m_unit->MultiplyUnit(*rhs.m_unit);
    if (nullptr == newUnit)
        return Quantity();

    return Quantity(newValue, *newUnit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::Divide(QuantityCR rhs) const
    {
    double newValue = m_magnitude / rhs.m_magnitude;

    auto newUnit = m_unit->DivideUnit(*rhs.m_unit);
    if (nullptr == newUnit)
        return Quantity();

    return Quantity(newValue, *newUnit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::Add(QuantityCR rhs) const
    {
    double convertedValue;
    if (SUCCESS != rhs.ConvertTo(m_unit, convertedValue))
        return Quantity();

    double newValue = m_magnitude + convertedValue;
    return  Quantity(newValue, *m_unit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::Subtract(QuantityCR rhs) const 
    {
    double convertedValue;
    if (SUCCESS != rhs.ConvertTo(m_unit, convertedValue))
        return Quantity();

    double newValue = m_magnitude - convertedValue;
    return  Quantity(newValue, *m_unit);
    }

bool Quantity::IsNullQuantity() const 
    { 
    bool stat = (0.0 == m_magnitude && nullptr == m_unit);
    return stat; // (0.0 == m_magnitude && nullptr == m_unit);
    }
