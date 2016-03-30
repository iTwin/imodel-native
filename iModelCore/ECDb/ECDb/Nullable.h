/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/Nullable.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//
//#include <Bentley/Bentley.h>
//#include "BeAssert.h"
//#include <cstddef>
//#include <utility>

#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! A class that represent a Nullable value. It is intented to be used with value types.
// @bsiclass                                                         Affan.Khan   03/16
//=======================================================================================
template<typename T>
struct Nullable
    {
    private:
        T m_value;
        bool m_isNull;

    public:
        Nullable() :m_isNull(true), m_value(T()) {}
        Nullable(T const& value) : m_value(value), m_isNull(false) {}
        Nullable(Nullable<T> const& rhs) :m_isNull(rhs.m_isNull), m_value(rhs.m_value) {}
        Nullable(Nullable<T> const&& rhs) :m_isNull(std::move(rhs.m_isNull)), m_value(std::move(rhs.m_value)) {}
        bool IsNull() const { return m_isNull; }
        T const& Value() const { BeAssert(!IsNull()); return m_value; }
        T& ValueR() { BeAssert(!IsNull()); return m_value; }
        bool operator == (Nullable<T> const& rhs) const
            {
            if (rhs.IsNull() != IsNull())
                return false;
            else if (rhs.IsNull() && IsNull())
                return true;
            else
                return rhs.Value() == Value();
            }
        bool operator == (std::nullptr_t rhs)const { return IsNull(); }
        bool operator != (Nullable<T> const& rhs) const { return !operator==(rhs); }
        bool operator != (std::nullptr_t rhs) const { return !operator==(rhs); }
        Nullable<T>& operator = (Nullable<T> const&& rhs)
            {
            if (&rhs != this)
                {
                m_value = std::move(rhs.m_value);
                m_isNull = std::move(rhs.m_isNull);
                }

            return *this;
            }
        Nullable<T>& operator = (Nullable<T> const& rhs)
            {
            if (&rhs != this)
                {
                m_value = rhs.m_value;
                m_isNull = rhs.m_isNull;
                }
            return *this;
            }
        Nullable<T>& operator = (T const& rhs)
            {
            m_value = rhs;
            m_isNull = false;
            return *this;
            }
        Nullable<T>& operator = (T const&& rhs)
            {
            m_value = std::move(rhs);
            m_isNull = false;

            return *this;
            }
        Nullable<T>& operator = (std::nullptr_t rhs)
            {
            m_isNull = true;
            return *this;
            }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
