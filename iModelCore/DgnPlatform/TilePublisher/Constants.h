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


//Shaders for display of mesh data
static std::string s_textureShaderCommon = 
"attribute vec3 a_pos;\n"
"attribute vec3 a_n;\n"
"attribute float a_batchId;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform mat3 u_nmx;\n"
"varying vec3 v_n;\n"
"varying vec3 v_pos;\n"
"void main(void) {\n"
"v_n = normalize(u_nmx * a_n);\n"
"v_pos = a_pos;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n";



static std::string s_texturedVertexShader =
"precision highp float;\n" 
"attribute vec2 a_texc;\n"
"varying vec2 v_texc;\n"
+ s_textureShaderCommon +
"v_texc = a_texc;\n"
"}\n";

static std::string s_untexturedVertexShader =
"precision highp float;\n" 
+ s_textureShaderCommon +
"}\n";

static std::string s_fragShaderCommon = 
"varying vec3 v_n;\n"
"varying vec3 v_pos;\n"
"uniform float u_specularExponent;\n"
"uniform vec3 u_specularColor;\n"
"void main(void) {\n"
"vec3 n = normalize(v_n);\n"
"vec3 toEye = normalize (-v_pos);\n"
"n = faceforward(n, vec3(0.0, 0.0, 1.0), -n);\n"
"float diffuseIntensity = .3 * n.z + .7 * czm_getLambertDiffuse(czm_sunDirectionEC, n);\n"
"vec3 toReflectedLight = reflect(-czm_sunDirectionEC, n);\n"
"float specular = max(dot(toReflectedLight, toEye), 0.0);\n"
"float specularIntensity=pow (specular, u_specularExponent);\n";

static std::string s_untexturedFragShader = 
"precision highp float;\n"
"uniform vec4 u_color;\n"
+ s_fragShaderCommon + 
"vec3 color = (u_color.rgb * diffuseIntensity) + (u_specularColor * specularIntensity);\n"
"gl_FragColor = vec4(color, u_color.a);\n"
"}\n";



static std::string s_texturedFragShader =
"precision highp float;\n"
"varying vec2 v_texc;\n"  
"uniform sampler2D u_tex;\n" 
+ s_fragShaderCommon + 
"vec4 textureColor = texture2D(u_tex, v_texc);\n"
"if (0.0 == textureColor.a) discard;\n"
"vec3 color = (textureColor.rgb * diffuseIntensity) + (u_specularColor * specularIntensity);\n"
"gl_FragColor = vec4(color.rgb * diffuseIntensity, textureColor.a);\n"
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

           var elemId = "None";
           curPickedObjects = pickedObjects;
           if (Cesium.defined(curPickedObjects)) {
               curPickedColor = curPickedObjects.color;
               curPickedObjects.color = highlightColor;
               elemId = pickedObjects.getProperty("element");
           } else {
               curPickedColor = null;
           }

           var field = document.getElementById("field_elementId");
           field.firstChild.nodeValue = elemId;
       }
   }, Cesium.ScreenSpaceEventType.MOUSE_MOVE);
});

</script>
<div style="z-index:10000; position:absolute;top:0;left:0;background-color:whitesmoke;opacity:0.5;padding:5px; margin:5px">
    <div>ElementId: <b id="field_elementId">None</b></div>
</div>
</body>
</html>)HTML";



// From DgnViewMaterial.cpp
static double const s_qvExponentMultiplier  = 48.0,
                    s_qvAmbient             = 1.0,
                    s_qvFinish              = 0.9,
                    s_qvSpecular            = 0.4,
                    s_qvReflect             = 0.0,
                    s_qvRefract             = 1.0,
                    s_qvDiffuse             = 0.6;



