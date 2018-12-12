//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/SourceNodeDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : SourceNodeDir
//-----------------------------------------------------------------------------

#pragma once


#include <ImagePP/h/HIterators.h>

#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HPUArray.h>

#include <STMInternal/Storage/IDTMFileDirectories/SourceSequenceDir.h>
#include <STMInternal/Storage/HTGFFSubDirIter.h>

namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourceNodeDir : public HTGFF::Directory
    {
private:
    friend class                    HTGFF::Directory;

    class                           SubNodeEditor;

    SourceSequenceDir*              m_pSources;

    virtual bool                    _Create                            (const CreateConfig&             pi_rCreateConfig,
                                                                        const UserOptions*              pi_pUserOptions) override;
    virtual bool                    _Load                              (const UserOptions*              pi_pUserOptions) override;
    virtual bool                    _Save                              () override;

protected:
    explicit                        SourceNodeDir                      ();

    bool                            ClearAll                           ();

public:
     static SourceNodeDir*    Create                             ();      // Should be private, Android problem.

    typedef HTGFF::SubDirIter<const SubNodeEditor>
                                    NodeCIter;
    typedef HTGFF::SubDirIter<SubNodeEditor>
                                    NodeIter;

    typedef time_t                  TimeType;


    struct                          Options;

     static uint32_t          s_GetVersion                       ();

    virtual                         ~SourceNodeDir                     ();

    
     bool                     IsSourcesNode                      () const;

     TimeType                 GetLastModifiedTime                () const;
     bool                     SetLastModifiedTime                (TimeType                        pi_lastModifiedTime);

     const SourceSequenceDir* GetSources                         () const;
     SourceSequenceDir*       GetSources                         ();

     NodeCIter                SubNodesBegin                      () const;
     NodeCIter                SubNodesEnd                        () const;

     NodeIter                 SubNodesBegin                      ();
     NodeIter                 SubNodesEnd                        ();

     SourceNodeDir*           AddGroupNode                       ();
     SourceNodeDir*           AddSourcesNode                     ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourceNodeDir::SubNodeEditor : public HTGFF::SubDirEditorBase<HTGFF::SubDirManager<SourceNodeDir>>
    {
public:
     const SourceNodeDir*     Get                                () const;
     SourceNodeDir*           Get                                ();
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceNodeDir::Options : public HTGFF::Directory::UserOptions
    {
    bool                            m_isSource;

    explicit                        Options                            (bool                            pi_isSource) 
        :   m_isSource(pi_isSource) {} 

    };



} //End namespace IDTMFile