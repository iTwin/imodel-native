//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSParser.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPSParser
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HPAParser.h"
#include "HPSNode.h"
#include "HPSWorldCluster.h"

BEGIN_IMAGEPP_NAMESPACE

class HPMPool;
class HFCBinStream;

class HPSParser : public HPAParser
    {
public:
    DEFINE_T_SUPER(HPAParser)

    HPSParser();
    virtual             ~HPSParser();

    virtual HPANode*    Parse(const HFCPtr<HPASession>& pi_pSession)    {
        return T_Super::Parse(pi_pSession);
        }


    virtual HPANode*    Parse(HFCBinStream*                  pi_pStream,
                              HPMPool*                       pi_pLog,
                              const HFCPtr<HFCURL>&          pi_rpURL = HFCPtr<HFCURL>(),
                              const HFCPtr<HPSWorldCluster>& pi_rpHPSWorldCluster = HFCPtr<HPSWorldCluster>(0),
                              const HGF2DWorldIdentificator* pi_pCurrentWorldId = 0);

    HPANodeList&        GetWorldRelatedNodes();

protected:

private:

    friend class HPSTokenizer;
    friend class HPSParserScope;
    friend class VariableTokenNode;
    friend class StatementTokenNode;
    friend class HPSNode;
    friend class PageStatementNode;
    friend class WorldStatementNode;
    friend class SelectWorldStatementNode;
    friend class DeclarationStatementNode;
    friend class ExpressionNode;
    friend class StringExpressionNode;
    friend class NumericExpressionNode;
    friend class ObjectExpressionNode;
    friend class PictureObjectNode;
    friend class StatementInstanciationNode;
    friend class ImageObjectNode;
    friend class ImageFileExpressionNode;
    friend class MosaicExpressionNode;
    friend class OnDemandMosaicExpressionNode;
    friend class TransformedImageExpressionNode;
    friend class ShapedImageExpressionNode;
    friend class FilteredImageExpressionNode;
    friend class TranslucentImageExpressionNode;
    friend class ColorizedBinaryImageExpressionNode;
    friend class AlphaCubeExpressionNode;
    friend class RGBCubeExpressionNode;
    friend class LUVCubeExpressionNode;
    friend class AlphaPaletteExpressionNode;
    friend class VectorObjectNode;
    friend class ShapeExpressionNode;
    friend class RectangleExpressionNode;
    friend class PolygonExpressionNode;
    friend class HoledShapeExpressionNode;
    friend class UnifiedShapeExpressionNode;
    friend class CommonShapeExpressionNode;
    friend class TransfoObjectNode;
    friend class AffineExpressionNode;
    friend class ProjectiveExpressionNode;
    friend class LocalProjectiveGridExpressionNode;
    friend class IdentityExpressionNode;
    friend class RotationExpressionNode;
    friend class ScalingExpressionNode;
    friend class TranslationExpressionNode;
    friend class ComposedExpressionNode;
    friend class FilterObjectNode;
    friend class ContrastExpressionNode;
    friend class BrightnessExpressionNode;
    friend class ConvolutionRGBExpressionNode;
    friend class Convolution3ExpressionNode;
    friend class AutoContrastStretchExpressionNode;
    friend class ContrastStretchExpressionNode;
    friend class TintExpressionNode;
    friend class InverterExpressionNode;
    friend class GammaCorrectionExpressionNode;
    friend class StatementDefinitionNode;
    friend class StatementDeclarationNode;
    friend class ReturnStatementNode;

    friend class HPSObjectStore;

    // Tokens

    HPAToken    ADJUST_tk;
    HPAToken    AFFINE_tk;
    HPAToken    ALPHACUBE_tk;
    HPAToken    ALPHAPALETTE_tk;
    HPAToken    ANGULAR_tk;
    HPAToken    ANTI_ALIASING_tk;
    HPAToken    ARROW_tk;
    HPAToken    AVERAGE_tk;
    HPAToken    AUTOCONTRASTSTRETCH_tk;
    HPAToken    BLANKIMAGE_tk;
    HPAToken    BLUR_tk;
    HPAToken    BOLD_tk;
    HPAToken    BOTTOM_tk;
    HPAToken    BRIGHTNESS_tk;
    HPAToken    CENTER_tk;
    HPAToken    CIRCLE_tk;
    HPAToken    COLORIZEDBINARYIMAGE_tk;
    HPAToken    COMMONSHAPE_tk;
    HPAToken    COMPOSED_tk;
    HPAToken    CONTRAST_tk;
    HPAToken    CONTRASTSTRETCH_tk;
    HPAToken    CONVOLUTION_tk;
    HPAToken    CONVOLUTIONRGB_tk;
    HPAToken    CONVOLUTION3_tk;
    HPAToken    CURVE_tk;
    HPAToken    DASHDOT_tk;
    HPAToken    DASHES_tk;
    HPAToken    DESPECKLE_tk;
    HPAToken    DIAMOND_tk;
    HPAToken    DOT_KW_tk;
    HPAToken    DOTS_tk;
    HPAToken    DRAWING_tk;
    HPAToken    EMBOSS_tk;
    HPAToken    ELLIPSE_tk;
    HPAToken    EXPORT_tk;
    HPAToken    EXTENT_tk;
    HPAToken    FILL_tk;
    HPAToken    FILLPATTERN_tk;
    HPAToken    FILTER_tk;
    HPAToken    FILTEREDIMAGE_tk;
    HPAToken    FIT_tk;
    HPAToken    FITTEDIMAGE_tk;
    HPAToken    FOLDER_tk;
    HPAToken    GAMMACORRECTION_tk;
    HPAToken    GEOREFERENCECONTEXT_tk;
    HPAToken    GLOW_tk;
    HPAToken    HEIGHT_tk;
    HPAToken    HOLEDSHAPE_tk;
    HPAToken    IDENTITY_tk;
    HPAToken    ITALIC_tk;

    HPAToken    IMAGECONTEXT_tk;
    HPAToken    IMAGEFILE_tk;
    HPAToken    IMPORTNUMBER_tk;
    HPAToken    IMPORTOBJECT_tk;
    HPAToken    IMPORTSTRING_tk;
    HPAToken    INCLUDE_tk;
    HPAToken    INVERTER_tk;
    HPAToken    LEFT_tk;
    HPAToken    LOCALPROJECTIVEGRID_tk;
    HPAToken    LOCATION_tk;
    HPAToken    LONGDASHDOT_tk;
    HPAToken    LONGDASHDOTDOT_tk;
    HPAToken    LONGDASHES_tk;
    HPAToken    LUVCUBE_tk;
    HPAToken    MOSAIC_tk;
    HPAToken    ONDEMANDMOSAIC_tk;
    HPAToken    MOVEDIMAGE_tk;
    HPAToken    MULTIRESIMAGE_tk;
    HPAToken    NONE_tk;
    HPAToken    NEAREST_NEIGHBOR_tk;
    HPAToken    NUMBER_tk;
    HPAToken    OBJECT_tk;
    HPAToken    OPENARROW_tk;
    HPAToken    ORIGIN_tk;
    HPAToken    PAGE_tk;
    HPAToken    PENSTYLE_tk;
    HPAToken    PERIMETER_tk;
    HPAToken    PICTURE_tk;
    HPAToken    PIXELSIZE_tk;
    HPAToken    PLAIN_tk;
    HPAToken    POINT_tk;
    HPAToken    POLYGON_tk;
    HPAToken    POLYLINE_tk;
    HPAToken    POLYSEGMENT_tk;
    HPAToken    PROJECTIVE_tk;
    HPAToken    RECTANGLE_tk;
    HPAToken    REPLACE_tk;
    HPAToken    RESAMPLEDIMAGE_tk;
    HPAToken    RETURN_tk;
    HPAToken    RGB_tk;
    HPAToken    RGBCUBE_tk;
    HPAToken    RIGHT_tk;
    HPAToken    ROTATION_tk;
    HPAToken    ROUND_tk;
    HPAToken    SCALING_tk;
    HPAToken    SCENE_tk;
    HPAToken    SEGMENT_tk;
    HPAToken    SELECTWORLD_tk;
    HPAToken    SETANNOTATIONICON_tk;
    HPAToken    SETLAYERON_tk;
    HPAToken    SETLAYEROFF_tk;
    HPAToken    SHADOW_tk;
    HPAToken    SHAPE_tk;
    HPAToken    SHAPESTYLE_tk;
    HPAToken    SHAPEDIMAGE_tk;
    HPAToken    SHARPEN_tk;
    HPAToken    STATEMENT_tk;
    HPAToken    STRING_tk;
    HPAToken    TEXT_tk;
    HPAToken    TEXTSTYLE_tk;
    HPAToken    TINT_tk;
    HPAToken    TOP_tk;
    HPAToken    TRANSFORMEDIMAGE_tk;
    HPAToken    TRANSLATION_tk;
    HPAToken    TRANSLUCENTIMAGE_tk;
    HPAToken    UNDERLINED_tk;
    HPAToken    UNIFIEDSHAPE_tk;
    HPAToken    USE_EXTENSION_tk;
    HPAToken    USING_tk;
    HPAToken    VECTORARRAY_tk;
    HPAToken    VECTOR_AWARENESS_tk;
    HPAToken    VECTORFILE_tk;
    HPAToken    WARPEDIMAGE_tk;
    HPAToken    WATERMARK_tk;
    HPAToken    WIDTH_tk;
    HPAToken    WORLD_tk;
    HPAToken    X_tk;
    HPAToken    Y_tk;

    HPAToken    LP_tk;
    HPAToken    RP_tk;
    HPAToken    LB_tk;
    HPAToken    RB_tk;
    HPAToken    COMMA_tk;
    HPAToken    EQ_tk;
    HPAToken    PLUS_tk;
    HPAToken    MINUS_tk;
    HPAToken    AT_tk;
    HPAToken    DOT_tk;
    HPAToken    TILDE_tk;

    HPAToken    Number_tk;
    HPAToken    String_tk;
    HPAToken    Identifier_tk;
    HPAToken    StatementIdentifier_tk;
    HPAToken    VariableIdentifier_tk;

    // Rules

    HPARule     PictureScript;
    HPARule     StatementList;
    HPARule     Statement;
    HPARule     PagesStatement;
    HPARule     PageList;
    HPARule     PageStatement;
    HPARule     WorldStatement;
    HPARule     SelectWorldStatement;
    HPARule     SetAnnotationIconStatement;
    HPARule     SetLayerOnStatement;
    HPARule     SetLayerOffStatement;
    HPARule     DeclarationStatement;
    HPARule     Expression;
    HPARule     ExpressionList;
    HPARule     StringExpression;
    HPARule     NumericExpression;
    HPARule     ObjectExpression;
    HPARule     StatementInstanciation;
    HPARule     PictureObject;
    HPARule     ImageObject;
    HPARule     ImageFileExpression;
    HPARule     MosaicExpression;
    HPARule     OnDemandMosaicExpression;
    HPARule     TransformedImageExpression;
    HPARule     ShapedImageExpression;
    HPARule     FilteredImageExpression;
    HPARule     TranslucentImageExpression;
    HPARule     ColorizedBinaryImageExpression;
    HPARule     AlphaCubeExpression;
    HPARule     RGBCubeExpression;
    HPARule     LUVCubeExpression;
    HPARule     GeoreferenceContextExpression;
    HPARule     ImageContextExpression;
    HPARule     AlphaPaletteExpression;
    HPARule     RangeExpression;
    HPARule     RangeExpressionList;
    HPARule     VectorObject;
    HPARule     ShapeExpression;
    HPARule     RectangleExpression;
    HPARule     PolygonExpression;
    HPARule     HoledShapeExpression;
    HPARule     UnifiedShapeExpression;
    HPARule     CommonShapeExpression;
    HPARule     TransformationObject;
    HPARule     IdentityExpression;
    HPARule     RotationExpression;
    HPARule     ScalingExpression;
    HPARule     TranslationExpression;
    HPARule     AffineExpression;
    HPARule     ProjectiveExpression;
    HPARule     ProjectiveList;
    HPARule     LocalProjectiveGridExpression;
    HPARule     ComposedExpression;
    HPARule     FilterObject;
    HPARule     ContrastExpression;
    HPARule     BrightnessExpression;
    HPARule     ConvolutionRGBExpression;
    HPARule     Convolution3Expression;
    HPARule     AutoContrastStretchExpression;
    HPARule     ContrastStretchExpression;
    HPARule     TintExpression;
    HPARule     InverterExpression;
    HPARule     GammaCorrectionExpression;
    HPARule     Parameter;
    HPARule     ParameterList;
    HPARule     StatementDefinition;
    HPARule     StatementDeclaration;
    HPARule     ReturnStatement;

    //This list contains all the world related nodes found in the PSS.
    HPANodeList m_WorldRelatedNodes;
    };
END_IMAGEPP_NAMESPACE