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

// printf (s_tilesetHtml, dataDirectory, rootName)
Utf8CP s_tilesetHtml =
"viewer.scene.primitives.add(new Cesium.Cesium3DTileset({\n"
"url: '%ls/%ls.json',\n"
"maximumScreenSpaceError: 2,\n"
"maximumNumberOfLoadedTiles: 1000,\n"
"debugShowBoundingVolume:false\n"
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

var curPickedObjects = null;

Cesium.when(tileset.readyPromise).then(function(tileset) {       
   %s
   viewer.camera.setView({
        "destination": new Cesium.Cartesian3(%f,%f,%f),
        "orientation": {
            direction: new Cesium.Cartesian3(%f,%f,%f),
            up: new Cesium.Cartesian3(%f,%f,%f)
        },
        "endTransform": tf
    });

   var handler = new Cesium.ScreenSpaceEventHandler(viewer.scene.canvas);
   handler.setInputAction(function(movement) {
       var pickedObjects = viewer.scene.pick(movement.endPosition);
       if (pickedObjects !== curPickedObjects) {
           var elemId = null;
           curPickedObjects = pickedObjects;
           if (Cesium.defined(curPickedObjects)) {
               elemId = pickedObjects.getProperty("element");
               if (0 == elemId)
                   elemId = null;
           }

           // Update field displaying ID of moused-over element
           var field = document.getElementById("field_elementId");
           field.firstChild.nodeValue = null != elemId ? elemId : "None";

           // Update tileset style to hilite moused-over element
           var style = undefined;
           if (null !== elemId) {
           style = new Cesium.Cesium3DTileStyle({
               "color": "(${element} === '" + elemId + "') ? color('#808080') : color('#ffffff')"
               });
           }

           tileset.style = style;
       }
   }, Cesium.ScreenSpaceEventType.MOUSE_MOVE);
});

</script>
<div style="z-index:10000; position:absolute;top:0;left:0;background-color:whitesmoke;opacity:0.5;padding:5px; margin:5px">
    <div>ElementId: <b id="field_elementId">None</b></div>
</div>
</body>
</html>)HTML";

