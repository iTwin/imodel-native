//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteSisterFile.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once


#include "ISMDataStore.h"
#include "../SMSQLiteFile.h"
#include "SMStoreUtils.h"

class SMSQLiteSisterFile
    {
    private:

        SMSQLiteFilePtr m_smSQLiteFile;

        SMSQLiteFilePtr m_smFeatureSQLiteFile;
        SMSQLiteFilePtr m_smClipSQLiteFile;
        SMSQLiteFilePtr m_smClipDefinitionSQLiteFile;
        BeFileName      m_projectFilesPath;
        bool            m_useTempPath;
        bool            m_removeTempGenFile;

        std::mutex m_defOpen;
        std::mutex m_featureOpen;
        std::mutex m_clipOpen;

        bool            GetSisterSQLiteFileName(WString& sqlFileName, SMStoreDataType dataType, bool useTempPath = true) const;

        SMSQLiteSisterFile() = default;

    public:
        SMSQLiteSisterFile(SMSQLiteFilePtr sqliteFile);

        ~SMSQLiteSisterFile();

        BENTLEY_SM_EXPORT SMSQLiteFilePtr GetSisterSQLiteFile(SMStoreDataType dataType, bool createSisterIfMissing, bool useTempPath = true);

		bool DoesSisterSQLiteFileExist(SMStoreDataType dataType) const;

        bool SetProjectFilesPath(BeFileName& projectFilesPath);

        BENTLEY_SM_EXPORT void SaveSisterFiles();

		void Compact();

        bool IsProjectFilesPathSet() const;

		void CopyClipSisterFile(SMStoreDataType dataType) const;

        bool IsUsingTempPath() const;

        bool SetUseTempPath(bool useTempPath);

        BENTLEY_SM_EXPORT void SetRemoveTempGenerationFile(bool removeTempGenFile);
    };
