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
QuantityBase::QuantityBase (double magnitude, UnitCR unit)
    {
    m_unit = &unit;
    m_magnitude = magnitude;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuantityBase::operator== (const QuantityBase& rhs) const
    {
    return !(*this != rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuantityBase::operator!= (const QuantityBase& rhs) const
    {
    return rhs.m_magnitude != m_magnitude || rhs.m_unit != m_unit;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase QuantityBase::operator*(const QuantityBase& rhs) const
    {
    QuantityBase result = *this;
    result *= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase QuantityBase::operator/(const QuantityBase& rhs) const
    {
    QuantityBase result = *this;
    result /= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase QuantityBase::operator+(const QuantityBase& rhs) const
    {
    QuantityBase result = *this;
    result += rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase QuantityBase::operator-(const QuantityBase& rhs) const
    {
    QuantityBase result = *this;
    result -= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase& QuantityBase::operator*=(const QuantityBase& rhs)
    {
    m_magnitude *= rhs.m_magnitude;
    UnitCR resultUnit = *m_unit * *rhs.m_unit;
    if (!resultUnit.IsRegistered())
        {
        m_error = true;
        m_errorMessage = Utf8PrintfString("Resulting unit is unregistered: %s", resultUnit.GetName());
        return *this;
        }

    m_unit = &resultUnit;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase& QuantityBase::operator/=(const QuantityBase& rhs)
    {
    m_magnitude /= rhs.m_magnitude;
    UnitCR resultUnit = *m_unit / *rhs.m_unit;
    if (!resultUnit.IsRegistered())
        {
        m_error = true;
        m_errorMessage = Utf8PrintfString("Resulting unit is unregistered: %s", resultUnit.GetName());
        return *this;
        }

    m_unit = &resultUnit;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase& QuantityBase::operator+=(const QuantityBase& rhs)
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
QuantityBase& QuantityBase::operator-=(const QuantityBase& rhs)
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

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity::Quantity(double magnitude, UnitCR unit) : QuantityBase(magnitude, unit)
    { 

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityPtr Quantity::Create (double magnitude, Utf8CP unitName)
    {
    auto unit = UnitRegistry::Instance().LookupUnit(unitName);
    if (nullptr == unit)
        return nullptr;

    auto quant = new Quantity(magnitude, *unit);
    return quant;
    }

/////*--------------------------------------------------------------------------------**//**
////* @bsimethod                                              Chris.Tartamella     02/16
////+---------------+---------------+---------------+---------------+---------------+------*/
////Constant::Constant (Utf8CP constantName, double magnitude, Unit &unit) : QuantityBase(magnitude, unit)
////    {
////    m_name = Utf8String(constantName);
////    }
////
/////*--------------------------------------------------------------------------------**//**
////* @bsimethod                                              Chris.Tartamella     02/16
////+---------------+---------------+---------------+---------------+---------------+------*/
////ConstantPtr Constant::Create (Utf8CP constantName, double magnitude, Utf8CP unitName)
////    {
////    auto unit = UnitRegistry::Instance().LookupUnit(unitName);
////    if (!unit.IsValid())
////        return nullptr;
////
////    auto constant = new Constant(constantName, magnitude, unit);
////    return constant;
////    }

