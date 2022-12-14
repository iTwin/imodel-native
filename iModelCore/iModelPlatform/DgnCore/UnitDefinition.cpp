/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define fc_tinyVal (1.0e-9)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int UnitDefinition::CompareRatios(double num1, double den1, double num2, double den2)
    {
    int     result;
    double  prod1 = num1 * den2;
    double  prod2 = num2 * den1;

    if (fc_tinyVal * fabs(prod1 + prod2) >= fabs(prod1 - prod2))
        result = 0;
    else if (prod1 > prod2)
        result = 1;
    else
        result = -1;

#if defined (debug)
    printf("Compare %f to %f\n", num1 / den1, num2 / den2);
    printf("  comparing %e with %e at tolerance %e\n", prod1, prod2, fc_tinyVal * fabs(prod1 + prod2));
    printf("  result: %d\n", result);
#endif

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitBase UnitDefinition::BaseFromInt(uint32_t val)
    {
    if (2 < val)
        { BeDataAssert(0); return UnitBase::None; }

    return (UnitBase) val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::UnitSystem UnitDefinition::SystemFromInt(uint32_t val)
    {
    if (3 < val)
        { BeDataAssert(0); return Dgn::UnitSystem::Undefined; }

    return (Dgn::UnitSystem) val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::Init(UnitBase base, Dgn::UnitSystem sys, double num, double den, Utf8CP label)
    {
    m_base = base;
    m_system = sys;
    m_numerator = num;
    m_denominator = den;
    
    if (NULL != label)
        m_label.assign(label);
    else
        m_label.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::Init(UnitDefinitionCR source)
    {
    m_base          = source.m_base;
    m_system        = source.m_system;
    m_numerator     = source.m_numerator;
    m_denominator   = source.m_denominator;
    m_label.assign(source.m_label.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::SetInvalid()
    {
    Init(UnitBase::None, Dgn::UnitSystem::Undefined, 0.0, 0.0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::SetLabel(Utf8CP val)
    {
    if (NULL != val)
        m_label.assign(val);
    else
        m_label.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitDefinition::UnitDefinition(UnitBase base, Dgn::UnitSystem sys, double num, double den, Utf8CP label)
    {
    Init(base, sys, num, den, label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinition::IsValid() const
    {
    return (0.0 < m_numerator && 0.0 < m_denominator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinition::IsEqual(UnitDefinitionCR other) const
    {
    if (m_base        != other.m_base        ||
        m_system      != other.m_system      ||
        m_numerator   != other.m_numerator   ||
        m_denominator != other.m_denominator)
        {
        return false;
        }

    return m_label.Equals(other.m_label);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinition::AreComparable(UnitDefinitionCR other) const
    {
    if (m_base != other.m_base)
        return false;

    if (!(IsValid() && other.IsValid()))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int UnitDefinition::CompareByScale(UnitDefinitionCR other) const
    {
    if (!AreComparable(other))
        return ERROR;

    return CompareRatios(m_numerator, m_denominator, other.m_numerator, other.m_denominator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double UnitDefinition::GetConversionFactorFrom(UnitDefinitionCR from) const
    {
    if (!AreComparable(from))
        return 1.0;

    return (m_numerator   * from.m_denominator) / (m_denominator * from.m_numerator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double UnitDefinition::ConvertDistanceFrom(double inputVal, UnitDefinitionCR from) const
    {
    if (!AreComparable(from))
        return inputVal;

    return (inputVal * m_numerator   * from.m_denominator) / (m_denominator * from.m_numerator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::Scale(double scaleByThis)
    {
    m_numerator /= scaleByThis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::Square()
    {
    m_numerator = m_numerator * m_numerator;
    m_denominator = m_denominator * m_denominator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitDefinition::Cube()
    {
    m_numerator = m_numerator * m_numerator * m_numerator;
    m_denominator = m_denominator * m_denominator * m_denominator;
    }
