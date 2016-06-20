/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDbSchemaManager.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
enum class ECSqlSystemProperty
    {
    ECInstanceId,
    ECClassId,
    SourceECInstanceId,
    SourceECClassId,
    TargetECInstanceId,
    TargetECClassId
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct ECDbSystemSchemaHelper : NonCopyableClass
    {
    public:
        static Utf8CP const ECDBSYSTEM_SCHEMANAME;
        static Utf8CP const ECINSTANCEID_PROPNAME;
        static Utf8CP const ECCLASSID_PROPNAME;
        static Utf8CP const SOURCEECINSTANCEID_PROPNAME;
        static Utf8CP const SOURCEECCLASSID_PROPNAME;
        static Utf8CP const TARGETECINSTANCEID_PROPNAME;
        static Utf8CP const TARGETECCLASSID_PROPNAME;

private:
    static Utf8CP const ECSQLSYSTEMPROPERTIES_CLASSNAME;

    //static class
    ECDbSystemSchemaHelper ();
    ~ECDbSystemSchemaHelper ();

    static ECN::ECPropertyCP GetECProperty (ECN::ECClassCR, Utf8CP propertyName);
    static Utf8CP GetPropertyName (ECSqlSystemProperty);

public:
    //! Gets the ECSqlSystemPropertiesClass ECClass.
    //! @return ECSqlSystemPropertiesClass class or nullptr in case of errors
    static ECN::ECClassCP GetECSqlSystemPropertiesClass (ECDbSchemaManagerCR);

    //! Gets the system property of the specified kind from the ECDb_System ECSchema.
    //! @return System property or nullptr in case of errors
    static ECN::ECPropertyCP GetSystemProperty (ECDbSchemaManagerCR, ECSqlSystemProperty);

    //! Checks whether the specified property is a system property of the given kind.
    //! @return true, if @p ecProperty is a system property of the given kind. false otherwise
    static bool IsSystemProperty (ECN::ECPropertyCR, ECSqlSystemProperty);

    //! If @p ecProperty is a system property, returns the kind of system property.
    //! @return true, if the given property is a system property. false, otherwise
    static bool TryGetSystemPropertyKind (ECSqlSystemProperty&, ECN::ECPropertyCR);

    static Utf8CP ToString (ECSqlSystemProperty);

    static ECN::ECClassCP GetClassForPrimitiveArrayPersistence(ECDbCR, ECN::PrimitiveType);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
