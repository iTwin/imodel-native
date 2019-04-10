/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/Common/ATPFileFinder.h $
|    $RCSfile: FileFinder.h,v $
|   $Revision: 1.5 $
|       $Date: 2012/05/17 16:53:10 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
