//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMMixedFeatureDir.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <Imagepp/all/h/HTGFFDirectory.h>
#include <Imagepp/all/h/IDTMFileDirectories/IDTMFeatureDir.h>

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Directory that can store features of mixed types.
*
* @see          FeatureDir
* @see          FeatureTileHandler
* @bsiclass                                                  Raymond.Gauthier   9/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MixedFeatureDir : public FeatureDir
    {
public:
    _HDLLg size_t                   GetIndex                           () const;

    _HDLLg static MixedFeatureDir*  Create                             ();      // Should be private, Android problem.

private:
    friend class                    HTGFF::Directory;
    explicit                        MixedFeatureDir                    ();

    virtual bool                    _IsUniform                         () const override {return false;}

    virtual bool                    _CreateFeatureHeaderDir            (HeaderDir*&                 po_rpHeaderDir,
                                                                        const CreateConfig&         pi_rCreateConfig,
                                                                        const UserOptions*          pi_pUserOptions) override;
    virtual bool                    _LoadFeatureHeaderDir              (HeaderDir*&                 po_rpHeaderDir,
                                                                        const UserOptions*          pi_pUserOptions) override;

    };


inline size_t MixedFeatureDir::GetIndex () const
    {
    return GetSubDirIndex();
    }


} //End namespace IDTMFile