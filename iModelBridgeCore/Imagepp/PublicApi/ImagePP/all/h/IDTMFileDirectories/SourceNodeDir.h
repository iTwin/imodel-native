//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/SourceNodeDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : SourceNodeDir
//-----------------------------------------------------------------------------

#pragma once


#include <ImagePP/h/HIterators.h>

#include <ImagePP/all/h/HTGFFDirectory.h>
#include <ImagePP/all/h/HPUArray.h>

#include <ImagePP/all/h/IDTMFileDirectories/SourceSequenceDir.h>
#include <ImagePP/all/h/HTGFFSubDirIter.h>

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
    _HDLLg static SourceNodeDir*    Create                             ();      // Should be private, Android problem.

    typedef HTGFF::SubDirIter<const SubNodeEditor>
                                    NodeCIter;
    typedef HTGFF::SubDirIter<SubNodeEditor>
                                    NodeIter;

    typedef time_t                  TimeType;


    struct                          Options;

    _HDLLg static uint32_t          s_GetVersion                       ();

    virtual                         ~SourceNodeDir                     ();

    
    _HDLLg bool                     IsSourcesNode                      () const;

    _HDLLg TimeType                 GetLastModifiedTime                () const;
    _HDLLg bool                     SetLastModifiedTime                (TimeType                        pi_lastModifiedTime);

    _HDLLg const SourceSequenceDir* GetSources                         () const;
    _HDLLg SourceSequenceDir*       GetSources                         ();

    _HDLLg NodeCIter                SubNodesBegin                      () const;
    _HDLLg NodeCIter                SubNodesEnd                        () const;

    _HDLLg NodeIter                 SubNodesBegin                      ();
    _HDLLg NodeIter                 SubNodesEnd                        ();

    _HDLLg SourceNodeDir*           AddGroupNode                       ();
    _HDLLg SourceNodeDir*           AddSourcesNode                     ();

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class SourceNodeDir::SubNodeEditor : public HTGFF::SubDirEditorBase<HTGFF::SubDirManager<SourceNodeDir>>
    {
public:
    _HDLLg const SourceNodeDir*     Get                                () const;
    _HDLLg SourceNodeDir*           Get                                ();
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