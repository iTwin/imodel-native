/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ViewDisplayRules.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECObjects/ECExpressions.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ElementSymbologyChildren;
typedef RefCountedPtr<struct SymbologyRules> SymbologyRulesPtr;
typedef RefCountedPtr<struct ElementSymbologyExpressionContext> ElementSymbologyExpressionContextPtr;

//=======================================================================================
//! 
//! 
// @bsiclass
//=======================================================================================
struct ElementSymbologyExpressionContext : ECN::SymbolExpressionContext
    {
private:
    ElementSymbologyChildren* m_children;
    ElementSymbologyExpressionContext ();
    
public:
    ~ElementSymbologyExpressionContext();
    ViewContextP    SyncContext(ViewContextP context);
    DgnModelCP      SyncModel(DgnModelCP dgnModel);
    ElementHandleCP SyncElement(ElementHandleCP eh);
    static ElementSymbologyExpressionContext* Get ();
    void SetContext(ViewContextR context);

    ViewContextP    GetContext() const;
    DgnModelCP      GetModel() const;
    ElementHandleCP GetElement() const;
    };

//=======================================================================================
//! 
//! 
// @bsiclass
//=======================================================================================
struct ExpressionContextElementSync
    {
private:
    ElementSymbologyExpressionContext&m_expressionContext;
    ViewContextP    m_vc;
    ElementHandleCP m_eh;
public:
    ExpressionContextElementSync(ElementSymbologyExpressionContext&expressionContext, ViewContextR viewContext, ElementHandleCR eh) : m_expressionContext(expressionContext)
        {
        m_vc = expressionContext.SyncContext(&viewContext);
        m_eh = expressionContext.SyncElement(&eh);
        }

    ~ExpressionContextElementSync()
        {
        m_expressionContext.SyncContext(m_vc);
        m_expressionContext.SyncElement(m_eh);
        }
    };

//=======================================================================================
//! 
//! 
// @bsiclass
//=======================================================================================
struct SymbologyRules : RefCountedBase
    {
protected:
    virtual void _PreprocessRules(ECN::ExpressionContextR context) = 0;
    virtual void _ParseRules() = 0;
    virtual void _Evaluate(ECN::ExpressionContextR context) = 0;

public:
    static SymbologyRulesPtr Create();
    void PreprocessRules(ECN::ExpressionContextR context);
    void ParseRules();
    void Evaluate(ECN::ExpressionContextR context);
    };


END_BENTLEY_DGNPLATFORM_NAMESPACE
