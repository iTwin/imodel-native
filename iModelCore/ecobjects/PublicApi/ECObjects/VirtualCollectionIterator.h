/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/VirtualCollectionIterator.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/Bentley.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*=================================================================================**//**
* This template is used by iterators that hide their implementation from the
* published API.  Hiding the implementation allows it to be improved, for example
* by adding new data members, without requiring callers to recompile.
*
* To use the template, an iterator class must:
*   1) Satisfy the requirements of RefCountedPtr usually by deriving from RefCountedBase.
*   2) The iterator must implement standard iterator traits : value_type and reference typedef

*   3) Provide the following methods:
*       bool             IsDifferent(MyIterator const& rhs) const;
*       void             MoveToNext ();
*       ReturnType       GetCurrent () const;
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <typename IteratorImplementation>
struct VirtualCollectionIterator : std::iterator<std::forward_iterator_tag, typename IteratorImplementation::value_type>
{
private:
    RefCountedPtr<IteratorImplementation> m_implementation;

public:
    VirtualCollectionIterator (IteratorImplementation* state)
        :m_implementation (state)
        {}
    VirtualCollectionIterator (IteratorImplementation& state) : m_implementation (&state)
        {
        }

    typename IteratorImplementation::reference operator*() const
        {
        return m_implementation->GetCurrent();
        }

    bool        operator==(VirtualCollectionIterator const& rhs) const {return !(*this != rhs);}

    bool        operator!=(VirtualCollectionIterator const& rhs) const
        {
        bool leftIsNull = m_implementation.IsNull();
        bool rightIsNull = rhs.m_implementation.IsNull();

        if (leftIsNull && rightIsNull)
            return false;

        if (!leftIsNull && !rightIsNull)
            return m_implementation->IsDifferent (*rhs.m_implementation.get());

        return true;
        }

    bool IsNull() const {return m_implementation.IsNull();}
    VirtualCollectionIterator&   operator++()
        {
        m_implementation->MoveToNext();

        return *this;
        }
};

//END_BENTLEY_NAMESPACE
END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
