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
<script src="scripts/Bentley/BimTiles.js"></script>
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
var viewer = new Cesium.Viewer('cesiumContainer', BimTiles.createCesiumViewerOptions(view));
viewer.extend(Cesium.viewerCesiumInspectorMixin);

BimTiles.fixupSandboxAttributes();

var tileset = BimTiles.loadTileset(viewer, view, function() {
   var fitViewButton = document.getElementById('fitViewButton');
   fitViewButton.onclick = function() {
       viewer.camera.viewBoundingSphere(tileset.boundingSphere, new Cesium.HeadingPitchRange (viewer.camera.heading, viewer.camera.pitch, 0.0));
   }
});

</script>
<div style="z-index:10000; position:absolute;top:0;left:0;padding:5px; margin:5px">
    <input type="button" id="fitViewButton" value="Fit View" />
</div>
</body>
</html>)HTML";

