/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/Constants.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY

#define GLTF_LINES 1
#define GLTF_LINE_STRIP 3
#define GLTF_TRIANGLES 4
#define GLTF_CULL_FACE 2884
#define GLTF_DEPTH_TEST 2929
#define GLTF_SIGNED_SHORT 5122
#define GLTF_UNSIGNED_SHORT 5123
#define GLTF_UINT32 5125
#define GLTF_FLOAT 5126
#define GLTF_RGB 6407
#define GLTF_RGBA 6408
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
#define JSON_Transform "transform"

//Shaders for display of mesh data
static std::string s_litTextureCommon = 
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
+ s_litTextureCommon +
"v_texc = a_texc;\n"
"}\n";

static std::string s_untexturedVertexShader =
"precision highp float;\n" 
+ s_litTextureCommon +
"}\n";

static std::string s_unlitTextureVertexShader =     // Used for reality meshes.
"precision highp float;\n" 
"attribute vec3 a_pos;\n"
"attribute float a_batchId;\n"
"attribute vec2 a_texc;\n"
"varying vec2 v_texc;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"void main(void) {\n"
"v_texc = a_texc;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);}\n";


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

static std::string s_unlitTextureFragmentShader =
"precision highp float;\n"
"varying vec2 v_texc;\n"  
"uniform sampler2D u_tex;\n" 
"void main(void) {gl_FragColor = texture2D(u_tex, v_texc);}\n";


// Polyline shaders.... (no lighting).
static std::string s_polylineVertexShader =
"precision highp float;\n" 
"attribute vec3 a_pos;\n"
"attribute float a_batchId;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"void main(void) {\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);}\n";

static std::string s_polylineFragmentShader =
"precision highp float;\n"
"uniform vec4 u_color;\n"
"void main(void) {gl_FragColor = vec4(u_color);}\n";

// From DgnViewMaterial.cpp
static double const s_qvExponentMultiplier  = 48.0,
                    s_qvAmbient             = 1.0,
                    s_qvFinish              = 0.9,
                    s_qvSpecular            = 0.4,
                    s_qvReflect             = 0.0,
                    s_qvRefract             = 1.0,
                    s_qvDiffuse             = 0.6;


static const WCharCP s_metadataExtension    = L"json";
static const WCharCP s_binaryDataExtension  = L"b3dm";

