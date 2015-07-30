/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECUnits/ECUnitsClassLocater.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

// Note: This is a barebones implementation targeting Graphite requirements.
// It supports applying UnitSpecifications and DisplayUnitSpecifications to ECProperties.

#include <ECObjects/ECObjects.h>

EC_TYPEDEFS (ECUnitsClassLocater);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<ECUnitsClassLocater> ECUnitsClassLocaterPtr;

//---------------------------------------------------------------------------------
// Because Graphite loads ECClasses from schemas dynamically, ECSchemas are not pre-
// processed for units info and units info is not cached.
// @bsistruct                                                    Paul.Connelly   09/12
//+---------------+---------------+---------------+---------------+---------------+------*/
struct ECUnitsClassLocater : RefCounted<IECClassLocater>
    {
    private:
        ECSchemaReadContextPtr   m_context;
        ECSchemaPtr              m_unitsSchema, m_koqSchema;
        static ECUnitsClassLocaterPtr s_unitsECClassLocaterPtr;

        bool Initialize();

        ECClassCP _LocateClass (Utf8CP schemaName, Utf8CP className);

    protected:
        ECUnitsClassLocater () {}
        ~ECUnitsClassLocater() {}

    public:
        static ECUnitsClassLocaterPtr Create();
        ECOBJECTS_EXPORT static bool LoadUnitsSchemas (ECSchemaReadContextR context);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
