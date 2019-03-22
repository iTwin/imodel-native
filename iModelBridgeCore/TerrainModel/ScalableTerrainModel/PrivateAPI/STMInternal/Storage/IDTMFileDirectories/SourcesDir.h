//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/SourcesDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : SourcesDir
//-----------------------------------------------------------------------------
#pragma once

#include <STMInternal/Storage/IDTMFileDirectories/SourceNodeDir.h>


namespace IDTMFile {



/*---------------------------------------------------------------------------------**//**
* @description  Sources directory. Enumerate all the original sources used to create the
*               IDTM file. This listing may be used to regenerate the IDTM file from
*               original sources.
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourcesDir : public SourceNodeDir
    {
public:
    virtual                         ~SourcesDir                                ();

     TimeType                 GetLastModifiedCheckTime                   () const;
     bool                     SetLastModifiedCheckTime                   (TimeType            pi_checkTime);



     TimeType                 GetLastSyncTime                            () const;
     bool                     SetLastSyncTime                            (TimeType            pi_lastSyncTime);

     bool                     ClearAll                                   ();



    /*
     * Externally managed fields format version control
     */
     bool                     SetSerializedSourceFormatVersion           (uint32_t            pi_version);
     bool                     SetContentConfigFormatVersion              (uint32_t            pi_version);
     bool                     SetImportSequenceFormatVersion             (uint32_t            pi_version);
     bool                     SetImportConfigFormatVersion               (uint32_t            pi_version);

     uint32_t                 GetSerializedSourceFormatVersion           () const;
     uint32_t                 GetContentConfigFormatVersion              () const;
     uint32_t                 GetImportSequenceFormatVersion             () const;
     uint32_t                 GetImportConfigFormatVersion               () const;

    explicit                        SourcesDir                                 ();      // Should be private, Android problem.

private:
    friend class                    HTGFF::Directory;

    };

} //End namespace IDTMFile