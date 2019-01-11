/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterCommandBuilder.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ImporterBaseFixture.h"

#define COMMANDPLUS(val) \
    m_command = m_command + val ;

/*================================================================================**//**
* @bsiclass                                                     Umar Hayat      05/16
+===============+===============+===============+===============+===============+======*/
struct ImporterCommandBuilder
{
    WString m_command;
public:

    ImporterCommandBuilder();
    virtual void createCommand();
    void addInputFile(WString fileName);
    void addOutputFile(WString fileName);
    void addDescription(WString fileName);
    void addUpdateFlag(WString description=L"");
    void addCompressFlag();
    void addNoAssertDialogFlag();
    void addNoThumbnails();
    void addConfigFile(WString fileName);
    void addPassword(WString pw);
    void resetCommand();

    void createCommandForDwgImporter();
};
