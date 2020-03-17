/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DynamicSchemaComparer.h"
#include <ECObjects/SchemaComparer.h>

//-----------------------------------------------------------------------------------------
// Logging macros
//-----------------------------------------------------------------------------------------
#define LOGGER_NAMESPACE_ORDBRIDGE    "ORDBridge"
#if defined (ANDROID)
#include <android/log.h>
#define ORDBRIDGE_LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#define ORDBRIDGE_LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#define ORDBRIDGE_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGGER_NAMESPACE_ORDBRIDGE, __VA_ARGS__);
#else
#include <Logging/BentleyLogging.h>
#define ORDBRIDGE_LOG                 (*BENTLEY_NAMESPACE_NAME::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_ORDBRIDGE))
#define ORDBRIDGE_LOGD(...)           ORDBRIDGE_LOG.debugv (__VA_ARGS__);
#define ORDBRIDGE_LOGI(...)           ORDBRIDGE_LOG.infov (__VA_ARGS__);
#define ORDBRIDGE_LOGW(...)           ORDBRIDGE_LOG.warningv (__VA_ARGS__);
#define ORDBRIDGE_LOGE(...)           ORDBRIDGE_LOG.errorv (__VA_ARGS__);
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DynamicSchemaComparer::RequireSchemaUpdate(BENTLEY_NAMESPACE_NAME::ECN::ECSchemaR dynamicSchema, 
                                                BENTLEY_NAMESPACE_NAME::ECN::ECSchemaCR existingSchema,
                                                BENTLEY_NAMESPACE_NAME::ECN::ECClassCR graphicalElement3dClass)
    {
    dynamicSchema.SetVersionMinor(existingSchema.GetVersionMinor());

    bvector<BENTLEY_NAMESPACE_NAME::ECN::ECSchemaCP> srcSchema;
    srcSchema.push_back(&existingSchema);
    bvector<BENTLEY_NAMESPACE_NAME::ECN::ECSchemaCP> destSchema;
    destSchema.push_back(&dynamicSchema);

    BENTLEY_NAMESPACE_NAME::ECN::SchemaComparer schemaComparer;

    BENTLEY_NAMESPACE_NAME::ECN::SchemaDiff diffs;
    BENTLEY_NAMESPACE_NAME::ECN::SchemaComparer::Options options(
        BENTLEY_NAMESPACE_NAME::ECN::SchemaComparer::DetailLevel::Full, 
        BENTLEY_NAMESPACE_NAME::ECN::SchemaComparer::DetailLevel::NoSchemaElements);
    if (SUCCESS != schemaComparer.Compare(diffs, srcSchema, destSchema, options))
        {
        ORDBRIDGE_LOG.infov("Schema comparer could not check differences; force to update schema.");
        return true;
        }

    auto& diffChanges = diffs.Changes();
    size_t diffCount = diffChanges.Count();
    if (0 != diffCount)
        {
        ORDBRIDGE_LOG.infov("Schema comparer detected %u difference(s):\n%s", diffCount, diffs.Changes().ToString().c_str());

        for (size_t i = 0; i < diffCount; ++i)
            {
            auto& diffChange = diffChanges[i];
            for (auto& classChangePtr : diffChange.Classes())
                {
                if (classChangePtr->GetOpCode() != ECN::ECChange::OpCode::Deleted)
                    continue;

                BENTLEY_NAMESPACE_NAME::ECN::ECEntityClassP ecClassP;
                dynamicSchema.CreateEntityClass(ecClassP, classChangePtr->GetChangeName());

                ecClassP->AddBaseClass(graphicalElement3dClass);
                ecClassP->SetClassModifier(ECN::ECClassModifier::Sealed);

                ORDBRIDGE_LOG.infov("Keeping existing dynamic class '%s' alive although it is not used anymore.", ecClassP->GetName().c_str());
                }
            }

        dynamicSchema.SetVersionMinor(dynamicSchema.GetVersionMinor() + 1);

        return true;
        }

    ORDBRIDGE_LOG.infov("New and stored schemas are identical; no update required.");
    return false;
    }