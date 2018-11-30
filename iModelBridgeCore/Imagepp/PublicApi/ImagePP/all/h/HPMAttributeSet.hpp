//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMAttributeSet.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HPMAttributeSet inline methods
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// iterator on start
//-----------------------------------------------------------------------------
inline HPMAttributeSet::HPMASiterator HPMAttributeSet::begin() const
    {
    return HPMASiterator(m_Attributes.begin());
    }


//-----------------------------------------------------------------------------
// iterator on end
//-----------------------------------------------------------------------------
inline HPMAttributeSet::HPMASiterator HPMAttributeSet::end() const
    {
    return HPMASiterator(m_Attributes.end());
    }


//-----------------------------------------------------------------------------
// Get the set's size
//-----------------------------------------------------------------------------
inline size_t HPMAttributeSet::size() const
    {
    return m_Attributes.size();
    }

//-----------------------------------------------------------------------------
// HasAttribute
//-----------------------------------------------------------------------------
template <typename AttributeT> bool HPMAttributeSet::HasAttribute() const
    {
    return FindCP(static_cast<const HPMAttributesID> (AttributeT::ATTRIBUTE_ID)) != 0;
    }

//-----------------------------------------------------------------------------
// Find for a particular attribute
//-----------------------------------------------------------------------------
template <typename AttributeT> AttributeT const* HPMAttributeSet::FindAttributeCP() const 
    {
    HPMGenericAttribute const* pAttribute = FindCP(static_cast<const HPMAttributesID> (AttributeT::ATTRIBUTE_ID));

    HASSERT(pAttribute != NULL ? NULL != dynamic_cast<const AttributeT*>(pAttribute) : true);
	
    return static_cast<AttributeT const*> (pAttribute);
    }

//-----------------------------------------------------------------------------
// Find for a particular attribute
//-----------------------------------------------------------------------------
template <typename AttributeT> AttributeT* HPMAttributeSet::FindAttributeP()
    {
    HPMGenericAttribute* pAttribute = FindP(static_cast<const HPMAttributesID> (AttributeT::ATTRIBUTE_ID));

    HASSERT(pAttribute != NULL ? NULL != dynamic_cast<AttributeT*>(pAttribute) : true);
	
    return static_cast<AttributeT*> (pAttribute);
    }

//-----------------------------------------------------------------------------
// Get a non-mutable pointer to an attribute if found
//-----------------------------------------------------------------------------
inline HPMGenericAttribute const* HPMAttributeSet::FindCP (const HPMAttributesID& pi_rID) const
    {
    AttributeMap::const_iterator Itr(m_Attributes.find(pi_rID));

    if (Itr != m_Attributes.end())
        {
        return (*Itr).second;
        }
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Get a mutable pointer to an attribute if found
//-----------------------------------------------------------------------------
inline HPMGenericAttribute* HPMAttributeSet::FindP (const HPMAttributesID& pi_rID) const
    {
    AttributeMap::const_iterator Itr(m_Attributes.find(pi_rID));

    if (Itr != m_Attributes.end())
        {
        return (*Itr).second;
        }
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Remove
//-----------------------------------------------------------------------------
template <typename AttributeT> void HPMAttributeSet::Remove()
    {
    // Remove old attribute if there is one
    AttributeMap::iterator Itr(m_Attributes.find(static_cast<const HPMAttributesID> (AttributeT::ATTRIBUTE_ID)));

    if (Itr != m_Attributes.end())
        m_Attributes.erase(Itr);
    }

END_IMAGEPP_NAMESPACE