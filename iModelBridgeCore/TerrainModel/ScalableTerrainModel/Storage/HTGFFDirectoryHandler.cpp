//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFDirectoryHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HTGFFDirectoryHandler.h>


namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryHandler::DirectoryHandler ()
    :   m_IsClosed(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryHandler::~DirectoryHandler ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectoryHandler::Save ()
    {
    if (m_IsClosed)
        return true;

    return _Save();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectoryHandler::Load ()
    {
    return _Load();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectoryHandler::Close ()
    {
    const bool Success = _Save();
    m_IsClosed = true;
    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectoryHandler::RegisterTo (Directory& pi_rDir)
    {
    return pi_rDir.Register(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryHandler::UnregisterFrom (Directory& pi_rDir)
    {
    if (m_IsClosed)
        return;

    pi_rDir.Unregister(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const AttributeManager& DirectoryHandler::AttributeMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.AttributeMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeManager& DirectoryHandler::AttributeMgr (Directory& pi_rDir)
    {
    return pi_rDir.AttributeMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const PacketManagerBase& DirectoryHandler::PacketMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.PacketMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketManagerBase& DirectoryHandler::PacketMgr (Directory& pi_rDir)
    {
    return pi_rDir.PacketMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const SubDirManagerBase& DirectoryHandler::SubDirMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.SubDirMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SubDirManagerBase& DirectoryHandler::SubDirMgr (Directory& pi_rDir)
    {
    return pi_rDir.SubDirMgr();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SingleDirHandlerConcretePart::SingleDirHandlerConcretePart (Directory& pi_rDir)
    :   m_rDir(pi_rDir)
    {
    RegisterTo(pi_rDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SingleDirHandlerConcretePart::~SingleDirHandlerConcretePart  ()
    {
    UnregisterFrom(m_rDir);
    }






} //End namespace HTGFF