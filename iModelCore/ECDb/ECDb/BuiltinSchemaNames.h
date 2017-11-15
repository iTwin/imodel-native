/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/BuiltinSchemaNames.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSCHEMA_ECDbSystem "ECDbSystem"
#define ECSCHEMA_ALIAS_ECDbSystem "ecdbsys"

#define ECDBSYS_CLASS_ClassECSqlSystemProperties "ClassECSqlSystemProperties"
#define ECDBSYS_CLASS_RelationshipECSqlSystemProperties "RelationshipECSqlSystemProperties"
#define ECDBSYS_CLASS_PointECSqlSystemProperties "PointECSqlSystemProperties"
#define ECDBSYS_CLASS_NavigationECSqlSystemProperties "NavigationECSqlSystemProperties"

#define ECDBSYS_PROP_ECInstanceId "ECInstanceId"
#define ECDBSYS_PROPALIAS_Id "Id"

#define ECDBSYS_PROP_ECClassId "ECClassId"
#define ECDBSYS_PROP_SourceECInstanceId "SourceECInstanceId"
#define ECDBSYS_PROPALIAS_SourceId "SourceId"
#define ECDBSYS_PROP_SourceECClassId "SourceECClassId"
#define ECDBSYS_PROP_TargetECInstanceId "TargetECInstanceId"
#define ECDBSYS_PROPALIAS_TargetId "TargetId"
#define ECDBSYS_PROP_TargetECClassId "TargetECClassId"
#define ECDBSYS_PROP_NavPropId "Id"
#define ECDBSYS_PROP_NavPropRelECClassId "RelECClassId"
#define ECDBSYS_PROP_PointX "X"
#define ECDBSYS_PROP_PointY "Y"
#define ECDBSYS_PROP_PointZ "Z"


#define ECSCHEMA_ECDbChangeSummaries "ECDbChangeSummaries"
#define ECSCHEMA_ALIAS_ECDbChangeSummaries "change"

#define ECDBCHANGE_CLASS_Summary "Summary"
#define ECDBCHANGE_CLASS_Instance "Instance"
#define ECDBCHANGE_CLASS_PropertyValue "PropertyValue"

#define ECDBCHANGE_PROP_Operation "Operation"
#define ECDBCHANGE_PROP_IdOfChangedInstance "IdOfChangedInstance"
#define ECDBCHANGE_PROP_ClassIdOfChangedInstance "ClassIdOfChangedInstance"
#define ECDBCHANGE_PROP_SummaryId "Summary.Id"

END_BENTLEY_SQLITE_EC_NAMESPACE