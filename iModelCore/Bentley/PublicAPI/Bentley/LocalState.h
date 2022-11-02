/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Interface of the local state store.
//  @bsiclass
//=======================================================================================
typedef std::shared_ptr<struct ILocalState> ILocalStatePtr;
struct ILocalState
    {
protected:
    virtual ~ILocalState () {}

    //! Saves the Utf8String value in the local state.
    //! @param nameSpace Namespace usually identifies code responsible for maintaing value.
    //! @param key Key identifying value in context of namespace.
    //! @param value String to save. Passing empty string will delete record.
    virtual void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) = 0;

    //! Returns a stored Utf8String from the local state.
    //! @param nameSpace Namespace usually identifies code responsible for maintaing value.
    //! @param key Key identifying value in context of namespace.
    //! @return String for given namespace and key. Returns empty if record does not exist.
    virtual Utf8String _GetValue (Utf8CP nameSpace, Utf8CP key) const = 0;

public:
    void SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) {_SaveValue(nameSpace, key, value);}
    Utf8String GetValue(Utf8CP nameSpace, Utf8CP key) const {return _GetValue(nameSpace, key);}
    };

//=======================================================================================
//! Implementation for runtime-only storage, without using any file persistence.
//  @bsiclass
//=======================================================================================
struct RuntimeLocalState : ILocalState
    {
public:
    typedef bmap<bpair<Utf8String, Utf8String>, Utf8String> Values;
protected:
    Values m_values;

protected:
    virtual void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
        {
        bpair<Utf8String, Utf8String> identifier(nameSpace, key);
        if (value.empty())
            m_values.erase(identifier);
        else
            m_values[identifier] = value;
        };

    virtual Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
        {
        bpair<Utf8String, Utf8String> identifier(nameSpace, key);

        auto iterator = m_values.find(identifier);
        if (iterator == m_values.end())
            return "";

        return iterator->second;
        };
public:
    //! Get internal values map - {Namespace,Key} to {Value}
    const Values& GetValues() const { return m_values; }
    //! Get internal values map - {Namespace,Key} to {Value}
    Values& GetValues() { return m_values; }
    };


END_BENTLEY_NAMESPACE