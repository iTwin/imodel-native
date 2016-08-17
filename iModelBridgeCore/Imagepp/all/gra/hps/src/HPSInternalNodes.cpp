//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSInternalNodes.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for internal HPS nodes
//---------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HFCLocalBinStream.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DLiteSegment.h>
#include <Imagepp/all/h/HGF2DLocalProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGFLUVCube.h>
#include <Imagepp/all/h/HGFRGBCube.h>

#include <Imagepp/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HIMOnDemandMosaic.h>
#include <Imagepp/all/h/HIMTranslucentImageCreator.h>
#include <Imagepp/all/h/HIMFilteredImage.h>

#include <Imagepp/all/h/HMDAnnotationIconsPDF.h>
#include <Imagepp/all/h/HMDVolatileLayerInfo.h>
#include <Imagepp/all/h/HMDVolatileLayers.h>

#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAPixelTypeReplacer.h>
#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRASamplingOptions.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileExtender.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPAlphaRange.h>
#include <Imagepp/all/h/HRPCustomConvFilter.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HRSObjectStore.h>

#include <Imagepp/all/h/HPSException.h>

#include "HPSInternalNodes.h"
#include "HPSParserScope.h"
#include "HPSSession.h"

#include <Imagepp/all/h/HPSNode.h>
#include <Imagepp/all/h/HPSParser.h>
#include <Imagepp/all/h/HPSTokenizer.h>
#include <Imagepp/all/h/HPSValueNode.h>

#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DPolySegment.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>

#define SESSION (static_cast<HPSSession*>(GetSession().GetPtr()))
#define PARSER ((HPSParser*)(SESSION->GetParser()))

//---------------------------------------------------------------------------
// Utility functions used by all nodes.

static inline double CalculateNumber(HPANode* pi_pNode, bool pi_MustBeAnInteger = false)
    {
    ((HPSValueNode*)pi_pNode)->Calculate();
    if (((HPSValueNode*)pi_pNode)->GetValueType() != HPSValueNode::NUMBER)
        throw HPSTypeMismatchException(pi_pNode, HPSTypeMismatchException::NUMBER);

    if (pi_MustBeAnInteger == true)
        {
        double DecimalPart = fmod(((HPSValueNode*)pi_pNode)->GetValue().m_Number, 1);

        if (DecimalPart != 0)
            {
            throw HPSTypeMismatchException(pi_pNode, HPSTypeMismatchException::INTEGER);
            }
        }

    return ((HPSValueNode*)pi_pNode)->GetValue().m_Number;
    }

static inline WString CalculateString(HPANode* pi_pNode)
    {
    ((HPSValueNode*)pi_pNode)->Calculate();
    if (((HPSValueNode*)pi_pNode)->GetValueType() != HPSValueNode::STRING)
        throw HPSTypeMismatchException(pi_pNode, HPSTypeMismatchException::STRING);
    return *(((HPSValueNode*)pi_pNode)->GetValue().m_pString);
    }

static inline HPSObjectValue* CalculateObject(HPANode* pi_pNode)
    {
    ((HPSValueNode*)pi_pNode)->Calculate();
    if (((HPSValueNode*)pi_pNode)->GetValueType() != HPSValueNode::OBJECT)
        throw HPSTypeMismatchException(pi_pNode, HPSTypeMismatchException::OBJECT);
    return ((HPSValueNode*)pi_pNode)->GetValue().m_pObjValue;
    }

//---------------------------------------------------------------------------
// This private object is used to decorate an HMDContext in order to be able
// to connect modifying node to the HMDContext so that the PSS script lines
// for those modifying node are include in the respective descriptive PSS.
// IMPORTANT : This class is really only important when using an on-demand mosaic
// though it is always use because the overhead was not deemed important.
class HPSDecoratedHMDContext : public HPSObjectValue
    {
public :

    HPSDecoratedHMDContext() :m_pContext(new HMDContext()) {}

    virtual ~HPSDecoratedHMDContext() {}

    void AddModifyingContextNode(const HFCPtr<HPANode>& pi_rpModifyingNode)
        {
        m_ModifyingContextNodes.push_back(pi_rpModifyingNode);
        }

    const HPANodeList& GetModifyingContextNodes()
        {
        return m_ModifyingContextNodes;
        }
    HMDContext& GetContext() {return *m_pContext;}

private :
    HFCPtr<HMDContext> m_pContext;
    HPANodeList m_ModifyingContextNodes;
    };

//---------------------------------------------------------------------------
class HPSAlphaPaletteObjectValue : public HPSObjectValue
    {
    public:
        HPSAlphaPaletteObjectValue(Byte pi_AlphaLevel) :m_DefaultLevel(pi_AlphaLevel)
            {
            memset(m_AlphaLevels, 255, 256);           
            }
        
        virtual ~HPSAlphaPaletteObjectValue(){};

        void    AddEntry(Byte pi_Index)
            {
            m_AlphaLevels[pi_Index] = m_DefaultLevel;
            }

        void    AddEntries(HPSAlphaPaletteObjectValue const& pi_Obj)
            {
            for (size_t i = 0; i < 256; i++)
                if (m_AlphaLevels[i] == 255)
                    m_AlphaLevels[i] = pi_Obj.m_AlphaLevels[i];
            }

        void    AddEntries(Byte pi_RangeFirst, Byte pi_RangeLast)
            {
            memset(m_AlphaLevels + pi_RangeFirst, m_DefaultLevel, pi_RangeLast - pi_RangeFirst + 1);
            }

        Byte const* GetEntries() const {return m_AlphaLevels;}

    private:
        HPSAlphaPaletteObjectValue(HPSAlphaPaletteObjectValue& pi_rObj);

        Byte m_AlphaLevels[256];
        Byte m_DefaultLevel;
    };

//---------------------------------------------------------------------------
PageStatementNode::PageStatementNode(HPAGrammarObject* pi_pObj,
                                     const HPANodeList& pi_rList,
                                     const HFCPtr<HPASession>& pi_pSession)
    : HPANode (pi_pObj, pi_rList, pi_pSession)
    {
    // inserting node in the page node list of the parser

    SESSION->AddPageNode(this);
    }

HPSRasterObjectValue* PageStatementNode::ComputeObject() const
    {
    HPSRasterObjectValue* pRasterValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));
    
    if (pRasterValue == NULL || pRasterValue->m_pObject == NULL)
        throw HPSTypeMismatchException(const_cast<PageStatementNode*>(this),
                                       HPSTypeMismatchException::IMAGE);
    if (!pRasterValue->m_pObject->GetExtent().IsDefined())
        throw HPSImageHasNoSizeException(const_cast<PageStatementNode*>(this));

    // is there a using?  (this offset the description pos if any)

    if (GetSubNodes().size() == 6)  // is there a description?
        {
#ifdef IPP_ATTRIBUTES_ON_HRA    // Do we have a client for this?
        HPMAttributeSet& Attribs = pRasterValue->m_pObject->LockAttributes();
        Attribs.Set(new HPSAttributeImageDescription(CalculateString(GetSubNodes()[4])));
        pRasterValue->m_pObject->UnlockAttributes();
#endif
        }

    return pRasterValue;
    }

//---------------------------------------------------------------------------
WorldStatementNode::WorldStatementNode(HPAGrammarObject* pi_pObj,
                                       const HPANodeList& pi_rList,
                                       const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    int32_t WorldID = (int32_t)CalculateNumber(GetSubNodes()[2], true);
    HPSTransfoObjectValue* pTransoValue = dynamic_cast<HPSTransfoObjectValue*>(CalculateObject(GetSubNodes()[4]));
    int32_t RefWorldID = (int32_t)CalculateNumber(GetSubNodes()[6], true);

    if (pTransoValue == NULL || pTransoValue->m_pObject == NULL)
        throw HPSTypeMismatchException(GetSubNodes()[4],
                                       HPSTypeMismatchException::TRANSFO);
    if ((WorldID < 1) || (WorldID > 255))
        throw HPSOutOfRangeException(GetSubNodes()[2], 1, 255);
    if ((RefWorldID < 0) || (RefWorldID > 255))
        throw HPSOutOfRangeException(GetSubNodes()[6], 0, 255);
      
    if (SESSION->GetWorld(WorldID) != 0)
        if (SESSION->GetWorld(WorldID)->IsUsedAsReference())
            throw HPSWorldAlreadyUsedException(GetSubNodes()[2]);
    if (!SESSION->ModifyCluster(WorldID, *pTransoValue->m_pObject, RefWorldID))
        throw HPSInvalidWorldException(GetSubNodes()[6]);

    PARSER->GetWorldRelatedNodes().push_back(this);
    }

//---------------------------------------------------------------------------
SelectWorldStatementNode::SelectWorldStatementNode(HPAGrammarObject* pi_pObj,
                                                   const HPANodeList& pi_rList,
                                                   const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    SESSION->ChangeCurrentWorld((HGF2DWorldIdentificator)CalculateNumber(pi_rList[2], true));
    if (SESSION->GetCurrentWorld() == 0)
        throw HPSInvalidWorldException(pi_rList[2]);

    PARSER->GetWorldRelatedNodes().push_back(this);
    }

//---------------------------------------------------------------------------
SetLayerStatementNode::SetLayerStatementNode(HPAGrammarObject*         pi_pObj,
                                             const HPANodeList&        pi_rList,
                                             const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPRECONDITION(pi_rList.size() > 4);

    bool DisplayMode;

    if (pi_pObj->GetName() == L"SetLayerOnStatement")
        {
        DisplayMode = true;
        }
    else
        {
        DisplayMode = false;
        }

    HPSDecoratedHMDContext* pContext = dynamic_cast<HPSDecoratedHMDContext*>(CalculateObject(GetSubNodes()[2]));

    if (pContext == 0)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE_CONTEXT);

    //First four nodes are always "SETLAYERON ( imContext," or "SETLO ( imContext,"
    HFCPtr<HMDLayers> pLayers(new HMDLayers());
    WString           LayerId;

    HPANode* pNode = GetSubNodes()[4];
    while (pNode)
        {
        LayerId = CalculateString(pNode->GetSubNodes().front());

        pLayers->AddLayer(new HMDLayerInfo(LayerId, DisplayMode));

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    HFCPtr<HMDLayers> pPrevFoundLayers = static_cast<HMDLayers*>(pContext->GetContext().GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO).GetPtr());

    if (pPrevFoundLayers != 0)
        {
        pPrevFoundLayers->Merge(pLayers);
        }
    else
        {
        //An HMDVolatileLayers is usually set in a HMDContext object but since the visibility of a layer in
        //already added in the list of visible layer of an ImageContext object cannot be changed in
        //a PSS script a HMDLayers object is used instead to save memory space and simplify the memory
        //management
        pContext->GetContext().AddMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO, ((HFCPtr<HMDMetaDataContainer>&)pLayers));
        }

    pContext->AddModifyingContextNode(HFCPtr<HPANode>(this));
    }

