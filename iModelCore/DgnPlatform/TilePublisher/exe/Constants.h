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

Utf8Char s_viewerHtmlPrefix[] =
R"HTML(
<!DOCTYPE html>
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
<script src="scripts/Bentley/BimWidgets.js"></script>
<script src="scripts/Bentley/BimToolbar.js"></script>
<style>
@import url(scripts/Cesium/Widgets/widgets.css);
@import url(scripts/Bentley/Bim.css);

html, body, #cesiumContainer {
width: 100%;
height: 100%;
margin: 0;
padding: 0;
overflow: hidden;
}
</style>
</head>
<body>
<div id="cesiumContainer"></div>

<script>

var viewJsonUrl = ')HTML";

// ...Insert URL to view JSON here...

Utf8Char s_viewerHtmlSuffix[] =
R"HTML(';
var viewset = new Bim.Viewset(viewJsonUrl);
Cesium.when(viewset.readyPromise).then(function() {
    var viewer = new Cesium.Viewer('cesiumContainer', viewset.createCesiumViewerOptions());
    viewer.extend(Bim.viewerInspectorMixin);

    var tileset = new Bim.Tileset(viewset, viewer);
    Cesium.when(tileset.readyPromise).then(function() {
        var toolbar = new Bim.Toolbar(viewer);
        var modelsButton = new Bim.ToolbarButton(toolbar, 'Models', Bim.createModelToggleWidget(tileset));
        var categoriesButton = new Bim.ToolbarButton(toolbar, 'Categories', Bim.createCategoryToggleWidget(tileset));
        // ###TODO: Views...
    });
});
</script>
</body>
</html>
)HTML";

