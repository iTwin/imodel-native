/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECEnabler.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects\ECObjects.h>
#include <Bentley\RefCounted.h>

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<ECEnabler>                  EnablerPtr;

//! base class ensuring that all enablers are refcounted
struct ECEnabler : RefCountedBase
    {
private:
    ECClassCP                 m_ecClass;

    ECEnabler(); // Hidden as part of the RefCounted pattern
    
protected:
    //! Protected as part of the RefCounted pattern
    ECOBJECTS_EXPORT ~ECEnabler(); 

    //! Subclasses of ECEnabler should implement a FactoryMethod to construct the enabler, as
    //! part of the RefCounted pattern.
    //! It should be of the form:
    //! /code
    //!   static ____EnablerPtr CreateEnabler (ECClassCR ecClass)
    //!       {
    //!       return new ____Enabler (ecClass);    
    //!       };
    //! /endcode
    //! where the ____ is a name specific to your subclass, and the parameters may vary per enabler.
    ECOBJECTS_EXPORT ECEnabler(ECClassCR ecClass);

    ECOBJECTS_EXPORT virtual wchar_t const * _GetName() const = 0;

public:
    
    //! Primarily for debugging/logging purposes. Should match your fully-qualified class name
    ECOBJECTS_EXPORT wchar_t const * GetName() const;
    
    ECOBJECTS_EXPORT ECClassCR       GetClass() const;
    };

END_BENTLEY_EC_NAMESPACE