//---------------------------------------------------------------------------
SetAnnotationIconStatementNode::SetAnnotationIconStatementNode(HPAGrammarObject*         pi_pObj,
                                                               const HPANodeList&        pi_rList,
                                                               const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    HPSDecoratedHMDContext* pContext = dynamic_cast<HPSDecoratedHMDContext*>(CalculateObject(GetSubNodes()[2]));
    
    if (pContext == 0)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE_CONTEXT);

    //SetLayerOnStatementNode can only be called once on a ImageContext object
    HASSERT(pContext->GetContext().GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO) == 0);
    HFCPtr<HMDAnnotationIconsPDF> pAnnotationIconsPDF(new HMDAnnotationIconsPDF());

    double BoolVal = CalculateNumber(GetSubNodes()[4], true);

    if (BoolVal == 0)
        {
        pAnnotationIconsPDF->SetRasterization(false);
        }
    else
        {
        HASSERT(BoolVal == 1);
        pAnnotationIconsPDF->SetRasterization(true);
        }

    pContext->GetContext().AddMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO,
                                                  (HFCPtr<HMDMetaDataContainer>&)pAnnotationIconsPDF);

    pContext->AddModifyingContextNode(HFCPtr<HPANode>(this));
    }

//---------------------------------------------------------------------------
DeclarationStatementNode::DeclarationStatementNode(HPAGrammarObject* pi_pObj,
                                                   const HPANodeList& pi_rList,
                                                   const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    SESSION->GetCurrentScope()->AddVariable(((const HFCPtr<HPATokenNode>&)pi_rList.front())->GetText(),
                                            (const HFCPtr<ExpressionNode>&)pi_rList[2]);
    }

//---------------------------------------------------------------------------
void ExpressionNode::Calculate()
    {
    if (GetSubNodes().size() == 3)
        {
        if (GetSubNodes()[2]->GetGrammarObject() == &PARSER->SHAPE_tk)
            {
            HPSRasterObjectValue* pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes().front()));

            if (pRasterObjectValue == NULL || pRasterObjectValue->m_pObject == NULL)
                throw HPSTypeMismatchException(GetSubNodes().front(),
                                               HPSTypeMismatchException::IMAGE);

            HFCPtr<HVEShape> pShape(new HVEShape(*(pRasterObjectValue->m_pObject->GetEffectiveShape())));

            HPSShapeObjectValue* pValue = new HPSShapeObjectValue(pShape);           
            pValue->AddHFCPtr();
            SetValue(pValue);
            }
        }
    else if (GetSubNodes().front()->GetGrammarObject() == &PARSER->VariableIdentifier_tk)
        {
        ((const HFCPtr<VariableRefNode>&)GetSubNodes().front())->Calculate();
        SetType(((const HFCPtr<VariableRefNode>&)GetSubNodes().front())->m_Type);

        SetValue(((const HFCPtr<VariableRefNode>&)GetSubNodes().front())->m_Value);

        if (((const HFCPtr<VariableRefNode>&)GetSubNodes().front())->GetValueOwnership() == true)
            {
            ((const HFCPtr<VariableRefNode>&)GetSubNodes().front())->SetValueOwnership(false);
            }
        else
            {
            SetValueOwnership(false);
            }
        }
    else
        {
        ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
        SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
        }
    }

//---------------------------------------------------------------------------
VariableTokenNode::VariableTokenNode(HPAGrammarObject* pi_pObj,
                                     ExpressionNode* pi_pNode,
                                     const HPASourcePos& pi_rLeftPos,
                                     const HPASourcePos& pi_rRightPos,
                                     const HFCPtr<HPSSession>& pi_pSession)
    : HPANode(pi_pObj, pi_rLeftPos, pi_rRightPos, (const HFCPtr<HPASession>&)pi_pSession),
      m_pExpressionNode(pi_pNode),
      m_IsCalculated(false)
    {
    }

VariableTokenNode::~VariableTokenNode()
    {
    if (m_IsCalculated && (m_Type == HPSValueNode::STRING))
        delete m_Value.m_pString;
    }

void VariableTokenNode::ResetExpression(ExpressionNode* pi_pNode)
    {
    m_pExpressionNode = pi_pNode;
    m_IsCalculated = false;
    }

void VariableTokenNode::Reset()
    {
    m_IsCalculated = false;
    }

void VariableTokenNode::Calculate()
    {
    HPRECONDITION(m_pExpressionNode != 0);
    if (!m_IsCalculated)
        {
        m_pExpressionNode->Calculate();
        m_IsCalculated = true;
        m_Type = m_pExpressionNode->GetValueType();
        if (m_Type == HPSValueNode::STRING)
            m_Value.m_pString = new WString(*(m_pExpressionNode->GetValue().m_pString));
        else
            {
            m_Value = m_pExpressionNode->GetValue();
            }
        }
    }

void VariableTokenNode::FreeValue()
    {
    HPRECONDITION(m_pExpressionNode != 0);

    if (m_IsCalculated == true)
        {
        m_IsCalculated = false;
        m_pExpressionNode->FreeValue();
        }
    }

bool VariableTokenNode::GetValueOwnership() const
    {
    return m_pExpressionNode->GetValueOwnership();
    }

void VariableTokenNode::SetValueOwnership(bool pi_IsOwner)
    {
    m_pExpressionNode->SetValueOwnership(pi_IsOwner);
    }

//---------------------------------------------------------------------------
VariableRefNode::VariableRefNode(HPAGrammarObject* pi_pObj,
                                 VariableTokenNode* pi_pNode,
                                 const HPASourcePos& pi_rLeftPos,
                                 const HPASourcePos& pi_rRightPos,
                                 const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rLeftPos, pi_rRightPos, pi_pSession),
      m_pTokenNode(pi_pNode)
    {
    }

VariableRefNode::~VariableRefNode()
    {
    /*
    if (m_pTokenNode != 0)
    {
        delete m_pTokenNode;
    }
    //VariableTokenNode

    m_pTokenNode->
    */
    }

void VariableRefNode::Calculate()
    {
    HPRECONDITION(m_pTokenNode != 0);
    m_pTokenNode->Calculate();
    m_Type           = m_pTokenNode->m_Type;
    m_Value          = m_pTokenNode->m_Value;
    }

void VariableRefNode::FreeValue()
    {
    HPRECONDITION(m_pTokenNode != 0);
    m_pTokenNode->FreeValue();
    }

void VariableRefNode::SetValueOwnership(bool pi_IsOwner)
    {
    m_pTokenNode->SetValueOwnership(pi_IsOwner);
    }

bool VariableRefNode::GetValueOwnership() const
    {
    return m_pTokenNode->GetValueOwnership();
    }

//---------------------------------------------------------------------------
StatementTokenNode::StatementTokenNode(HPAGrammarObject* pi_pObj,
                                       StatementDefinitionNode* pi_pNode,
                                       const HPASourcePos& pi_rLeftPos,
                                       const HPASourcePos& pi_rRightPos,
                                       const HFCPtr<HPSSession>& pi_pSession)
    : HPANode(pi_pObj, pi_rLeftPos, pi_rRightPos, (const HFCPtr<HPASession>&)pi_pSession),
      m_pStatementNode(pi_pNode)
    {
    }

//---------------------------------------------------------------------------
StringExpressionNode::StringExpressionNode(HPAGrammarObject* pi_pObj,
                                           const HPANodeList& pi_rList,
                                           const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    if (GetSubNodes().front()->GetGrammarObject() == &PARSER->String_tk)
        {
        SetValue(((const HFCPtr<HPATokenNode>&)(GetSubNodes().front()))->GetText());
        }
    }

void StringExpressionNode::Calculate()
    {
    // Value is already calculated if not a variable...
    }

//---------------------------------------------------------------------------
NumericExpressionNode::NumericExpressionNode(HPAGrammarObject* pi_pObj,
                                             const HPANodeList& pi_rList,
                                             const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    double Value = 0;
    if (pi_rList.front()->GetGrammarObject() == &PARSER->Number_tk)
        Value = ((const HFCPtr<HPANumberTokenNode>&)pi_rList[0])->GetValue();
    else if (pi_rList.size() == 2)
        {
        Value = ((const HFCPtr<HPANumberTokenNode>&)pi_rList[1])->GetValue();
        if (pi_rList[0]->GetGrammarObject() == &PARSER->MINUS_tk)
            Value = -Value;
        else if (pi_rList[0]->GetGrammarObject() != &PARSER->PLUS_tk)
            throw HPSInvalidNumericException(pi_rList[0]);
        }
    SetValue(Value);
    }

void NumericExpressionNode::Calculate()
    {
    // Value is already calculated if not a variable...
    }

//---------------------------------------------------------------------------
ObjectExpressionNode::ObjectExpressionNode(HPAGrammarObject* pi_pObj,
                                           const HPANodeList& pi_rList,
                                           const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    }

void ObjectExpressionNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
    }

//---------------------------------------------------------------------------
void PictureObjectNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
    }

//---------------------------------------------------------------------------
StatementInstanciationNode::StatementInstanciationNode(HPAGrammarObject* pi_pObj,
                                                       const HPANodeList& pi_rList,
                                                       const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    }

void StatementInstanciationNode::Calculate()
    {
    StatementDefinitionNode* pStatementNode = ((const HFCPtr<StatementTokenNode>&)(GetSubNodes().front()))->m_pStatementNode;
    SESSION->ChangeScope(pStatementNode->m_pScope);
    pStatementNode->m_pScope->Reset();

    // Setting parameters

    if (GetSubNodes().size() == 4)
        {
        unsigned short Pos = 0;
        HPANode* pNode = GetSubNodes()[2];
        while (pNode)
            {
            if (Pos >= pStatementNode->m_pScope->GetParameterCount())
                throw HPSTooManyParamException(pNode);
            pStatementNode->m_pScope->SetParameterValue(Pos, (const HFCPtr<ExpressionNode>&)(pNode->GetSubNodes().front()));
            if (pNode->GetSubNodes().size() == 3)
                pNode = pNode->GetSubNodes()[2];
            else
                pNode = 0;
            ++Pos;
            }
        if (Pos < pStatementNode->m_pScope->GetParameterCount())
            throw HPSTooFewParamException(this);
        }

    pStatementNode->m_pReturnNode->Calculate();
    SetValueFrom(pStatementNode->m_pReturnNode);
    SESSION->ChangeScope(pStatementNode->m_pScope->GetOwner());
    }

//---------------------------------------------------------------------------
void ImageObjectNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
    }

//---------------------------------------------------------------------------
ImageFileExpressionNode::ImageFileExpressionNode(HPAGrammarObject* pi_pObj,
                                                 const HPANodeList& pi_rList,
                                                 const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    }

