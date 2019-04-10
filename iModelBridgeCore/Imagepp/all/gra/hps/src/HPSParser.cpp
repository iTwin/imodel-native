//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSParser.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class HPSParser
//---------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HPSParser.h>
#include <ImagePP/all/h/HPSNode.h>
#include "HPSParserScope.h"
#include <ImagePP/all/h/HPSException.h>
#include <ImagePP/all/h/HPSTokenizer.h>
#include "HPSInternalNodes.h"
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HFCBinStream.h>

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
    pTok->SetCommentMarker(';');

    // Tokenizer : special symbols

    pTok->AddSymbol("(", LP_tk);
    pTok->AddSymbol(")", RP_tk);
    pTok->AddSymbol("{", LB_tk);
    pTok->AddSymbol("}", RB_tk);
    pTok->AddSymbol(",", COMMA_tk);
    pTok->AddSymbol("=", EQ_tk);
    pTok->AddSymbol("+", PLUS_tk);
    pTok->AddSymbol("-", MINUS_tk);
    pTok->AddSymbol("@", AT_tk);
    pTok->AddSymbol(".", DOT_tk);
    pTok->AddSymbol("~", TILDE_tk);

    // Tokenizer : reserved keywords

    pTok->AddSymbol("ADJUST", ADJUST_tk);
    pTok->AddSymbol("ADJ", ADJUST_tk);
    pTok->AddSymbol("AFFINE", AFFINE_tk);
    pTok->AddSymbol("AFF", AFFINE_tk);
    pTok->AddSymbol("ALPHACUBE", ALPHACUBE_tk);
    pTok->AddSymbol("ALC", ALPHACUBE_tk);
    pTok->AddSymbol("ALPHAPALETTE", ALPHAPALETTE_tk);
    pTok->AddSymbol("ALP", ALPHAPALETTE_tk);
    pTok->AddSymbol("ANGULAR", ANGULAR_tk);
    pTok->AddSymbol("ANG", ANGULAR_tk);
    pTok->AddSymbol("ANTI_ALIASING", ANTI_ALIASING_tk);
    pTok->AddSymbol("ANTIA", ANTI_ALIASING_tk);
    pTok->AddSymbol("ARROW", ARROW_tk);
    pTok->AddSymbol("ARR", ARROW_tk);
    pTok->AddSymbol("AUTOCONTRASTSTRETCH", AUTOCONTRASTSTRETCH_tk);
    pTok->AddSymbol("AVERAGE", AVERAGE_tk);
    pTok->AddSymbol("AVG", AVERAGE_tk);
    pTok->AddSymbol("BLANKIMAGE", BLANKIMAGE_tk);
    pTok->AddSymbol("BIMG", BLANKIMAGE_tk);
    pTok->AddSymbol("BLUR", BLUR_tk);
    pTok->AddSymbol("BOLD", BOLD_tk);
    pTok->AddSymbol("BOTTOM", BOTTOM_tk);
    pTok->AddSymbol("BOT", BOTTOM_tk);
    pTok->AddSymbol("BRIGHTNESS", BRIGHTNESS_tk);
    pTok->AddSymbol("BR", BRIGHTNESS_tk);
    pTok->AddSymbol("CENTER", CENTER_tk);
    pTok->AddSymbol("CTR", CENTER_tk);
    pTok->AddSymbol("CIRCLE", CIRCLE_tk);
    pTok->AddSymbol("CIR", CIRCLE_tk);
    pTok->AddSymbol("COLORIZEDBINARYIMAGE", COLORIZEDBINARYIMAGE_tk);
    pTok->AddSymbol("COLBI", COLORIZEDBINARYIMAGE_tk);
    pTok->AddSymbol("COMMONSHAPE", COMMONSHAPE_tk);
    pTok->AddSymbol("CMNS", COMMONSHAPE_tk);
    pTok->AddSymbol("COMPOSED", COMPOSED_tk);
    pTok->AddSymbol("CP", COMPOSED_tk);
    pTok->AddSymbol("CONTRAST", CONTRAST_tk);
    pTok->AddSymbol("CONTRASTSTRETCH", CONTRASTSTRETCH_tk);
    pTok->AddSymbol("CT", CONTRAST_tk);
    pTok->AddSymbol("CONVOLUTION", CONVOLUTION_tk);
    pTok->AddSymbol("CONV", CONVOLUTION_tk);
    pTok->AddSymbol("CONVOLUTIONRGB", CONVOLUTIONRGB_tk);
    pTok->AddSymbol("CONVRGB", CONVOLUTIONRGB_tk);
    pTok->AddSymbol("CONVOLUTION3", CONVOLUTION3_tk);
    pTok->AddSymbol("CONV3", CONVOLUTION3_tk);
    pTok->AddSymbol("CURVE", CURVE_tk);
    pTok->AddSymbol("CU", CURVE_tk);
    pTok->AddSymbol("DASHDOT", DASHDOT_tk);
    pTok->AddSymbol("DASHES", DASHES_tk);
    pTok->AddSymbol("DA", DASHES_tk);
    pTok->AddSymbol("DESPECKLE", DESPECKLE_tk);
    pTok->AddSymbol("DPKL", DESPECKLE_tk);
    pTok->AddSymbol("DIAMOND", DIAMOND_tk);
    pTok->AddSymbol("DIA", DIAMOND_tk);
    pTok->AddSymbol("DOT", DOT_KW_tk);
    pTok->AddSymbol("DOTS", DOTS_tk);
    pTok->AddSymbol("DRAWING", DRAWING_tk);
    pTok->AddSymbol("DR", DRAWING_tk);
    pTok->AddSymbol("EMBOSS", EMBOSS_tk);
    pTok->AddSymbol("EMB", EMBOSS_tk);
    pTok->AddSymbol("ELLIPSE", ELLIPSE_tk);
    pTok->AddSymbol("ELL", ELLIPSE_tk);
    pTok->AddSymbol("EXPORT", EXPORT_tk);
    pTok->AddSymbol("EXP", EXPORT_tk);
    pTok->AddSymbol("EXTENT", EXTENT_tk);
    pTok->AddSymbol("EXT", EXTENT_tk);
    pTok->AddSymbol("FILL", FILL_tk);
    pTok->AddSymbol("FILLPATTERN", FILLPATTERN_tk);
    pTok->AddSymbol("FPAT", FILLPATTERN_tk);
    pTok->AddSymbol("FILTER", FILTER_tk);
    pTok->AddSymbol("FIL", FILTER_tk);
    pTok->AddSymbol("FILTEREDIMAGE", FILTEREDIMAGE_tk);
    pTok->AddSymbol("FII", FILTEREDIMAGE_tk);
    pTok->AddSymbol("FIT", FIT_tk);
    pTok->AddSymbol("FITTEDIMAGE", FITTEDIMAGE_tk);
    pTok->AddSymbol("FITI", FITTEDIMAGE_tk);
    pTok->AddSymbol("FOLDER", FOLDER_tk);
    pTok->AddSymbol("FOL", FOLDER_tk);
    pTok->AddSymbol("GAMMACORRECTION", GAMMACORRECTION_tk);
    pTok->AddSymbol("GAMC", GAMMACORRECTION_tk);
    pTok->AddSymbol("GEOREFERENCECONTEXT", GEOREFERENCECONTEXT_tk);
    pTok->AddSymbol("GEOR", GEOREFERENCECONTEXT_tk);
    pTok->AddSymbol("GLOW", GLOW_tk);
    pTok->AddSymbol("HEIGHT", HEIGHT_tk);
    pTok->AddSymbol("HEI", HEIGHT_tk);
    pTok->AddSymbol("HOLEDSHAPE", HOLEDSHAPE_tk);
    pTok->AddSymbol("HSH", HOLEDSHAPE_tk);
    pTok->AddSymbol("IDENTITY", IDENTITY_tk);
    pTok->AddSymbol("IDTY", IDENTITY_tk);
    pTok->AddSymbol("ITALIC", ITALIC_tk);
    pTok->AddSymbol("ITAL", ITALIC_tk);
    pTok->AddSymbol("IMAGECONTEXT", IMAGECONTEXT_tk);
    pTok->AddSymbol("IFC", IMAGECONTEXT_tk);
    pTok->AddSymbol("IMAGEFILE", IMAGEFILE_tk);
    pTok->AddSymbol("IMF", IMAGEFILE_tk);
    pTok->AddSymbol("IMPORTNUMBER", IMPORTNUMBER_tk);
    pTok->AddSymbol("IMPN", IMPORTNUMBER_tk);
    pTok->AddSymbol("IMPORTOBJECT", IMPORTOBJECT_tk);
    pTok->AddSymbol("IMPO", IMPORTOBJECT_tk);
    pTok->AddSymbol("IMPORTSTRING", IMPORTSTRING_tk);
    pTok->AddSymbol("IMPS", IMPORTSTRING_tk);
    pTok->AddSymbol("INCLUDE", INCLUDE_tk);
    pTok->AddSymbol("INC", INCLUDE_tk);
    pTok->AddSymbol("INVERTER", INVERTER_tk);
    pTok->AddSymbol("INV", INVERTER_tk);
    pTok->AddSymbol("LEFT", LEFT_tk);
    pTok->AddSymbol("LOCATION", LOCATION_tk);
    pTok->AddSymbol("LOC", LOCATION_tk);
    pTok->AddSymbol("LOCALPROJECTIVEGRID", LOCALPROJECTIVEGRID_tk);
    pTok->AddSymbol("LONGDASHDOT", LONGDASHDOT_tk);
    pTok->AddSymbol("LONGDASHDOTDOT", LONGDASHDOTDOT_tk);
    pTok->AddSymbol("LONGDASHES", LONGDASHES_tk);
    pTok->AddSymbol("LUVCUBE", LUVCUBE_tk);
    pTok->AddSymbol("LUVC", LUVCUBE_tk);
    pTok->AddSymbol("MOSAIC", MOSAIC_tk);
    pTok->AddSymbol("MO", MOSAIC_tk);
    pTok->AddSymbol("MOVEDIMAGE", MOVEDIMAGE_tk);
    pTok->AddSymbol("MVI", MOVEDIMAGE_tk);
    pTok->AddSymbol("MULTIRESIMAGE", MULTIRESIMAGE_tk);
    pTok->AddSymbol("MRI", MULTIRESIMAGE_tk);
    pTok->AddSymbol("NONE", NONE_tk);
    pTok->AddSymbol("NEAREST_NEIGHBOR", NEAREST_NEIGHBOR_tk);
    pTok->AddSymbol("N_N", NEAREST_NEIGHBOR_tk);
    pTok->AddSymbol("NUMBER", NUMBER_tk);
    pTok->AddSymbol("NUM", NUMBER_tk);
    pTok->AddSymbol("OBJECT", OBJECT_tk);
    pTok->AddSymbol("OBJ", OBJECT_tk);
    pTok->AddSymbol("OPENARROW", OPENARROW_tk);
    pTok->AddSymbol("OARR", OPENARROW_tk);
    pTok->AddSymbol("ONDEMANDMOSAIC", ONDEMANDMOSAIC_tk);
    pTok->AddSymbol("ODMO", ONDEMANDMOSAIC_tk);
    pTok->AddSymbol("ORIGIN", ORIGIN_tk);
    pTok->AddSymbol("ORI", ORIGIN_tk);
    pTok->AddSymbol("PAGE", PAGE_tk);
    pTok->AddSymbol("PG", PAGE_tk);
    pTok->AddSymbol("PENSTYLE", PENSTYLE_tk);
    pTok->AddSymbol("PEN", PENSTYLE_tk);
    pTok->AddSymbol("PERIMETER", PERIMETER_tk);
    pTok->AddSymbol("PERI", PERIMETER_tk);
    pTok->AddSymbol("PICTURE", PICTURE_tk);
    pTok->AddSymbol("PIC", PICTURE_tk);
    pTok->AddSymbol("PIXELSIZE", PIXELSIZE_tk);
    pTok->AddSymbol("PIXS", PIXELSIZE_tk);
    pTok->AddSymbol("PLAIN", PLAIN_tk);
    pTok->AddSymbol("POINT", POINT_tk);
    pTok->AddSymbol("PT", POINT_tk);
    pTok->AddSymbol("POLYGON", POLYGON_tk);
    pTok->AddSymbol("POG", POLYGON_tk);
    pTok->AddSymbol("POLYLINE", POLYLINE_tk);
    pTok->AddSymbol("PLIN", POLYLINE_tk);
    pTok->AddSymbol("POLYSEGMENT", POLYSEGMENT_tk);
    pTok->AddSymbol("PROJECTIVE", PROJECTIVE_tk);
    pTok->AddSymbol("PROJ", PROJECTIVE_tk);
    pTok->AddSymbol("PSEG", POLYSEGMENT_tk);
    pTok->AddSymbol("RECTANGLE", RECTANGLE_tk);
    pTok->AddSymbol("RECT", RECTANGLE_tk);
    pTok->AddSymbol("REPLACE", REPLACE_tk);
    pTok->AddSymbol("REPL", REPLACE_tk);
    pTok->AddSymbol("RESAMPLEDIMAGE", RESAMPLEDIMAGE_tk);
    pTok->AddSymbol("RSI", RESAMPLEDIMAGE_tk);
    pTok->AddSymbol("RETURN", RETURN_tk);
    pTok->AddSymbol("RET", RETURN_tk);
    pTok->AddSymbol("RTN", RETURN_tk);
    pTok->AddSymbol("RGB", RGB_tk);
    pTok->AddSymbol("RGBCUBE", RGBCUBE_tk);
    pTok->AddSymbol("RGBC", RGBCUBE_tk);
    pTok->AddSymbol("RIGHT", RIGHT_tk);
    pTok->AddSymbol("ROTATION", ROTATION_tk);
    pTok->AddSymbol("ROT", ROTATION_tk);
    pTok->AddSymbol("ROUND", ROUND_tk);
    pTok->AddSymbol("SCALING", SCALING_tk);
    pTok->AddSymbol("SCA", SCALING_tk);
    pTok->AddSymbol("SCENE", SCENE_tk);
    pTok->AddSymbol("SCE", SCENE_tk);
    pTok->AddSymbol("SEGMENT", SEGMENT_tk);
    pTok->AddSymbol("SEG", SEGMENT_tk);
    pTok->AddSymbol("SELECTWORLD", SELECTWORLD_tk);
    pTok->AddSymbol("SELW", SELECTWORLD_tk);
    pTok->AddSymbol("SETANNOTATIONICON", SETANNOTATIONICON_tk);
    pTok->AddSymbol("SETAI", SETANNOTATIONICON_tk);
    pTok->AddSymbol("SETLAYERON", SETLAYERON_tk);
    pTok->AddSymbol("SETLO", SETLAYERON_tk);
    pTok->AddSymbol("SETLAYEROFF", SETLAYEROFF_tk);
    pTok->AddSymbol("SETLF", SETLAYEROFF_tk);
    pTok->AddSymbol("SHADOW", SHADOW_tk);
    pTok->AddSymbol("SHAD", SHADOW_tk);
    pTok->AddSymbol("SHAPE", SHAPE_tk);
    pTok->AddSymbol("SHP", SHAPE_tk);
    pTok->AddSymbol("SHAPESTYLE", SHAPESTYLE_tk);
    pTok->AddSymbol("SHPS", SHAPESTYLE_tk);
    pTok->AddSymbol("SHAPEDIMAGE", SHAPEDIMAGE_tk);
    pTok->AddSymbol("SHI", SHAPEDIMAGE_tk);
    pTok->AddSymbol("SHARPEN", SHARPEN_tk);
    pTok->AddSymbol("SHAR", SHARPEN_tk);
    pTok->AddSymbol("STATEMENT", STATEMENT_tk);
    pTok->AddSymbol("STAT", STATEMENT_tk);
    pTok->AddSymbol("STRING", STRING_tk);
    pTok->AddSymbol("STR", STRING_tk);
    pTok->AddSymbol("TEXT", TEXT_tk);
    pTok->AddSymbol("TXT", TEXT_tk);
    pTok->AddSymbol("TEXTSTYLE", TEXTSTYLE_tk);
    pTok->AddSymbol("TSTY", TEXTSTYLE_tk);
    pTok->AddSymbol("TINT", TINT_tk);
    pTok->AddSymbol("TOP", TOP_tk);
    pTok->AddSymbol("TRANSFORMEDIMAGE", TRANSFORMEDIMAGE_tk);
    pTok->AddSymbol("TFI", TRANSFORMEDIMAGE_tk);
    pTok->AddSymbol("TRANSLATION", TRANSLATION_tk);
    pTok->AddSymbol("TRL", TRANSLATION_tk);
    pTok->AddSymbol("TRANSLUCENTIMAGE", TRANSLUCENTIMAGE_tk);
    pTok->AddSymbol("TLI", TRANSLUCENTIMAGE_tk);
    pTok->AddSymbol("UNDERLINED", UNDERLINED_tk);
    pTok->AddSymbol("UNDL", UNDERLINED_tk);
    pTok->AddSymbol("UNIFIEDSHAPE", UNIFIEDSHAPE_tk);
    pTok->AddSymbol("UNFS", UNIFIEDSHAPE_tk);
    pTok->AddSymbol("USE_EXTENSION", USE_EXTENSION_tk);
    pTok->AddSymbol("U_E", USE_EXTENSION_tk);
    pTok->AddSymbol("USING", USING_tk);
    pTok->AddSymbol("VECTORARRAY", VECTORARRAY_tk);
    pTok->AddSymbol("VECA", VECTORARRAY_tk);
    pTok->AddSymbol("VECTOR_AWARENESS", VECTOR_AWARENESS_tk);
    pTok->AddSymbol("V_A", VECTOR_AWARENESS_tk);
    pTok->AddSymbol("VECTORFILE", VECTORFILE_tk);
    pTok->AddSymbol("VECF", VECTORFILE_tk);
    pTok->AddSymbol("WARPEDIMAGE", WARPEDIMAGE_tk);
    pTok->AddSymbol("WPI", WARPEDIMAGE_tk);
    pTok->AddSymbol("WATERMARK", WATERMARK_tk);
    pTok->AddSymbol("WMK", WATERMARK_tk);
    pTok->AddSymbol("WIDTH", WIDTH_tk);
    pTok->AddSymbol("WI", WIDTH_tk);
    pTok->AddSymbol("WORLD", WORLD_tk);
    pTok->AddSymbol("WO", WORLD_tk);
    pTok->AddSymbol("X", X_tk);
    pTok->AddSymbol("Y", Y_tk);

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
    SetLayerOnStatement.SetName("SetLayerOnStatement");
    SetLayerOffStatement.SetName("SetLayerOffStatement");

