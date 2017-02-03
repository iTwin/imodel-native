/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPch.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/CatchNonPortable.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <Bentley/DateTime.h>
#include <Bentley/BeTimeUtilities.h>
#include <algorithm>
#include <memory>

#include "ECDbInternalTypes.h"
#include "ECDbLogger.h"
#include "IssueReporter.h"
#include "ECDbImpl.h"

#include "ECDbProfileManager.h"
#include "ECDbProfileUpgrader.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECDbMapSchemaHelper.h"
#include "ECDbPolicyManager.h"

#include "ECDbSqlFunctions.h"

#include "ECDbMap.h"
#include "MapStrategy.h"
#include "DbSchema.h"
#include "SqlNames.h"
#include "DbSchemaPersistenceManager.h"
#include "ClassMap.h"
#include "PropertyMap.h"
#include "SystemPropertyMap.h"
#include "PropertyMapVisitor.h"
#include "RelationshipClassMap.h"
#include "ClassMappingInfo.h"
#include "ClassMapPersistenceManager.h"
#include "ClassMapPersistenceManager.h"
#include "ClassMapper.h"
#include "ECSchemaComparer.h"
#include "LightweightCache.h"
#include "ClassMapColumnFactory.h"
#include "ViewGenerator.h"
#include "ECDbSchemaPersistenceHelper.h"
#include "ECDbSchemaReader.h"
#include "ECDbSchemaWriter.h"
#include "ECSchemaValidator.h"
#include "SchemaImportContext.h"

#include "ECSql/NativeSqlBuilder.h"
#include "ECSql/Parser/SqlScan.h"
#include "ECSql/Parser/SqlNode.h"
#include "ECSql/Parser/IParseContext.h"
#include "ECSql/Parser/InternalNode.h"
#include "ECSql/Parser/SqlParse.h"
#include "ECSql/ECSqlParser.h"
#include "ECSql/ClassRefExp.h"
#include "ECSql/ComputedExp.h"
#include "ECSql/Exp.h"
#include "ECSql/JoinExp.h"
#include "ECSql/ListExp.h"
#include "ECSql/PropertyNameExp.h"
#include "ECSql/ValueExp.h"
#include "ECSql/WhereExp.h"
#include "ECSql/SelectStatementExp.h"
#include "ECSql/InsertStatementExp.h"
#include "ECSql/UpdateStatementExp.h"
#include "ECSql/DeleteStatementExp.h"
#include "ECSql/OptionsExp.h"
#include "ECSql/ExpHelper.h"
#include "ECSql/ECSqlTypeInfo.h"

#include "ECSql/ECSqlPrepareContext.h"
#include "ECSql/ECSqlDeletePreparer.h"

#include "ECSql/ECSqlStatementBase.h"
#include "ECSql/ECSqlStatementImpl.h"

#include "ECSql/IECSqlPrimitiveValue.h"

#include "ECSql/ECSqlFieldFactory.h"
#include "ECSql/ECSqlField.h"
#include "ECSql/PrimitiveECSqlField.h"
#include "ECSql/PrimitiveArrayECSqlField.h"
#include "ECSql/PointECSqlField.h"
#include "ECSql/StructECSqlField.h"
#include "ECSql/StructArrayECSqlField.h"
#include "ECSql/NavigationPropertyECSqlField.h"

#include "ECSql/ECSqlBinder.h"
#include "ECSql/ECSqlBinderFactory.h"
#include "ECSql/IdECSqlBinder.h"
#include "ECSql/NavigationPropertyECSqlBinder.h"
#include "ECSql/PointECSqlBinder.h"
#include "ECSql/PrimitiveECSqlBinder.h"
#include "ECSql/PrimitiveArrayECSqlBinder.h"
#include "ECSql/StructArrayECSqlBinder.h"
#include "ECSql/StructECSqlBinder.h"

#include "ECSql/ECSqlStatementNoopImpls.h"

#include "ECSql/ECInstanceAdapterHelper.h"
