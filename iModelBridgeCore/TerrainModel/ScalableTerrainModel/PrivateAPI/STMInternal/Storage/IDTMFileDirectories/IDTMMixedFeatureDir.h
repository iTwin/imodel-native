//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>

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
     size_t                   GetIndex                           () const;

     static MixedFeatureDir*  Create                             ();      // Should be private, Android problem.

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