/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Quantity.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityPtr Quantity::Create (double magnitude, Utf8CP unitName)
    {
    auto unit = UnitRegistry::Instance().LookupUnit(unitName);
    if (nullptr == unit)
        return nullptr;

    return new Quantity (magnitude, unit);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity::Quantity (double magnitude, UnitCP unit)
    {
    m_unit = unit;
    m_magnitude = magnitude;
    }

Quantity::Quantity(const Quantity& rhs)
    {
    m_unit = rhs.m_unit;
    m_magnitude = rhs.m_magnitude;
    m_error = false;
    m_errorMessage = "";
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Quantity::ConvertTo(Utf8CP unitName, double& value) const
    {
    auto newUnit = UnitRegistry::Instance().LookupUnit(unitName);
    if (nullptr == newUnit)
        return ERROR;

    if (m_unit == newUnit)
        {
        value = m_magnitude;
        return SUCCESS;
        }

    auto newFactor = m_unit->GetConversionTo(newUnit);
    value = m_magnitude * newFactor;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::operator== (const Quantity& rhs) const
    {
    if (m_unit == rhs.m_unit && m_magnitude == rhs.m_magnitude)
        return true;
    
    if (m_unit == rhs.m_unit)
        return false;

    double temp;
    if (SUCCESS != ConvertTo(rhs.m_unit->GetName(), temp))
        return false;

    return temp == rhs.m_magnitude;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Quantity::operator!= (const Quantity& rhs) const
    {
    return !(*this == rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::operator*(const Quantity& rhs) const
    {
    Quantity result = *this;
    result *= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::operator/(const Quantity& rhs) const
    {
    Quantity result = *this;
    result /= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::operator+(const Quantity& rhs) const
    {
    Quantity result = *this;
    result += rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity Quantity::operator-(const Quantity& rhs) const
    {
    Quantity result = *this;
    result -= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity& Quantity::operator*=(const Quantity& rhs)
    {
    //m_magnitude *= rhs.m_magnitude;
    //UnitCR resultUnit = *m_unit * *rhs.m_unit;
    //if (!resultUnit.IsRegistered())
    //    {
    //    m_error = true;
    //    m_errorMessage = Utf8PrintfString("Resulting unit is unregistered: %s", resultUnit.GetName());
    //    return *this;
    //    }

    //m_unit = &resultUnit;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity& Quantity::operator/=(const Quantity& rhs)
    {
    //m_magnitude /= rhs.m_magnitude;
    //UnitCR resultUnit = *m_unit / *rhs.m_unit;
    //if (!resultUnit.IsRegistered())
    //    {
    //    m_error = true;
    //    m_errorMessage = Utf8PrintfString("Resulting unit is unregistered: %s", resultUnit.GetName());
    //    return *this;
    //    }

    //m_unit = &resultUnit;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity& Quantity::operator+=(const Quantity& rhs)
    {
    if (m_unit != rhs.m_unit)
        {
        m_error = true;
        m_errorMessage = Utf8PrintfString("Units are not compatible.  left: %s.  right: %s", m_unit->GetName(), rhs.m_unit->GetName());
        return *this;
        }

    m_magnitude += rhs.m_magnitude;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity& Quantity::operator-=(const Quantity& rhs)
    {
    if (m_unit != rhs.m_unit)
        {
        m_error = true;
        m_errorMessage = Utf8PrintfString("Units are not compatible.  left: %s.  right: %s", m_unit->GetName(), rhs.m_unit->GetName());
        return *this;
        }

    m_magnitude -= rhs.m_magnitude;

    return *this;
    }
