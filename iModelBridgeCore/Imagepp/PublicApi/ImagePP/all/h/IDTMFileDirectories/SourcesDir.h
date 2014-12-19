//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/SourcesDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : SourcesDir
//-----------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/IDTMFileDirectories/SourceNodeDir.h>


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

    _HDLLg TimeType                 GetLastModifiedCheckTime                   () const;
    _HDLLg bool                     SetLastModifiedCheckTime                   (TimeType            pi_checkTime);



    _HDLLg TimeType                 GetLastSyncTime                            () const;
    _HDLLg bool                     SetLastSyncTime                            (TimeType            pi_lastSyncTime);

    _HDLLg bool                     ClearAll                                   ();



    /*
     * Externally managed fields format version control
     */
    _HDLLg bool                     SetSerializedSourceFormatVersion           (uint32_t            pi_version);
    _HDLLg bool                     SetContentConfigFormatVersion              (uint32_t            pi_version);
    _HDLLg bool                     SetImportSequenceFormatVersion             (uint32_t            pi_version);
    _HDLLg bool                     SetImportConfigFormatVersion               (uint32_t            pi_version);

    _HDLLg uint32_t                 GetSerializedSourceFormatVersion           () const;
    _HDLLg uint32_t                 GetContentConfigFormatVersion              () const;
    _HDLLg uint32_t                 GetImportSequenceFormatVersion             () const;
    _HDLLg uint32_t                 GetImportConfigFormatVersion               () const;

    explicit                        SourcesDir                                 ();      // Should be private, Android problem.

private:
    friend class                    HTGFF::Directory;

    };

} //End namespace IDTMFile