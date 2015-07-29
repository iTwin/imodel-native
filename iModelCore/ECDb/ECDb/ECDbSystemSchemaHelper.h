/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    SourceECInstanceId,
    SourceECClassId,
    TargetECInstanceId,
    TargetECClassId,
    ECPropertyPathId,
    ECArrayIndex,
    ParentECInstanceId,
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct ECDbSystemSchemaHelper : NonCopyableClass
    {
public:
    static Utf8CP const ECDBSYSTEM_SCHEMANAME;
    static Utf8CP const ECINSTANCEID_PROPNAME;
    static Utf8CP const SOURCEECINSTANCEID_PROPNAME;
    static Utf8CP const SOURCEECCLASSID_PROPNAME;
    static Utf8CP const TARGETECINSTANCEID_PROPNAME;
    static Utf8CP const TARGETECCLASSID_PROPNAME;
    static Utf8CP const ECPROPERTYPATHID_PROPNAME;
    static Utf8CP const ECARRAYINDEX_PROPNAME;
    static Utf8CP const PARENTECINSTANCEID_PROPNAME;
    //static Utf8CP const ECPROPERTYID_PROPNAME;
    static Utf8CP const OWNERECINSTANCEID_PROPNAME;

private:
    static Utf8CP const ECSQLSYSTEMPROPERTIES_CLASSNAME;

    //static class
    ECDbSystemSchemaHelper ();
    ~ECDbSystemSchemaHelper ();

    static ECN::ECClassCP GetECClass (ECN::ECSchemaCR ecdbSystemSchema, Utf8CP className);
    static ECN::ECPropertyCP GetECProperty (ECN::ECClassCR ecClass, Utf8CP propertyName);
    static Utf8CP GetPropertyName (ECSqlSystemProperty kind);

public:
    //! Gets the ECSqlSystemPropertiesClass ECClass.
    //! @param[in] schemaManager SchemaManager used to retrieve the ECDb_System ECSchema.
    //! @return ECSqlSystemPropertiesClass class or nullptr in case of errors
    static ECN::ECClassCP GetECSqlSystemPropertiesClass (ECDbSchemaManagerCR schemaManager);

    //! Gets the ECDb_System ECSchema.
    //! @param[in] schemaManager SchemaManager used to retrieve the ECDb_System ECSchema.
    //! @return ECDb_System ECSchema or nullptr in case of errors
    static ECN::ECSchemaCP GetSchema (ECDbSchemaManagerCR schemaManager);

    //! Gets the system property of the specified kind from the ECDb_System ECSchema.
    //! @param[in] schemaManager SchemaManager used to retrieve the respective system property
    //! @return System property or nullptr in case of errors
    static ECN::ECPropertyCP GetSystemProperty (ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind);

    //! Checks whether the specified property is a system property of the given kind.
    //! @param[in] ecProperty Property to check whether it is a system property of the given kind or not
    //! @return true, if @p ecProperty is a system property of the given kind. false otherwise
    static bool IsSystemProperty (ECN::ECPropertyCR ecProperty, ECSqlSystemProperty kind);

    //! If @p ecProperty is a system property, returns the kind of system property.
    //! @param[out] kind System property kind
    //! @param[in] ecProperty Property to check
    //! @return true, if the given property is a system property. false, otherwise
    static bool TryGetSystemPropertyKind (ECSqlSystemProperty& kind, ECN::ECPropertyCR ecProperty);

    static Utf8CP ToString (ECSqlSystemProperty systemProperty);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
