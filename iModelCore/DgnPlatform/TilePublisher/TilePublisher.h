/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY

#define BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE BEGIN_BENTLEY_RENDER_NAMESPACE namespace Tile3d {
#define END_BENTLEY_DGN_TILE3D_NAMESPACE } END_BENTLEY_RENDER_NAMESPACE

BEGIN_BENTLEY_DGN_TILE3D_NAMESPACE

//=======================================================================================
//! Publishes the contents of a DgnDb view as a Cesium tileset.
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct Publisher
{
    enum class Status
    {
        Success = SUCCESS,
        NoGeometry,
        Aborted,
        CantWriteToBaseDirectory,
        CantCreateSubDirectory,
        ErrorWritingScene,
        ErrorWritingNode,
    };
private:
    ViewControllerR     m_viewController;
    BeFileName          m_outputDir;
    WString             m_rootName;
public:
    Publisher(ViewControllerR viewController, BeFileNameCR outputDir, WStringCR tilesetName) : m_viewController(viewController), m_outputDir(outputDir), m_rootName(tilesetName) { }

    Status Publish();
};

END_BENTLEY_DGN_TILE3D_NAMESPACE

