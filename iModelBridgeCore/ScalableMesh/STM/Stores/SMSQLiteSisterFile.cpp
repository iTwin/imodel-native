//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteSisterFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableMeshPCH.h>
#include "SMSQLiteSisterFile.h"

bool SMSQLiteSisterFile::GetSisterSQLiteFileName(WString & sqlFileName, SMStoreDataType dataType)
    {
    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
            {
            assert(!m_smSQLiteFile.IsValid()); // Must have a valid SQLite database 
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

SMSQLiteFilePtr SMSQLiteSisterFile::GetSisterSQLiteFile(SMStoreDataType dataType)
    {
    SMSQLiteFilePtr sqlFilePtr;

    switch (dataType)
        {
        case SMStoreDataType::LinearFeature:
            {
            if (!m_smFeatureSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);

                _wremove(sqlFileName.c_str());

                m_smFeatureSQLiteFile = new SMSQLiteFile();
                m_smFeatureSQLiteFile->Create(sqlFileName);
                }

            sqlFilePtr = m_smFeatureSQLiteFile;
            }
            break;

        case SMStoreDataType::DiffSet:
            {
            if (!m_smClipSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);

                StatusInt status;
                m_smClipSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status);

                if (status == 0)
                    {
                    m_smClipSQLiteFile->Create(sqlFileName);
                    }
                }

            sqlFilePtr = m_smClipSQLiteFile;
            }
            break;

        case SMStoreDataType::ClipDefinition:
        case SMStoreDataType::Skirt:
            {
            if (!m_smClipDefinitionSQLiteFile.IsValid())
                {
                WString sqlFileName;
                GetSisterSQLiteFileName(sqlFileName, dataType);

                StatusInt status;
                m_smClipDefinitionSQLiteFile = SMSQLiteFile::Open(sqlFileName, false, status);

                if (status == 0)
                    {
                    m_smClipDefinitionSQLiteFile->Create(sqlFileName);
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

    //NEEDS_WORK_SM : Ugly, load/creation of the project files should be done explicitly
    //Force the opening/creation of project file in main thread to avoid global mutex.
    GetSisterSQLiteFile(SMStoreDataType::DiffSet);
    GetSisterSQLiteFile(SMStoreDataType::Skirt);

    return true;
    }

bool SMSQLiteSisterFile::IsProjectFilesPathSet()
    {
    return !m_projectFilesPath.empty();
    }
