//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSParser.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class HPSParser
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPSParser.h>
#include <Imagepp/all/h/HPSNode.h>
#include "HPSParserScope.h"
#include <Imagepp/all/h/HPSException.h>
#include <Imagepp/all/h/HPSTokenizer.h>
#include "HPSInternalNodes.h"
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HFCBinStream.h>

//---------------------------------------------------------------------------
// Instanciation of static node creators.  They are connected to
// grammar object inside the constructor of the parser.

#define DECLARE_NODE_CREATOR(NodeClass)                                 \
class NodeClass##Creator : public HPANodeCreator                    \
    {                                                                   \
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,              \
    const HPANodeList& pi_rList,            \
    const HFCPtr<HPASession>& pi_pSession)  \
        {                                                               \
        return new NodeClass(pi_pObj, pi_rList, pi_pSession);       \
        }                                                               \
    } s_##NodeClass##Creator;

class HPSNodeCreator : public HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        {
        return new HPSNode(pi_pObj, pi_rList, pi_pSession);
        }
    } s_HPSNodeCreator;

DECLARE_NODE_CREATOR(PageStatementNode)
DECLARE_NODE_CREATOR(WorldStatementNode)
DECLARE_NODE_CREATOR(SelectWorldStatementNode)
DECLARE_NODE_CREATOR(SetAnnotationIconStatementNode)
DECLARE_NODE_CREATOR(SetLayerStatementNode)
DECLARE_NODE_CREATOR(DeclarationStatementNode)
DECLARE_NODE_CREATOR(ExpressionNode)
DECLARE_NODE_CREATOR(StringExpressionNode)
DECLARE_NODE_CREATOR(NumericExpressionNode)
DECLARE_NODE_CREATOR(ObjectExpressionNode)
DECLARE_NODE_CREATOR(PictureObjectNode)
DECLARE_NODE_CREATOR(StatementInstanciationNode)
DECLARE_NODE_CREATOR(ImageObjectNode)
DECLARE_NODE_CREATOR(ImageFileExpressionNode)
DECLARE_NODE_CREATOR(MosaicExpressionNode)
DECLARE_NODE_CREATOR(OnDemandMosaicExpressionNode)
DECLARE_NODE_CREATOR(TransformedImageExpressionNode)
DECLARE_NODE_CREATOR(ShapedImageExpressionNode)
DECLARE_NODE_CREATOR(FilteredImageExpressionNode)
DECLARE_NODE_CREATOR(TranslucentImageExpressionNode)
DECLARE_NODE_CREATOR(ColorizedBinaryImageExpressionNode)
DECLARE_NODE_CREATOR(AlphaCubeExpressionNode)
DECLARE_NODE_CREATOR(RGBCubeExpressionNode)
DECLARE_NODE_CREATOR(LUVCubeExpressionNode)
DECLARE_NODE_CREATOR(GeoreferenceContextExpressionNode)
DECLARE_NODE_CREATOR(ImageContextExpressionNode)
DECLARE_NODE_CREATOR(AlphaPaletteExpressionNode)
DECLARE_NODE_CREATOR(VectorObjectNode)
DECLARE_NODE_CREATOR(ShapeExpressionNode)
DECLARE_NODE_CREATOR(RectangleExpressionNode)
DECLARE_NODE_CREATOR(PolygonExpressionNode)
DECLARE_NODE_CREATOR(HoledShapeExpressionNode)
DECLARE_NODE_CREATOR(UnifiedShapeExpressionNode)
DECLARE_NODE_CREATOR(CommonShapeExpressionNode)
DECLARE_NODE_CREATOR(TransfoObjectNode)
DECLARE_NODE_CREATOR(IdentityExpressionNode)
DECLARE_NODE_CREATOR(RotationExpressionNode)
DECLARE_NODE_CREATOR(ScalingExpressionNode)
DECLARE_NODE_CREATOR(TranslationExpressionNode)
DECLARE_NODE_CREATOR(AffineExpressionNode)
DECLARE_NODE_CREATOR(ProjectiveExpressionNode)
DECLARE_NODE_CREATOR(LocalProjectiveGridExpressionNode)
DECLARE_NODE_CREATOR(ComposedExpressionNode);
DECLARE_NODE_CREATOR(FilterObjectNode)
DECLARE_NODE_CREATOR(ContrastExpressionNode)
DECLARE_NODE_CREATOR(BrightnessExpressionNode)
DECLARE_NODE_CREATOR(ConvolutionRGBExpressionNode);
DECLARE_NODE_CREATOR(Convolution3ExpressionNode);
DECLARE_NODE_CREATOR(AutoContrastStretchExpressionNode);
DECLARE_NODE_CREATOR(ContrastStretchExpressionNode);
DECLARE_NODE_CREATOR(TintExpressionNode);
DECLARE_NODE_CREATOR(InverterExpressionNode);
DECLARE_NODE_CREATOR(GammaCorrectionExpressionNode);
DECLARE_NODE_CREATOR(StatementDefinitionNode)
DECLARE_NODE_CREATOR(StatementDeclarationNode)
DECLARE_NODE_CREATOR(ReturnStatementNode)

