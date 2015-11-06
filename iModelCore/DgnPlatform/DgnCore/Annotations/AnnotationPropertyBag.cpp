//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationPropertyBag.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/AnnotationPropertyBag.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::AnnotationPropertyBag() :
    T_Super()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::CopyFrom(AnnotationPropertyBagCR rhs)
    {
    m_integerProperties = rhs.m_integerProperties;
    m_realProperties = rhs.m_realProperties;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
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
// @bsimethod                                                   Jeff.Marker     05/2014
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
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::ClearAllProperties()
    {
    m_integerProperties.clear();
    m_realProperties.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
size_t AnnotationPropertyBag::ComputePropertyCount() const
    {
    return (m_integerProperties.size() + m_realProperties.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::T_Integer AnnotationPropertyBag::GetIntegerProperty(T_Key key) const
    {
    PRECONDITION(_IsIntegerProperty(key), 0);
    
    auto foundValue = m_integerProperties.find(key);
    POSTCONDITION(m_integerProperties.end() != foundValue, 0);

    return foundValue->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::SetIntegerProperty(T_Key key, T_Integer value)
    {
    PRECONDITION(_IsIntegerProperty(key), );
    
    m_integerProperties[key] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
AnnotationPropertyBag::T_Real AnnotationPropertyBag::GetRealProperty(T_Key key) const
    {
    PRECONDITION(_IsRealProperty(key), 0.0);

    auto foundValue = m_realProperties.find(key);
    POSTCONDITION(m_realProperties.end() != foundValue, 0.0);

    return foundValue->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::SetRealProperty(T_Key key, T_Real value)
    {
    PRECONDITION(_IsRealProperty(key), );
    
    m_realProperties[key] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationPropertyBag::MergeWith(AnnotationPropertyBagCR rhs)
    {
    for (auto const& rhsProp : rhs.m_integerProperties)
        m_integerProperties[rhsProp.first] = rhsProp.second;

    for (auto const& rhsProp : rhs.m_realProperties)
        m_realProperties[rhsProp.first] = rhsProp.second;
    }
