/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECContext.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
//! @addtogroup ECObjectsGroup
//! @beginGroup
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

        SchemaLocatorKey(){}
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
    typedef bset<SchemaLocatorKey>          SchemaLocatorSet;
    typedef bset<WString, WStringComparer>  SearchPathList;

    IStandaloneEnablerLocaterP              m_standaloneEnablerLocater;
    ECSchemaCachePtr                        m_knownSchemas;
    bvector<bool>                           m_knownSchemaDirtyStack;
    SchemaLocatorSet                        m_locators;
    SearchPathList                          m_searchPaths;
    bvector<SearchPathSchemaFileLocaterPtr> m_ownedLocators;
    IECSchemaRemapperCP                     m_remapper;
    bool                                    m_acceptLegacyImperfectLatestCompatibleMatch;
    bvector<WString>                        m_cultureStrings;

    
    SchemaLocatorSet::iterator  GetHighestLocatorInRange (uint32_t& prioirty);
    bool                        GetStandardPaths (bvector<WString>& standardPaths);

protected:
    //! Constructs a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT ECSchemaReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch);

    ECOBJECTS_EXPORT virtual void       _AddSchema (ECSchemaR schema);
public:
    IStandaloneEnablerLocaterP          GetStandaloneEnablerLocater();
    ECOBJECTS_EXPORT ECObjectsStatus    AddSchema(ECSchemaR schema);
    void                                RemoveSchema(ECSchemaR schema);
    ECSchemaPtr                         GetFoundSchema (SchemaKeyR key, SchemaMatchType matchType);

    ECOBJECTS_EXPORT ECSchemaCacheR GetFoundSchemas ();

    ECSchemaPtr         LocateSchema (SchemaKeyR key, bset<SchemaMatchType> const& matches);

    ECOBJECTS_EXPORT void AddExternalSchemaLocaters (bvector<ECN::IECSchemaLocaterP> const& schemaLocators);

    IECSchemaRemapperCP GetRemapper() const                         { return m_remapper; }
    void                SetRemapper (IECSchemaRemapperCP remapper)  { m_remapper = remapper; }
    void                ResolveClassName (WStringR serializedClassName, ECSchemaCR schema) const;
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! Creates a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @remarks This more-flexible override is primarily for internal use
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch = false);
    
    //! Creates a context for deserializing ECSchemas
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (bool acceptLegacyImperfectLatestCompatibleMatch = false);

    //! Adds a schema locater to the current context
    //! @param[in] locater  Locater to add to the current context
    ECOBJECTS_EXPORT void AddSchemaLocater (IECSchemaLocaterR locater);

    //! Removes a schema locater from the current context
    //! @param[in] locater  Locater to remove from the current context
    ECOBJECTS_EXPORT void RemoveSchemaLocater (IECSchemaLocaterR locater);

    //! Adds a file path that should be used to search for a matching schema name
    //! @param[in] path Path to the directory where schemas can be found
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP path);

    //! Adds a culture string that will be appended to the existing search paths
    //! when looking for localization supplemental schemas.
    //! @param[in] culture string in format cu-CU or just cu
    ECOBJECTS_EXPORT void AddCulture(WCharCP culture);

    //! Gets culture strings
    ECOBJECTS_EXPORT bvector<WString>* GetCultures();

    //! Set the last locater to be used when trying to find a schema
    //! @param[in] locater  Locater that should be used as the last locater when trying to find a schema
    ECOBJECTS_EXPORT void SetFinalSchemaLocater (IECSchemaLocaterR locater);

    //! Find the schema matching the schema key and using matchType as the match criteria. This uses the prioritized list of locators to find the schema.
    //! @param[in] key  The SchemaKey that defines the schema (name and version information) that is being looked for
    //! @param[in] matchType    The match type criteria used to locate the requested schema
    //! @returns An ECSchemaPtr.  This ptr will return false for IsValid() if the schema could not be located.
    ECOBJECTS_EXPORT ECSchemaPtr         LocateSchema (SchemaKeyR key, SchemaMatchType matchType);

    //! Gets the schemas cached by this context.
    //! @returns Schemas cached by this context
    ECOBJECTS_EXPORT ECSchemaCacheR GetCache ();
};

typedef RefCountedPtr<ECInstanceReadContext>      ECInstanceReadContextPtr;
//=======================================================================================
//! Context object used for instance creation and deserialization.
//=======================================================================================
struct ECInstanceReadContext : RefCountedBase
{
    // InstanceXml does not contain primitive type information. An IPrimitiveTypeResolver can assist the
    // ECInstanceReadContext in determining the type of a serialized primitive value.
    // If no IPrimitiveTypeResolver is supplied, the primitive type defined for the ECProperty is used.
    struct IPrimitiveTypeResolver
        {
/*__PUBLISH_SECTION_END__*/
        virtual PrimitiveType       _ResolvePrimitiveType (PrimitiveECPropertyCR ecproperty) const = 0;
        virtual PrimitiveType       _ResolvePrimitiveArrayType (ArrayECPropertyCR ecproperty) const = 0;
/*__PUBLISH_SECTION_START__*/
        };

/*__PUBLISH_SECTION_END__*/
private:
    IStandaloneEnablerLocaterP          m_standaloneEnablerLocater;
    StandaloneECInstancePtr             m_dummy;
    ECSchemaCR                          m_fallBackSchema;
    IPrimitiveTypeResolver const*       m_typeResolver;
    IECSchemaRemapperCP                 m_schemaRemapper;

protected:
    ECInstanceReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, ECSchemaCR fallBackSchema, IPrimitiveTypeResolver const* typeResolver) 
        : m_standaloneEnablerLocater (standaloneEnablerLocater), m_fallBackSchema (fallBackSchema), m_typeResolver (typeResolver), m_schemaRemapper (NULL)
        {
        }

    //! Will be called by ECInstance deserialization to create the ECInstances that it returns.
    //! The default implementation calls GetDefaultStandaloneEnabler() on the ecClass
    ECOBJECTS_EXPORT virtual IECInstancePtr _CreateStandaloneInstance (ECClassCR ecClass);
    
    virtual ECSchemaCP  _FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const = 0;

public:
    PrimitiveType           GetSerializedPrimitiveType (PrimitiveECPropertyCR ecprop) const  { return m_typeResolver != NULL ? m_typeResolver->_ResolvePrimitiveType (ecprop) : ecprop.GetType(); }
    PrimitiveType           GetSerializedPrimitiveArrayType (ArrayECPropertyCR ecprop) const { return m_typeResolver != NULL ? m_typeResolver->_ResolvePrimitiveArrayType (ecprop) : ecprop.GetPrimitiveElementType(); }

    void                    SetSchemaRemapper (IECSchemaRemapperCP remapper) { m_schemaRemapper = remapper; }
    void                    SetTypeResolver (IPrimitiveTypeResolver const* resolver) { m_typeResolver = resolver; }
    void                    ResolveSerializedPropertyName (WStringR name, ECClassCR ecClass) const  { if (NULL != m_schemaRemapper) m_schemaRemapper->ResolvePropertyName (name, ecClass); }
    void                    ResolveSerializedClassName (WStringR name, ECSchemaCR schema) const     { if (NULL != m_schemaRemapper) m_schemaRemapper->ResolveClassName (name, schema); }

    ECSchemaCP  FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const;

    IECInstancePtr           CreateStandaloneInstance (ECClassCR ecClass);

    ECSchemaCR GetFallBackSchema ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterP = NULL, IPrimitiveTypeResolver const* typeResolver = NULL);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaReadContextR, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema);
};
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
