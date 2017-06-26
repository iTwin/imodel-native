/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/CesiumConstants.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
<link href="scripts/Bimium/bimium.css" rel="stylesheet" />
<script src="scripts/Bimium/bimium.js"></script>
<style>

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
var viewer = new Bentley.TileSetViewer({ bimiumPath: './scripts/Bimium'});
viewer.activate(viewJsonUrl);

</script>
</body>
</html>
)HTML";


