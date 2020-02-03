/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>

USING_NAMESPACE_BENTLEY

static const char s_gltfMagic[]              = "glTF";
static const char s_b3dmMagic[]              = "b3dm";
static const char s_compositeTileMagic[]     = "cmpt";
static const char s_instanced3dMagic[]       = "i3dm";
static const char s_pointCloudMagic[]        = "pnts";

static const uint32_t s_b3dmVersion          = 1;
static const uint32_t s_gltfVersion          = 1;
static const uint32_t s_gltfSceneFormat      = 0;
static const uint32_t s_compositeTileVersion = 1; 
static const uint32_t s_instanced3dVersion   = 1;
static const uint32_t s_pointCloudVersion    = 1;


#define GLTF_LINES 1
#define GLTF_LINE_STRIP 3
#define GLTF_TRIANGLES 4
#define GLTF_CULL_FACE 2884
#define GLTF_DEPTH_TEST 2929
#define GLTF_SIGNED_BYTE 0x1400
#define GLTF_UNSIGNED_BYTE 0x1401
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
#define GLTF_INT_VEC2 0x8b53
#define GLTF_INT_VEC3 0x8b54
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

static std::string  s_shaderPrecision        = "precision highp float;\n"; 

static std::string  s_batchIdShaderAttribute = "attribute float a_batchId;\n";

static std::string s_octDecode = 
"float gltf_signNotZero(float value) { return value >= 0.0 ? 1.0 : -1.0; }\n"
"vec2 gltf_signNotZero(vec2 value) { return vec2(gltf_signNotZero(value.x), gltf_signNotZero(value.y));}\n"
" vec3 gltf_octDecode(vec2 encoded)\n"
"  {\n"
"     encoded = encoded / 255.0 * 2.0 - 1.0;\n"
"     vec3 v = vec3(encoded.x, encoded.y, 1.0 - abs(encoded.x) - abs(encoded.y));\n"
"     if (v.z < 0.0)\n"
"         v.xy = (1.0 - abs(v.yx)) * gltf_signNotZero(v.xy);\n"
"   return normalize(v);\n"
"  }\n";

//Shaders for display of mesh data
static std::string s_litVertexCommon = 
s_octDecode +
"attribute vec3 a_pos;\n"
"attribute vec2 a_n;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform mat3 u_nmx;\n"
"varying vec3 v_n;\n"
"varying vec3 v_pos;\n"
"void main(void) {\n"
"vec4 pos = u_mv * vec4(a_pos,1.0);\n"
"vec3 decodeNormal = gltf_octDecode(a_n);\n"
"v_n = u_nmx * decodeNormal;\n"
"v_pos = pos.xyz;\n"
"gl_Position = u_proj * pos;\n";

static std::string s_texturedVertexShader =
"attribute vec2 a_texc;\n"
"varying vec2 v_texc;\n"
+ s_litVertexCommon +
"v_texc = a_texc;\n"
"}\n";

static std::string s_untexturedVertexShader =  
s_litVertexCommon +
"}\n";

static std::string s_unlitTextureVertexShader =     // Used for reality meshes.
"attribute vec3 a_pos;\n"
"attribute vec2 a_texc;\n"
"varying vec2 v_texc;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"void main(void) {\n"
"v_texc = a_texc;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);}\n";


static std::string s_computeLighting = 
"void computeSingleLight (inout float diffuse, inout float specular, vec3 normal, vec3 toEye, vec3 lightDir, float lightIntensity, float specularExponent)\n"
"{\n"
"diffuse += lightIntensity * max(dot(normal,lightDir), 0.0);\n"
"vec3 toReflectedLight = reflect(lightDir, normal);\n"
"float specularDot = max(dot(toReflectedLight, toEye), 0.0);\n"
"specular += lightIntensity * pow(specularDot, specularExponent);\n"
"}"
"vec4 computeLighting (vec3 normalIn, vec3 position, float specularExponent, vec3 specularColor, vec4 inputColor)\n"
"{\n"
"vec3 normal = normalize(normalIn);\n"
"vec3 toEye = normalize (position);\n"
"normal = faceforward(normal, vec3(0.0, 0.0, 1.0), -normal);\n"
"float ambient = 0.2;\n"
"vec3 lightPos1 = vec3(-500, -200.0, 0.0);\n"
"vec3 lightPos2 = vec3(500, 200.0, 0.0);\n"
"vec3 lightDir1 = normalize(lightPos1 - position);\n"
"vec3 lightDir2 = normalize(lightPos2 - position);\n"
"float diffuseIntensity = 0.0;\n"
"float specularIntensity = 0.0;\n"
"computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, lightDir1, .35, specularExponent);\n"
"computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, lightDir2, .45, specularExponent);\n"
"computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, czm_sunDirectionEC, .25, specularExponent);\n"
"vec3   diffuseAndAmbient = inputColor.rgb * min (1.0, diffuseIntensity + ambient);\n"
"vec3   specular = specularColor.rgb * min (1.0, specularIntensity);\n"
"return vec4 (diffuseAndAmbient + specular, inputColor.a);\n"
"}\n";



static std::string s_untexturedFragShader = 
"uniform vec4 u_color;\n"
"varying vec3 v_n;\n"
"varying vec3 v_pos;\n"
"uniform float u_specularExponent;\n"
"uniform vec3 u_specularColor;\n"
+ s_computeLighting + 
"void main(void) {\n"
"gl_FragColor = computeLighting (v_n, v_pos, u_specularExponent, u_specularColor, u_color);}\n";

static std::string s_texturedFragShader =
"varying vec2 v_texc;\n"  
"uniform sampler2D u_tex;\n" 
"varying vec3 v_n;\n"
"varying vec3 v_pos;\n"
"uniform float u_specularExponent;\n"
"uniform vec3 u_specularColor;\n"
+ s_computeLighting +    
"void main(void) {\n"
"vec4 textureColor = texture2D(u_tex, v_texc);\n"
"if (0.0 == textureColor.a) discard;\n"
"gl_FragColor = computeLighting (v_n, v_pos, u_specularExponent, u_specularColor, textureColor); }\n";

static std::string s_unlitTextureFragmentShader =
"varying vec2 v_texc;\n"  
"uniform sampler2D u_tex;\n" 
"void main(void) {gl_FragColor = texture2D(u_tex, v_texc);}\n";


// Unlit shaders....
static std::string s_unlitVertexShader =
"attribute vec3 a_pos;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"void main(void) {\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);}\n";

