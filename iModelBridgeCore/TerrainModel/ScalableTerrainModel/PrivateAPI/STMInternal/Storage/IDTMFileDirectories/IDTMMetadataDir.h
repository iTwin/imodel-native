//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMMetadataDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HPUPacket.h>
#include <STMInternal/Storage/IDTMTypes.h>

namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* Metadata Directory interface types section
+---------------+---------------+---------------+---------------+---------------+------*/
typedef HPU::Packet                 Metadata;

/*---------------------------------------------------------------------------------**//**
* @description  Class used to store/manipulate a schema
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
enum SchemaType
    {
    IDTM_SCHEMA_T_XFM,
    IDTM_SCHEMA_T_ECFRAMEWORK,
    IDTM_SCHEMA_T_LIGHTWEIGHT,
    IDTM_SCHEMA_T_QTY,
    };
typedef HPU::Packet             Schema;

/*---------------------------------------------------------------------------------**//**
* @description  Base metadata directory. Implements common metadata accessors.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class MetadataDir : public HTGFF::Directory
    {
public:
     static uint32_t      s_GetVersion                       ();

     bool                 GetSchema                          (Schema&                 po_rSchema,
                                                                    SchemaType&             po_rType) const;
     bool                 SetSchema                          (const Schema&           pi_rSchema,
                                                                    SchemaType              pi_Type);

     bool                 Get                                (Metadata&               po_rMetaData) const;
     bool                 Set                                (const Metadata&         pi_rMetaData);

    virtual                     ~MetadataDir                       ();


protected:
    explicit                    MetadataDir                        ();
    };

/*---------------------------------------------------------------------------------**//**
* @description  Directory that store metadata for the whole dtm file.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DTMMetadataDir : public MetadataDir
    {
public:
    virtual                     ~DTMMetadataDir                    ();

    explicit                    DTMMetadataDir                     ();  // Should be private, Android problem.
private:
    friend class                HTGFF::Directory;
    };

/*---------------------------------------------------------------------------------**//**
* @description  Directory that can stores tile indexed metadata.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class PrimitiveMetadataDir : public MetadataDir
    {
public:
     bool                 Get                                (TileID                  pi_ID,
                                                                    Metadata&               po_rMetaData) const;
     bool                 Set                                (TileID                  pi_ID,
                                                                    const Metadata&         po_rMetaData);

    virtual                     ~PrimitiveMetadataDir              ();

    explicit                    PrimitiveMetadataDir               ();  // Should be private, Android problem.

private:
    friend class                HTGFF::Directory;
    };


} //End namespace IDTMFile