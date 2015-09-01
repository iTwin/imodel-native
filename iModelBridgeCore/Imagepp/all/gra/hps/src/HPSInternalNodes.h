//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSInternalNodes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Node types, in order of appearance in grammar description
// Note : the node for PictureScript rule is HPSNode.
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCMemoryLineStream.h>

#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGFLUVCube.h>
#include <Imagepp/all/h/HGFRGBCube.h>

#include <Imagepp/all/h/HMDContext.h>

#include <Imagepp/all/h/HPSNode.h>
#include <Imagepp/all/h/HPSValueNode.h>
#include "HPSSession.h"
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRFGeoreferenceContext.h>
#include <Imagepp/all/h/HRPFilter.h>
#include <Imagepp/all/h/HRPAlphaRange.h>

BEGIN_IMAGEPP_NAMESPACE

class HPSParser;
class HPMPool;
class HGF2DWorldCluster;
class HGF2DCoordSys;

template<class T>
class HPSValue_T : public HPSObjectValue
    {
public:
    HPSValue_T(HFCPtr<T> pObject) :m_pObject(pObject){};
    virtual ~HPSValue_T() {};

    HFCPtr<T> m_pObject;
    };

typedef HPSValue_T<HVEShape>               HPSShapeObjectValue;
typedef HPSValue_T<HRARaster>              HPSRasterObjectValue;
typedef HPSValue_T<HGF2DTransfoModel>      HPSTransfoObjectValue;
typedef HPSValue_T<HRPFilter>              HPSFilterObjectValue;
typedef HPSValue_T<HRPAlphaRange>          HPSAlphaRangeObjectValue;
typedef HPSValue_T<HGFColorSet>            HPSColorSetObjectValue;
typedef HPSValue_T<HRFGeoreferenceContext> HPSGeoreferenceContextObjectValue;

//---------------------------------------------------------------------------
class PageStatementNode : public HPANode
    {
public:
    PageStatementNode(HPAGrammarObject* pi_pObj,
                      const HPANodeList& pi_rList,
                      const HFCPtr<HPASession>& pi_pSession);
    HPSRasterObjectValue* ComputeObject() const;
    virtual ~PageStatementNode() { }
    };

//---------------------------------------------------------------------------
class WorldStatementNode : public HPANode
    {
public:
    WorldStatementNode(HPAGrammarObject* pi_pObj,
                       const HPANodeList& pi_rList,
                       const HFCPtr<HPASession>& pi_pSession);
    virtual ~WorldStatementNode()  { }
    };

//---------------------------------------------------------------------------
class SelectWorldStatementNode : public HPANode
    {
public:
    SelectWorldStatementNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession);
    virtual ~SelectWorldStatementNode()  { }
    };

//---------------------------------------------------------------------------
class SetLayerStatementNode : public HPANode
    {
public:
    SetLayerStatementNode(HPAGrammarObject*         pi_pObj,
                          const HPANodeList&        pi_rList,
                          const HFCPtr<HPASession>& pi_pSession);
    virtual ~SetLayerStatementNode()  { }
    };

//---------------------------------------------------------------------------
class SetAnnotationIconStatementNode : public HPANode
    {
public:
    SetAnnotationIconStatementNode(HPAGrammarObject*         pi_pObj,
                                   const HPANodeList&        pi_rList,
                                   const HFCPtr<HPASession>& pi_pSession);
    virtual ~SetAnnotationIconStatementNode()  { }
    };

//---------------------------------------------------------------------------
class DeclarationStatementNode : public HPANode
    {
public:
    DeclarationStatementNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession);
    virtual ~DeclarationStatementNode()  { }
    };

