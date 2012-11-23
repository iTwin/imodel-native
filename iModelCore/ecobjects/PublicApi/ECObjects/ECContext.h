/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECContext.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <Geom/GeomApi.h>
#include <ECObjects/StandaloneECInstance.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECSchemaReadContext>      ECSchemaReadContextPtr;
//=======================================================================================
//! Context object used for schema creation and deserialization.
//! @ingroup ECObjectsGroup
//=======================================================================================
struct ECSchemaReadContext : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;
friend struct SearchPathSchemaFileLocater;
    static const int CATEGORY_PARTITION_SIZE = 5000;
    enum PriorityPartiion
        {
        ReaderContext   = -1* CATEGORY_PARTITION_SIZE, //Whatever we found is off highest priority
        UserSpace       = 0*  CATEGORY_PARTITION_SIZE,
        External        = 1*  CATEGORY_PARTITION_SIZE,
        StandardPaths   = 2*  CATEGORY_PARTITION_SIZE,
        Final           = 3*  CATEGORY_PARTITION_SIZE,
        };

    struct SchemaLocatorKey
        {
        int                 m_priority;
        IECSchemaLocaterP   m_locator;

        bool operator < (SchemaLocatorKey const & rhs) const 
            {
            if (m_priority != rhs.m_priority)
                return m_priority < rhs.m_priority;//Order the higher priority ones first

            return m_locator < rhs.m_locator;
            }

        SchemaLocatorKey (IECSchemaLocaterP locator, int priority)
            :m_locator(locator), m_priority(priority)
            {}
        };

    struct WStringComparer : public std::binary_function<WString, WString, bool>
        {
        bool operator()(WStringCR s1, WStringCR s2) const
            {
            return s1.CompareTo(s2) < 0;
            }
        };
private:
    IStandaloneEnablerLocaterP                              m_standaloneEnablerLocater;
    ECSchemaCache                                           m_knownSchemas;
    bvector<bool>                                           m_knownSchemaDirtyStack;
    typedef bset<SchemaLocatorKey>                          SchemaLocatorSet;
    SchemaLocatorSet                                        m_locators;
    typedef bset<WString, WStringComparer>                  SearchPathList;
    SearchPathList                                          m_searchPaths;
    bvector<SearchPathSchemaFileLocaterPtr>                 m_ownedLocators;
    bool                            m_acceptLegacyImperfectLatestCompatibleMatch;

    
    SchemaLocatorSet::iterator  GetHighestLocatorInRange (UInt32& prioirty);
    bool                        GetStandardPaths (bvector<WString>& standardPaths);

protected:
    //! Constructs a context for deserializing ECSchemas
    //! @param[in] ecSchemaCache  Uses supplied schema cache as container to store schemas that are read
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT ECSchemaReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch);

    ECOBJECTS_EXPORT virtual void       _AddSchema (ECSchemaR schema);
public:
    IStandaloneEnablerLocaterP  GetStandaloneEnablerLocater();
    void                AddSchema(ECSchemaR schema);
    void                RemoveSchema(ECSchemaR schema);
    ECSchemaPtr         GetFoundSchema (SchemaKeyR key, SchemaMatchType matchType);
    ECSchemaCacheR     GetCache()  { return m_knownSchemas; }
    ECOBJECTS_EXPORT ECSchemaCacheR GetKnownSchemas ();

    ECSchemaPtr         LocateSchema (SchemaKeyR key, bset<SchemaMatchType> const& matches);

    ECOBJECTS_EXPORT void AddExternalSchemaLocaters (bvector<ECN::IECSchemaLocaterP> const& schemaLocators);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Host should call to establish standard search paths.
    ECOBJECTS_EXPORT static void Initialize (BeFileNameCR rootDirectory);

    //! Creates a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @remarks This more-flexible override is primarily for internal use
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch = false);
    
    //! Creates a context for deserializing ECSchemas
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (bool acceptLegacyImperfectLatestCompatibleMatch = false);

    ECOBJECTS_EXPORT void AddSchemaLocater (IECSchemaLocaterR);
    ECOBJECTS_EXPORT void RemoveSchemaLocater (IECSchemaLocaterR);
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP);
    ECOBJECTS_EXPORT void SetFinalSchemaLocater (IECSchemaLocaterR);

    //Find the schema matching the schema key and using matchType as the match criteria. This uses the prioritized list of locators to find the schema.
    //WIP_FUSION: Why is the SchemaKey argument non-null??
    ECOBJECTS_EXPORT ECSchemaPtr         LocateSchema (SchemaKeyR key, SchemaMatchType matchType);
};

typedef RefCountedPtr<ECInstanceReadContext>      ECInstanceReadContextPtr;
//=======================================================================================
//! Context object used for instance creation and deserialization.
//! @ingroup ECObjectsGroup
//=======================================================================================
struct ECInstanceReadContext : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    IStandaloneEnablerLocaterP          m_standaloneEnablerLocater;
    StandaloneECInstancePtr             m_dummy;
    ECSchemaCR                          m_fallBackSchema;
protected:
    ECInstanceReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, ECSchemaCR fallBackSchema) 
        : m_standaloneEnablerLocater (standaloneEnablerLocater), m_fallBackSchema (fallBackSchema)
        {
        }

    //! Will be called by ECInstance deserialization to create the ECInstances that it returns.
    //! The default implementation calls GetDefaultStandaloneEnabler() on the ecClass
    ECOBJECTS_EXPORT virtual IECInstancePtr _CreateStandaloneInstance (ECClassCR ecClass);
    
    virtual ECSchemaCP  _FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const = 0;

public:

    ECSchemaCP  FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const;

    IECInstancePtr           CreateStandaloneInstance (ECClassCR ecClass);

    ECSchemaCR GetFallBackSchema ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterP = NULL);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaReadContextR, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema);
};

END_BENTLEY_ECOBJECT_NAMESPACE
