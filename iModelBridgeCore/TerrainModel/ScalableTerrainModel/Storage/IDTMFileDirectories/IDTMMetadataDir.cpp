//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMMetadataDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h>
#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFPacketManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

namespace {
/* 
 * VERSIONNING 
 * Version 0:
 *      Current.
 */

const uint32_t DIRECTORY_VERSION = 0;
}

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MetadataDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

MetadataDir::~MetadataDir ()
    {

    }


MetadataDir::MetadataDir ()
    :   Directory(DIRECTORY_VERSION)
    {

    }


bool MetadataDir::GetSchema    (Schema&         po_rSchema,
                                SchemaType&     po_rType) const
    {
    uint32_t Type = 0;
    if (!AttributeMgr().Get (IDTM_ATTRIBUTEID_METADATADIR_SCHEMA_TYPE, Type))
        return false;

    if (!AttributeMgr().GetPacket<Byte> (IDTM_ATTRIBUTEID_METADATADIR_SCHEMA, po_rSchema))
        return false;

    po_rType = static_cast<SchemaType>(Type);
    return true;
    }


bool MetadataDir::SetSchema    (const Schema&   pi_rSchema,
                                SchemaType      pi_Type)
    {
    if (!AttributeMgr().Set (IDTM_ATTRIBUTEID_METADATADIR_SCHEMA_TYPE, static_cast<uint32_t>(pi_Type)))
        return false;
    if (!AttributeMgr().SetPacket<Byte> (IDTM_ATTRIBUTEID_METADATADIR_SCHEMA, pi_rSchema))
        {
        HASSERT(!"Schema out of sync!");
        return false;
        }

    return true;
    }


bool MetadataDir::Get (Metadata& po_rMetaData) const
    {
    return AttributeMgr().GetPacket<Byte> (IDTM_ATTRIBUTEID_METADATADIR_METADATA, po_rMetaData);
    }


bool MetadataDir::Set (const Metadata& pi_rMetaData)
    {
    return AttributeMgr().SetPacket<Byte> (IDTM_ATTRIBUTEID_METADATADIR_METADATA, pi_rMetaData);
    }


DTMMetadataDir::DTMMetadataDir ()
    {

    }


DTMMetadataDir::~DTMMetadataDir ()
    {

    }


PrimitiveMetadataDir::PrimitiveMetadataDir ()
    {

    }


PrimitiveMetadataDir::~PrimitiveMetadataDir ()
    {

    }


bool PrimitiveMetadataDir::Get (TileID      pi_ID,
                                Metadata&   po_rMetaData) const
    {
    return PacketMgr().Get(pi_ID, po_rMetaData);
    }


bool PrimitiveMetadataDir::Set (TileID          pi_ID,
                                const Metadata& po_rMetaData)
    {
    return PacketMgr().Set(pi_ID, po_rMetaData);
    }


} // End IDTMFile namespace