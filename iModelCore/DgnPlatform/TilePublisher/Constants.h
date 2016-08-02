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

Utf8String s_texturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"attribute vec2 a_texc;\n"
#if USE_BATCH_TABLE
"attribute float a_batchId;\n"
#endif
"varying vec2 v_texc;\n"
"void main(void) {\n"
"v_texc = a_texc;\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

Utf8String s_texturedFragShader =
"precision highp float;\n"
"varying vec2 v_texc;\n"
"uniform sampler2D u_tex;\n"
"void main(void) {\n"
"gl_FragColor = texture2D(u_tex, v_texc);\n"
"}\n";

Utf8String s_untexturedVertexShader =
"precision highp float;\n"
"attribute vec3 a_pos;\n"
"attribute vec3 a_n;\n"
#if USE_BATCH_TABLE
"attribute float a_batchId;\n"
#endif
"uniform mat4 u_mv;\n"
"uniform mat4 u_proj;\n"
"uniform mat3 u_nmx;\n"
"varying vec3 v_n;\n"
"void main(void) {\n"
"v_n = normalize(u_nmx * a_n);\n"
"gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);\n"
"}\n";

Utf8String s_untexturedFragShader =
"precision highp float;\n"
"varying vec3 v_n;\n"
"uniform vec4 u_color;\n"
"void main(void) {\n"
"vec3 n = normalize(v_n);\n"
"gl_FragColor = vec4(u_color.rgb * n.z, u_color.a);\n"
"}\n";

