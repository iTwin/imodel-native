/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECEnabler.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECSchema.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bset.h>

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<StandaloneECEnabler>        StandaloneECEnablerPtr;
typedef RefCountedPtr<ECEnabler>                  EnablerPtr;
typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;

//=======================================================================================    
//! base class ensuring that all enablers are refcounted
//=======================================================================================    
struct ECEnabler : RefCountedBase, IStandaloneEnablerLocater
    {
    //! Interface of functor that wants to process text-valued properties
    struct IPropertyProcessor 
        {
        //! Callback for primitive property on instance.
        //! @return true if the desired property was found and processing should stop; else return false if processing should continue.
        virtual bool _ProcessPrimitiveProperty (IECInstance const& instance, WCharCP propName, ECValue const& propValue) const = 0;
        };

    enum PropertyProcessingResult  {PROPERTY_PROCESSING_RESULT_Miss=0, PROPERTY_PROCESSING_RESULT_Hit=1, PROPERTY_PROCESSING_RESULT_NoCandidates=3};
    enum PropertyProcessingOptions {PROPERTY_PROCESSING_OPTIONS_SingleType=0, PROPERTY_PROCESSING_OPTIONS_AllTypes=1};

private:
    ECClassCR                    m_ecClass;
    IStandaloneEnablerLocaterP   m_standaloneInstanceEnablerLocater;    // can't be const because the m_standaloneInstanceEnablerLocater may grow if a new enabler is added to its cache

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
    //! @param ecClass The ECClass for which the enabler will be used
    //! @param structStandaloneEnablerLocater If NULL, we'll use GetDefaultStandaloneEnabler for embedded structs
    ECOBJECTS_EXPORT ECEnabler(ECClassCR ecClass, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);

    virtual WCharCP                 _GetName() const = 0;
    virtual ECObjectsStatus         _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const = 0;
    virtual ECObjectsStatus         _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const = 0;
    virtual UInt32                  _GetFirstPropertyIndex (UInt32 parentIndex) const = 0;
    virtual UInt32                  _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const = 0;
    virtual UInt32                  _GetPropertyCount () const = 0;
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const = 0;

    // IStandaloneEnablerLocater
    ECOBJECTS_EXPORT virtual StandaloneECEnablerPtr  _LocateStandaloneEnabler (WCharCP schemaName, WCharCP className);

#if defined (EXPERIMENTAL_TEXT_FILTER)
    ECOBJECTS_EXPORT virtual PropertyProcessingResult   _ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR, EC::PrimitiveType, IPropertyProcessor const&, PropertyProcessingOptions) const;
#endif
    virtual bool                    _HasChildProperties (UInt32 parentIndex) const = 0;

    ECOBJECTS_EXPORT         bool                       ProcessStructProperty (bset<ECClassCP>& failedClasses, bool& allStructsFailed, ECValueCR propValue, EC::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const;

public:
    //! Primarily for debugging/logging purposes. Should match your fully-qualified class name
    ECOBJECTS_EXPORT WCharCP                    GetName() const;
    
    //! Get the ECClass that this enabler 'enables'
    ECOBJECTS_EXPORT ECClassCR                  GetClass() const;
    
    //! Obtain a propertyIndex given an access string. The propertyIndex can be used with ECInstances enabled by this enabler for more efficient property value access
    ECOBJECTS_EXPORT ECObjectsStatus            GetPropertyIndex     (UInt32& propertyIndex, WCharCP propertyAccessString) const;
    
    //! Given a propertyIndex, find the corresponding property access string
    ECOBJECTS_EXPORT ECObjectsStatus            GetAccessString      (WCharCP& propertyAccessString, UInt32 propertyIndex) const;
    
    //! Get the first propertyIndex (used in conjunction with GetNextPropertyIndex for efficiently looping over property values.)
    ECOBJECTS_EXPORT UInt32                     GetFirstPropertyIndex (UInt32 parentIndex) const;
    
    //! Get the next (after inputIndex) propertyIndex (used in conjunction with GetNeGetFirstPropertyIndexxtPropertyIndex for efficiently looping over property values.)
    ECOBJECTS_EXPORT UInt32                     GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const;

    //! Return true if the property associated with parentIndex has child properties
    ECOBJECTS_EXPORT bool                       HasChildProperties (UInt32 parentIndex) const;

    //! Get vector of all property indices for property defined by parent index. An index of 0 means root properties.
    ECOBJECTS_EXPORT ECObjectsStatus         GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const;

//    ECOBJECTS_EXPORT bool                       HasChildValues (ECValueAccessorCR, IECInstanceCR) const;
//    ECOBJECTS_EXPORT ECValuesCollection         GetChildValues (ECValueAccessorCR, IECInstanceCR) const;
    ECOBJECTS_EXPORT UInt32                     GetPropertyCount () const;
    ECOBJECTS_EXPORT StandaloneECEnablerPtr     GetEnablerForStructArrayMember (WCharCP schemaName, WCharCP className); 

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

//=======================================================================================    
//! Base class for all relationship enablers
//=======================================================================================    
 struct IECRelationshipEnabler
 {
protected:
    virtual IECWipRelationshipInstancePtr _CreateWipRelationshipInstance () const = 0;
    virtual EC::ECRelationshipClassCR     _GetRelationshipClass() const = 0;

 public:
    //! Get a WipRelationshipInstance that is used to set relationship name and order Ids.
    ECOBJECTS_EXPORT IECWipRelationshipInstancePtr  CreateWipRelationshipInstance() const;
    ECOBJECTS_EXPORT EC::ECRelationshipClassCR      GetRelationshipClass() const;
 };

END_BENTLEY_EC_NAMESPACE
