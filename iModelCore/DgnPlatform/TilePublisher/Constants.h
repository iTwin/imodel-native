/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Constants.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY

#define USE_BATCH_TABLE 1

#define GLTF_TRIANGLES 4
#define GLTF_CULL_FACE 2884
#define GLTF_DEPTH_TEST 2929
#define GLTF_UNSIGNED_SHORT 5123
#define GLTF_UINT32 5125
#define GLTF_FLOAT 5126
#define GLTF_RGB 6407
#define GLTF_LINEAR 9729
#define GLTF_LINEAR_MIPMAP_LINEAR 9987
#define GLTF_CLAMP_TO_EDGE 33071
#define GLTF_ARRAY_BUFFER 34962
#define GLTF_ELEMENT_ARRAY_BUFFER 34963
#define GLTF_FRAGMENT_SHADER 35632
#define GLTF_VERTEX_SHADER 35633
#define GLTF_FLOAT_VEC2 35664
#define GLTF_FLOAT_VEC3 35665
#define GLTF_FLOAT_VEC4 35666
#define GLTF_FLOAT_MAT3 35675
#define GLTF_FLOAT_MAT4 35676
#define GLTF_SAMPLER_2D 35678

#define JSON_Root "root"
#define JSON_GeometricError "geometricError"
#define JSON_BoundingVolume "boundingVolume"
#define JSON_Sphere "sphere"
#define JSON_Box "box"
#define JSON_Children "children"
#define JSON_Content "content"

Utf8String s_texturedFragShader =
"precision highp float;\n"
"varying vec2 v_texc;\n"
"uniform sampler2D u_tex;\n"
"void main(void) {\n"
"gl_FragColor = texture2D(u_tex, v_texc);\n"
"}\n";

#if USE_BATCH_TABLE

Utf8String s_texturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"attribute vec2 a_texc;\n"
"attribute float a_batchId;\n"
"varying vec2 v_texc;\n"
"void main(void) {\n"
"v_texc = a_texc;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

Utf8String s_untexturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"attribute vec3 a_n;\n"
"attribute float a_batchId;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform mat3 u_nmx;\n"
"varying vec3 v_n;\n"
"void main(void) {\n"
"v_n = normalize(u_nmx * a_n);\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

#else

Utf8String s_texturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"attribute vec2 a_texc;\n"
"varying vec2 v_texc;\n"
"void main(void) {\n"
"v_texc = a_texc;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

Utf8String s_untexturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"attribute vec3 a_n;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform mat3 u_nmx;\n"
"varying vec3 v_n;\n"
"void main(void) {\n"
"v_n = normalize(u_nmx * a_n);\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

#endif

Utf8String s_untexturedFragShader =
"precision highp float;\n"
"varying vec3 v_n;\n"
"uniform vec4 u_color;\n"
"void main(void) {\n"
"vec3 n = normalize(v_n);\n"
"gl_FragColor = vec4(u_color.rgb * n.z, u_color.a);\n"
"}\n";

// printf(s_viewerHtml, dataDirectoryName, tilesetName,
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

var viewerOptions = {
    webgl: {
        alpha: true,
    },
};

var viewer = new Cesium.Viewer('cesiumContainer', {
infoBox:true,
scene3DOnly:true,
shadows : false,
contextOptions: viewerOptions,
globe: false,
skyBox: false,
skyAtmosphere: false
});

viewer.extend(Cesium.viewerCesiumInspectorMixin);

// They want to disallow scripts, but need to be able to load .json tilesets...
var iframe = document.getElementsByClassName('cesium-infoBox-iframe')[0];
iframe.removeAttribute('sandbox');

var tileset = viewer.scene.primitives.add(new Cesium.Cesium3DTileset({
url: '%ls/%ls.json',
maximumScreenSpaceError: 2,
maximumNumberOfLoadedTiles: 1000,
debugShowBoundingVolume:false
}));

var highlightColor = new Cesium.Color(0.5, 0.5, 0.5, 1);
var curPickedObjects = null;
var curPickedColor = null;

Cesium.when(tileset.readyPromise).then(function(tileset) {       
   var boundingSphere = tileset.boundingSphere;
   viewer.camera.viewBoundingSphere(boundingSphere); 
   viewer.camera.setView({
        "destination": new Cesium.Cartesian3(%f,%f,%f),
        "orientation": {
            direction: new Cesium.Cartesian3(%f,%f,%f),
            up: new Cesium.Cartesian3(%f,%f,%f)
        }
    });

   var handler = new Cesium.ScreenSpaceEventHandler(viewer.scene.canvas);
   handler.setInputAction(function(movement) {
       var pickedObjects = viewer.scene.pick(movement.endPosition);
       if (pickedObjects !== curPickedObjects) {
           if (Cesium.defined(curPickedObjects)) {
               curPickedObjects.color = curPickedColor;
           }

           curPickedObjects = pickedObjects;
           if (Cesium.defined(curPickedObjects)) {
               curPickedColor = curPickedObjects.color;
               curPickedObjects.color = highlightColor;
           } else {
               curPickedColor = null;
           }
       }
   }, Cesium.ScreenSpaceEventType.MOUSE_MOVE);
});

</script>
</body>
</html>)HTML";




