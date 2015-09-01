//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSParserScope.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HPSParserScope
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
inline HPSParserScope::HPSParserScope(HPSParser* pi_pParser)
    : m_pOwner(0), m_pParser(pi_pParser)
    {
    // Nothing to do
    }

//---------------------------------------------------------------------------
inline HPSParserScope::~HPSParserScope()
    {
    }

//---------------------------------------------------------------------------
inline void HPSParserScope::SetSession(const HFCPtr<HPSSession>& pi_pSession)
    {
    m_pSession = pi_pSession;
    }

//---------------------------------------------------------------------------
inline void HPSParserScope::SetOwner(HPSParserScope* pi_pScope)
    {
    m_pOwner = pi_pScope;
    }

//---------------------------------------------------------------------------
inline HPSParserScope* HPSParserScope::GetOwner() const
    {
    return m_pOwner;
    }

//---------------------------------------------------------------------------
inline bool HPSParserScope::IsTopScope() const
    {
    return m_pOwner == 0;
    }

//---------------------------------------------------------------------------
inline void HPSParserScope::Reset()
    {
    VariableList::iterator itr = m_VariableList.begin();
    while (itr != m_VariableList.end())
        {
        (*itr).second->Reset();
        ++itr;
        }

    }

//---------------------------------------------------------------------------
inline void HPSParserScope::SetParameterValue(short pi_Pos, ExpressionNode* pi_pNode)
    {
    m_ParameterList[pi_Pos]->ResetExpression(pi_pNode);
    }

//---------------------------------------------------------------------------
inline size_t HPSParserScope::GetParameterCount() const
    {
    return m_ParameterList.size();
    }
