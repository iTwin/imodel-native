/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <map>
#include <string>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/PlatformLib.h>
#include <ECDb/ECDbApi.h>
#include <BeSQLite/CloudSqlite.h>
#include <Bentley/BeThread.h>
#include <Napi/napi.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include "DgnDbWorker.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

#ifndef NAPI_CPP_EXCEPTIONS
#   error napi exceptions are not defined
#endif

// Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
// from running at program shutdown time, which would attempt to reset the reference when the environment is no longer valid.
#define SET_CONSTRUCTOR(t) Constructor() = Napi::Persistent(t); Constructor().SuppressDestruct();
#define DEFINE_CONSTRUCTOR static Napi::FunctionReference& Constructor() { static Napi::FunctionReference s_ctor; return s_ctor; }

#define THROW_JS_EXCEPTION(str) BeNapi::ThrowJsException(info.Env(), str);
#define THROW_JS_TYPE_EXCEPTION(str) BeNapi::ThrowJsTypeException(info.Env(), str);

#define ARGUMENT_IS_PRESENT(i) (info.Length() > (i))
#define ARGUMENT_IS_NOT_PRESENT(i) !ARGUMENT_IS_PRESENT(i)

#define ARGUMENT_IS_EMPTY(i) (ARGUMENT_IS_NOT_PRESENT(i) || info[i].IsUndefined() || info[i].IsNull())
#define ARGUMENT_IS_STRING(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsString())
#define ARGUMENT_IS_NUMBER(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsNumber())
#define ARGUMENT_IS_BOOL(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsBoolean())

#define ARGUMENT_IS_NOT_STRING(i) !ARGUMENT_IS_STRING(i)
#define ARGUMENT_IS_NOT_BOOL(i) !ARGUMENT_IS_BOOL(i)
#define ARGUMENT_IS_NOT_NUMBER(i) !ARGUMENT_IS_NUMBER(i)

#define REJECT_DEFERRED_AND_RETURN(deferred, message) {\
    deferred.Reject(Napi::Error::New(info.Env(), message).Value());\
    return deferred.Promise();\
    }

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var)\
    if (ARGUMENT_IS_NOT_PRESENT(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an object")\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_ANY_OBJ_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_PRESENT(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an object")\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !T::InstanceOf(info[i])) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an object of type " #T)\
    }\
    T* var = T::Unwrap(info[i].As<Napi::Object>());

#define REQUIRE_ARGUMENT_NATIVE_DGNDB(i, var) \
    DgnDb* var; \
    if (ARGUMENT_IS_NOT_PRESENT(i) || !(var = extractDgnDbFromNapiValue(info[i]))) \
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an object of type NativeDgnDb")\

#define REQUIRE_ARGUMENT_FUNCTION(i, var)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsFunction()) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a function")\
    }\
    Napi::Function var = info[i].As<Napi::Function>();

#define REQUIRE_ARGUMENT_STRING(i, var)\
    if (ARGUMENT_IS_NOT_STRING(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a string")\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value();

#define REQUIRE_ARGUMENT_STRING_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_STRING(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a string")\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value();

#define REQUIRE_ARGUMENT_STRING_ID(i, strId, T, id)\
    REQUIRE_ARGUMENT_STRING(i, strId)\
    T id(BeInt64Id::FromString(strId.c_str()).GetValue());

#define REQUIRE_ARGUMENT_STRING_ARRAY(i, var)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsArray()) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an array of strings")\
    }\
    bvector<Utf8String> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (arrValue.IsString())\
            var.push_back(arrValue.As<Napi::String>().Utf8Value());\
    }\

#define REQUIRE_ARGUMENT_STRING_ARRAY_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsArray()) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an array")\
    }\
    bvector<Utf8String> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (!arrValue.IsString()) {\
            Utf8PrintfString msg("Argument " #i " must be an array of strings. Item at index %d is not a string", arrIndex);\
            REJECT_DEFERRED_AND_RETURN(deferred, msg.c_str())\
        }\
        var.push_back(arrValue.As<Napi::String>().Utf8Value());\
    }

