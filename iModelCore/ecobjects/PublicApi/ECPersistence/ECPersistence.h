/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPersistence/ECPersistence.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define NO_USING_NAMESPACE_BENTLEY 1

/*__PUBLISH_SECTION_START__*/

#include <Bentley/Bentley.h>

// In many of the DgnPlatform libraries we redefine the below macros based on __cplusplus.  This is because there
// are existing C callers that we can not get rid of.  I've spoken to Sam and he recommends that for any new libraries we
// ONLY support cpp callers and therefore do not repeat this pattern.
#ifdef __ECOBJECTS_BUILD__
    #define ECPERSISTENCE_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECPERSISTENCE_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_EC_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace EC {

#define END_BENTLEY_EC_NAMESPACE    }}

#define USING_NAMESPACE_EC  using namespace Bentley::EC;

#define EC_TYPEDEFS(_name_)  \
        BEGIN_BENTLEY_EC_NAMESPACE      \
            struct _name_;      \
            typedef _name_ *         _name_##P;  \
            typedef _name_ &         _name_##R;  \
            typedef _name_ const*    _name_##CP; \
            typedef _name_ const&    _name_##CR; \
        END_BENTLEY_EC_NAMESPACE

EC_TYPEDEFS(IECConnection);
EC_TYPEDEFS(IECSchemaManager);
EC_TYPEDEFS(IECStatement);

