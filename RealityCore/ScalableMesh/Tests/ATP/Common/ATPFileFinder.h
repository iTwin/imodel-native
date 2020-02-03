/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/WString.h>

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Hiba.Dorias        04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
class ATPFileFinder
    {
    public:

        explicit               ATPFileFinder();
        virtual                ~ATPFileFinder();

        void                   FindFiles(const WString&          pi_rSourceFolderPath,
                                         WString&                pi_FilePaths,
                                         bool                    pi_SearchSubFolders) const;

        bool                   ParseFilePaths(WString&                pio_FilePaths,
                                              WString&                pio_FirstPath) const;
    };
