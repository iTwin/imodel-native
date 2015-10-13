/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/CfgVarExpr.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include    <DgnPlatform/DesktopTools/CfgVarExpr.h>
#include    <boost/spirit/include/classic_core.hpp>
#include    <boost/spirit/include/classic_ast.hpp>
#include    <boost/spirit/include/classic_tree_to_xml.hpp>
#include    <boost/spirit/include/classic_push_back_actor.hpp>
#include    <boost/spirit/include/classic_clear_actor.hpp>
#include    <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include    <algorithm>
#include    <iterator>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_BENTLEY_NAMESPACE

using namespace std;
using namespace boost::spirit::classic;

struct CfgVarExpressionParser;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool CfgVarExpression::Node::Evaluate (CfgVarExpression::Result &result) const
    {
    if (NULL != m_right.get ())
        return m_right->Evaluate (result);

    return true;
    }


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PathSplitter : public grammar<PathSplitter>
    {
    T_WStringVector _paths;

    template <typename ScannerT>
    struct definition
        {
        rule<ScannerT>  _character, _statement, _statements;


        definition (const PathSplitter &splitter)
            {
            PathSplitter &splitterRef = const_cast<PathSplitter&> (splitter);

            _character  = anychar_p - ch_p (';');
            _statement  = (*_character)[push_back_a (splitterRef._paths)];
            _statements = _statement >> *(ch_p (';') >> _statement);
            }

        rule<ScannerT>  start () const  { return _statements; }
        };
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CfgVarExpression::Cfg::Evaluate (CfgVarExpression::Result& result) const
    {
    Result      rightResult;
    if (!Node::Evaluate (rightResult))
        return false;

    // Right side of configuration variable cannot have more than one value.
    if (rightResult.size () > 1)
        return false;

    WString     macroValue;
    if (SUCCESS != ConfigurationManager::GetVariable (macroValue, m_value.c_str()))
        return true;

    if (macroValue.empty())
        return true; // It is okay to have null variable.

    PathSplitter        splitter;
    parse_info<wchar_t const *> info = parse (macroValue.c_str(), splitter, nothing_p);

    if (!info.full)
        return false;

    T_WStringVector::iterator it = splitter._paths.begin ();
    for (; it != splitter._paths.end (); ++it)
        {
        // Ignore empty expansions
        if (it->empty ())
            continue;

        WString value (it->c_str ());
        if (!rightResult.empty ())
            value.append (rightResult[0]);

        result.push_back (value);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
CfgVarExpression::NodePtr CfgVarExpression::CreateCfgVarExpr (WCharCP value)
    {
    CfgVarExpression::NodePtr result;
    
    CfgVarExpression::Cfg::Create(result, value);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
CfgVarExpression::NodePtr CfgVarExpression::CreateGeneralExpr (WCharCP value)
    {
    CfgVarExpression::NodePtr result;
    
    CfgVarExpression::General::Create(result, value);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool CfgVarExpression::General::Evaluate (CfgVarExpression::Result &result) const
    {
    if (!Node::Evaluate (result))
        return false;

    // General expression simply evaluates to its own value.
    if (result.empty ())
        {
        result.push_back (m_value);
        return true;
        }

    Result::reverse_iterator top = result.rbegin ();
    *top = m_value + *top;

    return true;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
CfgVarExpression::Statement::Statement (CfgVarExpression::Node& node)
    : m_node (&node)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Statement::Push (CfgVarExpression::Node& node)
    {
    NodePtr lastNode = NULL;
    NodePtr leftNode = m_node;
    while (NULL != leftNode.get ())
        {
        lastNode = leftNode;
        leftNode = leftNode->GetRightExpression ();
        }

    if (NULL == lastNode.get ())
        m_node = &node;
    else
        lastNode->SetRightExpression (node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool CfgVarExpression::Statement::Evaluate (CfgVarExpression::Result &result) const
    {
    if (NULL == m_node.get ())
        return true;

    return m_node->Evaluate (result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
CfgVarExpression::Statement const& CfgVarExpression::Statements::GetStatement (int index) const
    {
    return m_statements[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Statements::PushNewStatement ()
    {
    m_statements.push_back (Statement());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Statements::Push (CfgVarExpression::Node& node)
    {
    if (m_statements.empty ())
        PushNewStatement ();

    m_statements.rbegin ()->Push (node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool CfgVarExpression::Statements::Evaluate (CfgVarExpression::Result &result) const
    {
    bool evalResult = true;

    Collection::const_iterator siterator = m_statements.begin ();
    for (; siterator != m_statements.end (); ++siterator)
        {
        evalResult = siterator->Evaluate (result);
        if (!evalResult)
            break;
        }

    return evalResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Statements::Clear ()
    {
    m_statements.clear ();
    }


/*=================================================================================**//**
* Action defined to push Configuration Variable exprssion
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CfgNodePushBackAction
    {
    typedef ref_value_actor <CfgVarExpression *, CfgNodePushBackAction> Actor;


    template <typename ValueT>
    void act (CfgVarExpression *expression, ValueT const &value) const
        {
        BeAssert (false);
        }

    template <typename IteratorT>
    void act (CfgVarExpression *&expression, IteratorT const &first, IteratorT const &last) const
        {
        bwstring exp;
        copy ( first, last, back_insert_iterator<bwstring> (exp));

        CfgVarExpression::NodePtr cfgNode = expression->CreateCfgVarExpr (exp.c_str());
        expression->Push (*cfgNode);
        }

    static Actor Do (CfgVarExpression *&expression)
        {
        return Actor (expression);
        }
    };


/*=================================================================================**//**
* Action defined to push general expression.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GeneralNodePushBackAction
    {
    typedef ref_value_actor <CfgVarExpression *, GeneralNodePushBackAction> Actor;

    template <typename ValueT>
    void act (CfgVarExpression *&expression, ValueT const &value) const
        {
        bwstring exp;
        exp.push_back (value);

        CfgVarExpression::NodePtr generalNode = expression->CreateGeneralExpr (exp.c_str());
        expression->Push (generalNode);
        }

    template <typename IteratorT>
    void act (CfgVarExpression *&expression, IteratorT const &first, IteratorT const &last) const
        {
        bwstring exp;
        copy (first, last, back_insert_iterator<bwstring> (exp));
        CfgVarExpression::NodePtr generalNode = expression->CreateGeneralExpr (exp.c_str());
        expression->Push (*generalNode);
        }

    static Actor Do (CfgVarExpression *&expression)
        {
        return Actor (expression);
        }
    };

/*=================================================================================**//**
* Boost::Spirit based parser for configuration variable expression
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CreateNewStatementAction
    {
    typedef ref_value_actor <CfgVarExpression *, CreateNewStatementAction> Actor;

    template <typename ValueT>
    void act (CfgVarExpression *&expression, ValueT const &) const
        {
        expression->CreateNewStatement ();
        }

    template <typename IteratorT>
    void act (CfgVarExpression *&expression, IteratorT const &, IteratorT const &) const
        {
        expression->CreateNewStatement ();
        }

    static Actor Do (CfgVarExpression *&expression)
        {
        return Actor (expression);
        }
    };


/*=================================================================================**//**
* Boost::Spirit based parser for configuration variable expression
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CfgVarExpressionParser : grammar<CfgVarExpressionParser>
    {

    CfgVarExpression &m_expression;

/*=================================================================================**//**
* Parser definition
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    template <typename ScannerT>
    struct definition
        {
        rule<ScannerT>      _cfg;
        rule<ScannerT>      _beta;
        rule<ScannerT>      _statement;
        rule<ScannerT>      _statements;
        rule<ScannerT>      _separator;
        CfgVarExpression *  _expression;


        definition (const CfgVarExpressionParser &parser)
            {
            _expression = const_cast<CfgVarExpression*>(&parser.m_expression);
            _separator  = ch_p (';');
            _cfg        = chseq_p ("$(") >> (+(print_p- chseq_p ("$(") -ch_p ('(') -ch_p (')')-ch_p (';')  ))[CfgNodePushBackAction::Do (_expression)] >> ch_p (')');
            _beta       = (print_p - _separator);
            _statement  = +_cfg >> *(_beta[GeneralNodePushBackAction::Do(_expression)] >> *(_cfg | _beta[GeneralNodePushBackAction::Do(_expression)]));
            _statements = eps_p[CreateNewStatementAction::Do (_expression)] >> _statement >> *( _separator[CreateNewStatementAction::Do (_expression)] >> _statement) >> !_separator;
            }

        rule<ScannerT> const &start () { return _statements; }
        };


    CfgVarExpressionParser (CfgVarExpression &expression)
        : m_expression (expression)
        {}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool CfgVarExpression::Parse (WCharCP expression)
    {
    parse_info<wchar_t const *> info;
    CfgVarExpressionParser parser (*this);
    info = parse (expression, parser, nothing_p);
    return info.full;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Clear ()
    {
    m_statements.Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::CreateNewStatement ()
    {
    m_statements.PushNewStatement ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    YogeshSajanikar    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CfgVarExpression::Push (CfgVarExpression::Node& node)
    {
    m_statements.Push (node);
    }

END_BENTLEY_NAMESPACE