#define REQUIRE_ARGUMENT_NUMBER(i, var)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a number")\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_NUMBER_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a number")\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_INTEGER(i, var)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a number")\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_INTEGER_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a number")\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_UINTEGER(i, var)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an integer")\
    }\
    uint32_t var = info[i].As<Napi::Number>().Uint32Value();

#define REQUIRE_ARGUMENT_INTEGER_ARRAY(i, var)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsArray()) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an array of integer")\
    }\
    bvector<int32_t> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (arrValue.IsNumber())\
            var.push_back(arrValue.As<Napi::Number>().Int32Value());\
    }

#define REQUIRE_ARGUMENT_INTEGER_ARRAY_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsArray()) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an array")\
    }\
    bvector<int32_t> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (!arrValue.IsNumber()) {\
            Utf8PrintfString msg("Argument " #i " must be an array of numbers. Item at index %d is not a number", arrIndex);\
            REJECT_DEFERRED_AND_RETURN(deferred, msg.c_str())\
        }\
        var.push_back(arrValue.As<Napi::Number>().Int32Value());\
    }

#define REQUIRE_ARGUMENT_BOOL(i, var)\
    if (ARGUMENT_IS_NOT_BOOL(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a boolean")\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();

#define REQUIRE_ARGUMENT_BOOL_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_BOOL(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a boolean")\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();


#define OPTIONAL_ARGUMENT_BOOL(i, var, default)\
    bool var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_BOOL(i)) {\
        var = info[i].As<Napi::Boolean>().Value();\
    } else {\
        var = (default);\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an boolean")\
    }

#define OPTIONAL_ARGUMENT_BOOL_ASYNC(i, var, default, deferred)\
    bool var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_BOOL(i)) {\
        var = info[i].As<Napi::Boolean>().Value();\
    } else {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a boolean")\
    }

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default)\
    int var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_NUMBER(i)) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    } else {\
        var = (default);\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an integer")\
    }

#define OPTIONAL_ARGUMENT_UINTEGER(i, var, default)\
    int var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_NUMBER(i)) {\
        var = info[i].As<Napi::Number>().Uint32Value();\
    } else {\
        var = (default);\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be an unsigned integer")\
    }

#define OPTIONAL_ARGUMENT_INTEGER_ASYNC(i, var, default, deferred)\
    int var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_NUMBER(i)) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    } else {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an integer")\
    }

#define OPTIONAL_ARGUMENT_STRING(i, var)\
    Utf8String var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        ;\
    } else if (ARGUMENT_IS_STRING(i)) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    } else {\
        Utf8PrintfString msg("Argument " #i " is type %d. It must be a string or undefined", info[i].Type());\
        THROW_JS_TYPE_EXCEPTION(msg.c_str())\
    }

#define OPTIONAL_ARGUMENT_STRING_ASYNC(i, var, deferred)\
    Utf8String var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        ;\
    } else if (ARGUMENT_IS_STRING(i)) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    } else {\
        Utf8PrintfString msg("Argument " #i " is type %d. It must be a string or undefined", info[i].Type());\
        REJECT_DEFERRED_AND_RETURN(deferred, msg.c_str())\
    }

namespace IModelJsNative {

struct InlineGeometryPartsResult {
    uint32_t m_numCandidateParts = 0;
    uint32_t m_numRefsInlined = 0;
    uint32_t m_numPartsDeleted = 0;
};

inline Napi::String toJsString(Napi::Env env, Utf8CP val, size_t len) {
    if (nullptr == val) {
        val = "";
        len = 0;
    }
    return Napi::String::New(env, val, len);
}
inline Napi::String toJsString(Napi::Env env, Utf8CP val) { return toJsString(env, val, NAPI_AUTO_LENGTH); }
inline Napi::String toJsString(Napi::Env env, Utf8StringCR str) { return toJsString(env, str.c_str(), str.length()); }
inline Napi::String toJsString(Napi::Env env, BeInt64Id id) { return toJsString(env, id.ToHexStr()); }

typedef Napi::CallbackInfo const&  NapiInfoCR;

void registerCloudSqlite(Napi::Env env, Napi::Object exports);
Napi::Value getJsCloudContainer(Napi::Value arg);
CloudContainer* getCloudContainer(Napi::Value arg);

inline static Utf8String stringMember(Napi::Object const& obj, Utf8CP name, Utf8CP defaultVal = "") {
    auto member = obj.Get(name);
    return member.IsString() ? member.ToString().Utf8Value() : defaultVal;
}
inline static int intMember(Napi::Object const& obj, Utf8CP name, int defaultVal) {
    auto member = obj.Get(name);
    return member.IsNumber() ? member.ToNumber().Int32Value() : defaultVal;
}
inline static bool boolMember(Napi::Object const& obj, Utf8CP name, bool defaultVal) {
    auto member = obj.Get(name);
    return member.IsBoolean() ? member.ToBoolean().Value() : defaultVal;
}

inline static Utf8String requireString(Napi::Object const& obj, Utf8CP name) {
    auto strVal = stringMember(obj, name);
    if (strVal.empty())
        BeNapi::ThrowJsException(obj.Env(), Utf8PrintfString("must supply %s", name).c_str());
    return strVal;
}
inline static bool requireBool(Napi::Object const& obj, Utf8CP name) {
    auto member = obj.Get(name);
    if (!member.IsBoolean())
        BeNapi::ThrowJsException(obj.Env(), Utf8PrintfString("must supply %s", name).c_str());
    return  member.ToBoolean().Value();
}
inline static Napi::Array requireArray(Napi::Object const& obj, Utf8CP name) {
    auto member = obj.Get(name);
    if (!member.IsArray())
        BeNapi::ThrowJsException(obj.Env(), Utf8PrintfString("must supply array %s", name).c_str());
    return member.As<Napi::Array>();
}

enum class ProcessPolyfaceResult {
  Empty, // polyface had no triangles
  Bad, // polyface contains bad data that could not be fixed up
  Ok, // polyface was processed
};

struct JsInterop {
    [[noreturn]] static void throwSqlResult(Utf8CP msg, Utf8CP fileName, DbResult result) {
        BeNapi::ThrowJsException(Env(), Utf8PrintfString("%s [%s]: %s", msg, fileName, BeSQLiteLib::GetErrorString(result)).c_str(), result);
    }
    [[noreturn]] static void throwDgnDbStatus(DgnDbStatus);
    [[noreturn]] static void throwWrongClass() { throwDgnDbStatus(DgnDbStatus::WrongClass); }
    [[noreturn]] static void throwBadArg()     { throwDgnDbStatus(DgnDbStatus::BadArg); }
    [[noreturn]] static void throwInvalidId()  { throwDgnDbStatus(DgnDbStatus::InvalidId); }
    [[noreturn]] static void throwMissingId()  { throwDgnDbStatus(DgnDbStatus::MissingId); }
    [[noreturn]] static void throwBadSchema()  { throwDgnDbStatus(DgnDbStatus::BadSchema); }
    [[noreturn]] static void throwBadRequest() { throwDgnDbStatus(DgnDbStatus::BadRequest); }
    [[noreturn]] static void throwWriteError() { throwDgnDbStatus(DgnDbStatus::WriteError); }
    [[noreturn]] static void throwNotFound()   { throwDgnDbStatus(DgnDbStatus::NotFound); }
    [[noreturn]] static void throwSqlError()   { throwDgnDbStatus(DgnDbStatus::SQLiteError); }

    static bmap<Dgn::DgnDb*, BeFileName> s_openDgnDbFileNames;

    struct CrashReportingConfig
        {
        BeFileName m_crashDir;
        BeFileName m_dumpProcessorScriptFileName;
        std::map<std::string,std::string> m_params;
        size_t m_maxDumpsInDir;
        bool m_enableCrashDumps;
        bool m_wantFullMemory;
        bool m_needsVectorExceptionHandler;
        };

    BE_JSON_NAME(accessName)
    BE_JSON_NAME(accessToken)
    BE_JSON_NAME(alias)
    BE_JSON_NAME(blob)
    BE_JSON_NAME(blockSize)
    BE_JSON_NAME(briefcaseId)
    BE_JSON_NAME(cacheSize)
    BE_JSON_NAME(changesType)
    BE_JSON_NAME(checksumBlockNames)
    BE_JSON_NAME(client)
    BE_JSON_NAME(cloudContainer)
    BE_JSON_NAME(codeScope)
    BE_JSON_NAME(codeSpecId)
    BE_JSON_NAME(columnName)
    BE_JSON_NAME(compress)
    BE_JSON_NAME(containerId)
    BE_JSON_NAME(curlDiagnostics)
    BE_JSON_NAME(data)
    BE_JSON_NAME(date)
    BE_JSON_NAME(dbName)
    BE_JSON_NAME(defaultTxn)
    BE_JSON_NAME(description)
    BE_JSON_NAME(durationSeconds)
    BE_JSON_NAME(ecefLocation)
    BE_JSON_NAME(element)
    BE_JSON_NAME(errorNumber)
    BE_JSON_NAME(expires)
    BE_JSON_NAME(face)
    BE_JSON_NAME(fileExt)
    BE_JSON_NAME(fileName)
    BE_JSON_NAME(fonts)
    BE_JSON_NAME(forceUseId)
    BE_JSON_NAME(globalOrigin)
    BE_JSON_NAME(guid)
    BE_JSON_NAME(httpTimeout)
    BE_JSON_NAME(id)
    BE_JSON_NAME(index)
    BE_JSON_NAME(localFileName)
    BE_JSON_NAME(lockedBy)
    BE_JSON_NAME(logMask)
    BE_JSON_NAME(modelExtents)
    BE_JSON_NAME(minRequests)
    BE_JSON_NAME(name)
    BE_JSON_NAME(namespace)
    BE_JSON_NAME(nRequests)
    BE_JSON_NAME(numBytes)
    BE_JSON_NAME(offset)
    BE_JSON_NAME(openMode)
    BE_JSON_NAME(orientation)
    BE_JSON_NAME(origin)
    BE_JSON_NAME(pageSize)
    BE_JSON_NAME(parentId)
    BE_JSON_NAME(pathname)
    BE_JSON_NAME(projectExtents)
    BE_JSON_NAME(promise)
    BE_JSON_NAME(queryParam)
    BE_JSON_NAME(rootDir)
    BE_JSON_NAME(rootSubject)
    BE_JSON_NAME(row)
    BE_JSON_NAME(secure)
    BE_JSON_NAME(size)
    BE_JSON_NAME(skipFileCheck)
    BE_JSON_NAME(state)
    BE_JSON_NAME(storageType)
    BE_JSON_NAME(subId)
    BE_JSON_NAME(systemFont)
    BE_JSON_NAME(tableName)
    BE_JSON_NAME(tempFileBase)
    BE_JSON_NAME(timeout)
    BE_JSON_NAME(type)
    BE_JSON_NAME(uriParams)
    BE_JSON_NAME(user)
    BE_JSON_NAME(value)
    BE_JSON_NAME(writeable)
    BE_JSON_NAME(yesNo)

#define JSON_NAME(__val__) JsInterop::json_##__val__()

private:
    static void GetRowAsJson(BeJsValue json, BeSQLite::EC::ECSqlStatement &);
    static void RegisterOptionalDomains();
    static void InitializeSolidKernel();
    static void AddFallbackSchemaLocaters(ECDbR db, ECSchemaReadContextPtr schemaContext);
public:
    static void HandleAssertion(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type);
    static void GetECValuesCollectionAsJson(BeJsValue json, ECN::ECValuesCollectionCR);
    static ECN::ECClassCP GetClassFromInstance(BeSQLite::EC::ECDbCR ecdb, BeJsConst jsonInstance);
    static BeSQLite::EC::ECInstanceId GetInstanceIdFromInstance(BeSQLite::EC::ECDbCR ecdb, BeJsConst jsonInstance);
    static void InitLogging();
    static void Initialize(BeFileNameCR, Napi::Env, BeFileNameCR);
    static DgnDbPtr CreateIModel(Utf8StringCR filename, BeJsConst props);
    static DgnDbStatus GetECClassMetaData(BeJsValue results, DgnDbR db, Utf8CP schema, Utf8CP ecclass);
    static DgnDbStatus GetSchemaItem(BeJsValue results, DgnDbR db, Utf8CP schemaName, Utf8CP itemName);
    static DgnDbStatus GetElement(BeJsValue results, DgnDbR db, Napi::Object);
    static Napi::String InsertElement(DgnDbR db, Napi::Object props, Napi::Value options);
    static void UpdateElement(DgnDbR db, Napi::Object);
    static void DeleteElement(DgnDbR db, Utf8StringCR eidStr);
    static DgnDbStatus SimplifyElementGeometry(DgnDbR db, Napi::Object simplifyArgs);
    static InlineGeometryPartsResult InlineGeometryParts(DgnDbR db);
    static Napi::String InsertElementAspect(DgnDbR db, Napi::Object aspectProps);
    static void UpdateElementAspect(DgnDbR db, Napi::Object aspectProps);
    static void DeleteElementAspect(DgnDbR db, Utf8StringCR aspectIdStr);

    // Used by ExportGraphics, ExportPartGraphics, and GenerateElementMeshes.
    // Checks for common "bad" polyfaces, fixing them up if possible.
    // If bad or empty, returns status without processing.
    // Otherwise, fixes up the input polyface if applicable, passes it to supplied function, and returns Ok.
    static ProcessPolyfaceResult ProcessPolyface(PolyfaceQueryCR, bool wantParamsAndNormals, std::function<void(PolyfaceQueryCR)>);
    static DgnDbStatus ExportGraphics(DgnDbR db, Napi::Object const& exportProps);
    static DgnDbStatus ExportPartGraphics(DgnDbR db, Napi::Object const& exportProps);
    static Napi::Value GenerateElementMeshes(DgnDbR, Napi::Object const&);

    static DgnDbStatus ProcessGeometryStream(DgnDbR db, Napi::Object const& requestProps);
    static DgnDbStatus CreateBRepGeometry(DgnDbR db, Napi::Object const& createProps);
    static Napi::String InsertLinkTableRelationship(DgnDbR db, Napi::Object props);
    static void UpdateLinkTableRelationship(DgnDbR db, Napi::Object props);
    static void DeleteLinkTableRelationship(DgnDbR db, Napi::Object props);
    static Napi::String InsertCodeSpec(DgnDbR db, Utf8StringCR name, BeJsConst jsonProperties);
    static void UpdateCodeSpec(DgnDbR db, CodeSpecId codeSpecId, BeJsConst jsonProperties);
    static Napi::String InsertModel(DgnDbR db, Napi::Object);
    static void UpdateModel(DgnDbR db, Napi::Object props);
    static DgnDbStatus UpdateModelGeometryGuid(DgnDbR db, DgnModelId modelId);
    static void DeleteModel(DgnDbR db, Utf8StringCR idStr);
    static DgnDbStatus GetModel(Napi::Object results, DgnDbR db, BeJsConst inOpts);
    static void QueryModelExtents(BeJsValue extents, DgnDbR db, BeJsConst options);
    static DgnDbStatus QueryDefinitionElementUsage(BeJsValue usageInfo, DgnDbR db, bvector<Utf8String> const& idStringArray);
    static void UpdateProjectExtents(DgnDbR dgndb, BeJsConst newExtents);
    static void UpdateIModelProps(DgnDbR dgndb, BeJsConst);

    static DbResult CreateECDb(ECDbR, BeFileNameCR pathname);
    static DbResult OpenECDb(ECDbR, BeFileNameCR pathname, BeSQLite::Db::OpenParams const&);
    static DbResult ImportSchema(ECDbR ecdb, BeFileNameCR pathname);
    static DbResult ImportSchemasDgnDb(DgnDbR dgndb, bvector<Utf8String> const &schemaFileNames);
    static DbResult ImportXmlSchemas(DgnDbR dgndb, bvector<Utf8String> const &serializedXmlSchemas);
    static DbResult ImportFunctionalSchema(DgnDbR);
    static DgnRevisionPtr GetRevision(Utf8StringCR dbGuid, BeJsConst arg);
    static bvector<DgnRevisionPtr> GetRevisions(bool& containsSchemaChanges, Utf8StringCR dbGuid, BeJsConst changeSets);
    static RevisionStatus ApplySchemaChangeSet(BeFileNameCR dbFileName, bvector<DgnRevisionCP> const& revisions, RevisionProcessOption applyOption);
    static RevisionStatus DumpChangeSet(DgnDbR dgndb, BeJsConst changeSetToken);
    static DgnDbStatus ExtractChangedInstanceIdsFromChangeSets(BeJsValue, DgnDbR, const bvector<BeFileName>&);

    static BentleyStatus GetGeoCoordsFromIModelCoords(BeJsValue, DgnDbR, BeJsConst);
    static BentleyStatus GetIModelCoordsFromGeoCoords(BeJsValue, DgnDbR, BeJsConst);

    static void GetIModelProps(BeJsValue, DgnDbCR dgndb);
    static DgnElementIdSet FindGeometryPartReferences(bvector<Utf8String> const& partIds, bool is2d, DgnDbR db);

    static void ConcurrentQueryExecute(ECDbCR ecdb, Napi::Object request, Napi::Function callback);
    static Napi::Object  ConcurrentQueryResetConfig(Napi::Env, ECDbCR, Napi::Object);
    static Napi::Object  ConcurrentQueryResetConfig(Napi::Env, ECDbCR);
    static void GetTileTree(ICancellableP, DgnDbR db, Utf8StringCR id, Napi::Function& callback);
    static void GetTileContent(ICancellableP, DgnDbR db, Utf8StringCR treeId, Utf8StringCR tileId, Napi::Function& callback);
    static void SetMaxTileCacheSize(uint64_t maxBytes);

    [[noreturn]] static void ThrowJsException(Utf8CP msg);
    static Json::Value ExecuteTest(DgnDbR, Utf8StringCR testName, Utf8StringCR params);
    static NativeLogging::CategoryLogger GetNativeLogger();

    static Napi::Env& Env() { static Napi::Env s_env(nullptr); return s_env; }
    static intptr_t& MainThreadId() {static intptr_t s_mainThreadId; return s_mainThreadId;}
    static bool IsMainThread() { return BeThreadUtilities::GetCurrentThreadId() == MainThreadId(); }
    static bool IsJsExecutionDisabled();

    static void StepAsync(Napi::Function& callback, Statement& stmt);
    static void StepAsync(Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert);

    static void FormatCurrentTime(char* buf, size_t maxbuf);

    static void AddCrashReportDgnDb(Dgn::DgnDbR);
    static void RemoveCrashReportDgnDb(Dgn::DgnDbR);

    static void SetCrashReportProperty(Utf8CP key, Utf8CP value)
    #if defined (USING_GOOGLE_BREAKPAD) || defined (BENTLEYCONFIG_CRASHPAD)
        ;
    #else
        {}
    #endif

    static std::map<std::string, std::string> GetCrashReportProperties()
    #if defined (USING_GOOGLE_BREAKPAD) || defined (BENTLEYCONFIG_CRASHPAD)
        ;
    #else
        {
        std::map<std::string, std::string> noprops;
        return noprops;
        }
    #endif

    static void MaintainCrashDumpDir(int& maxNativeCrashTxtFileNo, CrashReportingConfig const&);
    static std::map<std::string,std::string> GetCrashReportPropertiesFromConfig(CrashReportingConfig const&);

    static void InitializeCrashReporting(CrashReportingConfig const&)
#if defined (USING_GOOGLE_BREAKPAD) || defined (BENTLEYCONFIG_CRASHPAD)
        ;
#else
        {}
#endif

    static void WriteFullElementDependencyGraphToFile(DgnDbR db, Utf8StringCR dotFileName);
    static void WriteAffectedElementDependencyGraphToFile(DgnDbR db, Utf8StringCR dotFileName, bvector<Utf8String> const& id64Array);
};
//=======================================================================================
// @bsiclass
//=======================================================================================
struct GeoServicesInterop
{
    static BentleyStatus GetGeographicCRSInterpretation(BeJsValue, BeJsConst);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NativeChangeset {
    private:
        std::unique_ptr<RevisionChangesFileReaderBase> m_reader;
        Db m_unusedDb;
        BeFileName m_fileName;
        std::unique_ptr<Changes> m_changes;
        Changes::Change m_currentChange;
        Utf8CP m_tableName;
        int m_columnCount;
        int m_indirect;
        int m_primaryKeyColumnCount;
        DbOpcode m_opcode;
        Byte* m_primaryKeyColumns;
        bool m_invert;
    private:
        Napi::Value SerializeValue(Napi::Env, DbValue&val);
        bool HasRow() { return IsOpen() && m_currentChange.IsValid(); }
        bool IsOpen() { return m_reader != nullptr; }
        bool IsValidColumnIndex(int col) { return HasRow() && (col>=0 && col< m_columnCount); }
        bool IsValidPrimaryKeyColumnIndex(int col) { return HasRow() && (col>=0 && col< m_primaryKeyColumnCount); }

    public:
        NativeChangeset():m_primaryKeyColumns(nullptr), m_tableName(nullptr), m_currentChange(nullptr, false), m_invert(false){}
        Napi::Value Open(Napi::Env env, Utf8CP changesetFile, bool invert);
        Napi::Value Close(Napi::Env env);
        Napi::Value Reset(Napi::Env env);
        Napi::Value Step(Napi::Env env);
        Napi::Value GetFileName(Napi::Env env);
        Napi::Value GetTableName(Napi::Env env);
        Napi::Value GetOpCode(Napi::Env env);
        Napi::Value IsIndirectChange(Napi::Env env);
        Napi::Value IsPrimaryKeyColumn(Napi::Env env, int col);
        Napi::Value GetColumnCount(Napi::Env env);
        Napi::Value GetColumnValueType(Napi::Env env, int col, int target);
        Napi::Value GetColumnValue(Napi::Env env, int col, int target);
        Napi::Value GetRow(Napi::Env env);
        Napi::Value GetDdlChanges(Napi::Env);
};

//=======================================================================================
//! TEXT HexStr(number INT)
// @bsiclass
//=======================================================================================
struct HexStrSqlFunction final : ScalarFunction
    {
    private:
        HexStrSqlFunction() : ScalarFunction("HexStr", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~HexStrSqlFunction() {}

        static HexStrSqlFunction& GetSingleton();
    };

//=======================================================================================
//! TEXT Str(number INT)
// @bsiclass
//=======================================================================================
struct StrSqlFunction final : ScalarFunction
    {
    private:
        StrSqlFunction() : ScalarFunction("Str", 1, DbValueType::TextVal) {}
        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override;

    public:
        ~StrSqlFunction() {}

        static StrSqlFunction& GetSingleton();
    };

template <typename STATUSTYPE>
Napi::Object CreateErrorObject0(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
    {
    auto error = Napi::Object::New(env);
    error["status"] = (int)errCode;
    if (nullptr != msg)
        error["message"] = msg;

    return error;
    }

template <typename STATUSTYPE>
Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
    {
    auto retObj = Napi::Object::New(env);
    retObj["error"] = CreateErrorObject0(errCode, msg, env);
    return retObj;
    }

Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal);

// DON'T change this flag directly. It is managed by BeObjectWrap only.
extern bool s_BeObjectWrap_inDestructor;

// Every ObjectWrap subclass must derive from BeObjectWrap!
template<typename OBJ>
struct BeObjectWrap : Napi::ObjectWrap<OBJ>
{
protected:
    BeObjectWrap(NapiInfoCR info) : Napi::ObjectWrap<OBJ>(info) {}

    // Every derived class must call this function on the first line of its destructor
    static void SetInDestructor()
        {
        BeAssert(!s_BeObjectWrap_inDestructor && "Call SetInDestructor once, as the first line of the subclass destructor.");
        s_BeObjectWrap_inDestructor = true;
        }

    ~BeObjectWrap()
        {
        BeAssert(s_BeObjectWrap_inDestructor && "Subclass should have called SetInDestructor at the start of its destructor");
        // We can re-enable JS callbacks, now that we know that the destructors for the subclass and its member variables have finished.
        s_BeObjectWrap_inDestructor = false;
        }
};

DgnDb* extractDgnDbFromNapiValue(Napi::Value);

} // namespace IModelJsNative
