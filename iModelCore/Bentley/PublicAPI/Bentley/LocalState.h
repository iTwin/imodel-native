/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/LocalState.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Interface of the local state store.
//  @bsiclass                                           Grigas.Petraitis        12/14
//=======================================================================================
struct ILocalState 
    {
    protected:
        virtual ~ILocalState () {}
    
        //! Saves the Utf8String value in the local state. Set to empty to delete value.
        //! @note The nameSpace and key pair must be unique.
        virtual void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) = 0;

        //! Returns a stored Utf8String from the local state. Returns empty if value does not exist.
        //! @note The nameSpace and key pair uniquely identifies the value.
        virtual Utf8String _GetValue (Utf8CP nameSpace, Utf8CP key) const = 0;

    public:
        void SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) {_SaveValue(nameSpace, key, value);}
        Utf8String GetValue(Utf8CP nameSpace, Utf8CP key) const {return _GetValue(nameSpace, key);}
    };

//=======================================================================================
//! Implementation for runtime-only storage, without using any file persistence.
//  @bsiclass                                           Vincas.Razma            09/18
//=======================================================================================
struct RuntimeLocalState : ILocalState
    {
    protected:
        bmap<bpair<Utf8String, Utf8String>, Utf8String> m_values;

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
        const bmap<bpair<Utf8String, Utf8String>, Utf8String>& GetValues() const { return m_values; }
        //! Get internal values map - {Namespace,Key} to {Value}
        bmap<bpair<Utf8String, Utf8String>, Utf8String>& GetValues() { return m_values; }
    };


END_BENTLEY_NAMESPACE