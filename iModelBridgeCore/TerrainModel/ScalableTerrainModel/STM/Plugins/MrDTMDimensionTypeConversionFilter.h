/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <TerrainModel/TerrainModel.h>

#include <ScalableTerrainModel/Import/Definitions.h>

#include <ScalableTerrainModel/Memory/Packet.h>
#include <ScalableTerrainModel/Memory/PacketAccess.h>
#include <ScalableTerrainModel/Import/Warnings.h>

#include <ScalableTerrainModel/Import/DataTypeDescription.h>

#include <ImagePP/all/h/IDTMFileDirectories/PointTypes.h>

/*---------------------------------------------------------------------------------**//**
* @description  Native MS 3D point class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <> struct IDTMFile::PointTrait<DPoint3d> : public IDTMFile::Point3dTraitMixin<DPoint3d> 
    {
    static point_type                       Create                 (coordinate_type         x,
                                                                    coordinate_type         y,
                                                                    coordinate_type         z) 
        {
        point_type pt = {x, y, z}; 
        return pt;
        }    
    };

BEGIN_BENTLEY_MRDTM_NAMESPACE



/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DimensionFilter
    {
    virtual void                                _Assign                (const Import::Packet&           pi_Src,
                                                                        Import::Packet&                 po_Dst) = 0;

    virtual void                                _Run                   () = 0;


protected:
    explicit                                    DimensionFilter ()
        {
        }

public:
    void                                        Assign                 (const Import::Packet&           pi_Src,
                                                                        Import::Packet&                 po_Dst)
        {
        _Assign(pi_Src, po_Dst);
        }

    void                                        Run                    ()
        {
        _Run();
        }


    };


/*---------------------------------------------------------------------------------**//**
* @description  
*    
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DimTypeConvSame__To__Same : public DimensionFilter
    {
    //ConstPacketProxy<byte>                      m_srcRawPacket;
    //PODPacketProxy<byte>                        m_dstRawPacket;

    virtual void                                _Assign                (const Import::Packet&           pi_Src,
                                                                        Import::Packet&                 po_Dst) override
        {
        //assert (pi_Src.Get<byte>() == po_Dst.Get<byte>());

        //pi_Src.AssignTo(m_srcRawPacket);
        //po_Dst.AssignTo(m_dstRawPacket);


        }


    virtual void                                _Run                   () override
        {
        //memcpy(m_dstRawPacket.Edit(), m_srcRawPacket.Get(), m_srcRawPacket.GetSize());
        }

public:
    class Binder : public Import::Plugin::V0::PacketBinder
        {
        virtual void                            _Bind                  (const Import::Packet&           pi_Src,
                                                                        Import::Packet&                 po_Dst) const override
            { po_Dst.BindReferToSameAs(pi_Src); }
        };

    explicit                                    DimTypeConvSame__To__Same
                                                                       (const Import::DimensionOrg&     src,
                                                                        const Import::DimensionOrg&     dst,
                                                                        const Import::FilteringConfig&  config,
                                                                        Import::Log&                    warnLog)
        {
        //TDORAY: Validate that data sizes match here
        }

    };


END_BENTLEY_MRDTM_NAMESPACE