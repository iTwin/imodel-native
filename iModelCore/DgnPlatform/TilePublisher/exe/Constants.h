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
<script data-main="scripts/index.js" src="scripts/require.js"></script>
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
require(['scripts/Bentley/Bim'], function(Bim) {
    var viewset = new Bim.Viewset(viewJsonUrl);
    Cesium.when(viewset.readyPromise).then(function() {
        var tileset = new Bim.Tileset(viewset);
        Cesium.when(tileset.readyPromise).then(function() {
            var viewer = new Bim.Viewer('cesiumContainer', tileset, viewset.createCesiumViewerOptions());
            viewer.createDefaultToolbar();
        });
    });
});
</script>
</body>
</html>
)HTML";

