//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpm/src/HPMAttributeSet.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HPMAttributeSet
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.

#include <Imagepp/all/h/HPMAttributeSet.h>

HPM_REGISTER_CLASS(HPMAttributeSet, HPMPersistentObject);


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HPMAttributeSet::HPMAttributeSet()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HPMAttributeSet::HPMAttributeSet(const HPMAttributeSet& pi_rObj)
    : m_Attributes(pi_rObj.m_Attributes)
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HPMAttributeSet& HPMAttributeSet::operator=(const HPMAttributeSet& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // MRx
        // Need to make a deep copy

        m_Attributes = pi_rObj.m_Attributes;
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// HasAttribute
//-----------------------------------------------------------------------------
bool HPMAttributeSet::HasAttribute(const HPMAttributesID& pi_rID) const 
    {
        return FindCP(pi_rID) != 0;
    }

//-----------------------------------------------------------------------------
// Set
//-----------------------------------------------------------------------------
void HPMAttributeSet::Set(const HFCPtr<HPMGenericAttribute>& pi_rpAttribute)
    {
    HASSERT(pi_rpAttribute != 0);

    // Insert specified attribute
    std::pair <AttributeMap::iterator, bool> result = m_Attributes.insert(AttributeMap::value_type(pi_rpAttribute->GetID(), pi_rpAttribute));

    // already exist, substitute 
    if(!result.second)
        {
        result.first->second = pi_rpAttribute;
        }
    }

//-----------------------------------------------------------------------------
// Remove
//-----------------------------------------------------------------------------
void HPMAttributeSet::Remove(const HPMGenericAttribute& pi_rAttribute)
    {
    // Remove old attribute if there is one
    AttributeMap::iterator Itr(m_Attributes.find(pi_rAttribute.GetID()));

    if (Itr != m_Attributes.end())
        m_Attributes.erase(Itr);
    }

//-----------------------------------------------------------------------------
// Clear before persistence fills us
//-----------------------------------------------------------------------------
void HPMAttributeSet::Clear()
    {
    m_Attributes.clear();
    }

//-----------------------------------------------------------------------------
// Find for a particular attribute
//----------------------------------------------------------------------------- 
const HFCPtr<HPMGenericAttribute> HPMAttributeSet::GetAttribute(const HPMAttributesID& pi_rID) const
    {
    return FindP(pi_rID);
    }

