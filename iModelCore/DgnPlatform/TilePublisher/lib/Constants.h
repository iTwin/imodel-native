/*-------------------------------------------------------------------------------1-------+
|
|     $Source: TilePublisher/lib/Constants.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
#define GLTF_NEAREST 0x2600
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

static std::string s_octDecode = R"RAW_STRING(
    float gltf_signNotZero(float value) { return value >= 0.0 ? 1.0 : -1.0; }
    vec2 gltf_signNotZero(vec2 value) { return vec2(gltf_signNotZero(value.x), gltf_signNotZero(value.y));}
    vec3 gltf_octDecode(vec2 encoded)
        {
        encoded = encoded / 255.0 * 2.0 - 1.0;
        vec3 v = vec3(encoded.x, encoded.y, 1.0 - abs(encoded.x) - abs(encoded.y));
        if (v.z < 0.0)
         v.xy = (1.0 - abs(v.yx)) * gltf_signNotZero(v.xy);
        return normalize(v);
        }
)RAW_STRING";

//Shaders for display of mesh data
static std::string s_litVertexCommon = 
s_octDecode + R"RAW_STRING(
    attribute vec3 a_pos;
    attribute vec2 a_n;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
    uniform mat3 u_nmx;
    varying vec3 v_n;
    varying vec3 v_pos;

    void main(void)
        {
        vec4 pos = u_mv * vec4(a_pos,1.0);
        vec3 decodeNormal = gltf_octDecode(a_n);
        v_n = u_nmx * decodeNormal;
        v_pos = pos.xyz;
        gl_Position = u_proj * pos;
)RAW_STRING";

static std::string s_texturedVertexShader = R"RAW_STRING(
    attribute vec2 a_texc;
    varying vec2 v_texc;
)RAW_STRING"
+ s_litVertexCommon + R"RAW_STRING(
    v_texc = a_texc;
    }
)RAW_STRING";

static std::string s_computeIndexedColor[3] =
    {
    R"RAW_STRING(
        uniform vec4 u_color;
        varying vec4 v_color;
        vec4 computeColor() { return u_color; }
    )RAW_STRING",

    R"RAW_STRING(
        attribute float a_colorIndex;
        uniform vec2 u_texStep;
        uniform sampler2D u_tex;
        varying vec4 v_color;

        vec2 computeColorSt(float colorIndex)
            {
            float stepX = u_texStep.x;
            float centerX = u_texStep.y;
            return vec2(centerX + (colorIndex * stepX), 0.5);
            }

        vec4 computeColor() { return texture2D(u_tex, computeColorSt(a_colorIndex)); }
    )RAW_STRING",

    R"RAW_STRING(
        attribute float a_colorIndex;
        uniform float u_texWidth;
        uniform vec4 u_texStep;
        uniform sampler2D u_tex;
        varying vec4 v_color;

        vec2 computeColorSt(float colorIndex)
            {
            float stepX = u_texStep.x;
            float centerX = u_texStep.y;
            float stepY = u_texStep.z;
            float centerY = u_texStep.w;

            float xId = mod(colorIndex, u_texWidth);
            float yId = floor(colorIndex / u_texWidth);
            return vec2(centerX + (xId * stepX), 1.0 - (centerY + (yId * stepY)));
            }

        vec4 computeColor() { return texture2D(u_tex, computeColorSt(a_colorIndex)); }
    )RAW_STRING"
    };

static std::string s_assignIndexedColor = R"RAW_STRING(
        v_color = computeColor();
        }
)RAW_STRING";

static std::string s_untexturedVertexShaders[3] =
    {
    s_computeIndexedColor[0] + s_litVertexCommon + s_assignIndexedColor,
    s_computeIndexedColor[1] + s_litVertexCommon + s_assignIndexedColor,
    s_computeIndexedColor[2] + s_litVertexCommon + s_assignIndexedColor,
    };

// Used for reality meshes.
static std::string s_unlitTextureVertexShader = R"RAW_STRING(
    attribute vec3 a_pos;
    attribute vec2 a_texc;
    varying vec2 v_texc;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
    void main(void)
        {
        v_texc = a_texc;
        gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);
        }
)RAW_STRING";

static std::string s_computeLighting = R"RAW_STRING(
    void computeSingleLight (inout float diffuse, inout float specular, vec3 normal, vec3 toEye, vec3 lightDir, float lightIntensity, float specularExponent)
        {
        diffuse += lightIntensity * max(dot(normal,lightDir), 0.0);
        vec3 toReflectedLight = reflect(lightDir, normal);
        float specularDot = max(dot(toReflectedLight, toEye), 0.0);
        specular += lightIntensity * pow(specularDot, specularExponent);
        }

    vec4 computeLighting (vec3 normalIn, vec3 position, float specularExponent, vec3 specularColor, vec4 inputColor)
        {
        vec3 normal = normalize(normalIn);
        vec3 toEye = normalize (position);
        normal = faceforward(normal, vec3(0.0, 0.0, 1.0), -normal);

        float ambient = 0.2;
        vec3 lightPos1 = vec3(-500, -200.0, 0.0);
        vec3 lightPos2 = vec3(500, 200.0, 0.0);
        vec3 lightDir1 = normalize(lightPos1 - position);
        vec3 lightDir2 = normalize(lightPos2 - position);

        float diffuseIntensity = 0.0;
        float specularIntensity = 0.0;

        computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, lightDir1, .35, specularExponent);
        computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, lightDir2, .45, specularExponent);
        computeSingleLight (diffuseIntensity, specularIntensity, normal, toEye, czm_sunDirectionEC, .25, specularExponent);

        vec3   diffuseAndAmbient = inputColor.rgb * min (1.0, diffuseIntensity + ambient);
        vec3   specular = specularColor.rgb * min (1.0, specularIntensity);

        return vec4 (diffuseAndAmbient + specular, inputColor.a);
        }
)RAW_STRING";


static std::string s_untexturedFragShader = R"RAW_STRING(
    varying vec4 v_color;
    varying vec3 v_n;
    varying vec3 v_pos;
    uniform float u_specularExponent;
    uniform vec3 u_specularColor;
)RAW_STRING"
+ s_computeLighting + R"RAW_STRING(
    void main(void)
        {
        gl_FragColor = computeLighting (v_n, v_pos, u_specularExponent, u_specularColor, v_color);
        }
)RAW_STRING";

static std::string s_texturedFragShader = R"RAW_STRING(
    varying vec2 v_texc;  
    uniform sampler2D u_tex; 
    varying vec3 v_n;
    varying vec3 v_pos;
    uniform float u_specularExponent;
    uniform vec3 u_specularColor;
)RAW_STRING"
+ s_computeLighting + R"RAW_STRING(
    void main(void)
        {
        vec4 textureColor = texture2D(u_tex, v_texc);
        if (0.0 == textureColor.a) discard;
        gl_FragColor = computeLighting (v_n, v_pos, u_specularExponent, u_specularColor, textureColor);
        }
)RAW_STRING";

static std::string s_unlitTextureFragmentShader = R"RAW_STRING(
    varying vec2 v_texc;  
    uniform sampler2D u_tex; 
    void main(void)
        {
        gl_FragColor = texture2D(u_tex, v_texc);
        }
)RAW_STRING";

// Unlit shaders....
static std::string s_unlitVertexCommon = R"RAW_STRING(
    attribute vec3 a_pos;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
)RAW_STRING";

static std::string s_unlitVertexMain = R"RAW_STRING(
    void main()
        {
        gl_Position = u_proj * u_mv * vec4(a_pos, 1.0);
        v_color = computeColor();
        }
)RAW_STRING";

static std::string s_unlitVertexShaders[3] =
    {
    s_unlitVertexCommon + s_computeIndexedColor[0] + s_unlitVertexMain,
    s_unlitVertexCommon + s_computeIndexedColor[1] + s_unlitVertexMain,
    s_unlitVertexCommon + s_computeIndexedColor[2] + s_unlitVertexMain,
    };

static std::string s_unlitFragmentShader = R"RAW_STRING(
    varying vec4 v_color;
    void main(void)
        {
        gl_FragColor = vec4(v_color);
        }
)RAW_STRING";

// Polyline shaders.
static std::string s_tesselatedPolylineVertexCommon = R"RAW_STRING(
    attribute vec3 a_pos;
    attribute vec3 a_direction;
    attribute vec3 a_vertexDelta;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
    uniform float u_width;
    varying vec2 v_texture;
    varying float v_length;
    varying float v_widthScale;

    void main(void)
        {
        v_length = length(a_direction);
        float pixelWidthRatio = 2. / (czm_viewport.z * czm_projection[0][0]);
        vec4 projection = czm_modelViewProjection * vec4(a_pos, 1.0);
        float width = projection.w * u_width * pixelWidthRatio;
        v_widthScale = 1.0 / width;

        vec3 toEye = czm_inverseNormal * vec3(0.0, 0.0, 1.0);
        vec3 extrusion = normalize(cross (a_direction, toEye));

        v_texture.x = a_vertexDelta.z * v_length + width * a_vertexDelta.x;
        v_texture.y = a_vertexDelta.y;

        gl_Position = czm_modelViewProjection * vec4(a_pos + (width * a_vertexDelta.x / v_length) * a_direction + width * a_vertexDelta.y * extrusion, 1.0);
        v_color = computeColor();
        }
)RAW_STRING";

static std::string s_tesselatedPolylineFragmentShader = R"RAW_STRING(
    uniform float u_feather;
    varying vec4 v_color;
    varying vec2 v_texture;
    varying float v_length;
    varying float v_widthScale;

    bool computePolylineColor (vec4 color, float distance)
        {
        if (distance > 1.0)
            return false;

        float alpha = distance > (1.0 - u_feather) ? ((1.0 - distance)/u_feather) : 1.0;
        gl_FragColor = vec4 (color.rgb, alpha);
        return true; 
        }

    void main()
        {
        float centerDistance;
             
        if (v_texture.x < 0.0)
            {
            float xTexture = -v_texture.x * v_widthScale;
            centerDistance = sqrt (xTexture * xTexture + v_texture.y * v_texture.y);
            }
        else if (v_texture.x > v_length)
            {
            float xTexture = (v_texture.x - v_length) * v_widthScale;
            centerDistance = sqrt (xTexture * xTexture + v_texture.y * v_texture.y);
            }
        else
            {
            centerDistance = abs (v_texture.y);
            }

        if (!computePolylineColor (v_color, centerDistance))
            discard;
        }
)RAW_STRING";

static std::string s_simplePolylineVertexCommon = R"RAW_STRING(
    attribute vec3 a_pos;
    uniform mat4 u_mv;
    uniform mat4 u_proj;
    uniform float u_width;
    varying vec2 v_windowPos;

    void main(void)
        {
        vec4 modelPos = vec4(a_pos, 1.0);
        gl_Position =  czm_modelViewProjection * modelPos;
        vec4 windowPos = czm_modelToWindowCoordinates(modelPos);
        v_windowPos = windowPos.xy / windowPos.w;
        v_color = computeColor();
        }
)RAW_STRING";

static std::string s_simplePolylineFragmentShader = R"RAW_STRING(
varying vec4 v_color;
varying vec2 v_windowPos;

void main(void)
    {
    gl_FragColor = v_color;
    }
)RAW_STRING";

static std::string s_simplePolylineVertexShaders[3] =
    {
    s_computeIndexedColor[0] + s_simplePolylineVertexCommon,
    s_computeIndexedColor[1] + s_simplePolylineVertexCommon,
    s_computeIndexedColor[2] + s_simplePolylineVertexCommon
    };

static std::string s_tesselatedPolylineVertexShaders[3] =
    {
    s_computeIndexedColor[0] + s_tesselatedPolylineVertexCommon,
    s_computeIndexedColor[1] + s_tesselatedPolylineVertexCommon,
    s_computeIndexedColor[2] + s_tesselatedPolylineVertexCommon
    };

// From DgnViewMaterial.cpp
static double const s_qvExponentMultiplier  = 48.0,
                    s_qvAmbient             = 1.0,
                    s_qvFinish              = 0.9,
                    s_qvSpecular            = 0.4,
                    s_qvReflect             = 0.0,
                    s_qvRefract             = 1.0,
                    s_qvDiffuse             = 0.6;

static const WCharCP s_metadataExtension        = L"json";

