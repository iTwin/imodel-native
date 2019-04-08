//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXIndexable.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HIDXIndexable
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
template<class T> inline HIDXIndexable<T>::HIDXIndexable(const T pi_Object)
    : m_Object(pi_Object)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class T> inline HIDXIndexable<T>::~HIDXIndexable()
    {
    // We have to destroy the attributes.
    AttributeMap::const_iterator Itr(m_Attributes.begin());

    while (Itr != m_Attributes.end())
        {
        delete (*Itr).second;
        ++Itr;
        }
    }


//-----------------------------------------------------------------------------
// Object accessor
//-----------------------------------------------------------------------------
template<class T> inline const T HIDXIndexable<T>::GetObject() const
    {
    return m_Object;
    }


//-----------------------------------------------------------------------------
// Give new attribute to the indexable (possessed by indexable)
//-----------------------------------------------------------------------------
template<class T> inline void HIDXIndexable<T>::AddAttribute(void const*         pi_Index,
                                                             HIDXAttribute* pi_pNewAttribute)
    {
    HASSERT(pi_pNewAttribute != 0);
    HASSERT(m_Attributes.find(pi_Index) == m_Attributes.end());

    m_Attributes.insert(AttributeMap::value_type(pi_Index, pi_pNewAttribute));
    }


//-----------------------------------------------------------------------------
// Retrieve an attribute
//-----------------------------------------------------------------------------
template<class T> inline HIDXAttribute* HIDXIndexable<T>::GetAttribute(void const* pi_Index) const
    {
    AttributeMap::const_iterator Itr(m_Attributes.find(pi_Index));

    // 2 returns to save an assignment
    if (Itr == m_Attributes.end())
        return 0;
    else
        return (*Itr).second;
    }


//-----------------------------------------------------------------------------
// Remove an attribute
//-----------------------------------------------------------------------------
template<class T> inline void HIDXIndexable<T>::RemoveAttribute(void const* pi_Index)
    {
    AttributeMap::iterator Itr(m_Attributes.find(pi_Index));

    if (Itr != m_Attributes.end())
        {
        delete (*Itr).second;
        m_Attributes.erase(Itr);
        }
    }


//-----------------------------------------------------------------------------
// Test if two indexables use the same object
//-----------------------------------------------------------------------------
template<class T> inline bool HIDXIndexable<T>::IndexesSameObjectAs(const HIDXIndexable<T>& pi_rOtherIndexable) const
    {
    // Checking the address is enough
    return m_Object == pi_rOtherIndexable.m_Object;
    }

END_IMAGEPP_NAMESPACE