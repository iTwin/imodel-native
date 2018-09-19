/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Quantity.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <complex> // for std::abs

BEGIN_BENTLEY_UNITS_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              David Fox-Rabinovitz     12/17
+---------------+---------------+---------------+---------------+---------------+------*/
static double absMax(double a, double b)
    {
    a = fabs(a);
    b = fabs(b);
    double diff = a - b;
    double max = (diff >= 0.0) ? a : b;
    return max;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              David Fox-Rabinovitz     12/17
+---------------+---------------+---------------+---------------+---------------+------*/
static double relDiff(double a, double b)
    {
    double max = absMax(a, b);
    double diff = fabs(a - b);
    if (max > 1.0e-12 && diff > 1.0e-12)
        return diff / max;
    return 0.0;
    }

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
Quantity::Quantity(QuantityCR rhs)
    {
    m_unit = rhs.m_unit;
    m_magnitude = rhs.m_magnitude;
    m_tolerance = rhs.m_tolerance;
    }

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

    UnitsProblemCode prob = m_unit->Convert(value, m_magnitude, unit);
    if (prob != UnitsProblemCode::NoProblem)
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

    if (nullptr == m_unit)
        return UnitsProblemCode::InvalidUnitName;

    return m_unit->Convert(value, m_magnitude, unit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                            David.Fox-Rabinovitz     02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::ConvertTo(UnitCP unit) const
    {
    double newValue;
    if (nullptr == unit)
        return *this; // returning the current Quantity (without conversion)

    UnitsProblemCode prob = GetConvertedMagnitude(newValue, unit);
    if(UnitsProblemCode::NoProblem != prob)
        return Quantity(); // when impossible to convert - return invalid quantity

    return Quantity(newValue, *unit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostEqual (QuantityCR rhs) const
    {
    if (IsNullQuantity() || rhs.IsNullQuantity())
        return false;

    if (m_unit == rhs.m_unit && almost_equal(m_magnitude, rhs.m_magnitude, m_tolerance))
        return true;

    if (m_unit == rhs.m_unit)
        return false;

    double temp;
    if (SUCCESS != ConvertTo(rhs.m_unit, temp))
        return false;

    return almost_equal(temp, rhs.m_magnitude, m_tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              David Fox-Rabinovitz     12/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::IsClose(QuantityCR rhs, double tolerance) const
    {
    if (IsNullQuantity() || rhs.IsNullQuantity() || GetPhenomenon() != rhs.GetPhenomenon())
        return false;

    double temp;
    if (SUCCESS != ConvertTo(rhs.m_unit, temp))
        return false;
    double rd = relDiff(rhs.m_magnitude, temp);
    return rd < tolerance;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::AlmostLessThan (QuantityCR rhs) const
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

END_BENTLEY_UNITS_NAMESPACE
