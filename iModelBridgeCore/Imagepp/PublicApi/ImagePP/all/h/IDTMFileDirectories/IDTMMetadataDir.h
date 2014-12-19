//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMMetadataDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <Imagepp/all/h/HTGFFDirectory.h>
#include <Imagepp/all/h/HPUPacket.h>
#include <Imagepp/all/h/IDTMTypes.h>

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
    _HDLLg static uint32_t      s_GetVersion                       ();

    _HDLLg bool                 GetSchema                          (Schema&                 po_rSchema,
                                                                    SchemaType&             po_rType) const;
    _HDLLg bool                 SetSchema                          (const Schema&           pi_rSchema,
                                                                    SchemaType              pi_Type);

    _HDLLg bool                 Get                                (Metadata&               po_rMetaData) const;
    _HDLLg bool                 Set                                (const Metadata&         pi_rMetaData);

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
    _HDLLg bool                 Get                                (TileID                  pi_ID,
                                                                    Metadata&               po_rMetaData) const;
    _HDLLg bool                 Set                                (TileID                  pi_ID,
                                                                    const Metadata&         po_rMetaData);

    virtual                     ~PrimitiveMetadataDir              ();

    explicit                    PrimitiveMetadataDir               ();  // Should be private, Android problem.

private:
    friend class                HTGFF::Directory;
    };


} //End namespace IDTMFile