#ifdef __HMR_DEBUG
    StatementIdentifier_tk.SetName("StatementIdentifier token");
    VariableIdentifier_tk.SetName("VariableIdentifier token");
    PictureScript.SetName("PictureScript");
    StatementList.SetName("StatementList");
    Statement.SetName("Statement");
    WorldStatement.SetName("WorldStatement");
    SetAnnotationIconStatement.SetName("SetAnnotationIconStatement");
    PageStatement.SetName("PageStatement");
    SelectWorldStatement.SetName("SelectWorldStatement");
    DeclarationStatement.SetName("DeclarationStatement");
    Expression.SetName("Expression");
    ExpressionList.SetName("ExpressionList");
    StringExpression.SetName("StringExpression");
    NumericExpression.SetName("NumericExpression");
    ObjectExpression.SetName("ObjectExpression");
    StatementInstanciation.SetName("StatementInstanciation");
    PictureObject.SetName("PictureObject");
    ImageObject.SetName("ImageObject");
    ImageFileExpression.SetName("ImageFileExpression");
    MosaicExpression.SetName("MosaicExpression");
    OnDemandMosaicExpression.SetName("OnDemandMosaicExpression");
    TransformedImageExpression.SetName("TransformedImageExpression");
    ShapedImageExpression.SetName("ShapedImageExpression");
    FilteredImageExpression.SetName("FilteredImageExpression");
    TranslucentImageExpression.SetName("TranslucentImageExpression");
    ColorizedBinaryImageExpression.SetName("ColorizedBinaryImageExpression");
    AlphaCubeExpression.SetName("AlphaCubeExpression");
    RGBCubeExpression.SetName("RGBCubeExpression");
    LUVCubeExpression.SetName("LUVCubeExpression");
    GeoreferenceContextExpression.SetName("GeoreferenceContextExpression");
    ImageContextExpression.SetName("ImageContextExpression");
    AlphaPaletteExpression.SetName("AlphaPaletteExpression");
    RangeExpression.SetName("RangeExpression");
    RangeExpressionList.SetName("RangeExpressionList");
    VectorObject.SetName("VectorObject");
    ShapeExpression.SetName("ShapeExpression");
    RectangleExpression.SetName("RectangleExpression");
    PolygonExpression.SetName("PolygonExpression");
    HoledShapeExpression.SetName("HoledShapeExpression");
    UnifiedShapeExpression.SetName("UnifiedShapeExpression");
    CommonShapeExpression.SetName("CommonShapeExpression");
    TransformationObject.SetName("TransformationObject");
    IdentityExpression.SetName("IdentityExpression");
    RotationExpression.SetName("RotationExpression");
    ScalingExpression.SetName("ScalingExpression");
    TranslationExpression.SetName("TranslationExpression");
    AffineExpression.SetName("AffineExpression");
    ProjectiveExpression.SetName("ProjectiveExpression");
    LocalProjectiveGridExpression.SetName("LocalProjectiveGridExpression");
    ComposedExpression.SetName("ComposedExpression");
    FilterObject.SetName("FilterObject");
    ContrastExpression.SetName("ContrastExpression");
    BrightnessExpression.SetName("BrightnessExpression");
    ConvolutionRGBExpression.SetName("ConvolutionRGBExpression");
    Convolution3Expression.SetName("Convolution3Expression");
    AutoContrastStretchExpression.SetName("AutoContrastStretchExpression");
    ContrastStretchExpression.SetName("ContrastStretchExpression");
    TintExpression.SetName("TintExpression");
    InverterExpression.SetName("InverterExpression");
    GammaCorrectionExpression.SetName("GammaCorrectionExpression");
    Parameter.SetName("Parameter");
    ParameterList.SetName("ParameterList");
    StatementDefinition.SetName("StatementDefinition");
    StatementDeclaration.SetName("StatementDeclaration");
    ReturnStatement.SetName("ReturnStatement");
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
