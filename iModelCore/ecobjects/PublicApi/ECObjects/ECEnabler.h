/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECEnabler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECSchema.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bset.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<StandaloneECEnabler>        StandaloneECEnablerPtr;
typedef RefCountedPtr<ECEnabler>                  ECEnablerPtr;
typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;

#define INVALID_PROPERTY_INDEX  0;

//=======================================================================================
//! An ECEnabler is the interface between an ECClass and an ECInstance. Every ECInstance
//! has an associated ECEnabler, typically shared among all ECInstances of that ECClass.
//!
//! The ECEnabler's primary job is to convert access strings (based on property names defined
//! in the ECClass) into integer property indices understood by the ECInstance.
//! Property indices are opaque integers, each of which uniquely identify a single property
//! of the ECClass. A property index of zero means "no such property index".
//! In the case of struct properties, each property of the struct is assigned a property
//! index unique among all property indices; the struct itself has its own property index which
//! serves as the "parent" property index for its own properties.
//!
//! Operations involving property indices are significantly more efficient than those involving
//! access strings and should be preferred where possible.
//! @addtogroup ECObjectsGroup
//! @beginGroup
//=======================================================================================
struct ECEnabler : RefCountedBase
/*__PUBLISH_SECTION_END__*/
    , IStandaloneEnablerLocater
/*__PUBLISH_SECTION_START__*/
    {
    //! Interface of functor that wants to process text-valued properties
    struct IPropertyProcessor
        {
        //! Callback for primitive property on instance.
        //! @return true if the desired property was found and processing should stop; else return false if processing should continue.
        virtual bool _ProcessPrimitiveProperty (IECInstance const& instance, Utf8CP propName, ECValue const& propValue) const = 0;
        };

#if defined (EXPERIMENTAL_TEXT_FILTER)
    enum PropertyProcessingResult  {PROPERTY_PROCESSING_RESULT_Miss=0, PROPERTY_PROCESSING_RESULT_Hit=1, PROPERTY_PROCESSING_RESULT_NoCandidates=3};
    enum PropertyProcessingOptions {PROPERTY_PROCESSING_OPTIONS_SingleType=0, PROPERTY_PROCESSING_OPTIONS_AllTypes=1};
#endif


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

    virtual Utf8CP                  _GetName() const = 0;
    virtual ECObjectsStatus         _GetPropertyIndex (uint32_t& propertyIndex, Utf8CP propertyAccessString) const = 0;
    virtual ECObjectsStatus         _GetAccessString  (Utf8CP& propertyAccessString, uint32_t propertyIndex) const = 0;
    virtual uint32_t                _GetFirstPropertyIndex (uint32_t parentIndex) const = 0;
    virtual uint32_t                _GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const = 0;
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const = 0;

    ECOBJECTS_EXPORT virtual ECPropertyCP   _LookupECProperty (uint32_t propertyIndex) const;
    ECOBJECTS_EXPORT virtual ECPropertyCP   _LookupECProperty (Utf8CP accessString) const;
    ECOBJECTS_EXPORT virtual bool           _IsPropertyReadOnly (uint32_t propertyIndex) const;

    // IStandaloneEnablerLocater
    ECOBJECTS_EXPORT virtual StandaloneECEnablerPtr  _LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className);

#if defined (EXPERIMENTAL_TEXT_FILTER)
    ECOBJECTS_EXPORT virtual PropertyProcessingResult   _ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR, ECN::PrimitiveType, IPropertyProcessor const&, PropertyProcessingOptions) const;
    ECOBJECTS_EXPORT         bool                       ProcessStructProperty (bset<ECClassCP>& failedClasses, bool& allStructsFailed, ECValueCR propValue, ECN::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts) const;
#endif
    virtual bool                    _HasChildProperties (uint32_t parentIndex) const = 0;
    virtual uint32_t                _GetParentPropertyIndex (uint32_t childIndex) const = 0;