void ImageFileExpressionNode::Calculate()
    {
    uint32_t    PageNumber = 0;
    HMDContext*             pContext   = 0;
    HRFGeoreferenceContext* pGeoreferenceContext = 0;
            
    if (GetSubNodes().size() >= 6)
        {
        PageNumber = (uint32_t)CalculateNumber(GetSubNodes()[4], true);

        if (GetSubNodes().size() >= 8)
            {
            HPSDecoratedHMDContext* pContextValue = dynamic_cast<HPSDecoratedHMDContext*>(CalculateObject(GetSubNodes()[6]));
            
            if (pContextValue == 0)
                throw HPSTypeMismatchException(GetSubNodes()[6],
                                               HPSTypeMismatchException::IMAGE_CONTEXT);

            pContext = &pContextValue->GetContext();

            if (GetSubNodes().size() >= 10)
                {                                
                HPSGeoreferenceContextObjectValue* pPersistentObject = dynamic_cast<HPSGeoreferenceContextObjectValue*>(CalculateObject(GetSubNodes()[8]));

                if (pPersistentObject == 0)
                    throw HPSInvalidObjectException(GetSubNodes()[8]);

                pGeoreferenceContext = pPersistentObject->m_pObject;
                }
            }
        }

    HFCPtr<HFCURL> pURL;

    GetFileURL(pURL);

    if (pURL == 0)
        throw HPSInvalidUrlException(GetSubNodes()[2]);
    HFCPtr<HRFRasterFile> pFile = HRFRasterFileFactory::GetInstance()->OpenFile(pURL, true);
    if (pFile == 0)
        throw HPSFileNotFoundException(this);

    HFCPtr<HRFRasterFile> pImprFile;
    
	if (pGeoreferenceContext != 0)
    	{
	    bool isGeoreferenceUpdated = pGeoreferenceContext->UpdateGeoreference(PageNumber, pFile);

    	HASSERT(isGeoreferenceUpdated == true);
        
        pImprFile = GenericImprove(pFile, 
                                   HRFiTiffCacheFileCreator::GetInstance(), 
                                   pGeoreferenceContext->UseSisterFile(), 
                                   pGeoreferenceContext->UseSisterFile(), 
                                   pGeoreferenceContext->GetDefaultRatioToMeterForSisterFile());
	    }		
    else
        {
        pImprFile = GenericImprove(pFile, HRFiTiffCacheFileCreator::GetInstance());
        }    

    HASSERT(pImprFile != 0);

    if (PageNumber >= pImprFile->CountPages())
        throw HPSPageNotFoundException(this);

    HFCPtr<HGF2DCoordSys> pImageWorld = SESSION->GetWorld(pImprFile->GetWorldIdentificator());
    if (pImageWorld == 0)
        throw HPSInvalidWorldException(this);

    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(SESSION->GetPool(), pImprFile, PageNumber, pImageWorld);
    pStore->SetThrowable(true);
    HFCPtr<HRARaster> pRaster(pStore->LoadRaster());
    if (pRaster == NULL)
        throw HPSNoImageInFileException(this);

    HFCPtr<HMDContext> pRasterContext(pRaster->GetContext());

    if ((pRasterContext != 0) && (pContext != 0))
        {
        HFCPtr<HMDMetaDataContainer> pMDContainerFromPSS;
        HFCPtr<HMDMetaDataContainer> pMDContainerFromRaster;
        bool                        InvalidateRaster(false);

        pMDContainerFromPSS    = pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO);
        pMDContainerFromRaster = pRasterContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_ANNOT_ICON_RASTERING_INFO);

        if ((pMDContainerFromPSS != 0) && (pMDContainerFromRaster != 0))
            {
            ((HFCPtr<HMDAnnotationIconsPDF>&)pMDContainerFromRaster)->
            SetRasterization(((HFCPtr<HMDAnnotationIconsPDF>&)pMDContainerFromPSS)->
                             GetRasterization());
            InvalidateRaster = true;
            }

        pMDContainerFromPSS    = pContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO);
        pMDContainerFromRaster = pRasterContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO);

        if ((pMDContainerFromPSS != 0) && (pMDContainerFromRaster != 0))
            {
            HFCPtr<HMDLayers>           pLayersFromPSS((HFCPtr<HMDLayers>&)pMDContainerFromPSS);
            HFCPtr<HMDVolatileLayers>   pVolatileLayersFromRaster((HFCPtr<HMDVolatileLayers>&)pMDContainerFromRaster);
            HMDVolatileLayerInfo*       pVolatileLayerInfo;
            unsigned short             LayerIndex;

            for (unsigned short LayerInd = 0; LayerInd < pVolatileLayersFromRaster->GetNbVolatileLayers(); LayerInd++)
                {
                pVolatileLayerInfo = pVolatileLayersFromRaster->GetVolatileLayerInfo(LayerInd);

                //If a the visiblitiy of a layer is specied in the PSS
                if (pLayersFromPSS->GetIndexFromKey(pVolatileLayerInfo->
                                                    GetLayerInfo()->
                                                    GetKeyName(), LayerIndex) == true)
                    {
                    pVolatileLayerInfo->SetVisibleState(pLayersFromPSS->GetLayer(LayerIndex)
                                                        ->GetInitialVisibleState());
                    }
                }

            InvalidateRaster = true;
            }

        if (InvalidateRaster == true)
            {
            pRaster->InvalidateRaster();
            }
        }
    HPSRasterObjectValue* pRasterObjectValue = new HPSRasterObjectValue(pRaster);

    pRasterObjectValue->AddHFCPtr();
    SetValue(pRasterObjectValue);
    }

void ImageFileExpressionNode::GetFileURL(HFCPtr<HFCURL>& po_rpFileURL) const
    {
#if 0
    // The file names in a PSS should be encoded in UTF8.
    AString FileNameA(CalculateString(GetSubNodes()[2]).c_str());
    WString FileName(FileNameA.c_str(), true/*isUtf8*/);
#else
    WString FileName(CalculateString(GetSubNodes()[2]).c_str());
#endif

    po_rpFileURL = HFCURL::Instanciate(FileName);  // Is it a fully qualified URL?
    if (po_rpFileURL == 0)  // no, maybe it a fully qualified pathname with drive letter
        {
        if ((FileName.size() > 2) &&
            ((FileName[1] == L':') || (FileName.substr(0, 2) == L"\\\\") || (FileName.substr(0,2) == L"//")))
            po_rpFileURL = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + FileName);
        else
            po_rpFileURL = SESSION->GetURL()->MakeURLTo(FileName);  // otherwise it is a relative path
        }
    }

//---------------------------------------------------------------------------
MosaicExpressionNode::MosaicExpressionNode(HPAGrammarObject* pi_pObj,
                                           const HPANodeList& pi_rList,
                                           const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    }

void MosaicExpressionNode::Calculate()
    {
    HFCPtr<HIMMosaic>     pMosaic(new HIMMosaic(SESSION->GetCurrentWorld()));
    HIMMosaic::RasterList Rasters;

    HPANode* pNode = GetSubNodes()[2];
    while (pNode)
        {
        HPSRasterObjectValue* pRasterValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));

        if (pRasterValue == 0 || pRasterValue->m_pObject == 0)
            throw HPSTypeMismatchException(pNode->GetSubNodes().front(),
                                           HPSTypeMismatchException::IMAGE);

        Rasters.push_back(pRasterValue->m_pObject);

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    pMosaic->Add(Rasters);

    HPSRasterObjectValue* pRasterObjectValue = new HPSRasterObjectValue(pMosaic.GetPtr()); 
    pRasterObjectValue->AddHFCPtr();
    SetValue(pRasterObjectValue);
    }

//---------------------------------------------------------------------------
OnDemandMosaicExpressionNode::OnDemandMosaicExpressionNode(HPAGrammarObject*         pi_pObj,
                                                           const HPANodeList&        pi_rList,
                                                           const HFCPtr<HPASession>& pi_pSession)
    : HPSValueNode(pi_pObj, pi_rList, pi_pSession)
    {
    m_PixelSizeRangeInit = false;
    }

OnDemandMosaicExpressionNode::~OnDemandMosaicExpressionNode()
    {
    LineStreamList::iterator StreamIter    = m_pLineStreams.begin();
    LineStreamList::iterator StreamIterEnd = m_pLineStreams.end();

    while (StreamIter != StreamIterEnd)
        {
        delete *StreamIter;
        StreamIter++;
        }
    }

void OnDemandMosaicExpressionNode::Calculate()
    {
    HIMOnDemandMosaic::RasterList Rasters;
    HFCPtr<HRAOnDemandRaster>     pOnDemandRaster;
    HFCPtr<HIMOnDemandMosaic>     pMosaic;
    WString                       PSSDescriptiveNode;
    HFCPtr<HPANode>               pNode(GetSubNodes()[2]);

    while (pNode)
        {
        HPSRasterObjectValue* pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));

        if (pRasterObjectValue == 0 || pRasterObjectValue->m_pObject == 0)
            throw HPSTypeMismatchException(pNode->GetSubNodes().front(),
                                           HPSTypeMismatchException::IMAGE);

        PSSDescriptiveNode.clear();

        GetRasterDescriptivePSS(pNode->GetSubNodes().front(), true, PSSDescriptiveNode);

        HFCPtr<HRARaster> pStoredRaster(pRasterObjectValue->m_pObject);

        UpdatePixelSizeRange(pStoredRaster, SESSION->GetCurrentWorld());

        ListOfRelatedURLs relatedURLs;

        //MST : Eventually we should have some service in Image++ 
        //      to allow the querying of the HRARaster for the presence of any unlimited raster.        
        HPSObjectStore::GetURLsFromChildrenNode(pNode, relatedURLs);         

        bool isDataChangingWithResolution = false;
        bool hasUnlimitedRasterSource = false;

        ListOfRelatedURLs::iterator urlIter(relatedURLs.begin());
        ListOfRelatedURLs::iterator urlIterEnd(relatedURLs.end());

        // Use the factory to avoid direct reference to file format and thus force linking to some external libraries.
        HRFRasterFileFactory* pRasterFileFactory = HRFRasterFileFactory::GetInstance();

        while ((urlIter != urlIterEnd) && 
               ((isDataChangingWithResolution == false) || 
                (hasUnlimitedRasterSource == false)))
            {                            
            if (pRasterFileFactory->IsKindOfFile(HRFFileId_WMS/*HRFWMSFile::CLASS_ID*/, *urlIter))
                {
                hasUnlimitedRasterSource = true;
                isDataChangingWithResolution = true;
                }
#if defined (__IPP_EXTERNAL_THIRD_PARTY_SUPPORTED)
            else if (pRasterFileFactory->IsKindOfFile(HRFFileId_PDF/*HRFPDFFile::CLASS_ID*/, *urlIter))
                {
                hasUnlimitedRasterSource = true;                
                }
#endif
            else if (pRasterFileFactory->IsKindOfFile(HRFFileId_VirtualEarth/*HRFVirtualEarthFile::CLASS_ID*/, *urlIter) ||
                     pRasterFileFactory->IsKindOfFile(HRFFileId_MapBox/*HRFVirtualEarthFile::CLASS_ID*/, *urlIter))
                {
                isDataChangingWithResolution = true;
                }
                               
            urlIter++;
            }
        
        pOnDemandRaster = new HRAOnDemandRaster(SESSION->GetPool(),
                                                pStoredRaster->IsOpaque(),
                                                pStoredRaster->GetEffectiveShape(),
                                                PSSDescriptiveNode,
                                                SESSION->GetWorldCluster(),
                                                SESSION->GetCurrentWorldID(),
                                                SESSION->GetURL(), 
                                                pStoredRaster->HasLookAhead(), 
                                                isDataChangingWithResolution, 
                                                hasUnlimitedRasterSource);

        Rasters.push_back(pOnDemandRaster);

        HPANodeList& pList = (HPANodeList&)(pNode->GetSubNodes());

        //Free the node to ensure that the raster that has just been loaded
        //(during the call to the method CalculateObject) isn't kept in
        //memory.
        (*pList.begin())->FreeValue();

        if (pNode->GetSubNodes().size() == 3)
            {
            HFCPtr<HPANode> pTempNode(pNode->GetSubNodes()[2]);
            pNode = pTempNode;
            }
        else
            pNode = 0;
        }

    pMosaic = new HIMOnDemandMosaic(SESSION->GetCurrentWorld(), 0, 0, HRFDownSamplingMethod::NONE);                   

    pMosaic->SetPrecomputedPixelSizeRange(m_PixelSizeRangeMinExt,
                                          m_PixelSizeRangeMaxExt);

    //Set the pixel type to a fixed value to avoid opening all the rasters to determine the pixel
    //type of the mosaic.
    pMosaic->SetPixelTypeInfo(true, new HRPPixelTypeV32R8G8B8A8());

    //Get the PSS describing the world statements if at least one world statement is
    //used in the PSS file.
    if (PARSER->GetWorldRelatedNodes().size() > 0)
        {
        HFCPtr<HPANode> pTempNode(new HPANode(0,
                                              PARSER->GetWorldRelatedNodes(),
                                              (const HFCPtr<HPASession>&)SESSION,
                                              false));

        PSSDescriptiveNode.clear();

        GetRasterDescriptivePSS(pTempNode, false, PSSDescriptiveNode);

        pMosaic->SetRepresentativePSSForWorlds(PSSDescriptiveNode);
        }

    pMosaic->Add(Rasters);

    HPSRasterObjectValue* pRasterObjectValue = new HPSRasterObjectValue(pMosaic.GetPtr()); 
    pRasterObjectValue->AddHFCPtr();

    SetValue(pRasterObjectValue);
    }

