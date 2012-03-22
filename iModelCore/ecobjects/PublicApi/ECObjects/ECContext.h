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

    typedef ECSchemaPtr ReadSchemaInfo;

private:
    IStandaloneEnablerLocaterP      m_standaloneEnablerLocater;
    
    bmap<SchemaKey, ReadSchemaInfo> m_knownSchemas;
    bvector<IECSchemaLocaterP>      m_locators;
    IECSchemaLocaterP               m_finalSchemaLocater;
    T_WStringVector                 m_searchPaths;
    bool                            m_hideSchemasFromLeakDetection;
    bool                            m_acceptLegacyImperfectLatestCompatibleMatch;

    bvector<IECSchemaLocaterP>& GetSchemaLocaters ();
    IECSchemaLocaterP           GetFinalSchemaLocater ();
    T_WStringVector&            GetSchemaPaths ();
    IStandaloneEnablerLocaterP  GetStandaloneEnablerLocater();
    bool                        GetHideSchemasFromLeakDetection();

    void                        ClearSchemaPaths();

protected:
    //! Constructs a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT ECSchemaReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch);

    
public:
    void               AddSchema(ECSchemaR schema, bool owned);
    void               RemoveSchema(ECSchemaR schema, bool owned);
    ReadSchemaInfo*    GetSchema (SchemaKeyCR key, SchemaMatchType matchType);

    ECOBJECTS_EXPORT void HideSchemasFromLeakDetection();

/*__PUBLISH_SECTION_START__*/

    //! Creates a context for deserializing ECSchemas
    //! @param[in] standaloneEnablerLocater  Used to find enablers for instantiating instances of ECCustomAttributes used in the read ECSchema
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    //! @remarks This more-flexible override is primarily for internal use
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (IStandaloneEnablerLocaterP standaloneEnablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch = false);
    
    //! Creates a context for deserializing ECSchemas
    //! @param[in] acceptLegacyImperfectLatestCompatibleMatch  If true, LatestCompatible only checks that the major version matches. A warning will be logged if minor version is too low, but the ECSchema will be accepted
    ECOBJECTS_EXPORT static ECSchemaReadContextPtr CreateContext (bool acceptLegacyImperfectLatestCompatibleMatch = false);

    ECOBJECTS_EXPORT void AddSchemaLocaters (bvector<EC::IECSchemaLocaterP>&);

    ECOBJECTS_EXPORT void AddSchemaLocater (IECSchemaLocaterR);
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP);
    ECOBJECTS_EXPORT void SetFinalSchemaLocater (IECSchemaLocaterR);
};

typedef RefCountedPtr<ECInstanceReadContext>      ECInstanceReadContextPtr;
//=======================================================================================
//! Context object used for instance creation and deserialization.</summary>
//=======================================================================================
struct ECInstanceReadContext /*__PUBLISH_ABSTRACT__*/ : RefCountedBase
{
/*__PUBLISH_SECTION_END__*/
private:
    IStandaloneEnablerLocaterP          m_standaloneEnablerLocater;
    StandaloneECInstancePtr             m_dummy;

protected:
    ECInstanceReadContext(IStandaloneEnablerLocaterP standaloneEnablerLocater = NULL) 
        : m_standaloneEnablerLocater (standaloneEnablerLocater)
        {
        }

    //! Will be called by ECInstance deserialization to create the ECInstances that it returns.
    //! The default implementation calls GetDefaultStandaloneEnabler() on the ecClass
    ECOBJECTS_EXPORT virtual IECInstancePtr _CreateStandaloneInstance (ECClassCR ecClass);
    
    virtual ECSchemaCP  _FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const = 0;

public:
    ECSchemaCP  FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const;

    IECInstancePtr           CreateStandaloneInstance (ECClassCR ecClass);

/*__PUBLISH_SECTION_START__*/

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocaterP = NULL);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceReadContextPtr CreateContext (ECSchemaReadContextR, ECSchemaPtr& foundSchema, IStandaloneEnablerLocaterP = NULL);
};

END_BENTLEY_EC_NAMESPACE
