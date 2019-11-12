/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECPresentation/Localization.h"
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Localization::Init()
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName sqlang;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlang);
    sqlang.AppendToPath(L"sqlang");
    sqlang.AppendToPath(L"ECPresentation_en.sqlang.db3");

    L10N::Shutdown();
    L10N::Initialize(L10N::SqlangFiles(sqlang));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Localization::Terminate()
    {
    BeSQLite::L10N::Shutdown();
    }