void OnDemandMosaicExpressionNode::UpdatePixelSizeRange(HFCPtr<HRARaster>&           pi_rpRaster,
                                                        const HFCPtr<HGF2DCoordSys>& pi_rpMosaicCoordSys)
    {
    //Initialize return pizel sizes to undefined
    if (m_PixelSizeRangeInit == false)
        {
        m_PixelSizeRangeMinExt = HGF2DExtent(pi_rpMosaicCoordSys);
        m_PixelSizeRangeMaxExt = m_PixelSizeRangeMinExt;
        }

    //Get pixel size from current source
    HIMOnDemandMosaic::GetPixelSizeRange(pi_rpRaster,
                                         pi_rpMosaicCoordSys,
                                         m_PixelSizeRangeInit,
                                         m_PixelSizeRangeMinArea,
                                         m_PixelSizeRangeMaxArea,
                                         m_PixelSizeRangeMinExt,
                                         m_PixelSizeRangeMaxExt);
    }

HFCMemoryLineStream& OnDemandMosaicExpressionNode::GetLineStream(WString& pi_rPSSFileName)
    {
    LineStreamList::const_iterator LineStreamIter    = m_pLineStreams.begin();
    LineStreamList::const_iterator LineStreamIterEnd = m_pLineStreams.end();

    while (LineStreamIter != LineStreamIterEnd)
        {
        HASSERT((*LineStreamIter)->GetURL()->IsCompatibleWith(HFCURLMemFile::CLASS_ID));

        HFCPtr<HFCURLMemFile> pURLMemFile(static_cast<HFCURLMemFile*>((*LineStreamIter)->GetURL().GetPtr()));

        if (pURLMemFile->GetFilename() == pi_rPSSFileName)
            break;
            
        LineStreamIter++;
        }

    if (LineStreamIter == LineStreamIterEnd)
        {
        HAutoPtr<HFCLocalBinStream> pFileStream(new HFCLocalBinStream(pi_rPSSFileName,
                                                                      HFC_READ_ONLY | HFC_SHARE_READ_ONLY));
        HArrayAutoPtr<Byte>        pDataBuffer(new Byte[(size_t)pFileStream->GetSize()]);
        size_t                      NbBytesRead = pFileStream->Read(pDataBuffer.get(),
                                                                    (size_t)pFileStream->GetSize());

        HASSERT(NbBytesRead == pFileStream->GetSize());

        HFCPtr<HFCBuffer>             pStreamBuffer(new HFCBuffer(pDataBuffer.release(),
                                                                  (size_t)pFileStream->GetSize()));

        HAutoPtr<HFCMemoryLineStream> pLineStream(new HFCMemoryLineStream(pi_rPSSFileName,
                                                                          '\n',
                                                                          pStreamBuffer));

        m_pLineStreams.push_back(pLineStream.release());

        LineStreamIter = --m_pLineStreams.end();
        }

    return *(*LineStreamIter);
    }

void OnDemandMosaicExpressionNode::GetRasterDescriptivePSS(const HFCPtr<HPANode>& pi_rpRasterNode,
                                                           bool                  pi_EnclosedLastLineByPG,
                                                           WString&               po_rPSS)
    {
    HPRECONDITION(pi_rpRasterNode->GetStartPos().m_pURL->GetURL() ==
                  pi_rpRasterNode->GetEndPos().m_pURL->GetURL());

    list<DescriptivePSSFileInfo> DescriptivePSS;
    HAutoPtr<HFCLocalBinStream>  pFileStream;
    WString                      FileName;
    bool                        IsFileInfoOfStartingNode = false;

    //Get the lines of each file that contains statement describing the raster
    GetDescriptivePSSInfoFromNodes(pi_rpRasterNode, DescriptivePSS);

    DescriptivePSSFileInfoList::iterator FileInfoIter    = DescriptivePSS.begin();
    DescriptivePSSFileInfoList::iterator FileInfoIterEnd = DescriptivePSS.end();

    while (FileInfoIter != FileInfoIterEnd)
        {
        FileName = FileInfoIter->m_pPSSFileName->GetURL().substr(7);

        HFCMemoryLineStream& rMemoryLineStream = GetLineStream(FileName);

        FileInfoIter->m_Lines.sort();

        WString                       LineRead;
        int32_t                     StartingColPos;
        int32_t                     EndingColPos;
        list<uint32_t>::const_iterator LineIter    = FileInfoIter->m_Lines.begin();
        list<uint32_t>::const_iterator LineIterEnd = FileInfoIter->m_Lines.end();

        if (FileInfoIter->m_pPSSFileName->GetURL() == pi_rpRasterNode->GetStartPos().m_pURL->GetURL())
            {
            IsFileInfoOfStartingNode = true;
            }

        while (LineIter != LineIterEnd)
            {
            StartingColPos = -1;
            EndingColPos   = -1;

            if ((IsFileInfoOfStartingNode == true) &&
                (pi_EnclosedLastLineByPG == true))
                {
                if (pi_rpRasterNode->GetStartPos().m_Line == *LineIter)
                    {
                    po_rPSS += L"PAGE(";
                    StartingColPos = pi_rpRasterNode->GetStartPos().m_Column - 1;
                    }

                if (pi_rpRasterNode->GetEndPos().m_Line == *LineIter)
                    {
                    EndingColPos = pi_rpRasterNode->GetEndPos().m_Column;
                    }
                }

            AddLineToDescriptivePSS(rMemoryLineStream, *LineIter, po_rPSS, StartingColPos, EndingColPos);
            LineIter++;
            }

        FileInfoIter++;
        }

    if (pi_EnclosedLastLineByPG == true)
        {
        po_rPSS += L")";
        }
    }

void OnDemandMosaicExpressionNode::AddLineToDescriptivePSS(HFCMemoryLineStream& pi_rLineStream,
                                                           uint32_t            pi_LineNb,
                                                           WString&             po_rPSS,
                                                           int32_t            pi_StartingColPos,
                                                           int32_t            pi_EndingColPos)
    {
    WString LineToWrite;
    size_t  NbBytesRead = pi_rLineStream.ReadLine(pi_LineNb - 1, LineToWrite);

    HASSERT(NbBytesRead != 0);

    //If the beginning of the line must not be added
    if (pi_StartingColPos != -1)
        {
        HASSERT(pi_EndingColPos < (int32_t)LineToWrite.size());

        //If the end of the line must not be added
        if (pi_EndingColPos != -1)
            {
            LineToWrite = LineToWrite.substr(pi_StartingColPos - 1,
                                             pi_EndingColPos - pi_StartingColPos);
            }
        else
            {
            LineToWrite = LineToWrite.substr(pi_StartingColPos - 1);
            }
        }
    else
        {
        LineToWrite = LineToWrite.substr(0,
                                         pi_EndingColPos - 1);
        }

    po_rPSS = po_rPSS + LineToWrite + L"\n";
    }

void OnDemandMosaicExpressionNode::GetDescriptivePSSInfoFromNodes(const HFCPtr<HPANode>&        pi_rDescriptiveNode,
                                                                  list<DescriptivePSSFileInfo>& pio_rDescriptivePSS)
    {
    //TR 274034 - For an ImageContextNode some modifying nodes might have been
    //attached to the ImageContextNode`s value. Those nodes, if any, need to be
    //part of the descriptive PSS.
    if (dynamic_cast<ImageContextExpressionNode*>(pi_rDescriptiveNode.GetPtr()) != 0)
        {        
        HASSERT(((HPSValueNode*)pi_rDescriptiveNode.GetPtr())->GetValueType() == OBJECT);

        HPSDecoratedHMDContext* pHPSDecoratedHMDContext = dynamic_cast<HPSDecoratedHMDContext*>(((HPSValueNode*)pi_rDescriptiveNode.GetPtr())->GetValue().m_pObjValue);

        if(pHPSDecoratedHMDContext != NULL)
            {
            const HPANodeList& ContextModifyingNodeList = pHPSDecoratedHMDContext->GetModifyingContextNodes();

            HPANodeList::const_iterator NodeIter    = ContextModifyingNodeList.begin();
            HPANodeList::const_iterator NodeIterEnd = ContextModifyingNodeList.end();

            while (NodeIter != NodeIterEnd)
                {
                MergeNodeInfoToDescriptivePSSInfo(*NodeIter, pio_rDescriptivePSS);

                NodeIter++;
                }
            }
        }

    //Add the line describing this node
    MergeNodeInfoToDescriptivePSSInfo(pi_rDescriptiveNode, pio_rDescriptivePSS);

    HPANodeList::const_iterator NodeIter;
    HPANodeList::const_iterator NodeIterEnd;

    if (pi_rDescriptiveNode->GetGrammarObject() == &PARSER->VariableIdentifier_tk)
        {
        const HPANodeList& rNodeList = ((HFCPtr<VariableRefNode>&)pi_rDescriptiveNode)->
                                       GetVariableTokenNode()->
                                       GetExpressionNode()->GetSubNodes();

        NodeIter = rNodeList.begin();
        NodeIterEnd = rNodeList.end();
        }
    else
        {
        const HPANodeList& rNodeList = pi_rDescriptiveNode->GetSubNodes();

        NodeIter    = rNodeList.begin();
        NodeIterEnd = rNodeList.end();
        }

    //Add the line describing the sub-nodes of the current node
    while (NodeIter != NodeIterEnd)
        {
        GetDescriptivePSSInfoFromNodes(*NodeIter,
                                       pio_rDescriptivePSS);

        NodeIter++;
        }
    }