public:
    ECOBJECTS_EXPORT ECPropertyCP               LookupECProperty (uint32_t propertyIndex) const;
    ECOBJECTS_EXPORT ECPropertyCP               LookupECProperty (Utf8CP accessString) const;
    ECOBJECTS_EXPORT bool                       IsPropertyReadOnly (uint32_t propertyIndex) const;

public:
    //! Primarily for debugging/logging purposes. Should match your fully-qualified class name
    //! @return An enabler name for debugging/logging purposes.
    ECOBJECTS_EXPORT Utf8CP                     GetName() const;

    //! Get the ECClass that this enabler 'enables'
    //! @return the ECClass associated with this ECEnabler.
    ECOBJECTS_EXPORT ECClassCR                  GetClass() const;

    //! Obtain a propertyIndex given an access string. The propertyIndex can be used with ECInstances enabled by this enabler for more efficient property value access
    //! @param[out]     propertyIndex           Will be set to the corresponding property index
    //! @param[in]      propertyAccessString    Access string for which to obtain the property index
    //! @return ECObjectsStatus::Success if access string successfully converted, or an error code.
    ECOBJECTS_EXPORT ECObjectsStatus            GetPropertyIndex     (uint32_t& propertyIndex, Utf8CP propertyAccessString) const;

    //! Given a propertyIndex, find the corresponding property access string
    //! @param[out]     propertyAccessString    Will be set to the corresponding access string
    //! @param[in]      propertyIndex           Property index for which to obtain an access string
    //! @return ECObjectsStatus::Success if property index successfully converted, or an error code.
    ECOBJECTS_EXPORT ECObjectsStatus            GetAccessString      (Utf8CP& propertyAccessString, uint32_t propertyIndex) const;

    //! Obtain the property index of the first property of this enabler's ECClass (if parentIndex == 0), or the first child property
    //! of the struct property indicated by parentIndex.
    //! @param[in]      parentIndex     The property index of the parent struct property, or 0 if not a member of a struct.
    //! @return The first property index, or 0 if no such property exists.
    ECOBJECTS_EXPORT uint32_t                   GetFirstPropertyIndex (uint32_t parentIndex) const;

    //! Get the next (after inputIndex) propertyIndex (used in conjunction with GetFirstPropertyIndex for efficiently looping over property values.)
    //! @param[in]      parentIndex     Property index of the parent struct property, or 0 if not a member of a struct.
    //! @param[in]      inputIndex      Index of the preceding property.
    //! @return The index of the property following inputIndex, or 0 if no such property exists.
    ECOBJECTS_EXPORT uint32_t                   GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const;

    //! Return true if the property associated with parentIndex has child properties
    //! @param[in]      parentIndex     Property index of the parent struct property, or 0 if not a member of a struct.
    //! @return True if the specified property has child properties (i.e., is a struct property)
    ECOBJECTS_EXPORT bool                       HasChildProperties (uint32_t parentIndex) const;

    //! If childIndex refers to a member of a struct, returns the index of the struct property which contains it
    //! @param[in]      childIndex The index of the child property
    //! @return The index of the parent property, or 0 if the child property has no parent (i.e. is not a member of a struct)
    ECOBJECTS_EXPORT uint32_t                   GetParentPropertyIndex (uint32_t childIndex) const;

    //! Get vector of all property indices for property defined by parent index.
    //! @param[out]     indices         Vector to hold the property indices.
    //! @param[in]      parentIndex     Property index of the parent struct property, or 0 if not a member of a struct.
    //! @return ECObjectsStatus::Success if vector of indices populated, or an error code.
    ECOBJECTS_EXPORT ECObjectsStatus         GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const;

    //! Get the IStandaloneEnablerLocater for this enabler
    //! @return an IStandaloneEnablerLocater
    ECOBJECTS_EXPORT IStandaloneEnablerLocaterR GetStandaloneEnablerLocater();

    //! Returns an enabler for the given class from the given schema.
    //! @param[in] schemaKey    The SchemaKey defining the schema (schema name and version info) that the className ECClass comes from
    //! @param[in] className    The name of the ECClass to retrieve the enabler for
    //! @return A StandaloneECEnabler that enables the requested class
    ECOBJECTS_EXPORT StandaloneECEnablerPtr     GetEnablerForStructArrayMember (SchemaKeyCR schemaKey, Utf8CP className); 
    
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
    ECOBJECTS_EXPORT PropertyProcessingResult ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR instance, ECN::PrimitiveType primitiveType, IPropertyProcessor const& proc, PropertyProcessingOptions opts = PROPERTY_PROCESSING_OPTIONS_SingleType) const;
