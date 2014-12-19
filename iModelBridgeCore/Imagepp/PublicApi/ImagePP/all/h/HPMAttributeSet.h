//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMAttributeSet.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HPMAttributeSet
//-----------------------------------------------------------------------------

#pragma once

#include "HPMAttribute.h"
#include "HPMPersistentObject.h"


class HPMAttributeSet : public HPMPersistentObject,
    public HPMShareableObject<HPMAttributeSet>
    {
    HPM_DECLARE_CLASS_DLL(_HDLLu, 1611)

public:

    // Primary methods.

    _HDLLu HPMAttributeSet();
    _HDLLu HPMAttributeSet(const HPMAttributeSet& pi_rObj);
    _HDLLu HPMAttributeSet& operator=(const HPMAttributeSet& pi_rObj);
    _HDLLu virtual ~HPMAttributeSet() {}
    
    template <typename AttributeT> AttributeT const*        FindAttributeCP () const;
    template <typename AttributeT> AttributeT*              FindAttributeP  ();  
    
    _HDLLu const HFCPtr<HPMGenericAttribute>                GetAttribute    (const HPMAttributesID& pi_rID) const;     
       
    _HDLLu bool                                             HasAttribute    (const HPMAttributesID& pi_rID) const;
    template <typename AttributeT> bool                     HasAttribute    () const;

    _HDLLu void                                             Set             (const HFCPtr<HPMGenericAttribute>& pi_rpAttribute);

    _HDLLu void                                             Remove          (const HPMGenericAttribute& pi_rAttribute);
    template <typename AttributeT>  void                    Remove          ();
   
    _HDLLu void                                             Clear           ();

    size_t                                                  size            () const;

    typedef map<HPMAttributesID, HFCPtr<HPMGenericAttribute> > AttributeMap;

    typedef set< HFCPtr<HPMGenericAttribute> > AttributeSet;

    class HPMASiterator
        {
    public:

        HPMASiterator()
            { }
        HPMASiterator(AttributeMap::const_iterator pi_Itr)
            {
            m_Itr = pi_Itr;
            }

        HPMASiterator& operator=(const HPMASiterator& pi_rObj)
            {
            m_Itr = pi_rObj.m_Itr;
            return *this;
            }

        bool operator==(HPMASiterator pi_Obj) const
            {
            return m_Itr == pi_Obj.m_Itr;
            }

        bool operator!=(HPMASiterator pi_Obj) const
            {
            return m_Itr != pi_Obj.m_Itr;
            }

        void operator++(int)
            {
            m_Itr++;
            }
        void operator++()
            {
            ++m_Itr;
            }

        HFCPtr<HPMGenericAttribute>
        operator*()
            {
            return (*m_Itr).second;
            }

    private:

        AttributeMap::const_iterator m_Itr;
        };

    HPMASiterator        begin() const;
    HPMASiterator        end() const;

private:

    AttributeMap    m_Attributes;
    
    HPMGenericAttribute const*   FindCP (const HPMAttributesID& pi_rID) const;
    HPMGenericAttribute*         FindP  (const HPMAttributesID& pi_rID) const;
    };

// Typedef for the set's iterator class
typedef HPMAttributeSet::HPMASiterator HPMAttributeSetIterator;


#include "HPMAttributeSet.hpp"


