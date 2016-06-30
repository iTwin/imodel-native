/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/LocalState.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! Interface of the local state store.
//  @bsiclass                                           Grigas.Petraitis        12/14
//=======================================================================================
struct ILocalState 
    {
    protected:
        virtual ~ILocalState () {}
    
        //! Saves the Utf8String value in the local state.
        //! @note The nameSpace and key pair must be unique.
        virtual void _SaveValue (Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) = 0;

        //! Returns a stored Utf8String from the local state.
        //! @note The nameSpace and key pair uniquely identifies the value.
        virtual Utf8String _GetValue (Utf8CP nameSpace, Utf8CP key) const = 0;

    public:
        void SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) {_SaveValue(nameSpace, key, value);}
        Utf8String GetValue(Utf8CP nameSpace, Utf8CP key) const {return _GetValue(nameSpace, key);}
    };
    
END_BENTLEY_NAMESPACE