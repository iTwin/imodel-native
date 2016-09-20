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
<script src="scripts/Bentley/BimStyler.js"></script>
<script src="scripts/Bentley/BimToolbar.js"></script>
<script src="scripts/Bentley/BimViews.js"></script>
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
var json = Bim.requestJson(viewJsonUrl, function(err, json)
    {
    if (Cesium.defined(err)) 
        {
        console.log("Failed to load " + viewJsonUrl); 
        return;
        } 
    else
        {
        var viewer = new Cesium.Viewer('cesiumContainer', Bim.createCesiumViewerOptions(json.geolocated));
        viewer.extend(Bim.viewerInspectorMixin);
        Bim.fixupSandboxAttributes();
        var tileset = Bim.loadTileset(viewer, json);

        Bim.CreateViewCategoryAndModelWidgets(viewer, tileset, json);
        }
    });
</script>
</body>
</html>
)HTML";

