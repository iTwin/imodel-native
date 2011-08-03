/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECContext.h $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <Geom\GeomApi.h>

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<ECSchemaDeserializationContext>      ECSchemaDeserializationContextPtr;
//=======================================================================================
//! Context object used for schema creation and deserialization.</summary>
//=======================================================================================
struct ECSchemaDeserializationContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;

private:
    IECSchemaOwnerR                 m_schemaOwner;
    IStandaloneEnablerLocaterR      m_standaloneEnablerLocater;

    bvector<IECSchemaLocaterP>      m_locators;
    IECSchemaLocaterP               m_finalSchemaLocater;
    T_WStringVector                 m_searchPaths;
    bool                            m_hideSchemasFromLeakDetection;
    bool                            m_acceptLegacyImperfectLatestCompatibleMatch;

    //! Constructs a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas deserialized with this context
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the deserialized ECSchema
    ECSchemaDeserializationContext(IECSchemaOwnerR schemaOwner, IStandaloneEnablerLocaterR standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch);

    bvector<IECSchemaLocaterP>& GetSchemaLocaters ();
    IECSchemaLocaterP           GetFinalSchemaLocater ();
    T_WStringVector&            GetSchemaPaths ();
    IECSchemaOwnerR             GetSchemaOwner();
    IStandaloneEnablerLocaterR  GetStandaloneEnablerLocater();
    bool                        GetHideSchemasFromLeakDetection();

    void                        ClearSchemaPaths();

public:
    ECOBJECTS_EXPORT void HideSchemasFromLeakDetection();

/*__PUBLISH_SECTION_START__*/

    //! Creates a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas deserialized with this context
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the deserialized ECSchema
    //! @remarks This more-flexible override is primarily for internal use
    ECOBJECTS_EXPORT static ECSchemaDeserializationContextPtr CreateContext (IECSchemaOwnerR schemaOwner, IStandaloneEnablerLocaterR standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch = false);
    
    //! Creates a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas deserialized with this context
    ECOBJECTS_EXPORT static ECSchemaDeserializationContextPtr CreateContext (ECSchemaCacheR schemaOwner, bool acceptLegacyImperfectLatestCompatibleMatch = false);

    ECOBJECTS_EXPORT void AddSchemaLocaters (bvector<EC::IECSchemaLocaterP>&);

    ECOBJECTS_EXPORT void AddSchemaLocater (IECSchemaLocaterR);
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP);
    ECOBJECTS_EXPORT void SetFinalSchemaLocater (IECSchemaLocaterR);
};

typedef RefCountedPtr<ECInstanceDeserializationContext>      ECInstanceDeserializationContextPtr;
//=======================================================================================
//! Context object used for instance creation and deserialization.</summary>
//=======================================================================================
struct ECInstanceDeserializationContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    ECSchemaCP                              m_schema;
    EC::ECSchemaDeserializationContextPtr   m_schemaContext;
    IStandaloneEnablerLocaterR              m_standaloneEnablerLocater;

    /* ctor */ ECInstanceDeserializationContext(ECSchemaCP schema, ECSchemaDeserializationContextP context, IStandaloneEnablerLocaterR standaloneEnablerLocater) 
        : m_standaloneEnablerLocater (standaloneEnablerLocater)
        {
        assert (NULL == schema || NULL == context && L"Either schema or context should be NULL");

        m_schema = schema;
        m_schemaContext = context;
        }

public:
    ECSchemaCP                          GetSchemaCP()  { return m_schema; }
    ECSchemaDeserializationContextPtr   GetSchemaContextPtr()  { return m_schemaContext; }
    IStandaloneEnablerLocaterR          GetStandaloneEnablerLocater() { return m_standaloneEnablerLocater; }

/*__PUBLISH_SECTION_START__*/

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceDeserializationContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterR);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceDeserializationContextPtr CreateContext (ECSchemaDeserializationContextR, IStandaloneEnablerLocaterR);
};

END_BENTLEY_EC_NAMESPACE
