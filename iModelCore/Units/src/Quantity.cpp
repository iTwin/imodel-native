/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitRegistry.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Units/UnitsPCH.h>

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityBase::QuantityBase (double quantity, Unit& unit) : m_unit(move(unit))
	{
	m_quantity = quantity;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool operator== (const QuantityBase& rhs) const
	{
	return !(*this != rhs);
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool operator!= (const QuantityBase& rhs) const
	{
	return rhs.m_quantity != m_quantity || rhs.m_unit != m_unit;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase operator*(const QuantityBase& rhs) const
	{
	QuantityBase result = *this;
	result *= rhs;
	return result;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase operator/(const QuantityBase& rhs) const
	{
	QuantityBase result = *this;
	result /= rhs;
	return result;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase operator+(const QuantityBase& rhs) const
	{
	QuantityBase result = *this;
	result += rhs;
	return result;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase operator-(const QuantityBase& rhs) const
	{
	QuantityBase result = *this;
	result -= rhs;
	return result;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase& operator*=(const QuantityBase& rhs)
	{
	m_quantity *= rhs.m_quantity;
	m_unit *= rhs.m_unit;

	return *this;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase& operator/=(const QuantityBase& rhs)
	{
	m_quantity /= rhs.m_quantity;
	m_unit /= rhs.m_unit;

	return *this;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase& operator+=(const QuantityBase& rhs)
	{
	m_quantity += rhs.m_quantity;
	m_unit += rhs.m_unit;

	return *this;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual QuantityBase& operator-=(const QuantityBase& rhs)
	{
	m_quantity -= rhs.m_quantity;
	m_unit -= rhs.m_unit;

	return *this;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Quantity::Quantity(double quantity, Unit& unit) : QuantityBase(quantity, unit) 
	{ 

	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
QuantityPtr Quantity::Create (double magnitude, Utf8CP unitName)
	{
	auto unit = UnitRegistry::Instance().LookupUnit(unitName);
	if (!unit.IsValid())
		return nullptr;

	auto quant = new Quantity(magnitude, *unit);
	return quant;
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Constant::Constant (Utf8CP constantName, double magnitude, Unit &unit) : QuantityBase(magnitude, unit)
	{
	m_name = Utf8String(constantName);
	}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
ConstantPtr Constant::Create (Utf8CP constantName, double magnitude, Utf8CP unitName)
	{
	auto unit = UnitRegistry::Instance().LookupUnit(unitName);
	if (!unit.IsValid())
		return nullptr;

	auto constant = new Constant(constantName, magnitude, unit);
	return constant;
	}