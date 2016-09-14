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

// For non-geolocated models, set the camera's viewing transform 
Utf8CP s_3dOnlyViewingFrameJs =
"var boundingSphere = tileset.boundingSphere;"
"var mtx = new Cesium.Matrix4();"
"var tf = Cesium.Transforms.eastNorthUpToFixedFrame(boundingSphere.center, Cesium.Ellipsoid.WGS84, mtx);";

// For geolocated models, don't set the camera's viewing transform explicitly
Utf8CP s_geoLocatedViewingFrameJs = "var tf = undefined;";

// Adjust height to terrain depth.
Utf8CP s_adjustTerrainString = 
"var groundPoint = new Cesium.Cartesian3(%f,%f,%f);\n"
"adjustTilesetHeight();\n"
"viewer.scene.terrainProviderChanged.addEventListener(function() { adjustTilesetHeight() }, this);\n"
"viewer.scene.globe.depthTestAgainstTerrain = false;\n";

// printf (s_tilesetHtml, dataDirectory, rootName)
Utf8CP s_tilesetHtml =
"viewer.scene.primitives.add(new Cesium.Cesium3DTileset({\n"
"url: '%ls/%ls.json',\n"
"maximumScreenSpaceError: 2,\n"
"maximumNumberOfLoadedTiles: 1000,\n"
"debugShowBoundingVolume:false,\n"
"refineToVisible:true\n"
"}));\n";

// printf(s_viewerHtml, tilesetHtml,
//        viewCenterX, viewCenterY, viewCenterZ,
//        zVecX, zVecY, zVecZ,
//        yVecZ, yVecY, yVecZ)
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
<script src="Cesium/Cesium.js"></script>
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

var viewerOptions = {
    webgl: {
        alpha: true,
    },
};

var viewer = new Cesium.Viewer('cesiumContainer', {
infoBox:true,
shadows : false,
contextOptions: viewerOptions,
%s
});

viewer.extend(Cesium.viewerCesiumInspectorMixin);

// They want to disallow scripts, but need to be able to load .json tilesets...
var iframe = document.getElementsByClassName('cesium-infoBox-iframe')[0];
iframe.removeAttribute('sandbox');

var tileset=%s
%s  // Tileset height adjustment. 

Cesium.when(tileset.readyPromise).then(function(tileset) {       
   %s  // View options.

   viewer.camera.setView({
        "destination": new Cesium.Cartesian3(%f,%f,%f),
        "orientation": {
            direction: new Cesium.Cartesian3(%f,%f,%f),
            up: new Cesium.Cartesian3(%f,%f,%f)
        },
        "endTransform": tf
    });

   var fitViewButton = document.getElementById('fitViewButton');
   fitViewButton.onclick = function() { viewer.camera.viewBoundingSphere(tileset.boundingSphere, new Cesium.HeadingPitchRange (viewer.camera.heading, viewer.camera.pitch, 0.0)); }
});

function adjustTilesetHeight()
    {
    // Height adjustment. For now just try to move the mininum down to ground level (without terrain).
    var terrainProvider = viewer.terrainProvider;
    if (undefined != groundPoint &&
        undefined != terrainProvider)
        {
        var cartographic    = Cesium.Cartographic.fromCartesian (groundPoint);
        var positions = [ cartographic ];
        var currentHeight = cartographic.height;

        Cesium.sampleTerrain (terrainProvider, 15, positions).then(function() 
            { 
            var targetHeight = positions[0].height; 
            var heightDelta  = targetHeight - currentHeight;

            var deltaMagnitude = heightDelta / Cesium.Cartesian3.magnitude(groundPoint);
            var delta = new Cesium.Cartesian3();
            Cesium.Cartesian3.multiplyByScalar (groundPoint, deltaMagnitude, delta);

            tileset.modelMatrix[12] += delta.x;
            tileset.modelMatrix[13] += delta.y;
            tileset.modelMatrix[14] += delta.z;
            });
        }
    }



</script>
<div style="z-index:10000; position:absolute;top:0;left:0;background-color:whitesmoke;opacity:0.5;padding:5px; margin:5px">
    <input type="button" id="fitViewButton" value="Fit View" />
</div>
</body>
</html>)HTML";

