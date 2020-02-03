/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/Parser.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//###############################
// INCLUDE FILES
//###############################
#include "ImageConverterPch.h"

#include <tchar.h>
#include "Parser.h"
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/htifftag.h>


#if 0

USING_NAMESPACE_IMAGEPP

typedef map<string, TIFFGeoKey, less<string>,
                    allocator<TIFFGeoKey> > GEOTIFFTAG_MAP;

typedef map<string, unsigned short, less<string>,
                    allocator<unsigned short> > GEOTIFFTAGVALUE_MAP;

typedef map<string, TIFFTag, less<string>,
                    allocator<TIFFTag> > GEOTIFFTAGTRANSFO_MAP;


extern GEOTIFFTAG_MAP g_GeoTiffTagMap;
extern GEOTIFFTAGVALUE_MAP g_GeoTiffTagValueMap;
extern GEOTIFFTAGTRANSFO_MAP g_GeoTiffTagTransfoMap;


//###############################
// PRIVATE CLASS
//###############################

class IdentifierNode : public HPATokenNode
{
    public:
        // Enum
        enum ValueType  { SHORT_TYPE, DOUBLE_TYPE, LONG_TYPE, ERROR_TYPE };

    private:
     
        ValueType m_Type;
        union {
            double m_Double;
            unsigned short m_Short;
            int32_t m_Long;
        } u;

    public:
        // Construction - destruction
            IdentifierNode(HPAGrammarObject* pi_pToken, 
                           const string& pi_rText,
                           const HPASourcePos& pi_rLeftPos,
                           const HPASourcePos& pi_rRightPos);
        virtual 
            ~IdentifierNode(){}

        ValueType 
            GetValueType() const { return m_Type; }
        unsigned short             GetShort() const { return u.m_Short; }
        int32_t
            GetLong() const { return u.m_Long; }
        double 
            GetDouble() const { return u.m_Double; }
};

class TagInfoNode : public HPANode
{
    public:
        // Construction - destruction
            TagInfoNode(HPAGrammarObject* pi_pObj, 
                        const HPANodeList& pi_rList,
                        HPAParser* pi_pParser);
        virtual 
            ~TagInfoNode(){}
};

class KeyInfoNode : public HPANode
{
    public:
        // Construction - destruction
            KeyInfoNode(HPAGrammarObject* pi_pObj, 
                        const HPANodeList& pi_rList,
                        HPAParser* pi_pParser);
        virtual 
            ~KeyInfoNode()  { }
};

class TagInfoNodeCreator : public HPANodeCreator
{
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            HPAParser* pi_pParser)
    {
        return new TagInfoNode(pi_pObj, pi_rList, pi_pParser);
    }
} s_TagInfoNodeCreator;

class KeyInfoNodeCreator : public HPANodeCreator
{
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            HPAParser* pi_pParser)
    {
        return new KeyInfoNode(pi_pObj, pi_rList, pi_pParser);
    }
} s_KeyInfoNodeCreator;

class GTIFFTokenizer : public HPADefaultTokenizer
{
    public:
        // Construction - destruction    
            GTIFFTokenizer(GTIFFParser* pi_pParser);
        virtual                 
            ~GTIFFTokenizer();

    protected:

        virtual HPANode*        
            MakeNode(HPAToken* pi_pToken, 
                     const string& pi_rText,
                     const HPASourcePos& pi_rLeft, 
                     const HPASourcePos& pi_rRight);

    private:

        GTIFFParser*    m_pParser;
};


//###############################
// TOKENIZER
//###############################


//-----------------------------------------------------------------
// GTIFFTokenizer
// 
// Constructor.
//-----------------------------------------------------------------
GTIFFTokenizer::GTIFFTokenizer(GTIFFParser* pi_pParser)
               :HPADefaultTokenizer(false), 
                m_pParser(pi_pParser)
{
}

//-----------------------------------------------------------------
// ~GTIFFTokenizer
// 
// Destructor.
//-----------------------------------------------------------------
GTIFFTokenizer::~GTIFFTokenizer()
{
}

//-----------------------------------------------------------------
// MakeNode
// 
// Build a node.
//-----------------------------------------------------------------
HPANode* GTIFFTokenizer::MakeNode(HPAToken* pi_pToken, 
                                  const string& pi_rText,
                                  const HPASourcePos& pi_rLeft, 
                                  const HPASourcePos& pi_rRight)
{
    HPANode* pNode;

    try
    {
        if (pi_pToken == &m_pParser->Identifier_tk)
        {
            pNode = new IdentifierNode(pi_pToken, pi_rText, pi_rLeft, pi_rRight);
            
            if( ((IdentifierNode*)pNode)->GetValueType() == IdentifierNode::ERROR_TYPE )
            {
                throw HPAException(pNode);
            }
        }
        else
            pNode = HPADefaultTokenizer::MakeNode(pi_pToken, pi_rText, pi_rLeft, pi_rRight);

        
    }
    catch(HPAException& Err)
    {
        pi_pToken =  &m_pParser->Error_tk;
    }
    
    return pNode;
}


//###############################
// PARSER
//###############################


//-----------------------------------------------------------------
// GTIFFParser
// 
// Constructor
//-----------------------------------------------------------------
GTIFFParser::GTIFFParser(HTIFFFile& pi_rTiffFile,
                         HTIFFGeoKey& pi_rGeoKey)
            :m_rGeoKey(pi_rGeoKey),
             m_rTiffFile(pi_rTiffFile)
{
    // Tokenizer setup

    GTIFFTokenizer* pTok = new GTIFFTokenizer(this);
    pTok->SetIdentifierToken(Identifier_tk);
    pTok->SetNumberToken(Number_tk);
    pTok->SetStringToken(String_tk);
    pTok->SetCommentMarker(';');

    pTok->AddSymbol("(", LP_tk);
    pTok->AddSymbol(")", RP_tk);
    pTok->AddSymbol(",", COMMA_tk);
    pTok->AddSymbol(":", COLON_tk);
    pTok->AddSymbol("-", MINUS_tk);
    pTok->AddSymbol(".", DOT_tk);

    pTok->AddSymbol("GEOTIFF_INFORMATION", GEOTIFF_INFORMATION_tk);
    pTok->AddSymbol("END_OF_GEOTIFF", END_OF_GEOTIFF_tk);
    pTok->AddSymbol("VERSION", VERSION_tk);
    pTok->AddSymbol("KEY_REVISION", KEY_REVISION_tk);
    pTok->AddSymbol("TAGGED_INFORMATION", TAGGED_INFORMATION_tk);
    pTok->AddSymbol("END_OF_TAGS", END_OF_TAGS_tk);
    pTok->AddSymbol("KEYED_INFORMATION", KEYED_INFORMATION_tk);
    pTok->AddSymbol("END_OF_KEYS", END_OF_KEYS_tk);
    pTok->AddSymbol("SHORT", SHORT_tk);
    pTok->AddSymbol("ASCII", ASCII_tk);
    pTok->AddSymbol("DOUBLE", DOUBLE_tk);

    pTok->SetErrorToken(Error_tk);


    // Grammar definition

    GeoTIFF = GEOTIFF_INFORMATION_tk + COLON_tk + Body + END_OF_GEOTIFF_tk + DOT_tk;
    //          || GEOTIFF_INFORMATION_tk + COLON_tk + Body + END_OF_GEOTIFF_tk + DOT_tk + Error_tk;

    Body = Header + Sections;

    Header =    HeaderEntry + Header
             || HeaderEntry;

    HeaderEntry =    VersionExpression
                  || KeyRevisionExpression;

    VersionExpression = VERSION_tk + COLON_tk + Number_tk;

    KeyRevisionExpression = KEY_REVISION_tk + COLON_tk + Number_tk;

    Sections =    Section + Sections
               || Section;

    Section =    TaggedInformation
              || KeyedInformation;

    TaggedInformation = TAGGED_INFORMATION_tk + COLON_tk + TagList + END_OF_TAGS_tk + DOT_tk;

    KeyedInformation = KEYED_INFORMATION_tk + COLON_tk + KeyList + END_OF_KEYS_tk + DOT_tk;

    TagList =    TagInfo + TagList
              || TagInfo;    

    TagInfo = Identifier_tk + LP_tk + Number_tk + COMMA_tk + Number_tk + RP_tk + COLON_tk + NumberList;

    NumberList =    NumericExpression + NumberList
                 || NumericExpression;

    NumericExpression =    Number_tk
                        || MINUS_tk + Number_tk;
     
    KeyList =    KeyInfo + KeyList
              || KeyInfo;

    KeyInfo = Identifier_tk + LP_tk + TypeExpression + COMMA_tk + Number_tk + RP_tk + COLON_tk + ValueExpression;

    TypeExpression = SHORT_tk || ASCII_tk || DOUBLE_tk;

    ValueExpression = String_tk || NumericExpression || Identifier_tk;


    // Connecting...

    SetTokenizer(pTok);
    SetStartRule(&GeoTIFF);

    // Node creator setup

    TagInfo(&s_TagInfoNodeCreator);
    KeyInfo(&s_KeyInfoNodeCreator);


#ifdef __HMR_DEBUG
    GeoTIFF.SetName("GeoTIFF");
    Body.SetName("Body");
    Header.SetName("Header");
    HeaderEntry.SetName("HeaderEntry");
    VersionExpression.SetName("VersionExpression");
    KeyRevisionExpression.SetName("KeyRevisionExpression");
    Sections.SetName("Sections");
    Section.SetName("Section");
    TaggedInformation.SetName("TaggedInformation");
    KeyedInformation.SetName("KeyedInformation");
    TagList.SetName("TagList");
    TagInfo.SetName("TagInfo");
    NumberList.SetName("NumberList");
    KeyList.SetName("KeyList");
    KeyInfo.SetName("KeyInfo");
    TypeExpression.SetName("TypeExpression");
    ValueExpression.SetName("ValueExpression");
#endif
}

//-----------------------------------------------------------------
// ~GTIFFParser
// 
// Destructor.
//-----------------------------------------------------------------
GTIFFParser::~GTIFFParser()
{

}


//###############################
// NODES
//###############################


//-----------------------------------------------------------------
// IdentifierNode
// 
// Constructor
//-----------------------------------------------------------------
IdentifierNode::IdentifierNode(HPAGrammarObject* pi_pToken, 
                               const string& pi_rText,
                               const HPASourcePos& pi_rLeftPos,
                               const HPASourcePos& pi_rRightPos)
               :HPATokenNode(pi_pToken, pi_rText, pi_rLeftPos, pi_rRightPos)
{
    ctype<char> Converter;
    string Upper = pi_rText;

    Converter.toupper(Upper.begin(), Upper.end());

    // Look in the geotif tag map for the identifier
    // If found set it, else search the geotif tag value map
    // if found set it, else search the transfo model map, if
    // found set it, else throw an error
    GEOTIFFTAG_MAP::iterator Itr = g_GeoTiffTagMap.find(Upper);

    if( Itr != g_GeoTiffTagMap.end() )
    {
        m_Type    = LONG_TYPE;
        u.m_Long  = (*Itr).second;
    }
    else
    {
        GEOTIFFTAGVALUE_MAP::iterator Itr2 = g_GeoTiffTagValueMap.find(Upper);
        
        if( Itr2 != g_GeoTiffTagValueMap.end() )
        {
            m_Type    = SHORT_TYPE;
            u.m_Short = (*Itr2).second;
        }
        else
        {
            GEOTIFFTAGTRANSFO_MAP::iterator Itr3 = g_GeoTiffTagTransfoMap.find(Upper);
        
            if( Itr3 != g_GeoTiffTagTransfoMap.end() )
            {
                m_Type    = LONG_TYPE;
                u.m_Long  = (*Itr3).second;
            }
            else
            {
                //HPAException Err(this);
                //throw Err;
                m_Type = ERROR_TYPE;
            }
        }
    }
}

