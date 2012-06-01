/*--------------------------------------------------------------------------------------+
|
|  $Source: src/ECCompare.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCompareCache::PropertyInfo::PropertyInfo (ECPropertyCR prop) : m_property (&prop), m_flags (0)
    {
    if (NULL != prop.GetAsArrayProperty())
        m_flags |= PROPERTYFLAG_Array;
    else if (NULL != prop.GetAsStructProperty())
        {
        BeAssert (false);       // should not happen, embedded structs are flattened out by property indices...
        m_property = NULL;
        }
    else
        {
        PrimitiveECPropertyP primitiveProp = prop.GetAsPrimitiveProperty();     // because IsDefined() is non-const...
        // Integer properties may or may not represent quantities...
        if (primitiveProp->GetType() == PRIMITIVETYPE_Integer && (primitiveProp->IsDefined (L"StandardValues") || primitiveProp->IsDefined (L"ExtendType")))
            m_flags |= PROPERTYFLAG_Enum;
        }

    if (prop.GetCustomAttribute (L"StrictComparisonOnly").IsValid())
        m_flags |= PROPERTYFLAG_Strict;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCompareCache::PropertyInfoMap const& ECClassCompareCache::GetProperties (ECEnablerCR enabler)
    {
    ECClassCR ecClass = enabler.GetClass();
    ClassMap::const_iterator iter = m_classMap.find (&ecClass);
    if (iter != m_classMap.end())
        return iter->second;

    PropertyInfosByIndex propertyMap;
    UInt32 highestIndex = 0;
    PopulatePropertyMap (propertyMap, highestIndex, ecClass, L"", enabler);

    PropertyInfoMap& properties = m_classMap[&ecClass];
    properties.resize (highestIndex + 1);
    for (PropertyInfosByIndex::const_iterator iter = propertyMap.begin(); iter != propertyMap.end(); ++iter)
        properties[iter->first] = iter->second;

    return properties;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClassCompareCache::PopulatePropertyMap (PropertyInfosByIndex& propertyMap, UInt32& highestPropertyIndex, EC::ECClassCR ecClass, WStringCR parentAccessString, ECEnablerCR enabler)
    {
    StructECPropertyCP      structProp;

    FOR_EACH (ECPropertyCP property, ecClass.GetProperties (true))
        {
        if (NULL == property)
            continue;

        WString accessString = parentAccessString;
        if (!accessString.empty())
            accessString.append (1, '.');

        accessString.append (property->GetName());

        if (NULL != (structProp = property->GetAsStructProperty()))
            {
            PopulatePropertyMap (propertyMap, highestPropertyIndex, structProp->GetType(), accessString, enabler);
            continue;
            }

        if (NULL != property->GetAsArrayProperty())
            accessString.append (L"[]");

        UInt32 propertyIndex;
        if (ECOBJECTS_STATUS_Success != enabler.GetPropertyIndex (propertyIndex, accessString.c_str()))
            {
            BeAssert (false); continue;
            }

        propertyMap[propertyIndex] = PropertyInfo (*property);
        if (propertyIndex > highestPropertyIndex)
            highestPropertyIndex = propertyIndex;
        }
    }

////////////////////////////////////////
//  ECPropertyDiff
///////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyDiff::GetBaselineValue (ECValueR v) const
    {
    return ECOBJECTS_STATUS_Success == m_baselineInstance.GetValue (v, m_propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyDiff::GetComparandValue (ECValueR v) const
    {
    return ECOBJECTS_STATUS_Success == m_comparandInstance.GetValue (v, m_propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECPropertyDiff::GetAccessString() const
    {
    WCharCP accessString = NULL;
    m_baselineInstance.GetEnabler().GetAccessString (accessString, m_propertyIndex);
    BeAssert (NULL != accessString);
    return accessString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double ECInstanceComparer::ScorePrimitives (ECValueCR lhs, ECValueCR rhs, bool compareArithmetically) const
    {
    BeAssert (lhs.IsNull() || rhs.IsNull() || (lhs.IsPrimitive() && rhs.IsPrimitive()));

    double score = lhs.IsNull() == rhs.IsNull() ? 0.0 : 1.0;
    if (0.0 == score && !lhs.IsNull() && lhs.GetPrimitiveType() == rhs.GetPrimitiveType())
        {
        if (!compareArithmetically)
            score = lhs.Equals (rhs) ? 0.0 : 1.0;
        else
            {
            switch (lhs.GetPrimitiveType())
                {
            case PRIMITIVETYPE_Double:
                score = ScoreDoubles (lhs.GetDouble(), rhs.GetDouble());
                break;
            case PRIMITIVETYPE_Point3D:
                // or sum of components / 3.0
                score = ScoreDoubles (lhs.GetPoint3D().MagnitudeSquared(), rhs.GetPoint3D().MagnitudeSquared());
                break;
            case PRIMITIVETYPE_Point2D:
                score = ScoreDoubles (lhs.GetPoint2D().MagnitudeSquared(), rhs.GetPoint2D().MagnitudeSquared());
                break;
            case PRIMITIVETYPE_Long:
                score = ScoreIntegers (lhs.GetLong(), rhs.GetLong());
                break;
            case PRIMITIVETYPE_Integer:
                score = ScoreIntegers (lhs.GetInteger(), rhs.GetInteger());
                break;
            default:
                score = lhs.Equals (rhs) ? 0.0 : 1.0;
                break;
                }
            }
        }

    return score;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double ECInstanceComparer::ScoreDoubles (double a, double b) const
    {
    a = fabs(a);
    b = fabs(b);
    double hi = std::max (a, b), lo = std::min (a, b);
    if (0.0 == hi)
        return 0.0;     // clearly both are 0.0
    
    double score = (hi - lo) / hi;
    return (score > m_tolerance) ? score : 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double ECInstanceComparer::ScoreIntegers (Int64 a, Int64 b) const
    {
    Int64 aa = abs(a), bb = abs(b);
    Int64 hi = std::max(aa,bb),
          lo = std::min(aa,bb);
    if (hi == 0)
        return hi == lo ? 0.0 : 1.0;
    else
        return ((double)(hi - lo))/((double)hi);
    }

/*---------------------------------------------------------------------------------**//**
* Compare elements of two arrays of same size.
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceComparer::CompareArrays (double& score, UInt32 propIdx, UInt32 nElems, bool requireEquality, bool isStruct)
    {
    score = 0.0;
    if (0 == nElems)
        return true;

    double totalScore = 0.0;
    for (UInt32 i = 0; i < nElems; i++)
        {
        double elemScore = 0.0;
        ECValue lhV, rhV;
        ECObjectsStatus lhStatus    = m_lhs.GetValue (lhV, propIdx, i),
                        rhStatus    = m_rhs.GetValue (rhV, propIdx, i);
        if (lhStatus != rhStatus)
            elemScore = 1.0;
        else if (ECOBJECTS_STATUS_Success == lhStatus)
            {
            if (!isStruct)
                elemScore = ScorePrimitives (lhV, rhV, true);
            else
                {
                if (lhV.IsNull() != rhV.IsNull())
                    elemScore = 1.0;
                else if (!lhV.IsNull())
                    {
                    // compare the two instances
                    IECInstancePtr lhInstance = lhV.GetStruct(),
                                   rhInstance = rhV.GetStruct();
                    ECInstanceComparer structComparer (*lhInstance, *rhInstance, m_compareType, m_tolerance, m_cacheHolder.GetCache());
                    if (requireEquality)
                        {
                        if (structComparer.CheckEquality())
                            elemScore = 0.0;
                        else
                            return false;
                        }
                    else
                        elemScore = structComparer.Compare (NULL);
                    }
                }
            }

        if (requireEquality && 0.0 != elemScore)
            return false;

        totalScore += elemScore;
        }

    score = totalScore / (double)nElems;
    return 0.0 == totalScore;
    }

/*---------------------------------------------------------------------------------**//**
* Compare class properties. Output sum of property diff scores and number of properties.
* Return false if not equal.
* If requireEquality, stop as soon as a mismatch is found.
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceComparer::CompareProperties (double& totalScore, UInt32& numProperties, ECInstanceDiff* diffs, bool requireEquality)
    {
    numProperties = 0;
    totalScore = 0.0;

    ECClassCompareCache::PropertyInfoMap const& propertyMap = m_cacheHolder.GetCache()->GetProperties (m_lhs.GetEnabler());
    UInt32 propIdxMax = (UInt32)propertyMap.size();
    for (UInt32 propIdx = 0; propIdx < propIdxMax; propIdx++)
        {
        ECClassCompareCache::PropertyInfo const& info = propertyMap[propIdx];
        if (info.IsNull() || (info.IsStrict() && COMPARE_Strict != m_compareType))
            continue;

        double propScore = 0.0;
        ECValue lhV, rhV;
        ECObjectsStatus lhStatus = m_lhs.GetValue (lhV, propIdx),
                        rhStatus = m_rhs.GetValue (rhV, propIdx);
        if (lhStatus != rhStatus)
            propScore = 1.0;        // they differ; that's all we can say
        else if (lhStatus != ECOBJECTS_STATUS_Success)
            continue;               // ignore this property, it's not relevant
        else if (info.IsArray())
            {
            if (lhV.IsNull() != rhV.IsNull())
                propScore = 1.0;
            else if (lhV.IsNull())
                propScore = 0.0;
            else
                {
                BeAssert (lhV.IsArray() && rhV.IsArray());
                // ###TODO: we could optimize here - compare all primitive properties first, then only compare arrays if cumulative score below some threshold
                ArrayInfo arrayInfo = lhV.GetArrayInfo();
                UInt32 lhCount = arrayInfo.GetCount();
                if (lhCount != rhV.GetArrayInfo().GetCount())
                    propScore = 1.0;            // don't bother comparing array elements if sizes differ
                else if (!CompareArrays (propScore, propIdx, lhCount, requireEquality, arrayInfo.IsStructArray()) && requireEquality)
                    return false;
                }
            }
        else        // primitive
            {
            propScore = ScorePrimitives (lhV, rhV, !info.IsEnum());
            }

        if (0.0 != propScore)
            {
            if (requireEquality)
                return false;
            else if (NULL != diffs)
                diffs->AddDiff (propIdx, propScore);
            }

        ++numProperties;
        totalScore += propScore;        
        }

    return 0.0 == totalScore;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceComparer::CheckEquality()
    {
    BeAssert (&m_lhs.GetEnabler().GetClass() == &m_rhs.GetEnabler().GetClass());

    double score;
    UInt32 nProperties;
    return CompareProperties (score, nProperties, NULL, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
double ECInstanceComparer::Compare (ECInstanceDiff* diffs)
    {
    if (NULL != diffs)
        {
        diffs->SetBaseline (m_lhs);
        diffs->SetComparand (m_rhs);
        }

    UInt32 nProperties;
    CompareProperties (m_score, nProperties, diffs, false);
    m_score = (0 == nProperties) ? 0.0 : m_score / (double)nProperties;

    if (NULL != diffs)
        diffs->SetScore (m_score);

    return m_score;
    }
    
