//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteSisterFile.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMSQLiteSisterFile.h"

bool SMSQLiteSisterFile::GetSisterSQLiteFileName(WString & sqlFileName, SMStoreDataType dataType)
    {
    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
        case SMStoreDataType::Graph:
            {
            assert(m_smSQLiteFile.IsValid()); // Must have a valid SQLite database 
            Utf8String dbFileName;
            bool result = m_smSQLiteFile->GetFileName(dbFileName);
            assert(result == true);

            sqlFileName.AssignUtf8(dbFileName.c_str());

            sqlFileName.append(L"_feature"); //temporary file, deleted after generation
            }
            return true;
            break;

        case SMStoreDataType::DiffSet:
            sqlFileName = m_projectFilesPath;
            sqlFileName.append(L"_clips");
            return true;
            break;
        case SMStoreDataType::ClipDefinition:
        case SMStoreDataType::Skirt:
        case SMStoreDataType::CoveragePolygon:
        case SMStoreDataType::CoverageName:
            sqlFileName = m_projectFilesPath;
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
      m_smClipDefinitionSQLiteFile(nullptr)
    {
    }

SMSQLiteSisterFile::~SMSQLiteSisterFile()
    {
    if (m_smFeatureSQLiteFile.IsValid())
        {
        Utf8String dbFileName;
        bool result = m_smFeatureSQLiteFile->GetFileName(dbFileName);
        assert(result == true);
        assert(m_smFeatureSQLiteFile->GetRefCount() == 1);
        m_smFeatureSQLiteFile->Close();
        m_smFeatureSQLiteFile = 0;
        WString dbFileNameW;
        dbFileNameW.AssignUtf8(dbFileName.c_str());
        _wremove(dbFileNameW.c_str());
        }
    }

SMSQLiteFilePtr SMSQLiteSisterFile::GetSisterSQLiteFile(SMStoreDataType dataType, bool createSisterIfMissing)
    {
    SMSQLiteFilePtr sqlFilePtr;

    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
        case SMStoreDataType::Graph:
            {
            assert(createSisterIfMissing == true);
            std::lock_guard<std::mutex> lock(m_featureOpen);
            if (!m_smFeatureSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);

                _wremove(sqlFileName.c_str());
                StatusInt status;
                m_smFeatureSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, SQLDatabaseType::SM_GENERATION_FILE);
                m_smFeatureSQLiteFile->Create(sqlFileName, SQLDatabaseType::SM_GENERATION_FILE);
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
                GetSisterSQLiteFileName(sqlFileName, dataType);
                    
                StatusInt status;
                m_smClipSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, SQLDatabaseType::SM_DIFFSETS_FILE);

                if (status == 0)
                    {
                    if (createSisterIfMissing)
                        {
#ifndef VANCOUVER_API
                        BeFileName path(sqlFileName);
                        if (!path.GetDirectoryName().DoesPathExist())
                            BeFileName::CreateNewDirectory(path.GetDirectoryName().GetWCharCP());
#else
                        BeFileName path(sqlFileName.GetWCharCP());
                        BeFileName dirname(BeFileName::GetDirectoryName(path).GetWCharCP());
                        if (!BeFileName::DoesPathExist(dirname))
                            BeFileName::CreateNewDirectory(dirname.GetWCharCP());
#endif
                        m_smClipSQLiteFile->Create(sqlFileName, SQLDatabaseType::SM_DIFFSETS_FILE);
                        }
                    else
                        {
                        m_smClipSQLiteFile = nullptr;
                        }
                    }
                }

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
                GetSisterSQLiteFileName(sqlFileName, dataType);

                StatusInt status;
                m_smClipDefinitionSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status, SQLDatabaseType::SM_CLIP_DEF_FILE);

                if (status == 0)
                    {
                    if (createSisterIfMissing)
                        {
#ifndef VANCOUVER_API
                        BeFileName path(sqlFileName);
                        if (!path.GetDirectoryName().DoesPathExist())
                            BeFileName::CreateNewDirectory(path.GetDirectoryName().GetWCharCP());
#else
                        BeFileName path(sqlFileName.GetWCharCP());
                        BeFileName dirname(BeFileName::GetDirectoryName(path).GetWCharCP());
                        if (!BeFileName::DoesPathExist(dirname))
                            BeFileName::CreateNewDirectory(dirname.GetWCharCP());
#endif
                        m_smClipDefinitionSQLiteFile->Create(sqlFileName, SQLDatabaseType::SM_CLIP_DEF_FILE);
                        }
                    else
                        { 
                        m_smClipDefinitionSQLiteFile = nullptr;
                        }
                    }
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

bool SMSQLiteSisterFile::SetProjectFilesPath(BeFileName & projectFilesPath)
    {
    if (m_projectFilesPath.length() > 0)
        return false;

    m_projectFilesPath = projectFilesPath;    
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

bool SMSQLiteSisterFile::IsProjectFilesPathSet()
    {
    return !m_projectFilesPath.empty();
    }
