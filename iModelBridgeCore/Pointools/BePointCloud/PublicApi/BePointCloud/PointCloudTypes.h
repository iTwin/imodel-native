/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudTypes.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

// 
// Common defines
//
#define UOR_PER_METER           1.0
#define TO_BOOL(x)              (0 != (x))

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/ExportMacros.h>

BEPOINTCLOUD_TYPEDEFS(IPointCloudChannel);
BEPOINTCLOUD_TYPEDEFS(IPointCloudDataQuery);
BEPOINTCLOUD_TYPEDEFS(IPointCloudQueryBuffers);
BEPOINTCLOUD_TYPEDEFS(IPointCloudChannelHandlerFilter);
BEPOINTCLOUD_TYPEDEFS(PointCloudScene);
BEPOINTCLOUD_TYPEDEFS(IPointCloudSymbologyChannel);
BEPOINTCLOUD_TYPEDEFS(PointCloudChannel);
BEPOINTCLOUD_TYPEDEFS(PointCloudChannelHandler);
BEPOINTCLOUD_TYPEDEFS(PointCloudChannelHandlerManager)
BEPOINTCLOUD_TYPEDEFS(PointCloudColorDef);
BEPOINTCLOUD_TYPEDEFS(OrientedBox);

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

struct  ChannelHandlerOnQueryCaller;
struct  ChannelHandlerOnDisplayCaller;
struct  PointCloudQueryBuffers;
struct  UserChannelQueryBuffers;
struct  PointCloudChannel;
struct  IPointCloudChannel;
struct  IPointChannelBuffer;
struct  IPointCloudQueryBuffers;
struct  IPointCloudFileQuery;
struct  IPointCloudImporter;
struct  PointCloudSelectPoints;
struct  PointCloudNeighborhoodChannelSettings;
struct  PointCloudEditSelectPoints; 
struct  IPointCloudFileEdit;
struct  IChannelsFileBuffer;
struct  ChannelHandlerCaller;
struct  IPointCloudSymbologyChannel;
struct  FilterChannelEditor;
struct  PointCloudColorDef;

typedef RefCountedPtr<ChannelHandlerOnQueryCaller>              ChannelHandlerOnQueryCallerPtr;
typedef RefCountedPtr<ChannelHandlerOnDisplayCaller>            ChannelHandlerOnDisplayCallerPtr;
typedef RefCountedPtr<PointCloudQueryBuffers>                   PointCloudQueryBuffersPtr;
typedef RefCountedPtr<UserChannelQueryBuffers>                  UserChannelQueryBuffersPtr;
typedef RefCountedPtr<PointCloudChannel>                        PointCloudChannelPtr;
typedef RefCountedPtr<IPointCloudChannel>                       IPointCloudChannelPtr;
typedef RefCountedPtr<IPointChannelBuffer>                      IPointChannelBufferPtr;
typedef RefCountedPtr<IPointCloudQueryBuffers>                  IPointCloudQueryBuffersPtr;
typedef RefCountedPtr<IPointCloudFileQuery>                     IPointCloudFileQueryPtr;
typedef RefCountedPtr<IPointCloudImporter>                      IPointCloudImporterPtr;
typedef RefCountedPtr<PointCloudSelectPoints>                   PointCloudSelectPointsPtr;
typedef RefCountedPtr<PointCloudNeighborhoodChannelSettings>    PointCloudNeighborhoodChannelSettingsPtr;
typedef RefCountedPtr<PointCloudEditSelectPoints>               PointCloudEditSelectPointsPtr;
typedef RefCountedPtr<IPointCloudFileEdit>                      IPointCloudFileEditPtr;
typedef RefCountedPtr<IChannelsFileBuffer>                      IChannelsFileBufferPtr;
typedef RefCountedPtr<ChannelHandlerCaller>                     ChannelHandlerCallerPtr;
typedef RefCountedPtr<IPointCloudSymbologyChannel>              IPointCloudSymbologyChannelPtr;
typedef RefCountedPtr<FilterChannelEditor>                      FilterChannelEditorPtr;
typedef RefCountedPtr<PointCloudColorDef>                       PointCloudColorDefPtr;

/*=================================================================================**//**
* \addtogroup Pointcloud
*/
//@{

/*__PUBLISH_SECTION_END__*/
typedef bvector<IPointCloudChannelP>       IPointCloudChannelVector;
typedef IPointCloudChannelVector const&    IPointCloudChannelVectorCR;

/*__PUBLISH_SECTION_START__*/
typedef bmap<WString, IPointCloudChannelPtr>     IPointCloudChannelPtrMap;
typedef IPointCloudChannelPtrMap&                IPointCloudChannelPtrMapR;
typedef IPointCloudChannelPtrMap const&          IPointCloudChannelPtrMapCR;

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PointCloudStatus
    {
    Loaded       = 1,
    FailedToLoad = 2,
    FileNotFound = 3,
    };

//@}

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
