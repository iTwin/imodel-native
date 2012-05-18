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

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<ECSchemaReadContext>      ECSchemaReadContextPtr;
//=======================================================================================
//! Context object used for schema creation and deserialization.</summary>
//=======================================================================================
struct ECSchemaReadContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
friend struct ECSchema;

private:
    IECSchemaOwnerR                 m_schemaOwner;
    IStandaloneEnablerLocaterP      m_standaloneEnablerLocater;

    bvector<IECSchemaLocaterP>      m_locators;
    IECSchemaLocaterP               m_finalSchemaLocater;
    T_WStringVector                 m_searchPaths;
    bool                            m_hideSchemasFromLeakDetection;
    bool                            m_acceptLegacyImperfectLatestCompatibleMatch;

    bvector<IECSchemaLocaterP>& GetSchemaLocaters ();
    IECSchemaLocaterP           GetFinalSchemaLocater ();
    T_WStringVector&            GetSchemaPaths ();
    IECSchemaOwnerR             GetSchemaOwner();
    IStandaloneEnablerLocaterP  GetStandaloneEnablerLocater();
    bool                        GetHideSchemasFromLeakDetection();

    void                        ClearSchemaPaths();

protected:
    //! Constructs a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas read with this context
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT ECSchemaReadContext(IECSchemaOwnerR schemaOwner, IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch);

    
public:
    ECOBJECTS_EXPORT void HideSchemasFromLeakDetection();

/*__PUBLISH_SECTION_START__*/

    //! Creates a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas read with this context
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @remarks This more-flexible override is primarily for internal use
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (IECSchemaOwnerR schemaOwner, IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch = false);
    
    //! Creates a context for deserializing ECSchemas
    //! @param[in] schemaOwner  This object will control the lifetime of any ECSchemas read with this context
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (IECSchemaOwnerR schemaOwner, bool acceptLegacyImperfectLatestCompatibleMatch = false);

    ECOBJECTS_EXPORT void AddSchemaLocaters (bvector<EC::IECSchemaLocaterP>&);

    ECOBJECTS_EXPORT void AddSchemaLocater (IECSchemaLocaterR);
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP);
    ECOBJECTS_EXPORT void SetFinalSchemaLocater (IECSchemaLocaterR);
};

typedef RefCountedPtr<ECInstanceReadContext>      ECInstanceReadContextPtr;
//=======================================================================================
//! Context object used for instance creation and deserialization.</summary>
//=======================================================================================
struct DLLPUBLIC_ATTRIBUTE ECInstanceReadContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    ECSchemaCP                          m_schema;
    EC::ECSchemaReadContextPtr          m_schemaContext;
    IStandaloneEnablerLocaterP          m_standaloneEnablerLocater;
    StandaloneECInstancePtr             m_dummy;

protected:
    ECInstanceReadContext(ECSchemaCP schema, ECSchemaReadContextP context, IStandaloneEnablerLocaterP standaloneEnablerLocater = NULL) 
        : m_standaloneEnablerLocater (standaloneEnablerLocater)
        {
        BeAssert (NULL == schema || NULL == context && L"Either schema or context should be NULL");

        m_schema = schema;
        m_schemaContext = context;
        }

    //! Will be called by ECInstance deserialization to create the ECInstances that it returns.
    //! The default implementation calls GetDefaultStandaloneEnabler() on the ecClass
    ECOBJECTS_EXPORT virtual IECInstancePtr _CreateStandaloneInstance (ECClassCR ecClass);
    
public:
    ECSchemaCP               GetSchemaCP()  { return m_schema; }
    ECSchemaReadContextPtr   GetSchemaContextPtr()  { return m_schemaContext; }

    IECInstancePtr           CreateStandaloneInstance (ECClassCR ecClass);

/*__PUBLISH_SECTION_START__*/

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterP = NULL);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaReadContextR, IStandaloneEnablerLocaterP = NULL);
};

END_BENTLEY_EC_NAMESPACE
