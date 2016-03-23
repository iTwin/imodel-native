/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/Common/ATPFileFinder.cpp $
|    $RCSfile: FileFinder.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2012/05/17 14:52:36 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ATPFileFinder.h"
#include <windows.h>
#include <ImagePP/h/HmrMacro.h>

inline void AddMissingBackslashToFolderPath(WString* pio_pFolderPath)
    {
    HASSERT(0 != pio_pFolderPath);

    if (pio_pFolderPath->at(pio_pFolderPath->size() - 1) != '\\')
        *pio_pFolderPath += TEXT("\\");
    }

inline void AddWildCardToFolderPath(WString* pio_pFolderPath)
    {
    HASSERT(0 != pio_pFolderPath);

    AddMissingBackslashToFolderPath(pio_pFolderPath);
    *pio_pFolderPath += TEXT("*.*");
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Hiba.Dorias   	04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ATPFileFinder::ATPFileFinder()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Hiba.Dorias   	04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ATPFileFinder::~ATPFileFinder()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  Searches the SourceFolderPath for files.
*               Returns a AString with the complete path of each file found
*               separated by a comma.
* @bsimethod                                                  Hiba.Dorias   	04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void ATPFileFinder::FindFiles(const WString& pi_rSourceFolderPath,
                              WString&       pi_FilePaths,  //=>should be initially an empty AString
                              bool        pi_SearchSubFolders) const
    {
    HANDLE hFindFile;
    WIN32_FIND_DATAW FindFileData;
    WString SourceFolderPathWithWildCard = pi_rSourceFolderPath;
    WString sourceFolderPathWithMissingBackslash = pi_rSourceFolderPath;
    WString SrcName;
    WString SrcNameWithWildcard;

    AddWildCardToFolderPath(&SourceFolderPathWithWildCard);
    AddMissingBackslashToFolderPath(&sourceFolderPathWithMissingBackslash);

    hFindFile = ::FindFirstFileW(SourceFolderPathWithWildCard.c_str(), &FindFileData);

    do
        {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {

            SrcName = sourceFolderPathWithMissingBackslash + FindFileData.cFileName;

            //it's a file!
            pi_FilePaths = pi_FilePaths + SrcName + L";";
            }
        else
            {
            SrcName = pi_rSourceFolderPath + FindFileData.cFileName;
            SrcNameWithWildcard = SrcName;
            AddWildCardToFolderPath(&SrcNameWithWildcard);

            if (!((wcscmp(FindFileData.cFileName, L".") == 0) ||
                  (wcscmp(FindFileData.cFileName, L"..") == 0)))
                {
                if (pi_SearchSubFolders)
                    {
                    WIN32_FIND_DATAW FindFileData2;
                    HANDLE hFindFile2;

                    hFindFile2 = ::FindFirstFileW(SrcNameWithWildcard.c_str(), &FindFileData2);

                    FindFiles(SrcName, pi_FilePaths, pi_SearchSubFolders);
                    }
                }
            }

        } while (::FindNextFileW(hFindFile, &FindFileData));

        ::FindClose(hFindFile);

    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the FilePaths AString, less the first path; which is in the
*               other AString.
*               Returns false if the FilePaths AString is empty.
* @bsimethod                                                  Hiba.Dorias   	04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
bool ATPFileFinder::ParseFilePaths(WString& pio_FilePaths,
                                   WString& pio_FirstPath) const
    {
    bool StringNotEmpty = false;
    pio_FirstPath = L"";

    WChar seps[] = L";";

    WChar* token1,
        *next_token1;

    token1 = wcstok_s(const_cast<WChar*>(pio_FilePaths.c_str()), seps, &next_token1);

    if (token1 != NULL)
        {
        pio_FirstPath = token1;
        StringNotEmpty = true;

        pio_FilePaths = next_token1;

        }

    return StringNotEmpty;
    }