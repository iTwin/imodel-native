//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFile.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMFile
//-----------------------------------------------------------------------------

#pragma once


#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFeatureArray.h>

#include <ImagePP/all/h/IDTMFileDirectories/IDTMMetadataDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMSpatialIndexDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMPointDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMFilteringDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMFeatureDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMLayerDir.h>
#include <ImagePP/all/h/IDTMFileDirectories/SourcesDir.h>

#include <ImagePP/all/h/IDTMFileDirectories/IDTMPointTileHandler.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMFeatureTileHandler.h>
#include <ImagePP/all/h/IDTMFileDirectories/IDTMBTreeIndexHandler.h>

#include <ImagePP/all/h/IDTMFileDirectories/IDTMPointPacketHandler.h>

#include <ImagePP/all/h/HTGFFDirectory.h>
#include <ImagePP/all/h/HTGFFFile.h>

#include <ImagePP/all/h/HTGFFSubDirIter.h>

// Local forward declarations
class TagFile;



namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* @description  Interface to a file that stores multi-resolution DTMs (MrDTMs). This
*               file is implemented using the HTGFF file format. Data is organized
*               / can be accessed via a directory hierarchy in which attributes can
*               be found and directory data can be indexed. Each directory has its
*               own separate packet indexing system.
*
*
* @see HTGFF::Directory
* @see HTGFF::File
* @see HTGFFFile
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class File : public HFCShareableObject<File>
    {
public:
    typedef int32_t              Status;

    typedef HFCPtr<File>            Ptr;
    typedef HFCPtr<File>            CPtr;

    class                           RootDir;

    _HDLLg static uint32_t          s_GetVersion                   ();

    _HDLLg static Ptr               Open                           (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode,
                                                                    Status&                 po_rStatus = EditErr());

    _HDLLg static Ptr               Open                           (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

    _HDLLg static CPtr              OpenReadOnly                   (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

    _HDLLg static Ptr               Create                         (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

    _HDLLg virtual                  ~File                          ();

    _HDLLg bool                     Save                           ();
    _HDLLg bool                     Close                          ();


    bool                            IsReadOnly                     () const {
        return m_file.IsReadOnly();
        }
    const AccessMode&               GetAccessMode                  () const {
        return m_file.GetAccessMode();
        }

    _HDLLg const RootDir*           GetRootDir                     () const;
    _HDLLg RootDir*                 GetRootDir                     ();

private:
    explicit                        File                           (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode);


    // Disable copies of any kind
    File                           (const File&             pi_rObj);
    File&                           operator=                      (const File&             pi_rObj);

    static File*                    OpenImpl                       (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode,
                                                                    Status&                 po_rStatus);

    _HDLLg static Status&           EditErr                        ();

    HTGFF::File                     m_file;
    };



/*---------------------------------------------------------------------------------**//**
* @description  The root directory. Used to access all IDTM directories and main/global
*               attributes.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class File::RootDir : public HTGFF::Directory
    {
private:
    friend class                HTGFF::File;
    class                       LayerDirEditor;

    explicit                    RootDir                            ();

public:
    typedef HTGFF::SubDirIter<const LayerDirEditor>
                                LayerDirCIter;
    typedef HTGFF::SubDirIter<LayerDirEditor>
                                LayerDirIter;

    _HDLLg static uint32_t      s_GetVersion                       ();

    virtual                     ~RootDir                           ();


    /*
     * Layers access
     */ 
    _HDLLg size_t               CountLayerDirs                     () const;

    _HDLLg LayerDirCIter        LayerDirsBegin                     () const;
    _HDLLg LayerDirCIter        LayerDirsEnd                       () const;

    _HDLLg LayerDirIter         LayerDirsBegin                     ();
    _HDLLg LayerDirIter         LayerDirsEnd                       ();


    _HDLLg bool                 HasLayerDir                        (size_t              pi_Index) const;

    _HDLLg const LayerDir*      GetLayerDir                        (size_t              pi_Index) const;
    _HDLLg LayerDir*            GetLayerDir                        (size_t              pi_Index);

    _HDLLg LayerDir*            CreateLayerDir                     (size_t              pi_Index);
    _HDLLg LayerDir*            AddLayerDir                        ();



    /*
     * Sources access
     */
    _HDLLg bool                 HasSourcesDir                      () const;

    _HDLLg const SourcesDir*    GetSourcesDir                      () const;
    _HDLLg SourcesDir*          GetSourcesDir                      ();

    _HDLLg SourcesDir*          CreateSourcesDir                   ();

    // TDORAY: Add a remove layer method
    // TDORAY: Add a find layer by name?




    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class File::RootDir::LayerDirEditor : public HTGFF::SubDirEditorBase<HTGFF::SubDirManager<LayerDir>>
    {
public:
    _HDLLg const LayerDir*      Get                                () const;
    _HDLLg LayerDir*            Get                                ();
    };


} //End namespace IDTMFile