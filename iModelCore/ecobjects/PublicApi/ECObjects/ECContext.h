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
friend  ECSchema;

private:
    IECSchemaOwnerR                 m_schemaOwner;
    IStandaloneEnablerLocatorR      m_standaloneEnablerLocator;

    bvector<IECSchemaLocatorP>      m_locators;
    T_WStringVector                 m_searchPaths;
    bool                            m_hideSchemasFromLeakDetection;

    ECSchemaDeserializationContext(IECSchemaOwnerR, IStandaloneEnablerLocatorR);

    bvector<IECSchemaLocatorP>& GetSchemaLocators ();
    T_WStringVector&            GetSchemaPaths ();
    IECSchemaOwnerR             GetSchemaOwner();
    IStandaloneEnablerLocatorR  GetStandaloneEnablerLocator();
    bool                        GetHideSchemasFromLeakDetection();

    void                        ClearSchemaPaths();

public:
    ECOBJECTS_EXPORT void HideSchemasFromLeakDetection();

/*__PUBLISH_SECTION_START__*/
    ECOBJECTS_EXPORT static ECSchemaDeserializationContextPtr CreateContext (IECSchemaOwnerR, IStandaloneEnablerLocatorR);

    ECOBJECTS_EXPORT void AddSchemaLocators (bvector<EC::IECSchemaLocatorP>&);

    ECOBJECTS_EXPORT void AddSchemaLocator (IECSchemaLocatorR);
    ECOBJECTS_EXPORT void AddSchemaPath (WCharCP);
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
    IStandaloneEnablerLocatorR              m_standaloneEnablerLocator;

    /* ctor */ ECInstanceDeserializationContext(ECSchemaCP schema, ECSchemaDeserializationContextP context, IStandaloneEnablerLocatorR standaloneEnablerLocator) 
        : m_standaloneEnablerLocator (standaloneEnablerLocator)
        {
        assert (NULL == schema || NULL == context && L"Either schema or context should be NULL");

        m_schema = schema;
        m_schemaContext = context;
        }

public:
    ECSchemaCP                          GetSchemaCP()  { return m_schema; }
    ECSchemaDeserializationContextPtr   GetSchemaContextPtr()  { return m_schemaContext; }
    IStandaloneEnablerLocatorR          GetStandaloneEnablerLocator() { return m_standaloneEnablerLocator; }

/*__PUBLISH_SECTION_START__*/

    //! - For use when the caller knows the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceDeserializationContextPtr CreateContext (ECSchemaCR, IStandaloneEnablerLocatorR);

    //! - For use when the caller does not know the schema of the instance he is deserializing.
    ECOBJECTS_EXPORT static ECInstanceDeserializationContextPtr CreateContext (ECSchemaDeserializationContextR, IStandaloneEnablerLocatorR);
};

END_BENTLEY_EC_NAMESPACE
