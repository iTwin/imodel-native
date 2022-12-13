/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/BeId.h>
#include <Bentley/Nullable.h>
#include <Bentley/DateTime.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/PerformanceLogger.h>
#include <Bentley/CatchNonPortable.h>
#include <Bentley/Nullable.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECObjects/SchemaComparer.h>
#include <ECDb/ECDbApi.h>
#include <algorithm>
#include <memory>

#include "ECDbInternalTypes.h"
#include "ECDbLogger.h"
#include "IssueReporter.h"
#include "DbUtilities.h"
#include "JsonPersistenceHelper.h"

#include "ECDbImpl.h"
#include "FeatureManager.h"

#include "ProfileManager.h"
#include "ProfileUpgrader.h"
#include "SchemaManagerDispatcher.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECDbMapSchemaHelper.h"
#include "PolicyManager.h"

#include "ECDbSqlFunctions.h"

#include "DbMappingManager.h"
#include "DbMapValidator.h"
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
#include "LightweightCache.h"
#include "ClassMapColumnFactory.h"
#include "ViewGenerator.h"
#include "SchemaPersistenceHelper.h"
#include "SchemaReader.h"
#include "SchemaWriter.h"
#include "RemapManager.h"
#include "SchemaValidator.h"
#include "SchemaImportContext.h"
#include "ChangeManager.h"
#include "ChangeIteratorImpl.h"
#include "ChangeSummaryExtractor.h"
#include "ConcurrentQueryManagerImpl.h"
#include "InstanceReaderImpl.h"
#include "ECSql/NativeSqlBuilder.h"
#include "ECSql/Parser/SqlScan.h"
#include "ECSql/Parser/SqlNode.h"
#include "ECSql/Parser/IParseContext.h"
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
#include "ECSql/PragmaStatementExp.h"
#include "ECSql/OptionsExp.h"
#include "ECSql/ExpHelper.h"
#include "ECSql/ECSqlTypeInfo.h"
#include "ECSql/CommonTableExp.h"

#include "ECSql/ECSqlPrepareContext.h"
#include "ECSql/ECSqlPreparer.h"
#include "ECSql/ECSqlSelectPreparer.h"
#include "ECSql/ECSqlInsertPreparer.h"
#include "ECSql/ECSqlUpdatePreparer.h"
#include "ECSql/ECSqlDeletePreparer.h"
#include "ECSql/ECSqlPropertyNameExpPreparer.h"

#include "ECSql/ECSqlStatementImpl.h"
#include "ECSql/ECSqlPreparedStatement.h"

#include "ECSql/ECSqlFieldFactory.h"
#include "ECSql/ECSqlField.h"
#include "ECSql/PrimitiveECSqlField.h"
#include "ECSql/PointECSqlField.h"
#include "ECSql/StructECSqlField.h"
#include "ECSql/ArrayECSqlField.h"
#include "ECSql/NavigationPropertyECSqlField.h"

#include "ECSql/ECSqlBinder.h"
#include "ECSql/IdECSqlBinder.h"
#include "ECSql/NavigationPropertyECSqlBinder.h"
#include "ECSql/PointECSqlBinder.h"
#include "ECSql/PrimitiveECSqlBinder.h"
#include "ECSql/ArrayECSqlBinder.h"
#include "ECSql/StructECSqlBinder.h"

#include "ECSql/PragmaStatementExp.h"
#include "ECSql/PragmaECSqlPreparedStatement.h"

#include "ECSql/ECSqlStatementNoopImpls.h"

#include "ECSql/ECInstanceAdapterHelper.h"