static std::string s_unlitFragmentShader =
"uniform vec4 u_color;\n"
"void main(void) {gl_FragColor = vec4(u_color);}\n";

// Polyline shaders.
static std::string s_polylineVertexShader =
"attribute vec3 a_pos;\n"
"attribute vec3 a_direction;\n"
"attribute vec3 a_vertexDelta;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform float u_width;\n"
"varying vec2 v_texture;\n"
"varying float v_length;\n"
"varying float v_widthScale;\n"
"void main(void) {\n"
"v_length = length(a_direction);\n"
"float pixelWidthRatio = 2. / (czm_viewport.z * czm_projection[0][0]);\n"
"vec3 toEye = czm_inverseNormal * vec3(0.0, 0.0, 1.0);\n"
"vec4 projection = czm_modelViewProjection * vec4(a_pos, 1.0);\n"
"float width = projection.w * u_width * pixelWidthRatio;\n"
"v_widthScale = 1.0 / width;\n"
"vec3 extrusion = normalize(cross (a_direction, toEye));\n"
"v_texture.x = a_vertexDelta.z * v_length + width * a_vertexDelta.x;\n"
"v_texture.y = a_vertexDelta.y;\n"
"gl_Position = czm_modelViewProjection * vec4(a_pos + (width * a_vertexDelta.x / v_length) * a_direction + width * a_vertexDelta.y * extrusion, 1.0);\n"
"}\n";


static std::string s_polylineFragmentShader =
"uniform float u_feather;\n"

"bool computePolylineColor (vec4 color, float distance)\n"
"    {\n"
"    if (distance > 1.0)\n"
"        return false;\n"
"    float alpha = distance > (1.0 - u_feather) ?  ((1.0 - distance)/u_feather) : 1.0;\n"
"    gl_FragColor = vec4 (color.rgb, alpha);\n"
"    return true;\n" 
"    }\n"
"uniform vec4 u_color;\n"
"varying vec2 v_texture;\n"
"varying float v_length;\n"
"varying float v_widthScale;\n"
"void main()\n"
" {\n"
" float centerDistance;\n"
"\n"     
" if (v_texture.x < 0.0)\n"
"   {\n"
"   float xTexture = -v_texture.x * v_widthScale;\n"
"   centerDistance = sqrt (xTexture * xTexture + v_texture.y * v_texture.y);\n"
"   }\n"
"else if (v_texture.x > v_length)\n"
"   {\n"
"   float xTexture = (v_texture.x - v_length) * v_widthScale;\n"
"   centerDistance = sqrt (xTexture * xTexture + v_texture.y * v_texture.y);\n"
"   }\n"
"else\n"
"   {\n"
"   centerDistance = abs (v_texture.y);\n"
"   }\n"
"if (!computePolylineColor (u_color, centerDistance))\n"
"       discard;\n"
"}\n";


// From DgnViewMaterial.cpp
static double const s_qvExponentMultiplier  = 48.0,
                    s_qvAmbient             = 1.0,
                    s_qvFinish              = 0.9,
                    s_qvSpecular            = 0.4,
                    s_qvReflect             = 0.0,
                    s_qvRefract             = 1.0,
                    s_qvDiffuse             = 0.6;


static const WCharCP s_metadataExtension        = L"json";
static const WCharCP s_binaryDataExtension  = L"b3dm";






