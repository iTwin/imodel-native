//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteSisterFile.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMSQLiteSisterFile.h"

BeFileName GetTempPathFromProjectPath(const BeFileName& path)
{
	BeFileName extraFileDir;

#ifndef VANCOUVER_API
	Desktop::FileSystem::BeGetTempPath(extraFileDir);
#else
	BeFileName::BeGetTempPath(extraFileDir);
#endif

	WString substrFile = path.c_str();
	substrFile.ReplaceAll(L"/", L"_");
	substrFile.ReplaceAll(L"\\", L"_");
	substrFile.ReplaceAll(L":", L"_");
	substrFile.ReplaceAll(L"\"", L"_");
	substrFile.ReplaceAll(L"'", L"_");

	extraFileDir.AppendToPath(substrFile.c_str());
	return extraFileDir;
}

bool SMSQLiteSisterFile::GetSisterSQLiteFileName(WString & sqlFileName, SMStoreDataType dataType, bool useTempPath) const
    {
    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
        case SMStoreDataType::Graph:
            {
            if (m_smSQLiteFile.IsValid())
                {                 
                Utf8String dbFileName;
                bool result = m_smSQLiteFile->GetFileName(dbFileName);
                assert(result == true);
                sqlFileName.AssignUtf8(dbFileName.c_str());            
                }
            else //For PWContextShare there is not local path, so used temp path.
                {
                sqlFileName = useTempPath ? GetTempPathFromProjectPath(m_projectFilesPath) : m_projectFilesPath;                
                }

            sqlFileName.append(L"_feature"); //temporary file, deleted after generation
            }
            return true;
            break;

        case SMStoreDataType::DiffSet:
            sqlFileName = useTempPath ? GetTempPathFromProjectPath(m_projectFilesPath) : m_projectFilesPath;
            sqlFileName.append(L"_clips");
            return true;
            break;
        case SMStoreDataType::ClipDefinition:
        case SMStoreDataType::Skirt:
        case SMStoreDataType::CoveragePolygon:
        case SMStoreDataType::CoverageName:
            sqlFileName = useTempPath ? GetTempPathFromProjectPath(m_projectFilesPath) : m_projectFilesPath;
            sqlFileName.append(L"_clipDefinitions");
            return true;
            break;

        default:
            assert(!"Unknown data type");
            break;
        }

    return false;
    }

SMSQLiteSisterFile::SMSQLiteSisterFile(SMSQLiteFilePtr sqliteFile)
    : m_smSQLiteFile(sqliteFile),
      m_smFeatureSQLiteFile(nullptr),
      m_smClipSQLiteFile(nullptr),
      m_smClipDefinitionSQLiteFile(nullptr), 
      m_useTempPath(true), 
      m_removeTempGenFile(true)
    {
    }

SMSQLiteSisterFile::~SMSQLiteSisterFile()
    {
    if (m_smFeatureSQLiteFile.IsValid() && m_removeTempGenFile)
        {
        Utf8String dbFileName;
        bool result = m_smFeatureSQLiteFile->GetFileName(dbFileName);
        assert(result == true);
        assert(m_smFeatureSQLiteFile->GetRefCount() == 1);
        m_smFeatureSQLiteFile->Close();
        m_smFeatureSQLiteFile = 0;
        remove(dbFileName.c_str());
        }
    }

void SMSQLiteSisterFile::SetRemoveTempGenerationFile(bool removeTempGenFile)
    {
    m_removeTempGenFile = removeTempGenFile;
    }

void SMSQLiteSisterFile::CopyClipSisterFile(SMStoreDataType dataType) const
{
	WString sqlFileNameSource, sqlFileName;
	GetSisterSQLiteFileName(sqlFileNameSource, dataType, true);
	GetSisterSQLiteFileName(sqlFileName, dataType, false);

	BeFileName::BeCopyFile(sqlFileNameSource.c_str(), sqlFileName.c_str());
}

void SMSQLiteSisterFile::CloseSisterFile(SMStoreDataType dataType)
{
    SMSQLiteFilePtr sqlFilePtr;

    switch (dataType)
    {
    case SMStoreDataType::LinearFeature:
    case SMStoreDataType::Graph:
    {
        std::lock_guard<std::mutex> lock(m_featureOpen);
        if (m_smFeatureSQLiteFile.IsValid() && m_smFeatureSQLiteFile->IsOpen())
        {
            m_smFeatureSQLiteFile->Close();
            m_smFeatureSQLiteFile = nullptr;
        }

    }
    break;

    case SMStoreDataType::DiffSet:
    {
        std::lock_guard<std::mutex> lock(m_clipOpen);
        if (m_smClipSQLiteFile.IsValid() && m_smClipSQLiteFile->IsOpen())
        {
            m_smClipSQLiteFile->Close();
            m_smClipSQLiteFile = nullptr;
        }
    }
    break;

    case SMStoreDataType::ClipDefinition:
    case SMStoreDataType::Skirt:
    case SMStoreDataType::CoveragePolygon:
    case SMStoreDataType::CoverageName:
    {
        std::lock_guard<std::mutex> lock(m_defOpen);
        if (m_smClipDefinitionSQLiteFile.IsValid() && m_smClipDefinitionSQLiteFile->IsOpen())
        {
            m_smClipDefinitionSQLiteFile->Close();
            m_smClipDefinitionSQLiteFile = nullptr;
        }
    }
    break;

    default:
        assert(!"Unknown datatype");
        break;
    }
}
   
