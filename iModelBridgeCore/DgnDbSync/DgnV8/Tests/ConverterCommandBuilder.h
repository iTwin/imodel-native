/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../ConverterInternal.h"

#define COMMANDPLUS(val) \
    m_command = m_command + val

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct ConverterCommandBuilder 
{
    BentleyApi::WString m_command;
public:

    ConverterCommandBuilder();
    virtual void CreateCommand();
    void AddInputFile(BentleyApi::WString fileName);
    void AddOutputFile(BentleyApi::WString fileName);
    void AddDescription(BentleyApi::WString fileName);
    void AddUpdateFlag(BentleyApi::WString description=L"");
    void AddCompressFlag();
    void AddNoAssertDialogFlag();
    void AddConfigFile(BentleyApi::WString fileName);
    void AddPassword(BentleyApi::WString pw);
    void AddRootModel(BentleyApi::WString rootModel);
    void AddRootModelForce3dFlag();
    void AddNamePrefix(BentleyApi::WString prefix);
    void AddEmbedDir(BentleyApi::WString dirName);
    void ResetCommand();
    void AddExpirationDate(BentleyApi::WString date);
    void AddInputGCS(BentleyApi::WString input);
    void AddOutputGCS(BentleyApi::WString output);

    void CreateCommandForDgnV8Converter();

};
