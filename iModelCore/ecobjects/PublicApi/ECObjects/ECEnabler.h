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
#include <ECObjects\ECInstance.h>
#include <ECObjects\ECSchema.h>
#include <Bentley\RefCounted.h>
#include <Bentley\bset.h>

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<ECEnabler>                  EnablerPtr;

//=======================================================================================    
//! base class ensuring that all enablers are refcounted
//=======================================================================================    
struct ECEnabler : RefCountedBase
    {
    //! Interface of functor that wants to process text-valued properties
    struct IPropertyProcessor 
        {
        //! Callback for primitive property on instance.
        //! @return true if the desired property was found and processing should stop; else return false if processing should continue.
        virtual bool _ProcessPrimitiveProperty (IECInstance const& instance, wchar_t const* propName, ECValue const& propValue) const = 0;
        };

    enum PropertyProcessingResult  {PROPERTY_PROCESSING_RESULT_Miss=0, PROPERTY_PROCESSING_RESULT_Hit=1, PROPERTY_PROCESSING_RESULT_NoCandidates=3};
    enum PropertyProcessingOptions {PROPERTY_PROCESSING_OPTIONS_SingleType=0, PROPERTY_PROCESSING_OPTIONS_AllTypes=1};

private:
    ECClassCR                 m_ecClass;

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

    ECOBJECTS_EXPORT virtual wchar_t const *            _GetName() const = 0;
    ECOBJECTS_EXPORT virtual ECObjectsStatus            _GetPropertyIndex (UInt32& propertyIndex, const wchar_t * propertyAccessString) const = 0;
#if defined (EXPERIMENTAL_TEXT_FILTER)
    ECOBJECTS_EXPORT virtual PropertyProcessingResult   _ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR, EC::PrimitiveType, IPropertyProcessor const&, PropertyProcessingOptions) const;
#endif

    ECOBJECTS_EXPORT         bool                       ProcessStructProperty (bset<ECClassCP>& failedClasses, bool& allStructsFailed, ECValueCR propValue, EC::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const;

public:
    UInt32                           m_privateRefCount;
    ECOBJECTS_EXPORT UInt32          AddRef();
    ECOBJECTS_EXPORT UInt32          Release();

    //! Primarily for debugging/logging purposes. Should match your fully-qualified class name
    ECOBJECTS_EXPORT wchar_t const * GetName() const;
    
    ECOBJECTS_EXPORT ECClassCR       GetClass() const;
    ECOBJECTS_EXPORT ECObjectsStatus       GetPropertyIndex (UInt32& propertyIndex, const wchar_t * propertyAccessString) const;

#if defined (EXPERIMENTAL_TEXT_FILTER)
    //! Call processor on all primitive-valued properties of specified type(s) on this instance. 
    //! Processing is terminated if the processor returns a non-zero value.
    //! @remarks This function returns immediately with PROPERTY_PROCESSING_RESULT_Miss if the class of \a instance is in \a failedClasses.
    //! If this function detects that the class has no properties of the specified type, it will return PROPERTY_PROCESSING_RESULT_Miss and
    //  also add the class to \a failedClasses.
    //! @param[in] failedClasses    The set of ECClasses that are known to have no properties of the specified type. Updated.
    //! @param[in] instance         The instance to scan for properties
    //! @param[in] proc             The property processor callback
    //! @param[in] primitiveType    The type of primitive property to process, unless \a opts is PROPERTY_PROCESSING_OPTIONS_AllTypes
    //! @param[in] opts             Processing options.
    //! @return The result of processing.
    ECOBJECTS_EXPORT PropertyProcessingResult ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR instance, EC::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts = PROPERTY_PROCESSING_OPTIONS_SingleType) const;
#endif
    };

END_BENTLEY_EC_NAMESPACE
