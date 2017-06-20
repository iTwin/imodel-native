/*--------------------------------------------------------------------------------------+
|
|  $Source: Dwg/Tests/ImporterCommandBuilder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ImporterCommandBuilder.h" 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImporterCommandBuilder::ImporterCommandBuilder()
    {
    m_command = L"";
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::createCommand()
    {
    createCommandForDwgImporter();
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::resetCommand()
    {
    m_command = L"";
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::createCommandForDwgImporter()
    {
    COMMANDPLUS(WString(L"DwgImporter.exe"))
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addInputFile(WString fileName)
    {
    COMMANDPLUS(L" --input=" + fileName)
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addOutputFile(WString fileName)
    {
    COMMANDPLUS(L" --output=" + fileName)
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addCompressFlag()
    {
    COMMANDPLUS(L" --compress ")
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addConfigFile(WString fileName)
    {
    COMMANDPLUS(L" --configuration=" + fileName)
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addDescription(WString description)
    {
    COMMANDPLUS(L" --description=" + description)
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addPassword(WString pw)
    {
    COMMANDPLUS(L" --password " + pw)
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addUpdateFlag(WString description)
    {
    if (description == L"")
        COMMANDPLUS(L" --update")
    else
        COMMANDPLUS(L" --update=" + description)
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ImporterCommandBuilder::addNoAssertDialogFlag()
    {
    COMMANDPLUS(L" --no-assert-dialogs")
    }
