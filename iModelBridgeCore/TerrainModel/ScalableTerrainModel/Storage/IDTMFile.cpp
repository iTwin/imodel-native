//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFile.h>
#include <STMInternal/Storage/HTGFFFile.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/IDTMFeatureArray.h>
#include "IDTMFileDefinition.h"

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>


namespace {
/* 
 * VERSIONNING 
 * Version 0:
 *      Current.
 */

const uint32_t FILE_VERSION = 0;

}
   
        
namespace IDTMFile {

static const FileDefinition IDTM_FILE_DEFINITION;


uint32_t File::s_GetVersion ()
{
    return FILE_VERSION;
}

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::Status& File::EditErr ()
    {
    static Status LAST_ERROR;
    return  LAST_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Open an IDTM file in specified mode.
* @param        The IDTM file path to open.
* @param        The access mode in which the file is to be opened
* @return       An open IDTM file pointer on success, 0 otherwise
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File* File::OpenImpl   (WCharCP             pi_rInputFilePath,
                        const AccessMode&   pi_rAccessMode,
                        Status&             po_rStatus)
{

    try
    {
        return new File(pi_rInputFilePath, pi_rAccessMode);
    }
    catch(const exception&)
        {
        return 0;
        }
    catch(HFCException& exception)
    {   
        if (dynamic_cast<HFCUnsupportedFileVersionException*>(&exception) != 0)
            po_rStatus = DTMFILE_UNSUPPORTED_VERSION;               
        return 0;

    }

}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::Ptr File::Open   (WCharCP                 pi_rInputFilePath,
                        const AccessMode&       pi_rAccessMode,
                        Status&                 po_rStatus)
    {
    return OpenImpl(pi_rInputFilePath, pi_rAccessMode, po_rStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::Ptr File::Open   (WCharCP                 pi_rInputFilePath,
                        Status&                 po_rStatus)
    {
    return OpenImpl(pi_rInputFilePath, HFC_READ_WRITE, po_rStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::CPtr File::OpenReadOnly  (WCharCP                 pi_rInputFilePath,
                                Status&                 po_rStatus)
    {
    return OpenImpl(pi_rInputFilePath, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, po_rStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
File::Ptr File::Create (WCharCP                 pi_rInputFilePath,
                        Status&                 po_rStatus)
    {
    return OpenImpl(pi_rInputFilePath, HFC_CREATE_ONLY, po_rStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description  IDTM file constructor. Throws std::exception on failure.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
File::File (WCharCP             pi_rInputFilePath,
            const AccessMode&   pi_rAccessMode)
    :   m_file(pi_rInputFilePath, pi_rAccessMode, IDTM_FILE_DEFINITION, FILE_VERSION) 
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  IDTM file destructor.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
File::~File ()
    {
    m_file.Close();
    }


/*---------------------------------------------------------------------------------**//**
* @description  Save an open file. Will also save all directory instances. File is never
*               saved when in read-only mode.
* @return       true on success, false otherwise (also when the file is closed)
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool File::Save ()
    {
    return m_file.Save();
    }


/*---------------------------------------------------------------------------------**//**
* @description  Close an open file.
* @return       true on success, false otherwise (also when to file was already closed).
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool File::Close ()
    {
    return m_file.Close();
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the root directory. Initialize it if not already done.
* @return       Root directory.
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const File::RootDir* File::GetRootDir () const
    {
    return m_file.GetRootDir<RootDir>();
    }


File::RootDir* File::GetRootDir ()
    {
    return m_file.GetRootDir<RootDir>();
    }


uint32_t File::RootDir::s_GetVersion ()
{
    return FILE_VERSION;
}

File::RootDir::RootDir ()
    {

    }


File::RootDir::~RootDir ()
    {

    }


size_t File::RootDir::CountLayerDirs () const
    {
    return SubDirMgr().GetCount(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS);
    }


File::RootDir::LayerDirCIter File::RootDir::LayerDirsBegin () const
    {
    return SubDirIterMgr<LayerDirCIter::value_type>().begin(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS,
                                                            SubDirMgr<LayerDir>());
    }

File::RootDir::LayerDirCIter File::RootDir::LayerDirsEnd () const
    {
    return SubDirIterMgr<LayerDirCIter::value_type>().end(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS,
                                                          SubDirMgr<LayerDir>());
    }

File::RootDir::LayerDirIter File::RootDir::LayerDirsBegin ()
    {
    return SubDirIterMgr<LayerDirIter::value_type>().begin(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS,
                                                           SubDirMgr<LayerDir>());
    }

File::RootDir::LayerDirIter File::RootDir::LayerDirsEnd ()
    {
    return SubDirIterMgr<LayerDirIter::value_type>().end(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS,
                                                         SubDirMgr<LayerDir>());
    }

bool File::RootDir::HasLayerDir (size_t pi_Index) const
{
    return SubDirMgr().Has(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS, pi_Index);
}

const LayerDir* File::RootDir::GetLayerDir  (size_t pi_Index) const
    {
    return const_cast<RootDir*>(this)->GetLayerDir(pi_Index);
    }

LayerDir* File::RootDir::GetLayerDir (size_t pi_Index)
    {
    return SubDirMgr<LayerDir>().Get(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS, pi_Index);
    }


LayerDir* File::RootDir::CreateLayerDir (size_t pi_Index)
{
    const CreateConfig DirCreateConfig(DataType::CreateVoid(),
                                       Compression::None::Create());

    return SubDirMgr<LayerDir>().Create(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS, pi_Index, DirCreateConfig);
}


LayerDir* File::RootDir::AddLayerDir ()
    {
    const CreateConfig DirCreateConfig(DataType::CreateVoid(),
                                       Compression::None::Create());

    return SubDirMgr<LayerDir>().Add(IDTM_DIRECTORYID_ROOTDIR_LAYER_SUBDIRS, DirCreateConfig);
    }

bool File::RootDir::HasSourcesDir () const
{
    return SubDirMgr().Has(IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR);
}

const SourcesDir* File::RootDir::GetSourcesDir () const
    {
    return SubDirMgr<SourcesDir>().Get(IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR);
    }

SourcesDir* File::RootDir::GetSourcesDir ()
    {
    return SubDirMgr<SourcesDir>().Get(IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR);
    }


SourcesDir* File::RootDir::CreateSourcesDir ()
    {
    static const SourceNodeDir::Options OPTIONS(false);
    const CreateConfig DirCreateConfig;

    return SubDirMgr<SourcesDir>().Create(IDTM_DIRECTORYID_ROOTDIR_SOURCES_SUBDIR, DirCreateConfig, &OPTIONS);
    }



const LayerDir* File::RootDir::LayerDirEditor::Get () const
    {
    return GetBase().Get(GetSubDirTagID(), GetSubDirIDIter());
    }

LayerDir* File::RootDir::LayerDirEditor::Get ()
    {
    return GetBase().Get(GetSubDirTagID(), GetSubDirIDIter());
    }


} //End namespace IDTMFile