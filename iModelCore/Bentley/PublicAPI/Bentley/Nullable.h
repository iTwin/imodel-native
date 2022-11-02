/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include<Bentley/Bentley.h>
#include<Bentley/BeAssert.h>

BEGIN_BENTLEY_NAMESPACE
//=======================================================================================
//! A class that represents a Nullable value. It is intended to be used with value types.
// @bsiclass
//=======================================================================================
template<typename T>
struct Nullable final
    {
    private:
        T m_value;
        bool m_isNull = true;

    public:
        Nullable() : m_value(T()) {}
        Nullable(std::nullptr_t) : m_value(T()) {}
        Nullable(T const& value) : m_value(value), m_isNull(false) {}
        Nullable(Nullable<T> const& rhs) : m_value(rhs.m_value), m_isNull(rhs.m_isNull) {}
        Nullable(Nullable<T>&& rhs) : m_value(std::move(rhs.m_value)), m_isNull(std::move(rhs.m_isNull)) {}
        Nullable(T&& value) : m_value(std::move(value)), m_isNull(false) {}

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

        T const* get() const { return IsValid() ? &m_value : nullptr; }
        T* get() { return IsValid() ? &m_value : nullptr; }

        T const& Value() const { BeAssert(IsValid()); return m_value; }
        T& ValueR() { BeAssert(IsValid()); return m_value; }

        T const& operator*() const { return Value(); }
        T& operator*() { return ValueR(); }
        T const* operator->() const { BeAssert(IsValid()); return &m_value; }
        T* operator->() { BeAssert(IsValid()); return &m_value; }
    };

END_BENTLEY_NAMESPACE
