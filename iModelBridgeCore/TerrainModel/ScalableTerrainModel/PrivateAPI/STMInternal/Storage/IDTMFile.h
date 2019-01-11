//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : IDTMFile
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/IDTMTypes.h>
#include <STMInternal/Storage/IDTMFeatureArray.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFilteringDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMLayerDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/SourcesDir.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointTileHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureTileHandler.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMBTreeIndexHandler.h>

#include <STMInternal/Storage/IDTMFileDirectories/IDTMPointPacketHandler.h>

#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFFile.h>

#include <STMInternal/Storage/HTGFFSubDirIter.h>


// Local forward declarations
class TagFile;



namespace IDTMFile {

#define DTMFILE_UNSUPPORTED_VERSION 3

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
class File : public BentleyApi::ImagePP::HFCShareableObject<File>
    {
public:
    typedef int32_t              Status;
        
    typedef BentleyApi::ImagePP::HFCPtr<File>            Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<File>            CPtr;

    class                           RootDir;

     static uint32_t          s_GetVersion                   ();

     static Ptr               Open                           (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode,
                                                                    Status&                 po_rStatus = EditErr());

     static Ptr               Open                           (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

     static CPtr              OpenReadOnly                   (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

     static Ptr               Create                         (WCharCP                 pi_rInputFilePath,
                                                                    Status&                 po_rStatus = EditErr());

     virtual                  ~File                          ();

     bool                     Save                           ();
     bool                     Close                          ();


    bool                            IsReadOnly                     () const {
        return m_file.IsReadOnly();
        }
    const AccessMode&               GetAccessMode                  () const {
        return m_file.GetAccessMode();
        }

     const RootDir*           GetRootDir                     () const;
     RootDir*                 GetRootDir                     ();

private:
    explicit                        File                           (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode);


    // Disable copies of any kind
    File                           (const File&             pi_rObj);
    File&                           operator=                      (const File&             pi_rObj);

    static File*                    OpenImpl                       (WCharCP                 pi_rInputFilePath,
                                                                    const AccessMode&       pi_rAccessMode,
                                                                    Status&                 po_rStatus);

     static Status&           EditErr                        ();

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

     static uint32_t      s_GetVersion                       ();

    virtual                     ~RootDir                           ();


    /*
     * Layers access
     */ 
     size_t               CountLayerDirs                     () const;

     LayerDirCIter        LayerDirsBegin                     () const;
     LayerDirCIter        LayerDirsEnd                       () const;

     LayerDirIter         LayerDirsBegin                     ();
     LayerDirIter         LayerDirsEnd                       ();


     bool                 HasLayerDir                        (size_t              pi_Index) const;

     const LayerDir*      GetLayerDir                        (size_t              pi_Index) const;
     LayerDir*            GetLayerDir                        (size_t              pi_Index);

     LayerDir*            CreateLayerDir                     (size_t              pi_Index);
     LayerDir*            AddLayerDir                        ();



    /*
     * Sources access
     */
     bool                 HasSourcesDir                      () const;

     const SourcesDir*    GetSourcesDir                      () const;
     SourcesDir*          GetSourcesDir                      ();

     SourcesDir*          CreateSourcesDir                   ();

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
     const LayerDir*      Get                                () const;
     LayerDir*            Get                                ();
    };


} //End namespace IDTMFile
