/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/exe/Constants.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY

// printf(s_viewerHtml, viewJson);
Utf8CP s_viewerHtml =
R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<!-- Use correct character set. -->
<meta charset="utf-8">
<!-- Tell IE to use the latest, best version. -->
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<!-- Make the application on mobile take up the full browser screen and disable user scaling. -->
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, minimum-scale=1, user-scalable=no">
<title>Cesium 3D Tiles generated from Bentley MicroStation</title>
<script src="scripts/Cesium/Cesium.js"></script>
<script src="scripts/Bentley/Bim.js"></script>
<script src="scripts/Bentley/BimInspectorWidget.js"></script>
<style>
@import url(Cesium/Widgets/widgets.css);

html, body, #cesiumContainer {
width: 100%%;
height: 100%%;
margin: 0;
padding: 0;
overflow: hidden;
}
</style>
</head>
<body>
<div id="cesiumContainer"></div>

<script>

var view = %s;
var viewer = new Cesium.Viewer('cesiumContainer', Bim.createCesiumViewerOptions(view));
viewer.extend(Bim.viewerInspectorMixin);

Bim.fixupSandboxAttributes();

Bim.loadTileset(viewer, view);

</script>
</body>
</html>)HTML";