void OnDemandMosaicExpressionNode::MergeNodeInfoToDescriptivePSSInfo(const HFCPtr<HPANode>&        pi_rpRasterNode,
                                                                     list<DescriptivePSSFileInfo>& pio_rDescriptivePSS)
    {
    DescriptivePSSFileInfoList::iterator InfoIter    = pio_rDescriptivePSS.begin();
    DescriptivePSSFileInfoList::iterator InfoIterEnd = pio_rDescriptivePSS.end();

    const HPASourcePos& rSourcePos = pi_rpRasterNode->GetStartPos();

    //Search if the file contains already a line from a previous descriptive statement
    while (InfoIter != InfoIterEnd)
        {
        if (InfoIter->m_pPSSFileName->GetURL() == rSourcePos.m_pURL->GetURL())
            {
            break;
            }

        InfoIter++;
        }

    if (InfoIter == InfoIterEnd)
        {
        DescriptivePSSFileInfo PSSFileInfo;
        PSSFileInfo.m_pPSSFileName = rSourcePos.m_pURL;
        pio_rDescriptivePSS.push_front(PSSFileInfo);
        InfoIter = pio_rDescriptivePSS.begin();
        }

    //Add the line if it isn't already describing another node
    list<uint32_t>::const_iterator LineIter    = (*InfoIter).m_Lines.begin();
    list<uint32_t>::const_iterator LineIterEnd = (*InfoIter).m_Lines.end();

    while (LineIter != LineIterEnd)
        {
        if (*LineIter == rSourcePos.m_Line)
            {
            break;
            }

        LineIter++;
        }

    if (LineIter == LineIterEnd)
        {
        (*InfoIter).m_Lines.push_back(rSourcePos.m_Line);
        }
    }

//---------------------------------------------------------------------------
void TransformedImageExpressionNode::Calculate()
    {
    HPSTransfoObjectValue* pTransfoObjectValue = dynamic_cast<HPSTransfoObjectValue*>(CalculateObject(GetSubNodes()[4]));
    HPSRasterObjectValue*  pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));

    if (pTransfoObjectValue == 0 || pTransfoObjectValue->m_pObject == 0)
        throw HPSTypeMismatchException(GetSubNodes()[4],
                                       HPSTypeMismatchException::TRANSFO);

    if (pRasterObjectValue == 0 || pRasterObjectValue->m_pObject == 0)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE);

    // If a USING operator is present, choose the right world

    HFCPtr<HGF2DCoordSys> pWorld = SESSION->GetCurrentWorld();
    if (GetSubNodes().size() == 8)
        {
        pWorld = SESSION->GetWorld((HGF2DWorldIdentificator)CalculateNumber(GetSubNodes()[6], true));
        if (pWorld == 0)
            throw HPSTypeMismatchException(GetSubNodes()[6],
                                           HPSTypeMismatchException::WORLD);
        }

    // Building a new coordsys reflecting the wished transformation

    HFCPtr<HGF2DCoordSys> pSrcSys = pRasterObjectValue->m_pObject->GetCoordSys();
    HFCPtr<HGF2DTransfoModel> pModel2 = pSrcSys->GetTransfoModelTo(pWorld);
    HFCPtr<HGF2DTransfoModel> pTempModel = pModel2->ComposeInverseWithDirectOf(*pTransfoObjectValue->m_pObject);
    pModel2->Reverse();
    HFCPtr<HGF2DTransfoModel> pModel3 = pTempModel->ComposeInverseWithDirectOf(*pModel2);
    HFCPtr<HGF2DCoordSys> pNewSys = new HGF2DCoordSys(*pModel3, pSrcSys);

    // Creating a RefToRaster to get a virtually transformed image

    HFCPtr<HRAReferenceToRaster> pTransformed = new HRAReferenceToRaster(pRasterObjectValue->m_pObject, pNewSys);

    HPSRasterObjectValue* pRefRasterObjectValue = new HPSRasterObjectValue(pTransformed.GetPtr()); 
    pRefRasterObjectValue->AddHFCPtr();

    SetValue(pRefRasterObjectValue);
    }

//---------------------------------------------------------------------------
void ShapedImageExpressionNode::Calculate()
    {
    HPSShapeObjectValue*   pShapeObjectValue = dynamic_cast<HPSShapeObjectValue*>(CalculateObject(GetSubNodes()[4]));
    HPSRasterObjectValue*  pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));

    if (pShapeObjectValue == 0 || pShapeObjectValue->m_pObject == 0)
        throw HPSShapeExpectedException(GetSubNodes()[4]);
    if (pRasterObjectValue == 0 || pRasterObjectValue->m_pObject == 0)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE);

    HFCPtr<HRARaster> pShaped(new HRAReferenceToRaster(pRasterObjectValue->m_pObject, *pShapeObjectValue->m_pObject));
    if (!(pShaped->GetExtent().IsDefined()))
        throw HPSImageHasNoSizeException(this);

    HPSRasterObjectValue* pRefRasterObjectValue = new HPSRasterObjectValue(pShaped); 
    pRefRasterObjectValue->AddHFCPtr();
    SetValue(pRefRasterObjectValue);
    }

//---------------------------------------------------------------------------
void FilteredImageExpressionNode::Calculate()
    {
    HPSFilterObjectValue*  pFilterObjectValue = dynamic_cast<HPSFilterObjectValue*>(CalculateObject(GetSubNodes()[4]));
    HPSRasterObjectValue*  pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));


    if (pFilterObjectValue == 0 || pFilterObjectValue->m_pObject == 0)
        throw HPSTypeMismatchException(GetSubNodes()[4],
                                       HPSTypeMismatchException::FILTER);

    if (pRasterObjectValue == 0 || pRasterObjectValue->m_pObject == 0)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE);

    HFCPtr<HIMFilteredImage> pFiltered(new HIMFilteredImage(pRasterObjectValue->m_pObject, pFilterObjectValue->m_pObject->Clone()));

    HPSRasterObjectValue* pRefRasterObjectValue = new HPSRasterObjectValue(pFiltered.GetPtr()); 
    pRefRasterObjectValue->AddHFCPtr();
    SetValue(pRefRasterObjectValue);
    }

