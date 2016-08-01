/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/TilePublisher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnPlatformApi.h>

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
        CantOpenBim,
        CantOpenView,
        CantWriteToBaseDirectory,
        CantCreateSubDirectory,
        ErrorWritingScene,
        ErrorWritingNode,
    };

    struct CreateParams
    {
        BeFileName      m_inputFileName;    //!< Path to the .bim
        Utf8String      m_viewName;         //!< Name of the view definition from which to publish
        BeFileName      m_outputDir;        //!< Directory in which to place the output
        Utf8String      m_tilesetName;      //!< Root name of the output tileset files

        CreateParams(BeFileNameCR inputFileName, Utf8StringCR viewName, BeFileNameCR outputDir, Utf8StringCR tilesetName)
            : m_inputFileName(inputFileName), m_viewName(viewName), m_outputDir(outputDir), m_tilesetName(tilesetName) { }

        CreateParams() { }
    };
private:
    CreateParams    m_createParams;
public:
    Publisher(CreateParams const& params);

    Status Publish();
};

END_BENTLEY_DGN_TILE3D_NAMESPACE