//---------------------------------------------------------------------------
class ExpressionNode : public HPSValueNode
    {
public:
    ExpressionNode(HPAGrammarObject* pi_pObj,
                   const HPANodeList& pi_rList,
                   const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ExpressionNode() { } //TBD
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class VariableTokenNode : public HPANode
    {
public:
    VariableTokenNode(HPAGrammarObject* pi_pObj,
                      ExpressionNode* pi_pNode,
                      const HPASourcePos& pi_rLeftPos,
                      const HPASourcePos& pi_rRightPos,
                      const HFCPtr<HPSSession>& pi_pSession);
    virtual         ~VariableTokenNode();
    virtual void    FreeValue();
    void            ResetExpression(ExpressionNode* pi_pNode);
    void            Reset();
    void            Calculate();
    void            SetValueOwnership(bool pi_IsOwner);
    bool           GetValueOwnership() const;
    ExpressionNode* GetExpressionNode() {
        return m_pExpressionNode;
        }

    HPSValueNode::ValueType m_Type;
    HPSValueNode::Value     m_Value;
private:
    bool           m_IsCalculated;
    ExpressionNode* m_pExpressionNode;
    };

//---------------------------------------------------------------------------
class VariableRefNode : public HPANode
    {
public:
    VariableRefNode(HPAGrammarObject*         pi_pObj,
                    VariableTokenNode*        pi_pNode,
                    const HPASourcePos&       pi_rLeftPos,
                    const HPASourcePos&       pi_rRightPos,
                    const HFCPtr<HPASession>& pi_pSession);
    virtual            ~VariableRefNode();
    virtual void       FreeValue();
    void               Calculate();
    void               SetValueOwnership(bool pi_IsOwner);
    bool              GetValueOwnership() const;
    VariableTokenNode* GetVariableTokenNode() {
        return m_pTokenNode;
        }

    HPSValueNode::ValueType m_Type;
    HPSValueNode::Value     m_Value;

private:
    VariableTokenNode* m_pTokenNode;
    };

//---------------------------------------------------------------------------
class StatementDefinitionNode;
class StatementTokenNode : public HPANode
    {
public:
    StatementTokenNode(HPAGrammarObject* pi_pObj,
                       StatementDefinitionNode* pi_pNode,
                       const HPASourcePos& pi_rLeftPos,
                       const HPASourcePos& pi_rRightPos,
                       const HFCPtr<HPSSession>& pi_pSession);
    virtual ~StatementTokenNode() { }
    StatementDefinitionNode* m_pStatementNode;
private:
    };

//---------------------------------------------------------------------------
class StringExpressionNode : public HPSValueNode
    {
public:
    StringExpressionNode(HPAGrammarObject* pi_pObj,
                         const HPANodeList& pi_rList,
                         const HFCPtr<HPASession>& pi_pSession);
    virtual ~StringExpressionNode() { } //TBD
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class NumericExpressionNode : public HPSValueNode
    {
public:
    NumericExpressionNode(HPAGrammarObject* pi_pObj,
                          const HPANodeList& pi_rList,
                          const HFCPtr<HPASession>& pi_pSession);
    virtual ~NumericExpressionNode() { } //TBD
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ObjectExpressionNode : public HPSValueNode
    {
public:
    ObjectExpressionNode(HPAGrammarObject* pi_pObj,
                         const HPANodeList& pi_rList,
                         const HFCPtr<HPASession>& pi_pSession);
    virtual ~ObjectExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class PictureObjectNode : public HPSValueNode
    {
public:
    PictureObjectNode(HPAGrammarObject* pi_pObj,
                      const HPANodeList& pi_rList,
                      const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~PictureObjectNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class StatementInstanciationNode : public HPSValueNode
    {
public:
    StatementInstanciationNode(HPAGrammarObject* pi_pObj,
                               const HPANodeList& pi_rList,
                               const HFCPtr<HPASession>& pi_pSession);
    virtual ~StatementInstanciationNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ImageObjectNode : public HPSValueNode
    {
public:
    ImageObjectNode(HPAGrammarObject* pi_pObj,
                    const HPANodeList& pi_rList,
                    const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ImageObjectNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ImageFileExpressionNode : public HPSValueNode
    {
public:
    ImageFileExpressionNode(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession);
    virtual ~ImageFileExpressionNode()  { }
    virtual void Calculate();
    void         GetFileURL(HFCPtr<HFCURL>& po_rpFileURL) const;
    };

//---------------------------------------------------------------------------
class MosaicExpressionNode : public HPSValueNode
    {
public:
    MosaicExpressionNode(HPAGrammarObject* pi_pObj,
                         const HPANodeList& pi_rList,
                         const HFCPtr<HPASession>& pi_pSession);
    virtual ~MosaicExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
struct DescriptivePSSFileInfo
    {
    HFCPtr<HFCURL> m_pPSSFileName;
    list<uint32_t>  m_Lines;
    };

typedef list<DescriptivePSSFileInfo> DescriptivePSSFileInfoList;
typedef list<HFCMemoryLineStream*>   LineStreamList;

class OnDemandMosaicExpressionNode : public HPSValueNode
    {
public:
    OnDemandMosaicExpressionNode(HPAGrammarObject*         pi_pObj,
                                 const HPANodeList&        pi_rList,
                                 const HFCPtr<HPASession>& pi_pSession);
    virtual ~OnDemandMosaicExpressionNode();

    virtual void Calculate();

private:

    void GetRasterDescriptivePSS(const HFCPtr<HPANode>& pi_rpRasterNode,
                                 bool                  pi_EnclosedLastLineByPG,
                                 WString&               po_rPSS);

    void AddLineToDescriptivePSS(HFCMemoryLineStream& pi_rFileStream,
                                 uint32_t            pi_LineNb,
                                 WString&             po_rPSS,
                                 int32_t            pi_StartingColPos = -1,
                                 int32_t            pi_EndingColPos   = -1);

    void GetDescriptivePSSInfoFromNodes(const HFCPtr<HPANode>&      pi_rDescriptiveNode,
                                        DescriptivePSSFileInfoList& pio_rDescriptivePSS);

    void MergeNodeInfoToDescriptivePSSInfo(const HFCPtr<HPANode>&      pi_rpRasterNode,
                                           DescriptivePSSFileInfoList& pio_rDescriptivePSS);

    void UpdatePixelSizeRange(HFCPtr<HRARaster>&     pi_rpRaster,
                              const HFCPtr<HGF2DCoordSys>& pi_rpMosaicCoordSys);

    HFCMemoryLineStream& GetLineStream(WString& pi_rPSSFileName);

    LineStreamList m_pLineStreams;

    //Pixel size range information
    bool           m_PixelSizeRangeInit;
    double         m_PixelSizeRangeMinArea;
    double         m_PixelSizeRangeMaxArea;
    HGF2DExtent    m_PixelSizeRangeMinExt;
    HGF2DExtent    m_PixelSizeRangeMaxExt;
    };

//---------------------------------------------------------------------------
class TransformedImageExpressionNode : public HPSValueNode
    {
public:
    TransformedImageExpressionNode(HPAGrammarObject* pi_pObj,
                                   const HPANodeList& pi_rList,
                                   const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~TransformedImageExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ShapedImageExpressionNode : public HPSValueNode
    {
public:
    ShapedImageExpressionNode(HPAGrammarObject* pi_pObj,
                              const HPANodeList& pi_rList,
                              const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ShapedImageExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class FilteredImageExpressionNode : public HPSValueNode
    {
public:
    FilteredImageExpressionNode(HPAGrammarObject* pi_pObj,
                                const HPANodeList& pi_rList,
                                const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~FilteredImageExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class TranslucentImageExpressionNode : public HPSValueNode
    {
public:
    TranslucentImageExpressionNode(HPAGrammarObject* pi_pObj,
                                   const HPANodeList& pi_rList,
                                   const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~TranslucentImageExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ColorizedBinaryImageExpressionNode : public HPSValueNode
    {
public:
    ColorizedBinaryImageExpressionNode(HPAGrammarObject* pi_pObj,
                                       const HPANodeList& pi_rList,
                                       const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ColorizedBinaryImageExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class AlphaCubeExpressionNode : public HPSValueNode
    {
public:
    AlphaCubeExpressionNode(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~AlphaCubeExpressionNode();
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class RGBCubeExpressionNode : public HPSValueNode
    {
public:
    RGBCubeExpressionNode(HPAGrammarObject* pi_pObj,
                          const HPANodeList& pi_rList,
                          const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~RGBCubeExpressionNode();
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class LUVCubeExpressionNode : public HPSValueNode
    {
public:
    LUVCubeExpressionNode(HPAGrammarObject* pi_pObj,
                          const HPANodeList& pi_rList,
                          const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~LUVCubeExpressionNode();
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class GeoreferenceContextExpressionNode : public HPSValueNode
    {
public:
    GeoreferenceContextExpressionNode(HPAGrammarObject*         pi_pObj,
                                      const HPANodeList&        pi_rList,
                                      const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~GeoreferenceContextExpressionNode() { }
    virtual void Calculate();  
    };

//---------------------------------------------------------------------------
class ImageContextExpressionNode : public HPSValueNode
    {
public:
    ImageContextExpressionNode(HPAGrammarObject*         pi_pObj,
                               const HPANodeList&        pi_rList,
                               const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ImageContextExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class AlphaPaletteExpressionNode : public HPSValueNode
    {
public:
    AlphaPaletteExpressionNode(HPAGrammarObject* pi_pObj,
                               const HPANodeList& pi_rList,
                               const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~AlphaPaletteExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class VectorObjectNode : public HPSValueNode
    {
public:
    VectorObjectNode(HPAGrammarObject* pi_pObj,
                     const HPANodeList& pi_rList,
                     const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~VectorObjectNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ShapeExpressionNode : public HPSValueNode
    {
public:
    ShapeExpressionNode(HPAGrammarObject* pi_pObj,
                        const HPANodeList& pi_rList,
                        const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ShapeExpressionNode();
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class RectangleExpressionNode : public HPSValueNode
    {
public:
    RectangleExpressionNode(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession)  {  }
    virtual ~RectangleExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class PolygonExpressionNode : public HPSValueNode
    {
public:
    PolygonExpressionNode(HPAGrammarObject* pi_pObj,
                          const HPANodeList& pi_rList,
                          const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~PolygonExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class HoledShapeExpressionNode : public HPSValueNode
    {
public:
    HoledShapeExpressionNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~HoledShapeExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class UnifiedShapeExpressionNode : public HPSValueNode
    {
public:
    UnifiedShapeExpressionNode(HPAGrammarObject* pi_pObj,
                               const HPANodeList& pi_rList,
                               const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~UnifiedShapeExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class CommonShapeExpressionNode : public HPSValueNode
    {
public:
    CommonShapeExpressionNode(HPAGrammarObject* pi_pObj,
                              const HPANodeList& pi_rList,
                              const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~CommonShapeExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class TransfoObjectNode : public HPSValueNode
    {
public:
    TransfoObjectNode(HPAGrammarObject* pi_pObj,
                      const HPANodeList& pi_rList,
                      const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~TransfoObjectNode();
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class IdentityExpressionNode : public HPSValueNode
    {
public:
    IdentityExpressionNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~IdentityExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class RotationExpressionNode : public HPSValueNode
    {
public:
    RotationExpressionNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~RotationExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ScalingExpressionNode : public HPSValueNode
    {
public:
    ScalingExpressionNode(HPAGrammarObject* pi_pObj,
                          const HPANodeList& pi_rList,
                          const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ScalingExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class TranslationExpressionNode : public HPSValueNode
    {
public:
    TranslationExpressionNode(HPAGrammarObject* pi_pObj,
                              const HPANodeList& pi_rList,
                              const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~TranslationExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class AffineExpressionNode : public HPSValueNode
    {
public:
    AffineExpressionNode(HPAGrammarObject* pi_pObj,
                         const HPANodeList& pi_rList,
                         const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~AffineExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ProjectiveExpressionNode : public HPSValueNode
    {
public:
    ProjectiveExpressionNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ProjectiveExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class LocalProjectiveGridExpressionNode : public HPSValueNode
    {
public:
    LocalProjectiveGridExpressionNode(HPAGrammarObject* pi_pObj,
                                      const HPANodeList& pi_rList,
                                      const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~LocalProjectiveGridExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ComposedExpressionNode : public HPSValueNode
    {
public:
    ComposedExpressionNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ComposedExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class FilterObjectNode : public HPSValueNode
    {
public:
    FilterObjectNode(HPAGrammarObject* pi_pObj,
                     const HPANodeList& pi_rList,
                     const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~FilterObjectNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ContrastExpressionNode : public HPSValueNode
    {
public:
    ContrastExpressionNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ContrastExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class BrightnessExpressionNode : public HPSValueNode
    {
public:
    BrightnessExpressionNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~BrightnessExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ConvolutionRGBExpressionNode : public HPSValueNode
    {
public:
    ConvolutionRGBExpressionNode(HPAGrammarObject* pi_pObj,
                                 const HPANodeList& pi_rList,
                                 const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ConvolutionRGBExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class Convolution3ExpressionNode : public HPSValueNode
    {
public:
    Convolution3ExpressionNode(HPAGrammarObject* pi_pObj,
                               const HPANodeList& pi_rList,
                               const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~Convolution3ExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class AutoContrastStretchExpressionNode : public HPSValueNode
    {
public:
    AutoContrastStretchExpressionNode(HPAGrammarObject* pi_pObj,
                                      const HPANodeList& pi_rList,
                                      const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~AutoContrastStretchExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ContrastStretchExpressionNode : public HPSValueNode
    {
public:
    ContrastStretchExpressionNode(HPAGrammarObject* pi_pObj,
                                  const HPANodeList& pi_rList,
                                  const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ContrastStretchExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class TintExpressionNode : public HPSValueNode
    {
public:
    TintExpressionNode(HPAGrammarObject* pi_pObj,
                       const HPANodeList& pi_rList,
                       const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~TintExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class InverterExpressionNode : public HPSValueNode
    {
public:
    InverterExpressionNode(HPAGrammarObject* pi_pObj,
                           const HPANodeList& pi_rList,
                           const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~InverterExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class GammaCorrectionExpressionNode : public HPSValueNode
    {
public:
    GammaCorrectionExpressionNode(HPAGrammarObject* pi_pObj,
                                  const HPANodeList& pi_rList,
                                  const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~GammaCorrectionExpressionNode() { }
    virtual void Calculate();
    };

//---------------------------------------------------------------------------
class ReturnStatementNode;
class StatementDefinitionNode : public HPANode
    {
public:
    StatementDefinitionNode(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession);
    virtual ~StatementDefinitionNode() { }
    ReturnStatementNode*   m_pReturnNode;
    HFCPtr<HPSParserScope> m_pScope;
    };

//---------------------------------------------------------------------------
class StatementDeclarationNode : public HPANode
    {
public:
    StatementDeclarationNode(HPAGrammarObject* pi_pObj,
                             const HPANodeList& pi_rList,
                             const HFCPtr<HPASession>& pi_pSession);
    virtual ~StatementDeclarationNode() { }
    HFCPtr<HPSParserScope> m_pScope;
    WString                m_Name;
    };

//---------------------------------------------------------------------------
class ReturnStatementNode : public HPSValueNode
    {
public:
    ReturnStatementNode(HPAGrammarObject* pi_pObj,
                        const HPANodeList& pi_rList,
                        const HFCPtr<HPASession>& pi_pSession)
        : HPSValueNode(pi_pObj, pi_rList, pi_pSession) { }
    virtual ~ReturnStatementNode() { }
    virtual void Calculate();
    };

END_IMAGEPP_NAMESPACE