#endif
    };

/*__PUBLISH_SECTION_END__*/

//=======================================================================================
//! Wraps another enabler
//=======================================================================================    
struct ECWrappedEnabler : ECEnabler
    {
private:
    ECEnablerPtr            m_enabler;

    ECWrappedEnabler (ECEnablerR enabler) : ECEnabler (enabler.GetClass(), NULL), m_enabler (&enabler) { }

    virtual Utf8CP                  _GetName() const { return m_enabler->GetName(); }
    virtual ECObjectsStatus         _GetPropertyIndex (uint32_t& propertyIndex, Utf8CP propertyAccessString) const { return m_enabler->GetPropertyIndex (propertyIndex, propertyAccessString); }
    virtual ECObjectsStatus         _GetAccessString  (Utf8CP& propertyAccessString, uint32_t propertyIndex) const { return m_enabler->GetAccessString (propertyAccessString, propertyIndex); }
    virtual uint32_t                _GetFirstPropertyIndex (uint32_t parentIndex) const { return m_enabler->GetFirstPropertyIndex (parentIndex); }
    virtual uint32_t                _GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const { return m_enabler->GetNextPropertyIndex (parentIndex, inputIndex); }
    virtual ECObjectsStatus         _GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const { return m_enabler->GetPropertyIndices (indices, parentIndex); }

    ECOBJECTS_EXPORT virtual ECPropertyCP   _LookupECProperty (uint32_t propertyIndex) const { return m_enabler->LookupECProperty (propertyIndex); }
    ECOBJECTS_EXPORT virtual ECPropertyCP   _LookupECProperty (Utf8CP accessString) const { return m_enabler->LookupECProperty (accessString); }
    ECOBJECTS_EXPORT virtual bool           _IsPropertyReadOnly (uint32_t propertyIndex) const { return m_enabler->IsPropertyReadOnly (propertyIndex); }

    // IStandaloneEnablerLocater
    ECOBJECTS_EXPORT virtual StandaloneECEnablerPtr  _LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className) { return m_enabler->LocateStandaloneEnabler (schemaKey, className); }

#if defined (EXPERIMENTAL_TEXT_FILTER)
    ECOBJECTS_EXPORT virtual PropertyProcessingResult   _ProcessPrimitiveProperties (bset<ECClassCP>& failedClasses, IECInstanceCR instance, ECN::PrimitiveType primType, IPropertyProcessor const& processor, PropertyProcessingOptions opts) const
        { return m_enabler->ProcessPrimitiveProperties (failedClasses, instance, primType, processor, opts); }
#endif
    virtual bool                    _HasChildProperties (uint32_t parentIndex) const { return m_enabler->HasChildProperties (parentIndex); }
    virtual uint32_t                _GetParentPropertyIndex (uint32_t childIndex) const override { return m_enabler->GetParentPropertyIndex (childIndex); }
public:
    static ECEnablerPtr Create (ECEnablerR enabler) { return new ECWrappedEnabler (enabler); }
    };

/*__PUBLISH_SECTION_START__*/

