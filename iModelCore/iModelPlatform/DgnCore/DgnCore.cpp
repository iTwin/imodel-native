/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <ECDb/ECDb.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DgnECSymbolProvider.h>
#include <DgnPlatform/Visualization.h>

BeThreadLocalStorage t_threadId;

double const fc_hugeVal = 1e37;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb::ThreadId DgnDb::GetThreadId() {return (ThreadId) t_threadId.GetValueAsInteger();}
void DgnDb::SetThreadId(ThreadId id) {t_threadId.SetValueAsInteger((int) id);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP DgnDb::GetThreadIdName()
    {
    switch (GetThreadId())
        {
        case ThreadId::Client:      return L"ClientThread";
        case ThreadId::IoPool:      return L"IoPool";
        case ThreadId::CpuPool:     return L"CpuPool";
        default:                    return L"UnknownThread";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PlatformLib::Host::ExceptionHandler& PlatformLib::Host::_SupplyExceptionHandler() {return *new ExceptionHandler();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR PlatformLib::Host::IKnownLocationsAdmin::GetLocalTempDirectoryBaseName()
    {
    return _GetLocalTempDirectoryBaseName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PlatformLib::Host::IKnownLocationsAdmin::GetLocalTempDirectory(BeFileNameR tempDir, WCharCP subDir)
    {
    tempDir = GetLocalTempDirectoryBaseName();

    if (NULL != subDir)
        {
        tempDir.AppendToPath(subDir);
        tempDir.AppendSeparator();
        }

    BeFileNameStatus status = BeFileName::CreateNewDirectory(tempDir);
    if (status != BeFileNameStatus::Success && status != BeFileNameStatus::AlreadyExists)
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR PlatformLib::Host::IKnownLocationsAdmin::GetDgnPlatformAssetsDirectory()
    {
    return _GetDgnPlatformAssetsDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PlatformLib::Host::ExceptionHandler::_ResetFloatingPointExceptions(uint32_t newFpuMask)
    {
    return BeNumerical::ResetFloatingPointExceptions(newFpuMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Host::_OnAssert(WCharCP _Message, WCharCP _File, unsigned _Line)
    {
    GetExceptionHandler()._HandleAssert(_Message, _File, _Line);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnHost::GetKeyIndex(DgnHost::Key& key)
    {
    static size_t s_highestKey = 0;
    BeAssert(key.m_key >= 0 && key.m_key<=s_highestKey); // make sure we're given a valid key

    if (0 == key.m_key)
        {
        BeSystemMutexHolder lock;

        if (0 == key.m_key)
            key.m_key = ++s_highestKey;
        }

    return key.m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::VarEntry& DgnHost::GetVarEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex(key);

    if (m_hostVar.size() < keyIndex+1)
        m_hostVar.resize(keyIndex+1, VarEntry());

    return m_hostVar[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::ObjEntry& DgnHost::GetObjEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex(key);

    if (m_hostObj.size() < keyIndex+1)
        m_hostObj.resize(keyIndex+1, ObjEntry());

    return m_hostObj[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnHost::IHostObject* DgnHost::GetHostObject(Key& key)
    {
    return GetObjEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostObject(Key& key, IHostObject* val)
    {
    GetObjEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void* DgnHost::GetHostVariable(Key& key)
    {
    return GetVarEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnHost::SetHostVariable(Key& key, void* val)
    {
    GetVarEntry(key).m_val = val;
    }

static PlatformLib::Host* s_host = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PlatformLib::Host* PlatformLib::QueryHost() {return s_host;}
PlatformLib::Host& PlatformLib::GetHost() {return *s_host;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::StaticInitialize()
    {
    BeSystemMutexHolder holdBeSystemMutexInScope;

    static bool s_staticInitialized = false;
    if (s_staticInitialized)
        return;

    bentleyAllocator_enableLowFragmentationCRTHeap();
    s_staticInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Initialize(Host& host)
    {
    StaticInitialize();

    if (nullptr != s_host)
        {
        BeAssert(false && "only call PlatformLib::Initialize once");
        return;
        }

    s_host = &host;

    DgnDb::SetThreadId(DgnDb::ThreadId::Client);

    host.Initialize();
    }

/** clean up static data held by host objects. */
void PlatformLib::Terminate(bool onProgramExit) {
    if (s_host)
        s_host->Terminate(onProgramExit);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Host::Terminate(bool onProgramExit) {

    ON_HOST_TERMINATE(m_geoCoordAdmin, onProgramExit);
    ON_HOST_TERMINATE(m_bRepGeometryAdmin, onProgramExit);

    // UnRegister Symbol Provider for ECExpressions
    IECSymbolProvider::UnRegisterExternalSymbolPublisher();

    TerminateDgnCore(onProgramExit);
    BeAssert(NULL == PlatformLib::QueryHost());
    }

#if defined (__unix__) // WIP_NONPORT
void _wassert(wchar_t const*, wchar_t const*, int) {BeAssert(false);}
#endif

/*---------------------------------------------------------------------------------**//**
* *Private* method called by Dgn::Host::Initialize.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Host::Initialize()
    {
    BeAssert(NULL == m_knownLocationsAdmin); m_knownLocationsAdmin = &_SupplyIKnownLocationsAdmin();
    BeAssert(NULL == m_exceptionHandler); m_exceptionHandler = &_SupplyExceptionHandler();
    BeAssert(NULL == m_geoCoordAdmin); m_geoCoordAdmin = &_SupplyGeoCoordinationAdmin();

    auto assetDir = m_knownLocationsAdmin->GetDgnPlatformAssetsDirectory();

    BeStringUtilities::Initialize(assetDir);
    ECDb::Initialize(m_knownLocationsAdmin->GetLocalTempDirectoryBaseName(),
                      &assetDir,
                       BeSQLiteLib::LogErrors::Yes);

    GeoCoordinates::BaseGCS::Initialize(GetGeoCoordinationAdmin()._GetDataDirectory().GetNameUtf8().c_str());

    DgnDomains::RegisterDomain(BisCoreDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    DgnDomains::RegisterDomain(GenericDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

    BeAssert(NULL == m_visualizationAdmin); m_visualizationAdmin = &_SupplyVisualizationAdmin();

    // ECSchemaReadContext::GetStandardPaths will append ECSchemas/ for us.
    ECN::ECSchemaReadContext::Initialize(assetDir);

    // Register Symbol Provider for ECExpressions
    IECSymbolProvider::RegisterExternalSymbolPublisher(&DgnECSymbolProvider::ExternalSymbolPublisher);

    BeAssert(NULL == m_bRepGeometryAdmin);     m_bRepGeometryAdmin     = &_SupplyBRepGeometryAdmin();

    FontManager::Initialize();

    auto tempDirBase = m_knownLocationsAdmin->GetLocalTempDirectoryBaseName();
    m_bRepGeometryAdmin->_Initialize(assetDir, tempDirBase);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Host::TerminateDgnCore(bool onProgramExit)
    {
    if (NULL == PlatformLib::QueryHost())
        {
        BeAssert(false);// && "Terminate called on a thread that is not associated with a host");
        return;
        }

    ON_HOST_TERMINATE(m_visualizationAdmin, onProgramExit);
    FontManager::Shutdown();

    for (ObjEntry& obj : m_hostObj)
        ON_HOST_TERMINATE(obj.m_val, onProgramExit);

    m_hostObj.clear();
    m_hostVar.clear();

    ON_HOST_TERMINATE(m_exceptionHandler, onProgramExit);
    ON_HOST_TERMINATE(m_knownLocationsAdmin, onProgramExit);

    s_host = nullptr;

    BeAssert(NULL == m_geoCoordAdmin);
    BeAssert(NULL == m_bRepGeometryAdmin);
    BeAssert(NULL == m_visualizationAdmin);
    BeAssert(NULL == m_exceptionHandler);
    BeAssert(NULL == m_knownLocationsAdmin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PlatformLib::Host::VisualizationAdmin&   PlatformLib::Host::_SupplyVisualizationAdmin()   {return *new VisualizationAdmin(std::make_unique<Dgn::Visualization>(nullptr));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void faceAttachmentFromJson(FaceAttachment& attachment, BeJsConst brep, uint32_t iSymb)
    {
    if (!brep["faceSymbology"][iSymb]["color"].isNull())
        {
        uint32_t color = brep["faceSymbology"][iSymb]["color"].asUInt();
        double transparency = !brep["faceSymbology"][iSymb]["transparency"].isNull() ? brep["faceSymbology"][iSymb]["transparency"].asDouble() : 0.0;
        attachment.SetColor(color, transparency);
        }

    if (!brep["faceSymbology"][iSymb]["materialId"].isNull())
        {
        BeInt64Id materialId(brep["faceSymbology"][iSymb]["materialId"].asUInt64());
        attachment.SetMaterial(materialId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void faceAttachmentToJson(BeJsValue faceValue, FaceAttachment const& attachment)
    {
    bool useColor = attachment.GetUseColor();
    bool useMaterial = attachment.GetUseMaterial();

    if (useColor)
        {
        faceValue["color"] = attachment.GetColor();
        if (0.0 != attachment.GetTransparency())
            faceValue["transparency"] = attachment.GetTransparency();
        }

    if (useMaterial)
        {
        faceValue["materialId"] = attachment.GetMaterial().ToHexStr();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IBRepEntityPtr PlatformLib::Host::BRepGeometryAdmin::BodyFromJson(BeJsConst brep)
    {
    if (brep["data"].isNull())
        return nullptr;

    ByteStream byteStream;
    brep["data"].GetBinary(byteStream);

    if (!byteStream.HasData())
        return nullptr;

    Transform entityTransform = Transform::FromIdentity();

    if (!brep["transform"].isNull())
        BeJsGeomUtils::TransformFromJson(entityTransform, brep["transform"]);

    IBRepEntityPtr entity;

    if (SUCCESS != T_HOST.GetBRepGeometryAdmin()._RestoreEntityFromMemory(entity, byteStream.GetData(), byteStream.GetSize(), entityTransform))
        return nullptr;

    if (brep["faceSymbology"].isArray())
        {
        uint32_t nSymb = (uint32_t) brep["faceSymbology"].size();
        T_FaceAttachmentsVec faceAttachmentsVec;

        for (uint32_t iSymb=0; iSymb < nSymb; iSymb++)
            {
            FaceAttachment attachment;

            faceAttachmentFromJson(attachment, brep, iSymb);
            faceAttachmentsVec.push_back(attachment);
            }

        entity->SetFaceMaterialAttachments(faceAttachmentsVec);
        }

    return entity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PlatformLib::Host::BRepGeometryAdmin::BodyToJson(BeJsValue value, IBRepEntityCR entity)
    {
    value.SetEmptyObject();

    size_t      bufferSize = 0;
    uint8_t*    buffer = nullptr;

    if (SUCCESS != T_HOST.GetBRepGeometryAdmin()._SaveEntityToMemory(&buffer, bufferSize, entity))
        return ;

    Utf8String  brepStr;
    Base64Utilities::Encode(brepStr, buffer, bufferSize, JsValueRef::base64Header);

    value["data"] = brepStr.c_str();
    value["type"] = (uint32_t) entity.GetEntityType(); // Can't be IBRepEntity::EntityType::Invalid if SaveEntityToMemory returned SUCCESS...

    Transform entityTransform = entity.GetEntityTransform();
    if (!entityTransform.IsIdentity())
        BeJsGeomUtils::TransformToJson(value["transform"], entityTransform);

    IFaceMaterialAttachmentsCP attachments = entity.GetFaceMaterialAttachments();
    if (nullptr != attachments)
        {
        T_FaceAttachmentsVec const& faceAttachmentsVec = attachments->_GetFaceAttachmentsVec();

        auto array = value["faceSymbology"];
        for (FaceAttachment attachment : faceAttachmentsVec)
            faceAttachmentToJson(array.appendValue(), attachment);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PlatformLib::Host::BRepGeometryAdmin& PlatformLib::Host::_SupplyBRepGeometryAdmin()
    {
    return *new BRepGeometryAdmin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
PlatformLib::Host::GeoCoordinationAdmin& PlatformLib::Host::_SupplyGeoCoordinationAdmin()
    {
    BeFileName geo = GetIKnownLocationsAdmin().GetGeoCoordinateDataDirectory();

    BeFileName path(geo);
    path.AppendToPath(L"DgnGeoCoord");
    if (!path.DoesPathExist())
        path = geo;
    return *DgnGeoCoordinationAdmin::Create(path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PlatformLib::ForwardAssertionFailures(BeAssertFunctions::T_BeAssertHandler* h)
    {
    BeAssertFunctions::SetBeAssertHandler(h);
    }
