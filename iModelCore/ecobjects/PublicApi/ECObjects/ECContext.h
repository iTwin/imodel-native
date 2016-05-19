/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECContext.h $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>

//__PUBLISH_SECTION_END__
#include <ECObjects/StandaloneECInstance.h>
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//! @addtogroup ECObjectsGroup
//! @beginGroup

typedef RefCountedPtr<ECSchemaReadContext>      ECSchemaReadContextPtr;
//=======================================================================================
//! Context object used for schema creation and deserialization.
//=======================================================================================
struct ECSchemaReadContext : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;
friend struct SearchPathSchemaFileLocater;
    struct WStringComparer : public std::binary_function<WString, WString, bool>
        {
        bool operator()(WStringCR s1, WStringCR s2) const
            {
            return s1.CompareTo(s2) < 0;
            }
        };
private:

    IStandaloneEnablerLocaterP              m_standaloneEnablerLocater;
    ECSchemaCachePtr                        m_knownSchemas;
    bvector<bool>                           m_knownSchemaDirtyStack;
    ECSchemaReadContextPtr                  m_conversionSchemas;
    bvector<IECSchemaLocaterP>                              m_locaters;
    int                                                     m_userAddedLocatersCount;
    int                                                     m_searchPathLocatersCount;
    typedef bset<WString, WStringComparer>                  SearchPathList;
    SearchPathList                          m_searchPaths;
    bvector<SearchPathSchemaFileLocaterPtr> m_ownedLocators;
    IECSchemaRemapperCP                     m_remapper;
    bool                                    m_acceptLegacyImperfectLatestCompatibleMatch;
    bvector<WString>                        m_cultureStrings;
    bool                                    m_preserveElementOrder = false;
    bool                                    m_preserveXmlComments = false;

    bool                        GetStandardPaths (bvector<WString>& standardPaths);

protected:
    //! Constructs a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @param[in] createConversionContext  If true a private schema read context is created to locate and store conversion schemas
    ECOBJECTS_EXPORT ECSchemaReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch, bool createConversionContext);

    //! Creates a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @param[in] createConversionContext  If true a private schema read context is created to locate and store conversion schemas
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch, bool createConversionContext);

    ECOBJECTS_EXPORT virtual void       _AddSchema (ECSchemaR schema);
public:
    IStandaloneEnablerLocaterP          GetStandaloneEnablerLocater();
    ECOBJECTS_EXPORT ECObjectsStatus    AddSchema(ECSchemaR schema);
    void                                RemoveSchema(ECSchemaR schema);
    ECSchemaPtr         GetFoundSchema (SchemaKeyCR key, SchemaMatchType matchType);

    ECOBJECTS_EXPORT ECObjectsStatus    AddConversionSchema(ECSchemaR schema);
    void                                RemoveConversionSchema(ECSchemaR schema);

    ECOBJECTS_EXPORT void AddSchemaLocaters (bvector<ECN::IECSchemaLocaterP> const& schemaLocators);

    IECSchemaRemapperCP GetRemapper() const                         { return m_remapper; }
    void                SetRemapper (IECSchemaRemapperCP remapper)  { m_remapper = remapper; }
    void                ResolveClassName (Utf8StringR serializedClassName, ECSchemaCR schema) const;


    bool GetPreserveElementOrder() { return m_preserveElementOrder; }
    void SetPreserveElementOrder(bool flag) { m_preserveElementOrder = flag; }

    bool GetPreserveXmlComments() { return m_preserveElementOrder; }
    void SetPreserveXmlComments(bool flag) 
        { 
        m_preserveXmlComments = flag; 
        // To preserve xml comments it's required that the element order is also preserved
        if (flag == true)
            m_preserveElementOrder = true;
        }

//__PUBLISH_SECTION_START__
public:
    //! Host should call to establish search paths for standard ECSchemas.
    //! @param[in] hostAssetsDirectory Directory to where the application has deployed assets that come with the API,
    //!            e.g. standard ECSchemas.
    //!            In the assets directory the standard ECSchemas have to be located in @b ECSchemas/Standard/.
    ECOBJECTS_EXPORT static void Initialize (BeFileNameCR hostAssetsDirectory);

    //! Gets the host assets directory to where the application deploys assets that come with the API, e.g. standard ECSchemas.
    //! Must have been set via ECSchemaReadContext::Initialize.
    //! @return Host assets directory
    ECOBJECTS_EXPORT static BeFileNameCR GetHostAssetsDirectory ();

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

    //! Adds a file path that should be used to search for a matching conversion schemas
    //! @param[in] path Path to the directory where conversion schemas can be found
    ECOBJECTS_EXPORT void AddConversionSchemaPath(WCharCP path);

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
    ECOBJECTS_EXPORT ECSchemaPtr LocateSchema (SchemaKeyR key, SchemaMatchType matchType);

    //! Gets the schemas cached by this context.
    //! @returns Schemas cached by this context
    ECOBJECTS_EXPORT ECSchemaCacheR GetCache ();

    //! Look for a _V8Conversion schema for the given schema
    //! @param[in] schemaName   The name of the schema to look for
    //! @param[in] versionMajor The major version of the schema to look for
    //! @param[in] versionMinor The minor version of the schema to look for
    ECOBJECTS_EXPORT ECSchemaPtr        LocateConversionSchemaFor(Utf8CP schemaName, int versionMajor, int versionMinor);

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
        virtual PrimitiveType       _ResolvePrimitiveType (PrimitiveECPropertyCR ecproperty) const = 0;
        virtual PrimitiveType       _ResolvePrimitiveArrayType (ArrayECPropertyCR ecproperty) const = 0;
        };

private:
    IStandaloneEnablerLocaterP          m_standaloneEnablerLocater;
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
    void                    ResolveSerializedPropertyName (Utf8StringR name, ECClassCR ecClass) const  { if (NULL != m_schemaRemapper) m_schemaRemapper->ResolvePropertyName (name, ecClass); }
    void                    ResolveSerializedClassName (Utf8StringR name, ECSchemaCR schema) const     { if (NULL != m_schemaRemapper) m_schemaRemapper->ResolveClassName (name, schema); }

    ECSchemaCP  FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const;

    IECInstancePtr           CreateStandaloneInstance (ECClassCR ecClass);

    ECSchemaCR GetFallBackSchema ();

public:

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterP = NULL, IPrimitiveTypeResolver const* typeResolver = NULL);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaReadContextR, ECSchemaCR fallBackSchema, ECSchemaPtr* foundSchema);
};
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE
