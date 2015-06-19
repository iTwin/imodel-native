/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/CfgVarExpr.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma     once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <vector>

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* Represents a configuration variable expression. A configuration variable
* expression recognizes following grammar:
*
* Configuration Variable        => CfgVar       := $([A-Za-z0-9]+)
* Statement Separator           => Sp           := ;
* General Character             => Gc           := Print Character - Sp
* Statement                     => St           := CfgVar+ (Gc (Gc|CfgVar)*)*
* Statements                    => Ss           := St ( Sp St)*
*
* One can also compose the expression using the 'Push' and 'Create*' functions.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  CfgVarExpression
    {

public:
    struct Node;
    typedef RefCountedPtr<Node>         NodePtr;
    typedef bvector<WString>            Result;

/*=================================================================================**//**
* Represents smallest recognizable part in the expression.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    struct Node : public RefCounted<IRefCounted>
        {
    protected:
        NodePtr m_right;
        WString m_value;

    public:
        explicit Node (WCharCP value) : m_value (value), m_right (NULL) {}
        NodePtr GetRightExpression () const             { return m_right; }
        void    SetRightExpression (NodePtr right)      { m_right = right; }
        virtual bool Evaluate (Result& result) const;
        const WString &GetValue () const { return m_value; }
        };

/*=================================================================================**//**
* Represents configuration variable node
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    struct Cfg : public Node
        {
    public:
        explicit Cfg (WCharCP value) : Node (value){}
        virtual bool Evaluate (Result &result) const override;
        static void Create (CfgVarExpression::NodePtr& ptr, WCharCP value)
            {
            ptr = new CfgVarExpression::Cfg (value);
            }
        };

/*=================================================================================**//**
* Represents general character node in the configuration variable expression.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    struct General : public Node
        {
    public:
        explicit General (WCharCP value) : Node (value) {}
        virtual bool Evaluate (Result &result) const override;
        static void Create (CfgVarExpression::NodePtr& ptr, WCharCP value)
            {
            ptr = new CfgVarExpression::General (value);
            }
        };

/*=================================================================================**//**
* Represents single independent unit for evaluation. Statements are separated by semicolon.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    struct Statement
        {
    private:
        NodePtr m_node;

    public:
        Statement() : m_node() {}
        explicit Statement (NodePtr exp);

        // Add the expression to the statement
        void Push (NodePtr exp);
        bool Evaluate (Result &result) const;

        NodePtr GetNode () const { return m_node; }
        };

/*=================================================================================**//**
* Represents collection of statements.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
    struct Statements
        {
    private:
        typedef bvector<Statement> Collection;
        Collection  m_statements;

    public:
        size_t GetCount () const {return m_statements.size ();}

        DESKTOP_TOOLS_EXPORT Statement const& GetStatement (int index) const;

        void PushNewStatement ();
        void Push (NodePtr expr);

        DESKTOP_TOOLS_EXPORT bool Evaluate (Result& result) const;
        void Clear ();
        };

private:
    Statements m_statements;

public:
    // Create a configuration variable expression from given string.
    DESKTOP_TOOLS_EXPORT NodePtr CreateCfgVarExpr (WCharCP cfgvar);

    // Create a general expression from given string
    DESKTOP_TOOLS_EXPORT NodePtr CreateGeneralExpr (WCharCP cfgvar);

    // Parse the new expression. Existing statements will be cleared.
    DESKTOP_TOOLS_EXPORT bool Parse (WCharCP expression);

    // Clear the expression
    DESKTOP_TOOLS_EXPORT void Clear ();

    DESKTOP_TOOLS_EXPORT void CreateNewStatement ();

    // Push the expression in the last created statement. If no statement
    // exists, a new statement is created.
    DESKTOP_TOOLS_EXPORT void Push (NodePtr node);

    // Get the parsed statements.
    Statements const& GetStatements () const   { return m_statements; }
    };

END_BENTLEY_NAMESPACE
