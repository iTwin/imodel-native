/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Nullable.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include<Bentley/Bentley.h>

BEGIN_BENTLEY_NAMESPACE
//=======================================================================================
//! A class that represents a Nullable value. It is intended to be used with value types.
// @bsiclass                                                         Affan.Khan   03/16
//=======================================================================================
template<typename T>
struct Nullable
    {
    private:
        T m_value;
        bool m_isNull;

    public:
        Nullable() : m_value(T()), m_isNull(true) {}
        Nullable(std::nullptr_t) : m_value(T()), m_isNull(true) {}
        Nullable(T const& value) : m_value(value), m_isNull(false) {}
        Nullable(Nullable<T> const& rhs) : m_value(rhs.m_value), m_isNull(rhs.m_isNull) {}
        Nullable(Nullable<T>&& rhs) : m_value(std::move(rhs.m_value)), m_isNull(std::move(rhs.m_isNull)) {}

        Nullable<T>& operator=(Nullable<T> const& rhs)
            {
            if (this != &rhs)
                {
                m_value = rhs.m_value;
                m_isNull = rhs.m_isNull;
                }
            return *this;
            }

        Nullable<T>& operator=(Nullable<T>&& rhs)
            {
            if (this != &rhs)
                {
                m_value = std::move(rhs.m_value);
                m_isNull = std::move(rhs.m_isNull);
                }

            return *this;
            }

        Nullable<T>& operator=(T const& rhs)
            {
            m_value = rhs;
            m_isNull = false;
            return *this;
            }

        Nullable<T>& operator=(T&& rhs)
            {
            m_value = std::move(rhs);
            m_isNull = false;
            return *this;
            }

        Nullable<T>& operator=(std::nullptr_t rhs)
            {
            m_isNull = true;
            return *this;
            }

        bool operator==(Nullable<T> const& rhs) const { return m_isNull == rhs.m_isNull && (m_isNull || m_value == rhs.m_value); }
        bool operator!=(Nullable<T> const& rhs) const { return !(*this == rhs); }
        bool operator==(std::nullptr_t)const { return m_isNull; }
        bool operator!=(std::nullptr_t rhs) const { return !(*this == rhs); }

        bool IsNull() const { return m_isNull; }
        bool IsValid() const { return !m_isNull; }
        T const& Value() const { BeAssert(IsValid()); return m_value; }
        T& ValueR() { BeAssert(IsValid()); return m_value; }
    };

END_BENTLEY_NAMESPACE
