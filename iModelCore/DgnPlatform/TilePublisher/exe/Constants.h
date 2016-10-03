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
<script src="scripts/Bentley/BimWidgets.js"></script>
<script src="scripts/Bentley/BimInspectorWidget.js"></script>
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
    <!-- Have to include the SVG filters in the page because Chrome does not load the 
     filters from and external file.
     https://bugs.chromium.org/p/chromium/issues/detail?id=109212 -->

<svg id="bim-filters"
 xmlns:svg="http://www.w3.org/2000/svg"
 xmlns="http://www.w3.org/2000/svg"
 version="1.1">
<filter
   style="color-interpolation-filters:sRGB"
   id="bim-filter-icon-normal">
  <feFlood
     id="feFlood4720"
     flood-color="rgb(0,0,0)"
     result="result1" />
  <feComposite
     id="feComposite4722"
     in2="SourceGraphic"
     in="result1"
     operator="in"
     result="result4" />
  <feFlood
     id="feFlood4752"
     result="result2"
     in="SourceGraphic"
     flood-color="rgb(255,255,255)" />
  <feComposite
     id="feComposite4756"
     in2="SourceGraphic"
     in="result2"
     operator="in" />
  <feOffset
     dx="0"
     dy="1"
     id="feOffset4758"
     result="result3" />
  <feComposite
     id="feComposite4760"
     in2="result3"
     in="result4" />
</filter>
<filter
   style="color-interpolation-filters:sRGB"
   id="bim-filter-icon-hover">
  <feFlood
     id="feFlood4720"
     flood-color="rgb(0,139,225)"
     result="result1" />
  <feComposite
     id="feComposite4722"
     in2="SourceGraphic"
     in="result1"
     operator="in"
     result="result4" />
  <feFlood
     id="feFlood4752"
     result="result2"
     in="SourceGraphic"
     flood-color="rgb(255,255,255)" />
  <feComposite
     id="feComposite4756"
     in2="SourceGraphic"
     in="result2"
     operator="in" />
  <feOffset
     dx="0"
     dy="1"
     id="feOffset4758"
     result="result3" />
  <feComposite
     id="feComposite4760"
     in2="result3"
     in="result4" />
</filter>
<filter
   style="color-interpolation-filters:sRGB"
   id="bim-filter-icon-press">
  <feFlood
     id="feFlood4732"
     flood-color="rgb(255,255,255)"
     result="result1" />
  <feComposite
     id="feComposite4734"
     in2="SourceGraphic"
     in="result1"
     operator="in" />
</filter>
</svg>

<div id="cesiumContainer"></div>

<script>

var viewJsonUrl = ')HTML";

// ...Insert URL to view JSON here...

Utf8Char s_viewerHtmlSuffix[] =
R"HTML(';
var viewset = new Bim.Viewset(viewJsonUrl);
Cesium.when(viewset.readyPromise).then(function() {
    var tileset = new Bim.Tileset(viewset);
    Cesium.when(tileset.readyPromise).then(function() {
        var viewer = new Bim.Viewer('cesiumContainer', tileset, { 'cesiumViewerOptions': viewset.createCesiumViewerOptions() });
        viewer.createDefaultToolbar();
    });
});
</script>
</body>
</html>
)HTML";

