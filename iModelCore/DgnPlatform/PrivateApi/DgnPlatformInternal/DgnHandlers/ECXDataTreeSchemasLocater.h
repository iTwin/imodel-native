/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/ECXDataTreeSchemasLocater.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//! @cond DONTINCLUDEINDOC
#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

#define BENTLEY_EXCLUDE_WINDOWS_HEADERS // don't let ECObjectsAPI.h include Windows-specific stuff
#include <ECObjects/ECObjectsAPI.h>

#include <Bentley/bvector.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ECXDataTreeSchemasLocater
{
private:
    ECN::ECSchemaPtr       m_elementTemplateSchemaHolder;
    ECN::ECSchemaPtr       m_customInterfaceSchemaHolder;
    ECN::ECSchemaPtr       m_detailingSymbolStyleSchemaHolder;
    
    ECXDataTreeSchemasLocater ();
    
    void            CreateElementTemplateSchema ();
    void            CreateCustomInterfaceSchema ();
    void            CreateDetailingSymbolStyleSchema ();

public:
    static void     RegisterSchemas ();
}; // ECXDataTreeSchemasLocater

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

//! @endcond
