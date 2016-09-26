//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/Stores/SMSQLiteSisterFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once


#include "ISMDataStore.h"
#include "..\SMSQLiteFile.h"
#include "SMStoreUtils.h"

class SMSQLiteSisterFile
    {
    private:

        SMSQLiteFilePtr m_smSQLiteFile;

        SMSQLiteFilePtr m_smFeatureSQLiteFile;
        SMSQLiteFilePtr m_smClipSQLiteFile;
        SMSQLiteFilePtr m_smClipDefinitionSQLiteFile;
        BeFileName      m_projectFilesPath;

        bool            GetSisterSQLiteFileName(WString& sqlFileName, SMStoreDataType dataType);

        SMSQLiteSisterFile() = default;

    public:
        SMSQLiteSisterFile(SMSQLiteFilePtr sqliteFile);

        ~SMSQLiteSisterFile();

        SMSQLiteFilePtr GetSisterSQLiteFile(SMStoreDataType dataType);

        bool SetProjectFilesPath(BeFileName& projectFilesPath);

        bool IsProjectFilesPathSet();

    };
