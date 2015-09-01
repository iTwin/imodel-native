//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPANode.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HPANode
//---------------------------------------------------------------------------
// This class defines a node in the parsing tree that results from the parsing
// of a source file.  A node describes a step that was reached when analyzing
// the source, this step corresponding to a grammar object in the grammar
// definition.  That grammar object is said to be "resolved" to that node.
//
// Each node has references to nodes that describe the substeps that were
// obtained in order to get this new node, according to a rule described in
// the grammer, unless the node is a "leave" in the parse tree, corresponding
// to a found token in the source stream.
//
// This is the ancestor class.  The exact type of the node depends on the
// resolved grammar object, but a type of node may be used by many grammar
// objects.
//---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HPASession.h>

BEGIN_IMAGEPP_NAMESPACE
class HPANode;
class HPAParser;
typedef vector<HFCPtr<HPANode> > HPANodeList;
class HPAGrammarObject;


struct HPASourcePos
    {
    HFCPtr<HFCURL> m_pURL;
    uint32_t       m_Line;
    uint32_t       m_Column;
    };

class HPANode : public HFCShareableObject<HPANode>
    // descendants savent quel info aller chercher dans les feuilles.
    {
public:

    IMAGEPP_EXPORT              HPANode(HPAGrammarObject*         pi_pObj,
                                const HPANodeList&        pi_rList,
                                const HFCPtr<HPASession>& pi_pSession,
                                bool                     pi_GetSubNodeOwnership = true);
    IMAGEPP_EXPORT              HPANode(HPAGrammarObject* pi_pObj,
                                const HPASourcePos& pi_rStartPos,
                                const HPASourcePos& pi_rEndPos,
                                const HFCPtr<HPASession>& pi_pSession);
    IMAGEPP_EXPORT virtual      ~HPANode();
    IMAGEPP_EXPORT virtual void FreeValue();

    HPAGrammarObject*   GetGrammarObject() const;
    HFCPtr<HPASession>  GetSession() const;
    const HPANodeList&  GetSubNodes() const;
    HPANode*            GetOwner() const;
    const HPASourcePos& GetStartPos() const;
    const HPASourcePos& GetEndPos() const;
    void                DeleteSubNodes();

protected:

    void                SetOwner(HPANode* pi_pNode);

    HPANodeList         m_SubNodes;

private:

    // Disabled methods
    HPANode(const HPANode&);
    HPANode& operator=(const HPANode&);

    // Data members
    HPAGrammarObject*   m_pGrammarObject;
    HFCPtr<HPASession>  m_pSession;
    HPANode*            m_pOwner;
    HPASourcePos        m_StartPos;
    HPASourcePos        m_EndPos;

#ifdef __HMR_DEBUG_MEMBER
public:
    IMAGEPP_EXPORT virtual void        PrintState(int pi_Level = 0);
#endif
    };

// The kind of object that connects a grammar object with the kind of node
// that it creates when it is resolved.  A grammar object uses a "creator"
// that allocates a node of proper type when resolution occurs.
// If that kind of node stores specific information, it must obtain it
// by scanning the subnodes and their grammar objects.

struct HPANodeCreator
    {
    virtual HPANode* Create(HPAGrammarObject* pi_pObj,
                            const HPANodeList& pi_rList,
                            const HFCPtr<HPASession>& pi_pSession) = 0;
    };

END_IMAGEPP_NAMESPACE
#include "HPANode.hpp"

