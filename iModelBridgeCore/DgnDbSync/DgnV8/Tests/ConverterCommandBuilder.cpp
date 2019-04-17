/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ConverterCommandBuilder.h"
#include <Bentley/BeTest.h>

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ConverterCommandBuilder::ConverterCommandBuilder()
    {
    m_command = L"";
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::CreateCommand()
    {
    CreateCommandForDgnV8Converter();
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::ResetCommand()
    {
    m_command = L"";
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::CreateCommandForDgnV8Converter()
    {
    BentleyApi::BeFileName exePath;
    BentleyApi::BeTest::GetHost().GetDocumentsRoot(exePath);
    exePath.AppendToPath(L"DgnV8Converter.exe");
    COMMANDPLUS(exePath.c_str());
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddInputFile(BentleyApi::WString fileName)
    {
    COMMANDPLUS(L" --input=" + fileName);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddOutputFile(BentleyApi::WString fileName)
    {
    COMMANDPLUS(L" --output=" + fileName);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddCompressFlag()
    {
    COMMANDPLUS(L" --compress ");
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddConfigFile(BentleyApi::WString fileName)
    {
    COMMANDPLUS(L" --configuration=" + fileName);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddDescription(BentleyApi::WString description)
    {
    COMMANDPLUS(L" --description=" + description);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddPassword(BentleyApi::WString pw)
    {
    COMMANDPLUS(L" --password " + pw);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddUpdateFlag(BentleyApi::WString description)
    {
    if (description == L"")
        COMMANDPLUS(L" --update");
    else
        COMMANDPLUS(L" --update=" + description);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddNoAssertDialogFlag()
    {
    COMMANDPLUS(L" --no-assert-dialogs");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddRootModel(BentleyApi::WString rootModel)
    {
    COMMANDPLUS(L" --root-model=" + rootModel );
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddRootModelForce3dFlag()
    {
    COMMANDPLUS(L" --root-model-force-3D" );
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddNamePrefix(BentleyApi::WString prefix)
    {
    COMMANDPLUS(L" --name-prefix=" + prefix );
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Umar Hayat      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddEmbedDir(BentleyApi::WString dirName)
    {
    COMMANDPLUS(L" --embed-directory=" + dirName );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                               Muhammad Hassan      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddExpirationDate(BentleyApi::WString date)
    {
    COMMANDPLUS(L" --expiration=" + date);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                               Muhammad Hassan      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddInputGCS(BentleyApi::WString input)
    {
    COMMANDPLUS(L" --input-gcs=" + input);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                               Muhammad Hassan      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConverterCommandBuilder::AddOutputGCS(BentleyApi::WString output)
    {
    COMMANDPLUS(L" --output-gcs=" + output);
    }
