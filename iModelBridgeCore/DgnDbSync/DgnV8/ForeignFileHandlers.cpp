/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <windows.h>
#include <Objidl.h>
#include <VersionedDgnV8Api/DgnPlatform/DgnFileIO/DgnV8FileIO.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

typedef DgnV8Api::DgnFileType*  (*GetFileTypeFuncPtr) (DgnV8Api::DgnFileFormatType);

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          05/17
+===============+===============+===============+===============+===============+======*/
struct  V8ForeignFileType : public DgnV8Api::DgnFileType
{
private:
    Utf8String              m_fileIOName;
    Utf8String              m_v8SdkDirectory;
    Utf8String              m_realdwgDirectory;
    DgnV8Api::DgnFileType*  m_fileIOType;
    bset<Utf8String>        m_fileExtensions;
    static Converter*       s_v8Converter;
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    Initialize ()
    {
    if (m_format == DgnV8Api::DgnFileFormatType::DWG || m_format == DgnV8Api::DgnFileFormatType::DXF)
        {
        Converter::InitializeDwgHost (BeFileName(m_v8SdkDirectory), BeFileName(m_realdwgDirectory));
        if (nullptr != s_v8Converter)
            Converter::InitializeDwgSettings (s_v8Converter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TryLoadFileIO ()
    {
    if (nullptr == m_fileIOType)
        {
        Initialize ();

        Utf8String  fullPath = m_v8SdkDirectory + "\\" + m_fileIOName;

        HINSTANCE h = ::LoadLibrary (fullPath.c_str());
        if (nullptr == h)
            {
            if (nullptr != s_v8Converter)
                s_v8Converter->ReportIssue (Converter::IssueSeverity::Error, Converter::IssueCategory::VisualFidelity(), Converter::Issue::FailedLoadingFileIO(), fullPath.c_str());
            else
                BeAssert (false && "Failed loading a file handler!");
            return  BentleyStatus::BSIERROR;
            }

        GetFileTypeFuncPtr  getTypeFunc = (GetFileTypeFuncPtr) ::GetProcAddress (h, "v8FileIOImplementer_getType");
        if (nullptr == getTypeFunc)
            {
            if (nullptr != s_v8Converter)
                s_v8Converter->ReportIssue (Converter::IssueSeverity::Error, Converter::IssueCategory::VisualFidelity(), Converter::Issue::MissingFileIOImplementer(), m_fileIOName.c_str());
            else
                BeAssert (false && "File handler missing a required function v8FileIOImplementer_getType!");
            return  BentleyStatus::BSIERROR;
            }
            
        m_fileIOType = getTypeFunc (m_format);
        }

    return nullptr == m_fileIOType ? BentleyStatus::BSIERROR : BentleyStatus::BSISUCCESS;
    }

public:

V8ForeignFileType (DgnV8Api::DgnFileFormatType type, Utf8CP ext, Utf8CP dllName, BentleyApi::BeFileNameCR v8Dir, BentleyApi::BeFileNameCP dwgDir = nullptr) : 
    DgnV8Api::DgnFileType(type), m_fileIOName(dllName), m_v8SdkDirectory(v8Dir.c_str()), m_fileIOType(nullptr)
    {
    // add the default extension name
    m_fileExtensions.insert (Utf8String(ext));
    // set RealDWG installation
    if (nullptr != dwgDir)
        m_realdwgDirectory.Assign (dwgDir->c_str());
    else
        m_realdwgDirectory.clear ();
    }

~V8ForeignFileType () { DELETE_AND_CLEAR(m_fileIOType); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileIO* Factory (DgnV8FileP v8file) override
    {
    return BentleyStatus::BSISUCCESS == TryLoadFileIO() ? m_fileIOType->Factory(v8file) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidateFile (DgnV8Api::DgnFileFormatType* type, int *majorV, int *minorV, bool *is3D, Bentley::IThumbnailPropertyValuePtr* thumbnail, WCharCP name) override
    {
    if ((nullptr == name || 0 == name[0]) || BentleyStatus::BSISUCCESS != TryLoadFileIO())
        return  false;

    return m_fileIOType->ValidateFile (type, majorV, minorV, is3D, thumbnail, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GetCapabilities (DgnV8Api::DgnFileCapabilities* c) override
    {
    if (nullptr != m_fileIOType)
        return  m_fileIOType->GetCapabilities (c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileIOP GetFileIO (DgnFileP pFile, WCharCP extension) override
    {
    auto found = m_fileExtensions.find (Utf8String(extension).ToLower());

    // allow a FileIO to handle wildcard extension "*":
    if (found == m_fileExtensions.end())
        found = m_fileExtensions.find (Utf8String("*"));

    if (found != m_fileExtensions.end())
        return this->Factory (pFile);

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddSupportedExtension (Utf8CP extensionName)
    {
    m_fileExtensions.insert (extensionName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetV8Converter (Converter* v8converter)
    {
    s_v8Converter = v8converter;
    }

};  // V8ForeignFileType


Converter*   V8ForeignFileType::s_v8Converter = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::RegisterForeignFileTypes (BentleyApi::BeFileNameCR v8dir, BentleyApi::BeFileNameCR realdwgDir)
    {
    /*-----------------------------------------------------------------------------------
    Register the FileIO implementation DLL's for all foreign file formats that need to be 
    demand loaded during the V8->BIM conversion.  Note that an implementation DLL is different
    than its file hanlder counterpart in V8, which is an MDL module.  The implementation 
    module we use here only depends on DgnPlatform, and does not depend on PowerPlatform.
    In essense, the implementation module is only a subset of the MDL file handler.
    -----------------------------------------------------------------------------------*/
#ifndef RealDwgVersion
    #error Must define RealDwgVersion!!
#endif
    static Utf8PrintfString s_dwgioName("DwgDgnIO%d.dll", RealDwgVersion);
    V8ForeignFileType*  filetypeInstance = new V8ForeignFileType(DgnV8Api::DgnFileFormatType::DWG, "dwg", s_dwgioName.c_str(), v8dir, &realdwgDir);
    if (nullptr == filetypeInstance)
        return;

    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::DXF, "dxf", s_dwgioName.c_str(), v8dir, &realdwgDir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::ThreeDS, "3ds", "3dsfileioImp.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::OBJ, "obj", "objfileioImp.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::SKP, "skp", "skpfileioImp.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::OpenNurbs, "3dm", "rhinolib.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::FBX, "fbx", "fbxfileioImp.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::IFC, "ifc", "IfcFileIO.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::Acute3D, "3mx", "MrMeshFileIO.dll", v8dir));
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::Acute3D, "3sm", "MrMeshFileIO.dll", v8dir));

    // Add the wildcard DWG as the last entry to give other FileIO's a chance, before unnecessarily loading lots of RealDWG DLL's.
    DgnV8Api::DgnFileTypeRegistry::AddFileType (new V8ForeignFileType(DgnV8Api::DgnFileFormatType::DWG, "*", s_dwgioName.c_str(), v8dir, &realdwgDir));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::InitV8ForeignFileTypes (Converter* v8converter)
    {
    V8ForeignFileType::SetV8Converter (v8converter);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