//-----------------------------------------------------------------
// TagInfoNode
// 
// Constructor
//-----------------------------------------------------------------
TagInfoNode::TagInfoNode(HPAGrammarObject* pi_pObj, 
                         const HPANodeList& pi_rList,
                         HPAParser* pi_pParser)
   : HPANode(pi_pObj, pi_rList, pi_pParser)
{

    int32_t Ctr = 0;
    int32_t Cx;
    int32_t Cy;
    double* pMatrix = 0;

    // Get the matrix size
    Cx = (int32_t )((HFCPtr<HPANumberTokenNode>&)pi_rList[2])->GetValue();
    Cy = (int32_t )((HFCPtr<HPANumberTokenNode>&)pi_rList[4])->GetValue();

    // Allocate the matrix
    pMatrix = new double[Cx * Cy];

    // Get the identifier
    TIFFTag TiffTag = (TIFFTag)((HFCPtr<IdentifierNode>&)pi_rList[0])->GetLong();

    // Iterate each node to build the matrix
    HPANode* pNode = pi_rList[7];
    
    while (pNode)
    {
        double Value;
        
        if (pNode->GetSubNodes().front()->GetSubNodes().size() == 2)
            Value = -((HFCPtr<HPANumberTokenNode>&)(pNode->GetSubNodes().front()->GetSubNodes()[1]))->GetValue();
        else
            Value = ((HFCPtr<HPANumberTokenNode>&)(pNode->GetSubNodes().front()->GetSubNodes().front()))->GetValue();

        pMatrix[Ctr++] = Value;

        // looping to next value
        if (pNode->GetSubNodes().size() == 2)
            pNode = pNode->GetSubNodes()[1];
        else
            pNode = 0;
    }

    // Try to set the matrix
    if( !((GTIFFParser*)pi_pParser)->m_rTiffFile.SetField(TiffTag, (int32_t)(Cx * Cy), pMatrix) )
        printf("\n\rError: Unable to set the tifftag %d", TiffTag);

    delete []pMatrix;
}

//-----------------------------------------------------------------
// KeyInfoNode
// 
// Constructor
//-----------------------------------------------------------------
KeyInfoNode::KeyInfoNode(HPAGrammarObject* pi_pObj, 
                         const HPANodeList& pi_rList,
                         HPAParser* pi_pParser)
   : HPANode(pi_pObj, pi_rList, pi_pParser)
{
    // Get the tag
    TIFFGeoKey KeyValue = (TIFFGeoKey)((HFCPtr<IdentifierNode>&)pi_rList[0])->GetLong();

    // Find the tag type
    HFCPtr<HPANode> pNode = pi_rList[7]->GetSubNodes().front();

    HPAGrammarObject* pObj = pNode->GetGrammarObject();
    
    if (pObj == &((GTIFFParser*)pi_pParser)->String_tk)
    {
        string Value = ((HFCPtr<HPATokenNode>&)pNode)->GetText();

        if( !((GTIFFParser*)pi_pParser)->m_rGeoKey.SetValues(KeyValue, Value.c_str()) )
            printf("\r\nError: Unable to set the tifftag %d with %s", KeyValue, Value.c_str());
    }
    else if (pObj == &((GTIFFParser*)pi_pParser)->NumericExpression)
    {
        double Value;

        if (pNode->GetSubNodes().size() == 2)
            Value = -((HFCPtr<HPANumberTokenNode>&)(pNode->GetSubNodes()[1]))->GetValue();
        else
            Value = ((HFCPtr<HPANumberTokenNode>&)(pNode->GetSubNodes().front()))->GetValue();

        if( !((GTIFFParser*)pi_pParser)->m_rGeoKey.SetValue(KeyValue, Value) )
            printf("\r\nError: Unable to set the tifftag %d with %d", KeyValue, Value);

    }
    else if (pObj == &((GTIFFParser*)pi_pParser)->Identifier_tk)
    {
        unsigned short Value = ((HFCPtr<IdentifierNode>&)pNode)->GetShort();
        
        if( !((GTIFFParser*)pi_pParser)->m_rGeoKey.SetValue(KeyValue, Value) )
            printf("\r\nError: Unable to set the tifftag %d with %d", KeyValue, Value);
    }
    else
    {
        HPAException Err(this);
        throw Err;
    }
}

#endif