//=======================================================================================    
//! Base class for all relationship enablers
//=======================================================================================
 struct IECRelationshipEnabler
 {
protected:
    //! Get a WipRelationshipInstance that is used to set relationship name and order Ids.
    virtual IECWipRelationshipInstancePtr _CreateWipRelationshipInstance () const = 0;

    //! Returns the relationship class that this enabler 'enables'
    virtual ECN::ECRelationshipClassCR     _GetRelationshipClass() const = 0;

 public:
    //! Get a WipRelationshipInstance that is used to set relationship name and order Ids.
    ECOBJECTS_EXPORT IECWipRelationshipInstancePtr  CreateWipRelationshipInstance() const;
    //! Returns the relationship class that this enabler 'enables'
    ECOBJECTS_EXPORT ECN::ECRelationshipClassCR      GetRelationshipClass() const;
 };

 /*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
//! A derived helper enabler that allows you to generates property indices based on a array of
//! property strings. This will only work on simple instances with no embedded structs.
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
 template <typename DerivedClass, typename BaseEnablerClass>
 struct          PropertyIndexedEnabler  : BaseEnablerClass
    {
    private:
        virtual ECObjectsStatus _GetPropertyIndex (uint32_t& propertyIndex, Utf8CP propertyAccessString) const override
            {
            for (uint32_t index = 0; index < DerivedClass::MAX_PROPERTY_COUNT; ++index)
                {
                if (0 == BeStringUtilities::Stricmp(propertyAccessString, DerivedClass::PropertyNameList[index]))
                    {
                    propertyIndex = index + 1;
                    return ECObjectsStatus::Success;
                    }
                }
            return ECObjectsStatus::InvalidPropertyAccessString;
            }

        virtual ECObjectsStatus _GetAccessString  (Utf8CP& propertyAccessString, uint32_t propertyIndex) const override
            {
            if (propertyIndex > DerivedClass::MAX_PROPERTY_COUNT || propertyIndex <= 0)
                return ECObjectsStatus::IndexOutOfRange;

            propertyAccessString = DerivedClass::PropertyNameList[propertyIndex -1];
            return ECObjectsStatus::Success;
            }

        virtual uint32_t        _GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const override
            {
            if (inputIndex> 0 && inputIndex <= DerivedClass::MAX_PROPERTY_COUNT)
                return ++inputIndex;
            return 0;
            }

        
        virtual ECObjectsStatus _GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const override
            {
            for (uint32_t index = 0; index < DerivedClass::MAX_PROPERTY_COUNT; ++index)
                indices.push_back(index + 1);
            return ECObjectsStatus::Success;

            }
        virtual uint32_t        _GetFirstPropertyIndex (uint32_t parentIndex) const override {return 1;}

        virtual bool            _HasChildProperties (uint32_t parentIndex) const override {return false;}
        virtual uint32_t        _GetParentPropertyIndex (uint32_t childIndex) const override { return 0; }

    protected:
        PropertyIndexedEnabler (ECClassCR ecClass, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
            :BaseEnablerClass(ecClass, structStandaloneEnablerLocater)
            {}
    };

/*---------------------------------------------------------------------------------**//**
* Flattens out the non-struct property indices of an ECEnabler such that property indices
* are returned in order, recursing into struct members.
* Does not return property indices which represent structs.
* @bsistruct                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyIndexFlatteningIterator
    {
private:
    struct State
        {
        bvector<uint32_t>     m_propertyIndices;
        uint32_t            m_listIndex;

        bool                Init (ECEnablerCR enabler, uint32_t parentPropertyIndex);
        uint32_t            GetPropertyIndex() const { return m_propertyIndices[m_listIndex]; }
        bool                IsEnd() const { return m_listIndex >= m_propertyIndices.size(); }
        };

    ECEnablerCR         m_enabler;
    bvector<State>      m_states;

    bool                InitForCurrent();
public:
    ECOBJECTS_EXPORT PropertyIndexFlatteningIterator (ECEnablerCR enabler);
    ECOBJECTS_EXPORT bool   GetCurrent (uint32_t& propertyIndex) const;
    ECOBJECTS_EXPORT bool   MoveNext();
    ECEnablerCR             GetEnabler() const { return m_enabler; }
    };

 /*__PUBLISH_SECTION_START__*/
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
