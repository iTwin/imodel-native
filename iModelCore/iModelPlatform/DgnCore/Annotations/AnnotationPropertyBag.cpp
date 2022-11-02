//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/AnnotationPropertyBag.h>

USING_NAMESPACE_BENTLEY_DGN

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::AnnotationPropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::CopyFrom(AnnotationPropertyBagCR rhs)
    {
    m_integerProperties = rhs.m_integerProperties;
    m_realProperties = rhs.m_realProperties;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool AnnotationPropertyBag::HasProperty(T_Key key) const
    {if (_IsIntegerProperty(key))
        return (m_integerProperties.end() != m_integerProperties.find(key));
    
    if (_IsRealProperty(key))
        return (m_realProperties.end () != m_realProperties.find(key));
    
    BeAssert(false); // Unknown/unexpected property type.
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::ClearProperty(T_Key key)
    {
    if (_IsIntegerProperty(key))
        {
        m_integerProperties.erase(key);
        return;
        }

    if (_IsRealProperty(key))
        {
        m_realProperties.erase(key);
        return;
        }
    
    BeAssert(false); // Unknown/unexpected property type.
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::ClearAllProperties()
    {
    m_integerProperties.clear();
    m_realProperties.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t AnnotationPropertyBag::ComputePropertyCount() const
    {
    return (m_integerProperties.size() + m_realProperties.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::T_Integer AnnotationPropertyBag::GetIntegerProperty(T_Key key) const
    {
    PRECONDITION(_IsIntegerProperty(key), 0);
    
    auto foundValue = m_integerProperties.find(key);
    POSTCONDITION(m_integerProperties.end() != foundValue, 0);

    return foundValue->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::SetIntegerProperty(T_Key key, T_Integer value)
    {
    PRECONDITION(_IsIntegerProperty(key), );
    
    m_integerProperties[key] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::T_Real AnnotationPropertyBag::GetRealProperty(T_Key key) const
    {
    PRECONDITION(_IsRealProperty(key), 0.0);

    auto foundValue = m_realProperties.find(key);
    POSTCONDITION(m_realProperties.end() != foundValue, 0.0);

    return foundValue->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::SetRealProperty(T_Key key, T_Real value)
    {
    PRECONDITION(_IsRealProperty(key), );
    
    m_realProperties[key] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::MergeWith(AnnotationPropertyBagCR rhs)
    {
    for (auto const& rhsProp : rhs.m_integerProperties)
        m_integerProperties[rhsProp.first] = rhsProp.second;

    for (auto const& rhsProp : rhs.m_realProperties)
        m_realProperties[rhsProp.first] = rhsProp.second;
    }
