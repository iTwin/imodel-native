/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Iota.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

/// @cond BENTLEY_SDK_Internal

#include <iterator>

BEGIN_BENTLEY_NAMESPACE

//! This is used by test code that uses for_each/parallel_for.  It was used when experimenting with 
//!   parallel libraries where you have to be iterating rather than using a for loop to get the parallelized behavior.
struct Iota
    {
    size_t m_count;
    Iota (size_t c) : m_count(c) {;}

    struct const_iterator : std::iterator<std::random_access_iterator_tag, size_t>
        {
        size_t  m_i;
        const_iterator (size_t i) : m_i(i)                      {;}

        size_t operator*() const                                {return m_i;}

        bool operator ==(const_iterator const& rhs) const       {return m_i == rhs.m_i;}
        bool operator !=(const_iterator const& rhs) const       {return m_i != rhs.m_i;}
        bool operator > (const_iterator const& rhs) const       {return m_i >  rhs.m_i;}
        bool operator >=(const_iterator const& rhs) const       {return m_i >= rhs.m_i;}
        bool operator <=(const_iterator const& rhs) const       {return m_i <= rhs.m_i;}
        bool operator < (const_iterator const& rhs) const       {return m_i <  rhs.m_i;}

        size_t              operator-(const_iterator const& other) const  { return m_i - other.m_i; }

        // forward iterator
        const_iterator&     operator++()                        { ++m_i; return *this;          }
        const_iterator      operator++(int)                     { return const_iterator(m_i++); }

        // Bidirectional iterator requirements
        const_iterator&     operator--()                        { --m_i; return *this;          }
        const_iterator      operator--(int)                     { return const_iterator(m_i--); }

        // Random access iterator requirements
        size_t              operator[](const size_t& __n) const { return __n; }
        const_iterator&     operator+=(const size_t& __n)       { m_i += __n; return *this; }
        const_iterator      operator+(const size_t& __n) const  { return const_iterator(m_i + __n); }
        const_iterator&     operator-=(const size_t& __n)       { m_i -= __n; return *this; }
        const_iterator      operator-(const size_t& __n) const  { return const_iterator(m_i - __n); }
        //const _Iterator&    base() const        { return m_i; }
        };

    const_iterator begin() const {return const_iterator(0);}
    const_iterator end()   const {return const_iterator(m_count);}
    };


END_BENTLEY_NAMESPACE

/// @endcond BENTLEY_SDK_Internal