//---------------------------------------------------------------------------
// Constructor of the parser
//---------------------------------------------------------------------------
HPSParser::HPSParser()
    {
    // Tokenizer setup

    HPSTokenizer* pTok = new HPSTokenizer(this);
    pTok->SetIdentifierToken(Identifier_tk);
    pTok->SetNumberToken(Number_tk);
    pTok->SetStringToken(String_tk);
    pTok->SetCommentMarker(L';');

    // Tokenizer : special symbols

    pTok->AddSymbol(L"(", LP_tk);
    pTok->AddSymbol(L")", RP_tk);
    pTok->AddSymbol(L"{", LB_tk);
    pTok->AddSymbol(L"}", RB_tk);
    pTok->AddSymbol(L",", COMMA_tk);
    pTok->AddSymbol(L"=", EQ_tk);
    pTok->AddSymbol(L"+", PLUS_tk);
    pTok->AddSymbol(L"-", MINUS_tk);
    pTok->AddSymbol(L"@", AT_tk);
    pTok->AddSymbol(L".", DOT_tk);
    pTok->AddSymbol(L"~", TILDE_tk);

    // Tokenizer : reserved keywords

    pTok->AddSymbol(L"ADJUST", ADJUST_tk);
    pTok->AddSymbol(L"ADJ", ADJUST_tk);
    pTok->AddSymbol(L"AFFINE", AFFINE_tk);
    pTok->AddSymbol(L"AFF", AFFINE_tk);
    pTok->AddSymbol(L"ALPHACUBE", ALPHACUBE_tk);
    pTok->AddSymbol(L"ALC", ALPHACUBE_tk);
    pTok->AddSymbol(L"ALPHAPALETTE", ALPHAPALETTE_tk);
    pTok->AddSymbol(L"ALP", ALPHAPALETTE_tk);
    pTok->AddSymbol(L"ANGULAR", ANGULAR_tk);
    pTok->AddSymbol(L"ANG", ANGULAR_tk);
    pTok->AddSymbol(L"ANTI_ALIASING", ANTI_ALIASING_tk);
    pTok->AddSymbol(L"ANTIA", ANTI_ALIASING_tk);
    pTok->AddSymbol(L"ARROW", ARROW_tk);
    pTok->AddSymbol(L"ARR", ARROW_tk);
    pTok->AddSymbol(L"AUTOCONTRASTSTRETCH", AUTOCONTRASTSTRETCH_tk);
    pTok->AddSymbol(L"AVERAGE", AVERAGE_tk);
    pTok->AddSymbol(L"AVG", AVERAGE_tk);
    pTok->AddSymbol(L"BLANKIMAGE", BLANKIMAGE_tk);
    pTok->AddSymbol(L"BIMG", BLANKIMAGE_tk);
    pTok->AddSymbol(L"BLUR", BLUR_tk);
    pTok->AddSymbol(L"BOLD", BOLD_tk);
    pTok->AddSymbol(L"BOTTOM", BOTTOM_tk);
    pTok->AddSymbol(L"BOT", BOTTOM_tk);
    pTok->AddSymbol(L"BRIGHTNESS", BRIGHTNESS_tk);
    pTok->AddSymbol(L"BR", BRIGHTNESS_tk);
    pTok->AddSymbol(L"CENTER", CENTER_tk);
    pTok->AddSymbol(L"CTR", CENTER_tk);
    pTok->AddSymbol(L"CIRCLE", CIRCLE_tk);
    pTok->AddSymbol(L"CIR", CIRCLE_tk);
    pTok->AddSymbol(L"COLORIZEDBINARYIMAGE", COLORIZEDBINARYIMAGE_tk);
    pTok->AddSymbol(L"COLBI", COLORIZEDBINARYIMAGE_tk);
    pTok->AddSymbol(L"COMMONSHAPE", COMMONSHAPE_tk);
    pTok->AddSymbol(L"CMNS", COMMONSHAPE_tk);
    pTok->AddSymbol(L"COMPOSED", COMPOSED_tk);
    pTok->AddSymbol(L"CP", COMPOSED_tk);
    pTok->AddSymbol(L"CONTRAST", CONTRAST_tk);
    pTok->AddSymbol(L"CONTRASTSTRETCH", CONTRASTSTRETCH_tk);
    pTok->AddSymbol(L"CT", CONTRAST_tk);
    pTok->AddSymbol(L"CONVOLUTION", CONVOLUTION_tk);
    pTok->AddSymbol(L"CONV", CONVOLUTION_tk);
    pTok->AddSymbol(L"CONVOLUTIONRGB", CONVOLUTIONRGB_tk);
    pTok->AddSymbol(L"CONVRGB", CONVOLUTIONRGB_tk);
    pTok->AddSymbol(L"CONVOLUTION3", CONVOLUTION3_tk);
    pTok->AddSymbol(L"CONV3", CONVOLUTION3_tk);
    pTok->AddSymbol(L"CURVE", CURVE_tk);
    pTok->AddSymbol(L"CU", CURVE_tk);
    pTok->AddSymbol(L"DASHDOT", DASHDOT_tk);
    pTok->AddSymbol(L"DASHES", DASHES_tk);
    pTok->AddSymbol(L"DA", DASHES_tk);
    pTok->AddSymbol(L"DESPECKLE", DESPECKLE_tk);
    pTok->AddSymbol(L"DPKL", DESPECKLE_tk);
    pTok->AddSymbol(L"DIAMOND", DIAMOND_tk);
    pTok->AddSymbol(L"DIA", DIAMOND_tk);
    pTok->AddSymbol(L"DOT", DOT_KW_tk);
    pTok->AddSymbol(L"DOTS", DOTS_tk);
    pTok->AddSymbol(L"DRAWING", DRAWING_tk);
    pTok->AddSymbol(L"DR", DRAWING_tk);
    pTok->AddSymbol(L"EMBOSS", EMBOSS_tk);
    pTok->AddSymbol(L"EMB", EMBOSS_tk);
    pTok->AddSymbol(L"ELLIPSE", ELLIPSE_tk);
    pTok->AddSymbol(L"ELL", ELLIPSE_tk);
    pTok->AddSymbol(L"EXPORT", EXPORT_tk);
    pTok->AddSymbol(L"EXP", EXPORT_tk);
    pTok->AddSymbol(L"EXTENT", EXTENT_tk);
    pTok->AddSymbol(L"EXT", EXTENT_tk);
    pTok->AddSymbol(L"FILL", FILL_tk);
    pTok->AddSymbol(L"FILLPATTERN", FILLPATTERN_tk);
    pTok->AddSymbol(L"FPAT", FILLPATTERN_tk);
    pTok->AddSymbol(L"FILTER", FILTER_tk);
    pTok->AddSymbol(L"FIL", FILTER_tk);
    pTok->AddSymbol(L"FILTEREDIMAGE", FILTEREDIMAGE_tk);
    pTok->AddSymbol(L"FII", FILTEREDIMAGE_tk);
    pTok->AddSymbol(L"FIT", FIT_tk);
    pTok->AddSymbol(L"FITTEDIMAGE", FITTEDIMAGE_tk);
    pTok->AddSymbol(L"FITI", FITTEDIMAGE_tk);
    pTok->AddSymbol(L"FOLDER", FOLDER_tk);
    pTok->AddSymbol(L"FOL", FOLDER_tk);
    pTok->AddSymbol(L"GAMMACORRECTION", GAMMACORRECTION_tk);
    pTok->AddSymbol(L"GAMC", GAMMACORRECTION_tk);
    pTok->AddSymbol(L"GEOREFERENCECONTEXT", GEOREFERENCECONTEXT_tk);
    pTok->AddSymbol(L"GEOR", GEOREFERENCECONTEXT_tk);
    pTok->AddSymbol(L"GLOW", GLOW_tk);
    pTok->AddSymbol(L"HEIGHT", HEIGHT_tk);
    pTok->AddSymbol(L"HEI", HEIGHT_tk);
    pTok->AddSymbol(L"HOLEDSHAPE", HOLEDSHAPE_tk);
    pTok->AddSymbol(L"HSH", HOLEDSHAPE_tk);
    pTok->AddSymbol(L"IDENTITY", IDENTITY_tk);
    pTok->AddSymbol(L"IDTY", IDENTITY_tk);
    pTok->AddSymbol(L"ITALIC", ITALIC_tk);
    pTok->AddSymbol(L"ITAL", ITALIC_tk);
    pTok->AddSymbol(L"IMAGECONTEXT", IMAGECONTEXT_tk);
    pTok->AddSymbol(L"IFC", IMAGECONTEXT_tk);
    pTok->AddSymbol(L"IMAGEFILE", IMAGEFILE_tk);
    pTok->AddSymbol(L"IMF", IMAGEFILE_tk);
    pTok->AddSymbol(L"IMPORTNUMBER", IMPORTNUMBER_tk);
    pTok->AddSymbol(L"IMPN", IMPORTNUMBER_tk);
    pTok->AddSymbol(L"IMPORTOBJECT", IMPORTOBJECT_tk);
    pTok->AddSymbol(L"IMPO", IMPORTOBJECT_tk);
    pTok->AddSymbol(L"IMPORTSTRING", IMPORTSTRING_tk);
    pTok->AddSymbol(L"IMPS", IMPORTSTRING_tk);
    pTok->AddSymbol(L"INCLUDE", INCLUDE_tk);
    pTok->AddSymbol(L"INC", INCLUDE_tk);
    pTok->AddSymbol(L"INVERTER", INVERTER_tk);
    pTok->AddSymbol(L"INV", INVERTER_tk);
    pTok->AddSymbol(L"LEFT", LEFT_tk);
    pTok->AddSymbol(L"LOCATION", LOCATION_tk);
    pTok->AddSymbol(L"LOC", LOCATION_tk);
    pTok->AddSymbol(L"LOCALPROJECTIVEGRID", LOCALPROJECTIVEGRID_tk);
    pTok->AddSymbol(L"LONGDASHDOT", LONGDASHDOT_tk);
    pTok->AddSymbol(L"LONGDASHDOTDOT", LONGDASHDOTDOT_tk);
    pTok->AddSymbol(L"LONGDASHES", LONGDASHES_tk);
    pTok->AddSymbol(L"LUVCUBE", LUVCUBE_tk);
    pTok->AddSymbol(L"LUVC", LUVCUBE_tk);
    pTok->AddSymbol(L"MOSAIC", MOSAIC_tk);
    pTok->AddSymbol(L"MO", MOSAIC_tk);
    pTok->AddSymbol(L"MOVEDIMAGE", MOVEDIMAGE_tk);
    pTok->AddSymbol(L"MVI", MOVEDIMAGE_tk);
    pTok->AddSymbol(L"MULTIRESIMAGE", MULTIRESIMAGE_tk);
    pTok->AddSymbol(L"MRI", MULTIRESIMAGE_tk);
    pTok->AddSymbol(L"NONE", NONE_tk);
    pTok->AddSymbol(L"NEAREST_NEIGHBOR", NEAREST_NEIGHBOR_tk);
    pTok->AddSymbol(L"N_N", NEAREST_NEIGHBOR_tk);
    pTok->AddSymbol(L"NUMBER", NUMBER_tk);
    pTok->AddSymbol(L"NUM", NUMBER_tk);
    pTok->AddSymbol(L"OBJECT", OBJECT_tk);
    pTok->AddSymbol(L"OBJ", OBJECT_tk);
    pTok->AddSymbol(L"OPENARROW", OPENARROW_tk);
    pTok->AddSymbol(L"OARR", OPENARROW_tk);
    pTok->AddSymbol(L"ONDEMANDMOSAIC", ONDEMANDMOSAIC_tk);
    pTok->AddSymbol(L"ODMO", ONDEMANDMOSAIC_tk);
    pTok->AddSymbol(L"ORIGIN", ORIGIN_tk);
    pTok->AddSymbol(L"ORI", ORIGIN_tk);
    pTok->AddSymbol(L"PAGE", PAGE_tk);
    pTok->AddSymbol(L"PG", PAGE_tk);
    pTok->AddSymbol(L"PENSTYLE", PENSTYLE_tk);
    pTok->AddSymbol(L"PEN", PENSTYLE_tk);
    pTok->AddSymbol(L"PERIMETER", PERIMETER_tk);
    pTok->AddSymbol(L"PERI", PERIMETER_tk);
    pTok->AddSymbol(L"PICTURE", PICTURE_tk);
    pTok->AddSymbol(L"PIC", PICTURE_tk);
    pTok->AddSymbol(L"PIXELSIZE", PIXELSIZE_tk);
    pTok->AddSymbol(L"PIXS", PIXELSIZE_tk);
    pTok->AddSymbol(L"PLAIN", PLAIN_tk);
    pTok->AddSymbol(L"POINT", POINT_tk);
    pTok->AddSymbol(L"PT", POINT_tk);
    pTok->AddSymbol(L"POLYGON", POLYGON_tk);
    pTok->AddSymbol(L"POG", POLYGON_tk);
    pTok->AddSymbol(L"POLYLINE", POLYLINE_tk);
    pTok->AddSymbol(L"PLIN", POLYLINE_tk);
    pTok->AddSymbol(L"POLYSEGMENT", POLYSEGMENT_tk);
    pTok->AddSymbol(L"PROJECTIVE", PROJECTIVE_tk);
    pTok->AddSymbol(L"PROJ", PROJECTIVE_tk);
    pTok->AddSymbol(L"PSEG", POLYSEGMENT_tk);
    pTok->AddSymbol(L"RECTANGLE", RECTANGLE_tk);
    pTok->AddSymbol(L"RECT", RECTANGLE_tk);
    pTok->AddSymbol(L"REPLACE", REPLACE_tk);
    pTok->AddSymbol(L"REPL", REPLACE_tk);
    pTok->AddSymbol(L"RESAMPLEDIMAGE", RESAMPLEDIMAGE_tk);
    pTok->AddSymbol(L"RSI", RESAMPLEDIMAGE_tk);
    pTok->AddSymbol(L"RETURN", RETURN_tk);
    pTok->AddSymbol(L"RET", RETURN_tk);
    pTok->AddSymbol(L"RTN", RETURN_tk);
    pTok->AddSymbol(L"RGB", RGB_tk);
    pTok->AddSymbol(L"RGBCUBE", RGBCUBE_tk);
    pTok->AddSymbol(L"RGBC", RGBCUBE_tk);
    pTok->AddSymbol(L"RIGHT", RIGHT_tk);
    pTok->AddSymbol(L"ROTATION", ROTATION_tk);
    pTok->AddSymbol(L"ROT", ROTATION_tk);
    pTok->AddSymbol(L"ROUND", ROUND_tk);
    pTok->AddSymbol(L"SCALING", SCALING_tk);
    pTok->AddSymbol(L"SCA", SCALING_tk);
    pTok->AddSymbol(L"SCENE", SCENE_tk);
    pTok->AddSymbol(L"SCE", SCENE_tk);
    pTok->AddSymbol(L"SEGMENT", SEGMENT_tk);
    pTok->AddSymbol(L"SEG", SEGMENT_tk);
    pTok->AddSymbol(L"SELECTWORLD", SELECTWORLD_tk);
    pTok->AddSymbol(L"SELW", SELECTWORLD_tk);
    pTok->AddSymbol(L"SETANNOTATIONICON", SETANNOTATIONICON_tk);
    pTok->AddSymbol(L"SETAI", SETANNOTATIONICON_tk);
    pTok->AddSymbol(L"SETLAYERON", SETLAYERON_tk);
    pTok->AddSymbol(L"SETLO", SETLAYERON_tk);
    pTok->AddSymbol(L"SETLAYEROFF", SETLAYEROFF_tk);
    pTok->AddSymbol(L"SETLF", SETLAYEROFF_tk);
    pTok->AddSymbol(L"SHADOW", SHADOW_tk);
    pTok->AddSymbol(L"SHAD", SHADOW_tk);
    pTok->AddSymbol(L"SHAPE", SHAPE_tk);
    pTok->AddSymbol(L"SHP", SHAPE_tk);
    pTok->AddSymbol(L"SHAPESTYLE", SHAPESTYLE_tk);
    pTok->AddSymbol(L"SHPS", SHAPESTYLE_tk);
    pTok->AddSymbol(L"SHAPEDIMAGE", SHAPEDIMAGE_tk);
    pTok->AddSymbol(L"SHI", SHAPEDIMAGE_tk);
    pTok->AddSymbol(L"SHARPEN", SHARPEN_tk);
    pTok->AddSymbol(L"SHAR", SHARPEN_tk);
    pTok->AddSymbol(L"STATEMENT", STATEMENT_tk);
    pTok->AddSymbol(L"STAT", STATEMENT_tk);
    pTok->AddSymbol(L"STRING", STRING_tk);
    pTok->AddSymbol(L"STR", STRING_tk);
    pTok->AddSymbol(L"TEXT", TEXT_tk);
    pTok->AddSymbol(L"TXT", TEXT_tk);
    pTok->AddSymbol(L"TEXTSTYLE", TEXTSTYLE_tk);
    pTok->AddSymbol(L"TSTY", TEXTSTYLE_tk);
    pTok->AddSymbol(L"TINT", TINT_tk);
    pTok->AddSymbol(L"TOP", TOP_tk);
    pTok->AddSymbol(L"TRANSFORMEDIMAGE", TRANSFORMEDIMAGE_tk);
    pTok->AddSymbol(L"TFI", TRANSFORMEDIMAGE_tk);
    pTok->AddSymbol(L"TRANSLATION", TRANSLATION_tk);
    pTok->AddSymbol(L"TRL", TRANSLATION_tk);
    pTok->AddSymbol(L"TRANSLUCENTIMAGE", TRANSLUCENTIMAGE_tk);
    pTok->AddSymbol(L"TLI", TRANSLUCENTIMAGE_tk);
    pTok->AddSymbol(L"UNDERLINED", UNDERLINED_tk);
    pTok->AddSymbol(L"UNDL", UNDERLINED_tk);
    pTok->AddSymbol(L"UNIFIEDSHAPE", UNIFIEDSHAPE_tk);
    pTok->AddSymbol(L"UNFS", UNIFIEDSHAPE_tk);
    pTok->AddSymbol(L"USE_EXTENSION", USE_EXTENSION_tk);
    pTok->AddSymbol(L"U_E", USE_EXTENSION_tk);
    pTok->AddSymbol(L"USING", USING_tk);
    pTok->AddSymbol(L"VECTORARRAY", VECTORARRAY_tk);
    pTok->AddSymbol(L"VECA", VECTORARRAY_tk);
    pTok->AddSymbol(L"VECTOR_AWARENESS", VECTOR_AWARENESS_tk);
    pTok->AddSymbol(L"V_A", VECTOR_AWARENESS_tk);
    pTok->AddSymbol(L"VECTORFILE", VECTORFILE_tk);
    pTok->AddSymbol(L"VECF", VECTORFILE_tk);
    pTok->AddSymbol(L"WARPEDIMAGE", WARPEDIMAGE_tk);
    pTok->AddSymbol(L"WPI", WARPEDIMAGE_tk);
    pTok->AddSymbol(L"WATERMARK", WATERMARK_tk);
    pTok->AddSymbol(L"WMK", WATERMARK_tk);
    pTok->AddSymbol(L"WIDTH", WIDTH_tk);
    pTok->AddSymbol(L"WI", WIDTH_tk);
    pTok->AddSymbol(L"WORLD", WORLD_tk);
    pTok->AddSymbol(L"WO", WORLD_tk);
    pTok->AddSymbol(L"X", X_tk);
    pTok->AddSymbol(L"Y", Y_tk);

    // Grammar definition

    PictureScript           =    HPAProduction(StatementList);

    StatementList           =    StatementList + Statement
                                 || Statement;

    Statement               =    WorldStatement
                                 || SelectWorldStatement
                                 || SetLayerOnStatement
                                 || SetLayerOffStatement
                                 || SetAnnotationIconStatement
                                 || DeclarationStatement
                                 || StatementDefinition
                                 || PageStatement;
    //                        || IncludeStatement  (handled by tokenizer)

    WorldStatement         =    // World(ID, Transfo, RefWorldID)
        WORLD_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + RP_tk;

    PageStatement           =    // PAGE(image [USING worldID][, description])
        PAGE_tk + LP_tk + Expression + RP_tk
        || PAGE_tk + LP_tk + Expression + USING_tk + Expression + RP_tk
        || PAGE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk
        || PAGE_tk + LP_tk + Expression + USING_tk + Expression + COMMA_tk + Expression + RP_tk;

    // We add this one for doc purpose only.  Inclusion is handled by tokenizer.
    // IncludeStatement     =    INCLUDE_tk + String_tk;

    SelectWorldStatement    =    SELECTWORLD_tk + LP_tk + Expression + RP_tk;

    //SETLAYERON ( imContext , list of layer IDs)
    SetLayerOnStatement     = SETLAYERON_tk + LP_tk + Expression + COMMA_tk + ExpressionList + RP_tk;

    //SETLAYEROFF ( imContext , list of layer IDs)
    SetLayerOffStatement    = SETLAYEROFF_tk + LP_tk + Expression + COMMA_tk + ExpressionList + RP_tk;

    //SETANNOTATIONICON ( imContext, display )
    SetAnnotationIconStatement = SETANNOTATIONICON_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    DeclarationStatement    =    Identifier_tk + EQ_tk + Expression;

    Expression              =    StringExpression
                                 || NumericExpression
                                 || ObjectExpression
                                 || VariableIdentifier_tk
                                 || Expression + DOT_tk + SHAPE_tk;   // shape property

    ExpressionList          =    Expression + COMMA_tk + ExpressionList
                                 || Expression;

    StringExpression        =    HPAProduction(String_tk);

    NumericExpression       =    PLUS_tk + Number_tk
                                 || MINUS_tk + Number_tk
                                 || Number_tk;


    ObjectExpression        =    PictureObject
                                 || FilterObject
                                 || TransformationObject
                                 || StatementInstanciation
                                 || AlphaCubeExpression
                                 || RGBCubeExpression
                                 || LUVCubeExpression
                                 || GeoreferenceContextExpression
                                 || ImageContextExpression
                                 || AlphaPaletteExpression;

    StatementInstanciation  =    StatementIdentifier_tk + LP_tk + ExpressionList + RP_tk
                                 || StatementIdentifier_tk + LP_tk + RP_tk;

    PictureObject           =    VectorObject
                                 || ImageObject;

    ImageObject             =    ImageFileExpression
                                 || MosaicExpression
                                 || OnDemandMosaicExpression
                                 || TransformedImageExpression
                                 || ShapedImageExpression
                                 || FilteredImageExpression
                                 || TranslucentImageExpression
                                 || ColorizedBinaryImageExpression;

    //IMAGEFILE ( filename, [, pageNumber] [, imageContext] [, georeferenceContext])    
    ImageFileExpression     =    IMAGEFILE_tk + LP_tk + Expression + RP_tk
                                 || IMAGEFILE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk
                                 || IMAGEFILE_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + RP_tk
                                 || IMAGEFILE_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + RP_tk;

    MosaicExpression        =    MOSAIC_tk + LP_tk + ExpressionList + RP_tk;

    OnDemandMosaicExpression        =    ONDEMANDMOSAIC_tk + LP_tk + ExpressionList + RP_tk;

    TransformedImageExpression = // TransformedImage(image, transfo [Using WorldID])
        TRANSFORMEDIMAGE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk
        || TRANSFORMEDIMAGE_tk + LP_tk + Expression + COMMA_tk + Expression + USING_tk + Expression + RP_tk;

    ShapedImageExpression   =    // First object : image, second object : shape
        SHAPEDIMAGE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    FilteredImageExpression =    // First object : image, second object : filter
        FILTEREDIMAGE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    TranslucentImageExpression = // First object : image, others : alpha cubes or alpha palettes,
        // or global opacity factor in %
        TRANSLUCENTIMAGE_tk + LP_tk + ExpressionList + RP_tk;

    // ColorizedBinaryImage(Source, R0, G0, B0, R1, G1, B1)
    ColorizedBinaryImageExpression = COLORIZEDBINARYIMAGE_tk + LP_tk + Expression + COMMA_tk
                                     + Expression + COMMA_tk
                                     + Expression + COMMA_tk
                                     + Expression + COMMA_tk
                                     + Expression + COMMA_tk
                                     + Expression + COMMA_tk
                                     + Expression + RP_tk;

    AlphaCubeExpression     =    // AlphaCube(RGBCube | LUVCube, AlphaLevel)
        ALPHACUBE_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    RGBCubeExpression       =    // RGBCube(Rmin, Rmax, Gmin, Gmax, Bmin, Bmax)
        RGBCUBE_tk + LP_tk + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + RP_tk;

    LUVCubeExpression       =    // LUVCube(Lmin, Lmax, Umin, Umax, Vmin, Vmax)
        LUVCUBE_tk + LP_tk + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + RP_tk;

    //GEOREFERENCECONTEXT(defaultRatioToMeterForRaster, defaultRatioToMeterForSisterFile, useSisterFile, usePCSLinearUnit, useDefaultUnitForGeoModel, interpretAsIntergraphUnit)        
    GeoreferenceContextExpression = GEOREFERENCECONTEXT_tk + LP_tk + NumericExpression + COMMA_tk + NumericExpression + COMMA_tk + NumericExpression + COMMA_tk + NumericExpression + COMMA_tk + NumericExpression + COMMA_tk + NumericExpression + RP_tk;

    ImageContextExpression  =    IMAGECONTEXT_tk + LP_tk + RP_tk;
    
    AlphaPaletteExpression  =    // AlphaPalette(AlphaLevel, palette indexes | palette range)
        ALPHAPALETTE_tk + LP_tk + Expression + COMMA_tk + RangeExpressionList + RP_tk;

    RangeExpression         =    Expression
                                 || Expression + TILDE_tk + Expression;

    RangeExpressionList     =    RangeExpression + COMMA_tk + RangeExpressionList
                                 || RangeExpression;

    VectorObject            =    HPAProduction(ShapeExpression);

    ShapeExpression         =    RectangleExpression
                                 || PolygonExpression
                                 || HoledShapeExpression
                                 || UnifiedShapeExpression
                                 || CommonShapeExpression;

    RectangleExpression     =    // Rectangle(x1, y1, x2, y2 [Using WorldID])
        RECTANGLE_tk + LP_tk + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + RP_tk
        || RECTANGLE_tk + LP_tk + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + COMMA_tk
        + Expression + USING_tk
        + Expression + RP_tk;

    PolygonExpression       =    // Polygon(x1, y1, x2, y2, ... xn, yn, x1, y1 [Using WorldID])
        POLYGON_tk + LP_tk + ExpressionList + RP_tk
        || POLYGON_tk + LP_tk + ExpressionList + USING_tk + Expression + RP_tk;

    // HoledShape(BaseShape, Hole1, Hole2, ...)
    HoledShapeExpression    =    HOLEDSHAPE_tk + LP_tk + ExpressionList + RP_tk;

    UnifiedShapeExpression  =    UNIFIEDSHAPE_tk + LP_tk + ExpressionList + RP_tk;

    CommonShapeExpression   =    COMMONSHAPE_tk + LP_tk + ExpressionList + RP_tk;

    TransformationObject    =    IdentityExpression
                                 || RotationExpression
                                 || ScalingExpression
                                 || TranslationExpression
                                 || AffineExpression
                                 || ProjectiveExpression
                                 || LocalProjectiveGridExpression
                                 || ComposedExpression;

    IdentityExpression      =    IDENTITY_tk + LP_tk + RP_tk;

    RotationExpression      =    // Rotation(Angle, CenterX, CenterY)
        ROTATION_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + RP_tk;

    ScalingExpression       =    // Scaling(ScalingX, ScalingY, CenterX, CenterY)
        SCALING_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + RP_tk;

    TranslationExpression   =    // Translation(DeltaX, DeltaY)
        TRANSLATION_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    AffineExpression        =    // Affine(A1, A2, A0, B1, B2, B0)
        AFFINE_tk + LP_tk +
        Expression + COMMA_tk + Expression + COMMA_tk +
        Expression + COMMA_tk + Expression + COMMA_tk +
        Expression + COMMA_tk + Expression + RP_tk;

    ProjectiveExpression    =    // Projective(A00, A01, A02, A10, A11, A12, A20, A21, A22)
        PROJECTIVE_tk +
        LP_tk +
        Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk +
        Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk +
        Expression + COMMA_tk + Expression + COMMA_tk + Expression +
        RP_tk;

    ProjectiveList          =    ProjectiveExpression + COMMA_tk + ProjectiveList
                                 || ProjectiveExpression;

    // LocalProjectiveGrid(Extent, NbTileX, NbTileY, GlobalAffine, ProjectiveList)
    // where Extent         = x1, y1, x2, y2
    //       ProjectiveList = Projective00, Projective10, ...Projective01, Projective11...
    LocalProjectiveGridExpression
    =   LOCALPROJECTIVEGRID_tk +
        LP_tk +
        Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk + Expression + COMMA_tk +
        Expression + COMMA_tk + Expression + COMMA_tk +
        AffineExpression + COMMA_tk +
        ProjectiveList +
        RP_tk;

    ComposedExpression      =    COMPOSED_tk + LP_tk + ExpressionList + RP_tk;

    FilterObject            =    ContrastExpression
                                 || BrightnessExpression
                                 || ConvolutionRGBExpression
                                 || Convolution3Expression
                                 || AutoContrastStretchExpression
                                 || ContrastStretchExpression
                                 || TintExpression
                                 || InverterExpression
                                 || GammaCorrectionExpression;

    ContrastExpression      =    CONTRAST_tk + LP_tk + Expression + RP_tk;

    BrightnessExpression    =    BRIGHTNESS_tk + LP_tk + Expression + RP_tk;

    ConvolutionRGBExpression =   CONVOLUTIONRGB_tk + LP_tk + Expression + COMMA_tk + Expression + COMMA_tk +
                                 Expression + COMMA_tk + ExpressionList + RP_tk;

    Convolution3Expression  =    CONVOLUTION3_tk + LP_tk + Expression + COMMA_tk +
                                 Expression + COMMA_tk +
                                 Expression + RP_tk;

    AutoContrastStretchExpression  // AutoContrastStretch(Image, CutoffPercentage, [HistogramPrecision])
    =    AUTOCONTRASTSTRETCH_tk + LP_tk + Expression + COMMA_tk +
         Expression + COMMA_tk +
         Expression + RP_tk
         || AUTOCONTRASTSTRETCH_tk + LP_tk + Expression + COMMA_tk +
         Expression + RP_tk;

    ContrastStretchExpression    // ContrastStretch(CutOffMin, CutOffMax)
    =    CONTRASTSTRETCH_tk + LP_tk + Expression + COMMA_tk + Expression + RP_tk;

    // Tint(RLevel, GLevel, BLevel)
    TintExpression          =    TINT_tk + LP_tk + Expression + COMMA_tk +
                                 Expression + COMMA_tk +
                                 Expression + RP_tk;

    // Inverter()
    InverterExpression      =    INVERTER_tk + LP_tk + RP_tk;

    // GammaCorrection(Value)
    GammaCorrectionExpression =  GAMMACORRECTION_tk + LP_tk + Expression + RP_tk;

    Parameter               =    NUMBER_tk + Identifier_tk
                                 || STRING_tk + Identifier_tk
                                 || OBJECT_tk + Identifier_tk;

    ParameterList           =    Parameter + COMMA_tk + ParameterList
                                 || Parameter;

    StatementDefinition     =    StatementDeclaration + LB_tk + StatementList + ReturnStatement + RB_tk
                                 || StatementDeclaration + LB_tk + ReturnStatement + RB_tk;

    StatementDeclaration    =    STATEMENT_tk + Identifier_tk + LP_tk + ParameterList + RP_tk
                                 || STATEMENT_tk + Identifier_tk + LP_tk + RP_tk;

    ReturnStatement         =    RETURN_tk + Expression;

    // Connecting...

    SetTokenizer(pTok);
    SetStartRule(&PictureScript);

    // Node creator setup

    PictureScript(&s_HPSNodeCreator);
    PageStatement(&s_PageStatementNodeCreator);
    WorldStatement(&s_WorldStatementNodeCreator);
    SelectWorldStatement(&s_SelectWorldStatementNodeCreator);
    SetLayerOnStatement(&s_SetLayerStatementNodeCreator);
    SetLayerOffStatement(&s_SetLayerStatementNodeCreator);
    SetAnnotationIconStatement(&s_SetAnnotationIconStatementNodeCreator);
    DeclarationStatement(&s_DeclarationStatementNodeCreator);
    Expression(&s_ExpressionNodeCreator);
    ObjectExpression(&s_ObjectExpressionNodeCreator);
    StringExpression(&s_StringExpressionNodeCreator);
    NumericExpression(&s_NumericExpressionNodeCreator);
    PictureObject(&s_PictureObjectNodeCreator);
    StatementInstanciation(&s_StatementInstanciationNodeCreator);
    ImageObject(&s_ImageObjectNodeCreator);
    ImageFileExpression(&s_ImageFileExpressionNodeCreator);
    MosaicExpression(&s_MosaicExpressionNodeCreator);
    OnDemandMosaicExpression(&s_OnDemandMosaicExpressionNodeCreator);
    TransformedImageExpression(&s_TransformedImageExpressionNodeCreator);
    ShapedImageExpression(&s_ShapedImageExpressionNodeCreator);
    FilteredImageExpression(&s_FilteredImageExpressionNodeCreator);
    TranslucentImageExpression(&s_TranslucentImageExpressionNodeCreator);
    ColorizedBinaryImageExpression(&s_ColorizedBinaryImageExpressionNodeCreator);
    AlphaCubeExpression(&s_AlphaCubeExpressionNodeCreator);
    RGBCubeExpression(&s_RGBCubeExpressionNodeCreator);
    LUVCubeExpression(&s_LUVCubeExpressionNodeCreator);
    GeoreferenceContextExpression(&s_GeoreferenceContextExpressionNodeCreator);
    ImageContextExpression(&s_ImageContextExpressionNodeCreator);
    AlphaPaletteExpression(&s_AlphaPaletteExpressionNodeCreator);
    VectorObject(&s_VectorObjectNodeCreator);
    ShapeExpression(&s_ShapeExpressionNodeCreator);
    RectangleExpression(&s_RectangleExpressionNodeCreator);
    PolygonExpression(&s_PolygonExpressionNodeCreator);
    HoledShapeExpression(&s_HoledShapeExpressionNodeCreator);
    UnifiedShapeExpression(&s_UnifiedShapeExpressionNodeCreator);
    CommonShapeExpression(&s_CommonShapeExpressionNodeCreator);
    TransformationObject(&s_TransfoObjectNodeCreator);
    IdentityExpression(&s_IdentityExpressionNodeCreator);
    RotationExpression(&s_RotationExpressionNodeCreator);
    ScalingExpression(&s_ScalingExpressionNodeCreator);
    TranslationExpression(&s_TranslationExpressionNodeCreator);
    AffineExpression(&s_AffineExpressionNodeCreator);
    ProjectiveExpression(&s_ProjectiveExpressionNodeCreator);
    LocalProjectiveGridExpression(&s_LocalProjectiveGridExpressionNodeCreator);
    ComposedExpression(&s_ComposedExpressionNodeCreator);
    FilterObject(&s_FilterObjectNodeCreator);
    ContrastExpression(&s_ContrastExpressionNodeCreator);
    ConvolutionRGBExpression(&s_ConvolutionRGBExpressionNodeCreator);
    Convolution3Expression(&s_Convolution3ExpressionNodeCreator);
    AutoContrastStretchExpression(&s_AutoContrastStretchExpressionNodeCreator);
    ContrastStretchExpression(&s_ContrastStretchExpressionNodeCreator);
    BrightnessExpression(&s_BrightnessExpressionNodeCreator);
    TintExpression(&s_TintExpressionNodeCreator);
    InverterExpression(&s_InverterExpressionNodeCreator);
    GammaCorrectionExpression(&s_GammaCorrectionExpressionNodeCreator);
    StatementDefinition(&s_StatementDefinitionNodeCreator);
    StatementDeclaration(&s_StatementDeclarationNodeCreator);
    ReturnStatement(&s_ReturnStatementNodeCreator);

    //Should be always set because both statements use the same expression node
    SetLayerOnStatement.SetName(L"SetLayerOnStatement");
    SetLayerOffStatement.SetName(L"SetLayerOffStatement");

#ifdef __HMR_DEBUG
    StatementIdentifier_tk.SetName(L"StatementIdentifier token");
    VariableIdentifier_tk.SetName(L"VariableIdentifier token");
    PictureScript.SetName(L"PictureScript");
    StatementList.SetName(L"StatementList");
    Statement.SetName(L"Statement");
    WorldStatement.SetName(L"WorldStatement");
    SetAnnotationIconStatement.SetName(L"SetAnnotationIconStatement");
    PageStatement.SetName(L"PageStatement");
    SelectWorldStatement.SetName(L"SelectWorldStatement");
    DeclarationStatement.SetName(L"DeclarationStatement");
    Expression.SetName(L"Expression");
    ExpressionList.SetName(L"ExpressionList");
    StringExpression.SetName(L"StringExpression");
    NumericExpression.SetName(L"NumericExpression");
    ObjectExpression.SetName(L"ObjectExpression");
    StatementInstanciation.SetName(L"StatementInstanciation");
    PictureObject.SetName(L"PictureObject");
    ImageObject.SetName(L"ImageObject");
    ImageFileExpression.SetName(L"ImageFileExpression");
    MosaicExpression.SetName(L"MosaicExpression");
    OnDemandMosaicExpression.SetName(L"OnDemandMosaicExpression");
    TransformedImageExpression.SetName(L"TransformedImageExpression");
    ShapedImageExpression.SetName(L"ShapedImageExpression");
    FilteredImageExpression.SetName(L"FilteredImageExpression");
    TranslucentImageExpression.SetName(L"TranslucentImageExpression");
    ColorizedBinaryImageExpression.SetName(L"ColorizedBinaryImageExpression");
    AlphaCubeExpression.SetName(L"AlphaCubeExpression");
    RGBCubeExpression.SetName(L"RGBCubeExpression");
    LUVCubeExpression.SetName(L"LUVCubeExpression");
    GeoreferenceContextExpression.SetName(L"GeoreferenceContextExpression");
    ImageContextExpression.SetName(L"ImageContextExpression");
    AlphaPaletteExpression.SetName(L"AlphaPaletteExpression");
    RangeExpression.SetName(L"RangeExpression");
    RangeExpressionList.SetName(L"RangeExpressionList");
    VectorObject.SetName(L"VectorObject");
    ShapeExpression.SetName(L"ShapeExpression");
    RectangleExpression.SetName(L"RectangleExpression");
    PolygonExpression.SetName(L"PolygonExpression");
    HoledShapeExpression.SetName(L"HoledShapeExpression");
    UnifiedShapeExpression.SetName(L"UnifiedShapeExpression");
    CommonShapeExpression.SetName(L"CommonShapeExpression");
    TransformationObject.SetName(L"TransformationObject");
    IdentityExpression.SetName(L"IdentityExpression");
    RotationExpression.SetName(L"RotationExpression");
    ScalingExpression.SetName(L"ScalingExpression");
    TranslationExpression.SetName(L"TranslationExpression");
    AffineExpression.SetName(L"AffineExpression");
    ProjectiveExpression.SetName(L"ProjectiveExpression");
    LocalProjectiveGridExpression.SetName(L"LocalProjectiveGridExpression");
    ComposedExpression.SetName(L"ComposedExpression");
    FilterObject.SetName(L"FilterObject");
    ContrastExpression.SetName(L"ContrastExpression");
    BrightnessExpression.SetName(L"BrightnessExpression");
    ConvolutionRGBExpression.SetName(L"ConvolutionRGBExpression");
    Convolution3Expression.SetName(L"Convolution3Expression");
    AutoContrastStretchExpression.SetName(L"AutoContrastStretchExpression");
    ContrastStretchExpression.SetName(L"ContrastStretchExpression");
    TintExpression.SetName(L"TintExpression");
    InverterExpression.SetName(L"InverterExpression");
    GammaCorrectionExpression.SetName(L"GammaCorrectionExpression");
    Parameter.SetName(L"Parameter");
    ParameterList.SetName(L"ParameterList");
    StatementDefinition.SetName(L"StatementDefinition");
    StatementDeclaration.SetName(L"StatementDeclaration");
    ReturnStatement.SetName(L"ReturnStatement");
#endif
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPSParser::~HPSParser()
    {

    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HPANode* HPSParser::Parse(HFCBinStream*                  pi_pStream,
                          HPMPool*                       pi_pPool,
                          const HFCPtr<HFCURL>&          pi_rpURL,
                          const HFCPtr<HPSWorldCluster>& pi_rpHPSWorldCluster,
                          const HGF2DWorldIdentificator* pi_pCurrentWorldId)
    {
    HFCPtr<HPASession> pSession;
    HFCPtr<HFCURL>     pURL;

    try
        {
        if (pi_rpURL != 0)
            {
            pURL = pi_rpURL;
            }
        else
            {
            pURL = pi_pStream->GetURL();
            }

        if (pi_rpHPSWorldCluster != 0)
            {
            pSession = new HPSSession(pi_pStream,
                                      pi_pPool,
                                      pURL,
                                      new HPSParserScope(this),
                                      pi_rpHPSWorldCluster,
                                      pi_pCurrentWorldId);
            }
        else //Use the default HPS world cluster.
            {
            HASSERT(pi_pCurrentWorldId == 0);

            pSession = new HPSSession(pi_pStream,
                                      pi_pPool,
                                      pURL,
                                      new HPSParserScope(this));
            }

        return HPAParser::Parse(pSession);
        }
    catch (...)
        {
        if (pSession != 0)
            {
            ((HFCPtr<HPSSession>&)pSession)->Clear();
            }
        throw;
        }
    }

HPANodeList& HPSParser::GetWorldRelatedNodes()
    {
    return m_WorldRelatedNodes;
    }