SMSQLiteFilePtr SMSQLiteSisterFile::GetSisterSQLiteFile(SMStoreDataType dataType, bool createSisterIfMissing, bool useTempPath)
    {
    SMSQLiteFilePtr sqlFilePtr;

    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
        case SMStoreDataType::Graph:
            {
            assert(createSisterIfMissing == true || m_smSQLiteFile->IsShared());
            std::lock_guard<std::mutex> lock(m_featureOpen);
            if (!m_smFeatureSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);

                Utf8String sqlNameUtf8(sqlFileName.c_str());

                StatusInt status;

                if (!m_smSQLiteFile->IsShared())
                    { 
                    remove(sqlNameUtf8.c_str());
                    m_smFeatureSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, false, SQLDatabaseType::SM_GENERATION_FILE, createSisterIfMissing);
                    //m_smFeatureSQLiteFile->Create(sqlFileName, SQLDatabaseType::SM_GENERATION_FILE);
                    BeAssert(status == SUCCESS || !createSisterIfMissing);
                    }
                else
                    {         
#ifndef NDEBUG
                    //Non sharing process should have create the sister file before sharing process can access it.
#if defined(__APPLE__) || defined(ANDROID)
                    struct stat buffer;
                    assert(stat(sqlNameUtf8.c_str(), &buffer) == 0);
#else
                    struct _stat64i32 buffer;
                    assert(_wstat(sqlFileName.c_str(), &buffer) == 0);
#endif
#endif
                    m_smFeatureSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, true, SQLDatabaseType::SM_GENERATION_FILE, false);
                    BeAssert(status == SUCCESS);
                    }
                }

            sqlFilePtr = m_smFeatureSQLiteFile;
            }
            break;

        case SMStoreDataType::DiffSet:
            {
            std::lock_guard<std::mutex> lock(m_clipOpen);
            if (!m_smClipSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType, useTempPath);
                    
                StatusInt status;
                m_smClipSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, false, SQLDatabaseType::SM_DIFFSETS_FILE, createSisterIfMissing);
                BeAssert(status == SUCCESS || !createSisterIfMissing);

            sqlFilePtr = m_smClipSQLiteFile;
            }
            break;

        case SMStoreDataType::ClipDefinition:
        case SMStoreDataType::Skirt:
        case SMStoreDataType::CoveragePolygon:
        case SMStoreDataType::CoverageName:            
            {
            std::lock_guard<std::mutex> lock(m_defOpen);
            if (!m_smClipDefinitionSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType, useTempPath);

                StatusInt status;
                m_smClipDefinitionSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, false, SQLDatabaseType::SM_CLIP_DEF_FILE, createSisterIfMissing);
                BeAssert(status == SUCCESS || createSisterIfMissing == false);
                }

            sqlFilePtr = m_smClipDefinitionSQLiteFile;
            }
            break;

        default:
            assert(!"Unknown datatype");
            break;
        }

    return sqlFilePtr;
    }

bool SMSQLiteSisterFile::DoesSisterSQLiteFileExist(SMStoreDataType dataType) const
	{
	WString sqlFileName;
	if (!GetSisterSQLiteFileName(sqlFileName, dataType))
		return false;

#ifdef VANCOUVER_API
	return BeFileName::DoesPathExist(sqlFileName.c_str());
#else
	return BeFileName(sqlFileName).DoesPathExist();
#endif
	}

bool SMSQLiteSisterFile::SetProjectFilesPath(BeFileName & projectFilesPath)
    {
    /*if (m_projectFilesPath.length() > 0)
        return false;*/

    m_projectFilesPath = projectFilesPath;    
    return true;
    }

bool SMSQLiteSisterFile::SetUseTempPath(bool useTempPath)
    {    
    m_useTempPath = useTempPath;  
    return true;
    }

void SMSQLiteSisterFile::SaveSisterFiles()
    {
    if (m_smFeatureSQLiteFile.IsValid())
        { 
        m_smFeatureSQLiteFile->Save();
        }

    if (m_smClipSQLiteFile.IsValid())
        {
        m_smClipSQLiteFile->Save();
        }

    if (m_smClipDefinitionSQLiteFile.IsValid())
        {
        m_smClipDefinitionSQLiteFile->Save();
        }        
    }

void SMSQLiteSisterFile::Compact()
    {
	//should we also compact the main file? operation can take a while
	if (m_smFeatureSQLiteFile.IsValid())
	{
		m_smFeatureSQLiteFile->Compact();
	}

	if (m_smClipSQLiteFile.IsValid())
	{
		m_smClipSQLiteFile->Compact();
	}

	if (m_smClipDefinitionSQLiteFile.IsValid())
	{
		m_smClipDefinitionSQLiteFile->Compact();
	}
    }

bool SMSQLiteSisterFile::IsProjectFilesPathSet() const
    {
    return !m_projectFilesPath.empty();
    }

bool SMSQLiteSisterFile::IsUsingTempPath() const
    {    
    return m_useTempPath;
    }