//---------------------------------------------------------------------------
void TranslucentImageExpressionNode::Calculate()
    {
    // Vérifier type second objet : alphacube = produire un himtranslucent
    //                              alphapalette = produire un hrapixeltypereplacer
    //                              nombre (%) = setter global opacity, himtranslucent...
    // pour palette s'assurer que l'image est 8 bits indexée?

    list<HRPAlphaRange*> AlphaRanges;
    HFCPtr<HPSAlphaPaletteObjectValue> pAlphaPalette = 0;
    HFCPtr<HRARaster> pRaster;

    double GlobalOpacity = -1;
    HPANode* pNode = GetSubNodes()[2];
    while (pNode)
        {
        ((const HFCPtr<HPSValueNode>&)(pNode->GetSubNodes().front()))->Calculate();
        if (((const HFCPtr<HPSValueNode>&)(pNode->GetSubNodes().front()))->GetValueType() == HPSValueNode::NUMBER)
            {
            GlobalOpacity = ((const HFCPtr<HPSValueNode>&)(pNode->GetSubNodes().front()))->GetValue().m_Number;
            if ((GlobalOpacity > 100) || (GlobalOpacity < 0))
                throw HPSOutOfRangeException(pNode->GetSubNodes().front(), 0, 100);
            }
        else if (((const HFCPtr<HPSValueNode>&)(pNode->GetSubNodes().front()))->GetValueType() == HPSValueNode::OBJECT)
            {
            // Simply take the value that has been calculated before
            HPSObjectValue* pObj = ((const HFCPtr<HPSValueNode>&) pNode->GetSubNodes().front())->GetValue().m_pObjValue;

            HPSRasterObjectValue*       pRasterValue = NULL;
            HPSAlphaPaletteObjectValue* pAlphaPaletteValue = NULL;
            HPSAlphaRangeObjectValue*   pAlphaRangeValue = NULL;
            if (NULL != (pRasterValue = dynamic_cast<HPSRasterObjectValue*>(pObj)))
                {
                if (pRaster != 0)
                    throw HPSInvalidObjectException( this);
                if (pAlphaPalette != 0)
                    throw HPSInvalidObjectException( this);
                pRaster = pRasterValue->m_pObject;
                }
            else if (NULL != (pAlphaPaletteValue = dynamic_cast<HPSAlphaPaletteObjectValue*>(pObj)))
                {
                if (pAlphaPalette != NULL)
                    pAlphaPalette->AddEntries(*pAlphaPaletteValue);
                else
                    pAlphaPalette = pAlphaPaletteValue;
                }
            else if (NULL != (pAlphaRangeValue = dynamic_cast<HPSAlphaRangeObjectValue*>(pObj)))
                {
                AlphaRanges.push_back(pAlphaRangeValue->m_pObject);
                }
            else
                throw HPSInvalidObjectException( this);
            }
        else
            {
            throw HPSTypeMismatchException(pNode->GetSubNodes().front(), HPSTypeMismatchException::OBJECT_OR_NUMBER);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    if (pAlphaPalette != 0)
        {
        if (AlphaRanges.size() != 0)
            throw HPSInvalidObjectException( this);

        HIMTranslucentImageCreator TheCreator(pRaster);
        TheCreator.SetTranslucencyMethod(HIMTranslucentImageCreator::COMPOSE);

        HFCPtr<HRARaster> pTranslucentRaster = TheCreator.CreateTranslucentRaster(pAlphaPalette->GetEntries());
        if (!pTranslucentRaster)
            throw HPSAlphaPaletteNotSupportedException( this);

        HPSRasterObjectValue* pRefRasterObjectValue = new HPSRasterObjectValue(pTranslucentRaster); 
        pRefRasterObjectValue->AddHFCPtr();
        SetValue(pRefRasterObjectValue);
        }
    else if ((AlphaRanges.size() != 0) || (GlobalOpacity != -1))
        {
        HIMTranslucentImageCreator TheCreator(pRaster);
        TheCreator.SetTranslucencyMethod(HIMTranslucentImageCreator::COMPOSE);

        if (GlobalOpacity == -1)
            TheCreator.SetDefaultAlpha(255);
        else
            TheCreator.SetDefaultAlpha((Byte)(GlobalOpacity * 255 / 100));
        while (AlphaRanges.size() != 0)
            {
            TheCreator.AddAlphaRange(*(AlphaRanges.front()));
            AlphaRanges.pop_front();
            }

        HFCPtr<HRARaster> pTranslucentRaster = TheCreator.CreateTranslucentRaster();

        HPSRasterObjectValue* pRefRasterObjectValue = new HPSRasterObjectValue(pTranslucentRaster); 
        pRefRasterObjectValue->AddHFCPtr();
        SetValue(pRefRasterObjectValue);
        }
    else
        throw HPSTranslucentInfoNotFoundException(this);
    }

//---------------------------------------------------------------------------
void ColorizedBinaryImageExpressionNode::Calculate()
    {
    HPSRasterObjectValue* pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));

    if (pRasterObjectValue == NULL || pRasterObjectValue->m_pObject == NULL)
        throw HPSTypeMismatchException(GetSubNodes()[2], HPSTypeMismatchException::IMAGE);
    
    // the source must be a 1Bit bitmap
    if (pRasterObjectValue->m_pObject->GetPixelType()->CountPixelRawDataBits() == 1)
        {
        uint32_t RLevel0 = (uint32_t)CalculateNumber(GetSubNodes()[4], true);
        uint32_t GLevel0 = (uint32_t)CalculateNumber(GetSubNodes()[6], true);
        uint32_t BLevel0 = (uint32_t)CalculateNumber(GetSubNodes()[8], true);
        uint32_t RLevel1 = (uint32_t)CalculateNumber(GetSubNodes()[10], true);
        uint32_t GLevel1 = (uint32_t)CalculateNumber(GetSubNodes()[12], true);
        uint32_t BLevel1 = (uint32_t)CalculateNumber(GetSubNodes()[14], true);

        

        if ((RLevel0 < 0) || (RLevel0 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[4], 0, 255);
        if ((GLevel0 < 0) || (GLevel0 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[6], 0, 255);
        if ((BLevel0 < 0) || (BLevel0 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[8], 0, 255);
        if ((RLevel1 < 0) || (RLevel1 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[10], 0, 255);
        if ((GLevel1 < 0) || (GLevel1 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[12], 0, 255);
        if ((BLevel1 < 0) || (BLevel1 > 255))
            throw HPSOutOfRangeException(GetSubNodes()[14], 0, 255);

        HRPPixelPalette Palette(2, HRPPixelTypeV24R8G8B8().GetChannelOrg());
        uint32_t Value = RLevel0 + (GLevel0 << 8) + (BLevel0 << 16);
        Palette.AddEntry(&Value);
        Value = RLevel1 + (GLevel1 << 8) + (BLevel1 << 16);
        Palette.AddEntry(&Value);

        HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(Palette);
        HFCPtr<HRARaster> pColorized(new HRAPixelTypeReplacer(pRasterObjectValue->m_pObject, pPixelType));

        HPSRasterObjectValue* pReplacerRasterObjectValue = new HPSRasterObjectValue(pColorized); 
        pReplacerRasterObjectValue->AddHFCPtr();
        SetValue(pReplacerRasterObjectValue);
        }
    else
        {
        // Do nothing, return the source raster
        ChangeValueOwnership(((HFCPtr<HPSValueNode>&)GetSubNodes()[2]));
        SetValue(pRasterObjectValue);
        }
    }

//---------------------------------------------------------------------------
AlphaCubeExpressionNode::~AlphaCubeExpressionNode()
    {
    }

void AlphaCubeExpressionNode::Calculate()
    {
    HPSColorSetObjectValue* pColorSetObjectValue = dynamic_cast<HPSColorSetObjectValue*>(CalculateObject(GetSubNodes()[2]));

    if (pColorSetObjectValue == NULL || pColorSetObjectValue->m_pObject == NULL)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::COLOR_SET);

    double Opacity = CalculateNumber(GetSubNodes()[4]);

    if ((Opacity > 100) || (Opacity < 0))
        throw HPSOutOfRangeException(GetSubNodes()[4], 0, 100);

    HFCPtr<HRPAlphaRange> pAlphaRange(new HRPAlphaRange(pColorSetObjectValue->m_pObject, (Byte)(Opacity * 255 / 100)));

    HPSAlphaRangeObjectValue* pAlphaRangeObjectValue = new HPSAlphaRangeObjectValue(pAlphaRange); 
    pAlphaRangeObjectValue->AddHFCPtr();
    SetValue(pAlphaRangeObjectValue);
    }

//---------------------------------------------------------------------------
RGBCubeExpressionNode::~RGBCubeExpressionNode()
    {
    }

void RGBCubeExpressionNode::Calculate()
    {
    short Rmin, Rmax, Gmin, Gmax, Bmin, Bmax;

    Rmin = (short)CalculateNumber(GetSubNodes()[2], true);
    if ((Rmin < 0) || (Rmin > 255))
        throw HPSOutOfRangeException(GetSubNodes()[2], 0, 255);

    Rmax = (short)CalculateNumber(GetSubNodes()[4], true);
    if ((Rmax < 0) || (Rmax > 255))
        throw HPSOutOfRangeException(GetSubNodes()[4], 0, 255);

    Gmin = (short)CalculateNumber(GetSubNodes()[6], true);
    if ((Gmin < 0) || (Gmin > 255))
        throw HPSOutOfRangeException(GetSubNodes()[6], 0, 255);

    Gmax = (short)CalculateNumber(GetSubNodes()[8], true);
    if ((Gmax < 0) || (Gmax > 255))
        throw HPSOutOfRangeException(GetSubNodes()[8], 0, 255);

    Bmin = (short)CalculateNumber(GetSubNodes()[10], true);
    if ((Bmin < 0) || (Bmin > 255))
        throw HPSOutOfRangeException(GetSubNodes()[10], 0, 255);

    Bmax = (short)CalculateNumber(GetSubNodes()[12], true);
    if ((Bmax < 0) || (Bmax > 255))
        throw HPSOutOfRangeException(GetSubNodes()[12], 0, 255);

    HFCPtr<HGFRGBCube> pCube(new HGFRGBCube((Byte)Rmin, (Byte)Rmax, (Byte)Gmin, (Byte)Gmax, (Byte)Bmin, (Byte)Bmax));

    HPSColorSetObjectValue* pObjectValue = new HPSColorSetObjectValue(pCube.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
LUVCubeExpressionNode::~LUVCubeExpressionNode()
    {
    }

void LUVCubeExpressionNode::Calculate()
    {
    const double Llower = CalculateNumber(GetSubNodes()[2]);
    if ((Llower < HGFLUVCube::L_MIN) || (Llower > HGFLUVCube::L_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[2], HGFLUVCube::L_MIN, HGFLUVCube::L_MAX);

    const double Lupper = CalculateNumber(GetSubNodes()[4]);
    if ((Lupper < HGFLUVCube::L_MIN) || (Lupper > HGFLUVCube::L_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[4], HGFLUVCube::L_MIN, HGFLUVCube::L_MAX);

    const double Ulower = CalculateNumber(GetSubNodes()[6]);
    if ((Ulower < HGFLUVCube::U_MIN) || (Ulower > HGFLUVCube::U_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[6], HGFLUVCube::U_MIN, HGFLUVCube::U_MAX);

    const double Uupper = CalculateNumber(GetSubNodes()[8]);
    if ((Uupper < HGFLUVCube::U_MIN) || (Uupper > HGFLUVCube::U_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[8], HGFLUVCube::U_MIN, HGFLUVCube::U_MAX);

    const double Vlower = CalculateNumber(GetSubNodes()[10]);
    if ((Vlower < HGFLUVCube::V_MIN) || (Vlower > HGFLUVCube::V_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[10], HGFLUVCube::V_MIN, HGFLUVCube::V_MAX);

    const double Vupper = CalculateNumber(GetSubNodes()[12]);
    if ((Vlower < HGFLUVCube::V_MIN) || (Vlower > HGFLUVCube::V_MAX))
        throw HPSOutOfRangeException(GetSubNodes()[12], HGFLUVCube::V_MIN, HGFLUVCube::V_MAX);

    HFCPtr<HGFLUVCube> pCube(new HGFLUVCube(Llower, Lupper, Ulower, Uupper, Vlower, Vupper));

    HPSColorSetObjectValue* pObjectValue = new HPSColorSetObjectValue(pCube.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void GeoreferenceContextExpressionNode::Calculate()
    {          
    HASSERT(GetSubNodes().size() == 14);
    
    double defaultRatioToMeterForRaster = CalculateNumber(GetSubNodes()[2]);
    double defaultRatioToMeterForSisterFile = CalculateNumber(GetSubNodes()[4]);
    bool   useSisterFile = CalculateNumber(GetSubNodes()[6], true) != 0;
    bool   usePCSLinearUnit = CalculateNumber(GetSubNodes()[8], true) != 0;
    //bool   useDefaultUnitForGeoModel = CalculateNumber(GetSubNodes()[10], true) != 0;  //Setting not supported anymore on vancouver 
    bool   interpretAsIntergraphUnit = CalculateNumber(GetSubNodes()[12], true) != 0;
        
    HFCPtr<HRFGeoreferenceContext> pGeoreferenceContext(new HRFGeoreferenceContext(defaultRatioToMeterForRaster, 
                                                                                   defaultRatioToMeterForSisterFile, 
                                                                                   useSisterFile, 
                                                                                   usePCSLinearUnit,
                                                                                   interpretAsIntergraphUnit));        

    HPSGeoreferenceContextObjectValue* pObjectValue = new HPSGeoreferenceContextObjectValue(pGeoreferenceContext); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ImageContextExpressionNode::Calculate()
    {
    HFCPtr<HPSDecoratedHMDContext> pContext(new HPSDecoratedHMDContext());

    pContext->AddHFCPtr();
    SetValue(pContext);
    }

//---------------------------------------------------------------------------
void AlphaPaletteExpressionNode::Calculate()
    {
    double Opacity = CalculateNumber(GetSubNodes()[2]);
    if ((Opacity < 0.0) || (Opacity > 100.0))
        throw HPSOutOfRangeException(GetSubNodes()[2], 0, 100);

    HFCPtr<HPSAlphaPaletteObjectValue> pPaletteObject(new HPSAlphaPaletteObjectValue((Byte)(Opacity * 255 / 100)));

    HPANode* pNode = GetSubNodes()[4];
    while (pNode)
        {
        if (pNode->GetSubNodes().front()->GetSubNodes().size() == 1)
            {
            int32_t Index = (int32_t)CalculateNumber(pNode->GetSubNodes().front()->GetSubNodes()[0], true);
            if ((Index < 0) || (Index > 255))
                throw HPSOutOfRangeException(GetSubNodes()[2], 0, 255);

            pPaletteObject->AddEntry((Byte)Index);
            }
        else
            {
            int32_t IndexLeft = (int32_t)CalculateNumber(pNode->GetSubNodes().front()->GetSubNodes()[0], true);
            int32_t IndexRight = (int32_t)CalculateNumber(pNode->GetSubNodes().front()->GetSubNodes()[2], true);
            if ((IndexLeft < 0) || (IndexLeft > 255))
                throw HPSOutOfRangeException(pNode->GetSubNodes().front()->GetSubNodes()[0], 0, 255);
            if ((IndexRight < 0) || (IndexRight > 255))
                throw HPSOutOfRangeException(pNode->GetSubNodes().front()->GetSubNodes()[2], 0, 255);
            if (IndexRight < IndexLeft)
                throw HPSInvalidNumericException( pNode->GetSubNodes().front()->GetSubNodes()[0]);

            pPaletteObject->AddEntries((Byte)IndexLeft, (Byte)IndexRight);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    pPaletteObject->AddHFCPtr();
    SetValue(pPaletteObject);
    }

//---------------------------------------------------------------------------
void VectorObjectNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
    }

//---------------------------------------------------------------------------
ShapeExpressionNode::~ShapeExpressionNode()
    {
    }

void ShapeExpressionNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes().front());
    }

//---------------------------------------------------------------------------
void RectangleExpressionNode::Calculate()
    {
    double X1 = CalculateNumber(GetSubNodes()[2]);
    double Y1 = CalculateNumber(GetSubNodes()[4]);
    double X2 = CalculateNumber(GetSubNodes()[6]);
    double Y2 = CalculateNumber(GetSubNodes()[8]);

    if (X1 > X2)
        throw HPSInvalidCoordsException(GetSubNodes()[2]);
    if (Y1 > Y2)
        throw HPSInvalidCoordsException(GetSubNodes()[4]);

    // If a USING operator is present, choose the right coord sys
    HFCPtr<HGF2DCoordSys> pWorld = SESSION->GetCurrentWorld();
    if (GetSubNodes().size() == 12)
        {
        pWorld = SESSION->GetWorld((HGF2DWorldIdentificator)CalculateNumber(GetSubNodes()[10], true));
        if (pWorld == 0)
            throw HPSTypeMismatchException(GetSubNodes()[10],
                                           HPSTypeMismatchException::WORLD);
        }

    HFCPtr<HVEShape> pShape(new HVEShape(X1, Y1, X2, Y2, pWorld));

    HPSShapeObjectValue* pObjectValue = new HPSShapeObjectValue(pShape); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void PolygonExpressionNode::Calculate()
    {
    vector<double> ListOfValues;
    ListOfValues.reserve(40);  // arbitrary value good for polygon up to 20 points.

    // If a USING operator is present, choose the right coord sys
    HFCPtr<HGF2DCoordSys> pWorld = SESSION->GetCurrentWorld();
    if (GetSubNodes().size() == 6)
        {
        pWorld = SESSION->GetWorld((HGF2DWorldIdentificator)CalculateNumber(GetSubNodes()[4], true));
        if (pWorld == 0)
            throw HPSTypeMismatchException(GetSubNodes()[4],
                                           HPSTypeMismatchException::WORLD);
        }

    HVE2DPolySegment PolyLine(pWorld);

    HPANode* pNode = GetSubNodes()[2];
    while (pNode)
        {
        double XCoord = CalculateNumber(pNode->GetSubNodes().front());
        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        if (pNode == 0)
            throw HPSTooFewParamException( this);
        double YCoord = CalculateNumber(pNode->GetSubNodes().front());
        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        PolyLine.AppendPosition(HGF2DPosition(XCoord, YCoord));
        }

    if (PolyLine.GetStartPoint() != PolyLine.GetEndPoint())
        throw HPSInvalidCPolygonException( this);

    if ((PolyLine.AutoCrosses()) || (PolyLine.IsAutoContiguous()))
        throw HPSInvalidCPolygonException( this);

    HFCPtr<HVEShape> pShape(new HVEShape(HVE2DPolygonOfSegments(PolyLine)));

    HPSShapeObjectValue* pObjectValue = new HPSShapeObjectValue(pShape); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void HoledShapeExpressionNode::Calculate()
    {
    HFCPtr<HVEShape> pHoledShape;
    HPANode*         pNode = GetSubNodes()[2];

    while (pNode)
        {
        HPSShapeObjectValue* pShapeObjectValue = dynamic_cast<HPSShapeObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));
        if (pShapeObjectValue == NULL || pShapeObjectValue->m_pObject == NULL)
            throw HPSShapeExpectedException(pNode->GetSubNodes().front());

        if (pHoledShape == 0)
            pHoledShape = new HVEShape(*pShapeObjectValue->m_pObject);
        else
            {
            pHoledShape->Differentiate(*pShapeObjectValue->m_pObject);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    if (pHoledShape == 0)
        throw HPSTooFewParamException(this);

    HPSShapeObjectValue* pObjectValue = new HPSShapeObjectValue(pHoledShape); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void UnifiedShapeExpressionNode::Calculate()
    {
    HFCPtr<HVEShape> pUnifiedShape;
    HPANode*         pNode = GetSubNodes()[2];

    while (pNode)
        {
        HPSShapeObjectValue* pShapeObjectValue = dynamic_cast<HPSShapeObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));
        if (pShapeObjectValue == NULL || pShapeObjectValue->m_pObject == NULL)
            throw HPSShapeExpectedException(pNode->GetSubNodes().front());

        if (pUnifiedShape == 0)
            pUnifiedShape = new HVEShape(*pShapeObjectValue->m_pObject);
        else
            {
            pUnifiedShape->Unify(*pShapeObjectValue->m_pObject);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    if (pUnifiedShape == 0)
        throw HPSTooFewParamException( this);

    HPSShapeObjectValue* pObjectValue = new HPSShapeObjectValue(pUnifiedShape); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void CommonShapeExpressionNode::Calculate()
    {
    HFCPtr<HVEShape> pCommonShape;
    HPANode*         pNode = GetSubNodes()[2];

    while (pNode)
        {
        HPSShapeObjectValue* pShapeObjectValue = dynamic_cast<HPSShapeObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));
        if (pShapeObjectValue == NULL || pShapeObjectValue->m_pObject == NULL)
            throw HPSShapeExpectedException(pNode->GetSubNodes().front());

        if (pCommonShape == 0)
            pCommonShape = new HVEShape(*pShapeObjectValue->m_pObject);
        else
            {
            pCommonShape->Intersect(*pShapeObjectValue->m_pObject);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    if (pCommonShape == 0)
        throw HPSTooFewParamException( this);

    HPSShapeObjectValue* pObjectValue = new HPSShapeObjectValue(pCommonShape); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
TransfoObjectNode::~TransfoObjectNode()
    {
    }

void TransfoObjectNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    ChangeValueOwnership(((const HFCPtr<HPSValueNode>&)GetSubNodes().front()));
    SetValue(((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->GetValue().m_pObjValue);
    }

//---------------------------------------------------------------------------
void IdentityExpressionNode::Calculate()
    {
    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(new HGF2DIdentity()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void RotationExpressionNode::Calculate()
    {
    double Angle = CalculateNumber(GetSubNodes()[2]);
    double CenterX = CalculateNumber(GetSubNodes()[4]);
    double CenterY = CalculateNumber(GetSubNodes()[6]);
    HFCPtr<HGF2DSimilitude> pTransfo = new HGF2DSimilitude();  // units!?!!?!?!
    pTransfo->AddRotation(Angle * PI/180, CenterX, CenterY);  // in degrees CCW

    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ScalingExpressionNode::Calculate()
    {
    double ScaleX = CalculateNumber(GetSubNodes()[2]);
    double ScaleY = CalculateNumber(GetSubNodes()[4]);
    double CenterX = CalculateNumber(GetSubNodes()[6]);
    double CenterY = CalculateNumber(GetSubNodes()[8]);

    // Validate scaling parameters
    if (ScaleX == 0.0 || ScaleY == 0.0)
        {
        throw HPSTransfoParameterInvalidException(this);
        }

    HFCPtr<HGF2DStretch> pTransfo(new HGF2DStretch());   // units!?!!?!?!
    pTransfo->AddAnisotropicScaling(ScaleX, ScaleY, CenterX, CenterY);

    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void TranslationExpressionNode::Calculate()
    {
    double DeltaX = CalculateNumber(GetSubNodes()[2]);
    double DeltaY = CalculateNumber(GetSubNodes()[4]);
    // units!?!!?!?!
    HFCPtr<HGF2DTranslation> pTransfo(new HGF2DTranslation(HGF2DDisplacement(DeltaX, DeltaY)));
    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void AffineExpressionNode::Calculate()
    {
    double A1 = CalculateNumber(GetSubNodes()[2]);
    double A2 = CalculateNumber(GetSubNodes()[4]);
    double A0 = CalculateNumber(GetSubNodes()[6]);
    double B1 = CalculateNumber(GetSubNodes()[8]);
    double B2 = CalculateNumber(GetSubNodes()[10]);
    double B0 = CalculateNumber(GetSubNodes()[12]);
    // units!?!!?!?!

    // Validate parameters to the affine model
    if ((A1 == 0.0) && (A2 == 0.0) ||
        (B1 == 0.0) && (B2 == 0.0) ||
        (A1 == 0.0) && (B1 == 0.0) ||
        (A2 == 0.0) && (B2 == 0.0))
        {
        throw HPSTransfoParameterInvalidException( this);
        }

    HFCPtr<HGF2DAffine> pTransfo(new HGF2DAffine());
    pTransfo->SetByMatrixParameters(A0, A1, A2, B0, B1, B2);

    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ProjectiveExpressionNode::Calculate()
    {
    HFCMatrix<3,3> Matrix;
    Matrix[0][0] = CalculateNumber(GetSubNodes()[2]);
    Matrix[0][1] = CalculateNumber(GetSubNodes()[4]);
    Matrix[0][2] = CalculateNumber(GetSubNodes()[6]);
    Matrix[1][0] = CalculateNumber(GetSubNodes()[8]);
    Matrix[1][1] = CalculateNumber(GetSubNodes()[10]);
    Matrix[1][2] = CalculateNumber(GetSubNodes()[12]);
    Matrix[2][0] = CalculateNumber(GetSubNodes()[14]);
    Matrix[2][1] = CalculateNumber(GetSubNodes()[16]);
    Matrix[2][2] = CalculateNumber(GetSubNodes()[18]);

    // Validate parameters to the projective model
    if ((Matrix[2][2] == 0.0) ||
        ((Matrix[0][0] == 0.0) && (Matrix[0][1] == 0.0)) ||
        ((Matrix[0][0] == 0.0) && (Matrix[1][0] == 0.0)) ||
        ((Matrix[0][1] == 0.0) && (Matrix[1][1] == 0.0)) ||
        ((Matrix[1][0] == 0.0) && (Matrix[1][1] == 0.0)))
        {
        throw HPSTransfoParameterInvalidException(this);
        }

    HFCPtr<HGF2DProjective> pTransfo(new HGF2DProjective(Matrix));

    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void LocalProjectiveGridExpressionNode::Calculate()
    {
    double X1 = CalculateNumber(GetSubNodes()[2]);
    double Y1 = CalculateNumber(GetSubNodes()[4]);
    double X2 = CalculateNumber(GetSubNodes()[6]);
    double Y2 = CalculateNumber(GetSubNodes()[8]);

    HGF2DLiteExtent Extent(X1, Y1, X2, Y2);

    int32_t NbTileX = (int32_t)CalculateNumber(GetSubNodes()[10], true);
    if (NbTileX <= 0)
        throw HPSTransfoParameterInvalidException(this);

    int32_t NbTileY = (int32_t)CalculateNumber(GetSubNodes()[12], true);
    if (NbTileY <= 0)
        throw HPSTransfoParameterInvalidException(this);

    HPSTransfoObjectValue* pGlobalAffineObjectValue = dynamic_cast<HPSTransfoObjectValue*>(CalculateObject(GetSubNodes()[14]));
    if (pGlobalAffineObjectValue == NULL || !pGlobalAffineObjectValue->m_pObject->IsCompatibleWith(HGF2DAffine::CLASS_ID))
        throw HPSTransfoParameterInvalidException(this);

    HPANode* pNode = GetSubNodes()[16];

    list<HFCPtr<HGF2DTransfoModel> > ProjectiveList;
    while (pNode)
        {
        HPSTransfoObjectValue* pProjectiveObjectValue = dynamic_cast<HPSTransfoObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));
        if (pProjectiveObjectValue == NULL)
            throw HPSTransfoParameterInvalidException(this);

        ProjectiveList.push_back(pProjectiveObjectValue->m_pObject);

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;

        }

    if (ProjectiveList.size() != NbTileX * NbTileY)
        throw HPSTransfoParameterInvalidException(this);


    HGF2DLocalProjectiveGrid* pTransfo =
        new HGF2DLocalProjectiveGrid(static_cast<HGF2DAffine&>(*pGlobalAffineObjectValue->m_pObject),
                                     Extent,
                                     NbTileX,
                                     NbTileY,
                                     ProjectiveList);

    // Creating a RefToRaster to get a virtually transformed image
    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ComposedExpressionNode::Calculate()
    {
    HFCPtr<HGF2DTransfoModel> pTransfo;
    HPANode*                  pNode = GetSubNodes()[2];

    while (pNode)
        {
        HPSTransfoObjectValue* pTransfoObjectValue = dynamic_cast<HPSTransfoObjectValue*>(CalculateObject(pNode->GetSubNodes().front()));

        if (pTransfoObjectValue == NULL || pTransfoObjectValue->m_pObject == NULL)
            throw HPSTypeMismatchException(pNode->GetSubNodes().front(),
                                           HPSTypeMismatchException::TRANSFO);

        if (pTransfo == 0)
            {
            pTransfo = pTransfoObjectValue->m_pObject->Clone();
            }
        else
            {
            // Compose the two models
            pTransfo = pTransfo->ComposeInverseWithDirectOf(*pTransfoObjectValue->m_pObject);
            HASSERT(pTransfo != 0);
            }

        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    HPSTransfoObjectValue* pObjectValue = new HPSTransfoObjectValue(pTransfo.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void FilterObjectNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->Calculate();
    ChangeValueOwnership(((const HFCPtr<HPSValueNode>&)GetSubNodes().front()));
    SetValue(((const HFCPtr<HPSValueNode>&)GetSubNodes().front())->GetValue().m_pObjValue);
    }

//---------------------------------------------------------------------------
void ContrastExpressionNode::Calculate()
    {
    double ModifLevel = CalculateNumber(GetSubNodes()[2]);
    if ((ModifLevel < -100.0) || (ModifLevel > 100.0))
        throw HPSOutOfRangeException(GetSubNodes()[2], -100, 100);
    short Param = (short)((ModifLevel < 0) ? (ModifLevel * 128 / 100 ) : (ModifLevel * 127 / 100));
    HFCPtr<HRPFilter> pFilter(new HRPContrastFilter((int8_t)Param));

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void BrightnessExpressionNode::Calculate()
    {
    double ModifLevel = CalculateNumber(GetSubNodes()[2]);
    if ((ModifLevel < -100.0) || (ModifLevel > 100.0))
        throw HPSOutOfRangeException(GetSubNodes()[2], -100, 100);
    HFCPtr<HRPFilter> pFilter(new HRPColorBalanceFilter((int32_t)(ModifLevel * 256 / 100)));

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ConvolutionRGBExpressionNode::Calculate()
    {
    // Getting matrix parameters

    uint32_t Width = (uint32_t)CalculateNumber(GetSubNodes()[2], true);
    uint32_t Height = (uint32_t)CalculateNumber(GetSubNodes()[4], true);
    uint32_t CenterPos = (uint32_t)CalculateNumber(GetSubNodes()[6], true);

    if ((Height < 1) || (Height > 64))
        throw HPSOutOfRangeException(GetSubNodes()[4], 1, 64);
    if ((Width < 1) || (Width > 64))
        throw HPSOutOfRangeException(GetSubNodes()[2], 1, 64);

    // Getting the matrix

    size_t MatrixSize = Height * Width;
    HArrayAutoPtr<int32_t> pMatrix(new int32_t[MatrixSize]);
    size_t i = 0;
    HPANode* pNode = GetSubNodes()[8];
    while (pNode)
        {
        if (i >= MatrixSize)
            throw HPSTooManyParamException(pNode);
        pMatrix[i++] = (int32_t)CalculateNumber(pNode->GetSubNodes().front(), true);
        if (pNode->GetSubNodes().size() == 3)
            pNode = pNode->GetSubNodes()[2];
        else
            pNode = 0;
        }

    if (CenterPos >= (Width * Height))
        throw HPSOutOfRangeException(GetSubNodes()[6], 0, (Width * Height) - 1);

    // Creating the filter
    HFCPtr<HRPFilter> pFilter(new HRPCustomConvFilter(Width, Height,
                                                      CenterPos % Width, CenterPos / Width,
                                                      pMatrix));
    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void Convolution3ExpressionNode::Calculate()
    {
    // Getting matrix parameters

    uint32_t FactorA = (uint32_t)CalculateNumber(GetSubNodes()[2], true);
    uint32_t FactorB = (uint32_t)CalculateNumber(GetSubNodes()[4], true);
    uint32_t FactorC = (uint32_t)CalculateNumber(GetSubNodes()[6], true);

    // Making the matrix

    int32_t Matrix[3][3];
    Matrix[0][0] = Matrix[0][2] = Matrix[2][0] = Matrix[2][2] = FactorA;
    Matrix[0][1] = Matrix[1][0] = Matrix[1][2] = Matrix[2][1] = FactorB;
    Matrix[1][1] = FactorC;

    // Creating the filter
    HFCPtr<HRPFilter> pFilter(new HRPCustomConvFilter(3, 3, 1, 1, &Matrix[0][0]));

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void AutoContrastStretchExpressionNode::Calculate()
    {
    HPSRasterObjectValue* pRasterObjectValue = dynamic_cast<HPSRasterObjectValue*>(CalculateObject(GetSubNodes()[2]));
    uint32_t CutOffPercentage = (uint32_t)CalculateNumber(GetSubNodes()[4]);
    uint32_t HistogramPrecision = 100;

    if (GetSubNodes().size() > 6)
        HistogramPrecision = (uint32_t)CalculateNumber(GetSubNodes()[6], true);

    if (pRasterObjectValue == NULL || pRasterObjectValue->m_pObject == NULL)
        throw HPSTypeMismatchException(GetSubNodes()[2],
                                       HPSTypeMismatchException::IMAGE);
    if ((CutOffPercentage < 0) || (CutOffPercentage > 99))
        throw HPSOutOfRangeException(GetSubNodes()[4], 0, 99);
    if ((HistogramPrecision < 1) || ( HistogramPrecision > 100))
        throw HPSOutOfRangeException(GetSubNodes()[6], 1, 100);

    // Get histogram for the specified image

    HFCPtr<HRPHistogram> pHistogram = new HRPHistogram(256);
    HRASamplingOptions SamplingOptions;
    SamplingOptions.SetPyramidImageSize((Byte)HistogramPrecision);
    HRAHistogramOptions HistogramOptions(pHistogram, 0, SamplingOptions);
    pRasterObjectValue->m_pObject->ComputeHistogram(&HistogramOptions);

    uint32_t Total = 0;
    for (unsigned short j = 0; j < 256; j++)
        Total += pHistogram->GetEntryCount(j);

    // calculate the MIN range pos

    double NbPixelsToRemove = (double)Total * CutOffPercentage / 200;
    uint32_t MinValue = 0;
    uint32_t PixelCount = 0;
    while (PixelCount < NbPixelsToRemove)
        PixelCount += pHistogram->GetEntryCount(MinValue++);
    if (PixelCount > NbPixelsToRemove)
        MinValue--;

    // calculate the MAX range pos

    uint32_t MaxValue = 255;
    PixelCount = 0;
    while (PixelCount < NbPixelsToRemove)
        PixelCount += pHistogram->GetEntryCount(MaxValue--);
    if (PixelCount > NbPixelsToRemove)
        MaxValue++;

    HRPHistogramScalingFilter* pFilter;

    pFilter = new HRPHistogramScalingFilter();
    for (unsigned short i = 0; i < 3; i++)  // 24-bit pixel size
        pFilter->SetInterval(i,(unsigned short)MinValue, (unsigned short)MaxValue);

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void ContrastStretchExpressionNode::Calculate()
    {
    uint32_t CutOffLeft = (uint32_t)CalculateNumber(GetSubNodes()[2], true);
    uint32_t CutOffRight = (uint32_t)CalculateNumber(GetSubNodes()[4], true);

    if ((CutOffLeft < 0) || (CutOffLeft > 99))
        throw HPSOutOfRangeException(GetSubNodes()[2], 0, 99);
    if ((CutOffRight < 1) || (CutOffRight > 100))
        throw HPSOutOfRangeException(GetSubNodes()[4], 1, 100);
    if (CutOffLeft >= CutOffRight)
        throw HPSInvalidNumericException(GetSubNodes()[2]);

    CutOffLeft = CutOffLeft * 255 / 100;
    CutOffRight = CutOffRight * 255 / 100;

    HFCPtr<HRPHistogramScalingFilter> pFilter(new HRPHistogramScalingFilter());

    for (unsigned short i = 0; i < 3; i++)  // 24-bit pixel size
        pFilter->SetInterval(i, (unsigned short)CutOffLeft, (unsigned short)CutOffRight);

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(pFilter.GetPtr()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void TintExpressionNode::Calculate()
    {
    unsigned char Level[3];
    if (GetSubNodes().size() > 6)
        {
        uint32_t RLevel = (uint32_t)CalculateNumber(GetSubNodes()[2], true);
        uint32_t GLevel = (uint32_t)CalculateNumber(GetSubNodes()[4], true);
        uint32_t BLevel = (uint32_t)CalculateNumber(GetSubNodes()[6], true);
        if ((RLevel < 0) || (RLevel > 255))
            throw HPSOutOfRangeException(GetSubNodes()[2], 0, 255);
        if ((GLevel < 0) || (GLevel > 255))
            throw HPSOutOfRangeException(GetSubNodes()[4], 0, 255);
        if ((BLevel < 0) || (BLevel > 255))
            throw HPSOutOfRangeException(GetSubNodes()[6], 0, 255);
        Level[0] = (unsigned char)RLevel;
        Level[1] = (unsigned char)GLevel;
        Level[2] = (unsigned char)BLevel;
        }

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(new HRPTintFilter(Level)); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void InverterExpressionNode::Calculate()
    {
    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(new HRPInvertFilter()); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
void GammaCorrectionExpressionNode::Calculate()
    {
    double Level = CalculateNumber(GetSubNodes()[2]);

    HPSFilterObjectValue* pObjectValue = new HPSFilterObjectValue(new HRPGammaFilter(Level)); 
    pObjectValue->AddHFCPtr();
    SetValue(pObjectValue);
    }

//---------------------------------------------------------------------------
StatementDefinitionNode::StatementDefinitionNode(HPAGrammarObject* pi_pObj,
                                                 const HPANodeList& pi_rList,
                                                 const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    m_pReturnNode = (const HFCPtr<ReturnStatementNode>&)pi_rList[pi_rList.size()-2];
    m_pScope = ((const HFCPtr<StatementDeclarationNode>&)pi_rList.front())->m_pScope;
    m_pScope->GetOwner()->AddStatement(((const HFCPtr<StatementDeclarationNode>&)pi_rList.front())->m_Name, this);
    SESSION->ChangeScope(m_pScope->GetOwner());
    }

//---------------------------------------------------------------------------
StatementDeclarationNode::StatementDeclarationNode(HPAGrammarObject* pi_pObj,
                                                   const HPANodeList& pi_rList,
                                                   const HFCPtr<HPASession>& pi_pSession)
    : HPANode(pi_pObj, pi_rList, pi_pSession)
    {
    // adding scope

    m_Name = ((const HFCPtr<HPATokenNode>&)pi_rList[1])->GetText();
    m_pScope = new HPSParserScope(PARSER);
    m_pScope->SetSession(SESSION);
    m_pScope->SetOwner(SESSION->GetCurrentScope());
    SESSION->ChangeScope(m_pScope);

    // adding parameters

    if (pi_rList.size() == 5)
        {
        HPANode* pNode = pi_rList[3];
        while (pNode)
            {
            m_pScope->AddParameter(pNode->GetSubNodes().front());
            if (pNode->GetSubNodes().size() == 3)
                pNode = pNode->GetSubNodes()[2];
            else
                pNode = 0;
            }
        }
    }

//---------------------------------------------------------------------------
void ReturnStatementNode::Calculate()
    {
    ((const HFCPtr<HPSValueNode>&)GetSubNodes()[1])->Calculate();
    SetValueFrom((const HFCPtr<HPSValueNode>&)GetSubNodes()[1]);
    }
