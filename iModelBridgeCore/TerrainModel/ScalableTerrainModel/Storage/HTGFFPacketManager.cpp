//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFPacketManager.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFPacketManager.h>
#include <STMInternal/Storage/HTGFFPacketIdIter.h>

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& PacketManagerBase::GetType () const
    {
    return GetFile().GetDataType();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Compression& PacketManagerBase::GetCompression () const
    {
    return GetFile().GetDataCompressType();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double PacketManagerBase::GetCompressionRatio () const
    {
    return GetFile().GetCompressionRatio();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter PacketManagerBase::beginId () const
    {
    return GetFile().PacketIDsBegin();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter PacketManagerBase::endId () const
    {
    return GetFile().PacketIDsEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter PacketManagerBase::FindIdIterFor (PacketID pi_ID) const
    {
    return GetFile().FindPacketIterFor(pi_ID);
    }


} //End namespace HTGFF