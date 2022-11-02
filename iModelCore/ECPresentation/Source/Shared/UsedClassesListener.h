/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECInstanceChangeEvents.h>
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IUsedClassesListener
    {
    virtual ~IUsedClassesListener() {}
    virtual void _OnClassUsed(ECClassCR, bool polymorphically) = 0;
    };

/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
struct ECDbUsedClassesListenerWrapper : IUsedClassesListener
    {
    IConnectionCR m_connection;
    IECDbUsedClassesListener& m_wrappedListener;
    ECDbUsedClassesListenerWrapper(IConnectionCR connection, IECDbUsedClassesListener& wrappedListener)
        : m_connection(connection), m_wrappedListener(wrappedListener)
        {}
    void _OnClassUsed(ECClassCR ecClass, bool polymorphically) override {m_wrappedListener.NotifyClassUsed(m_connection.GetECDb(), ecClass, polymorphically);}
    IECDbUsedClassesListener& GetWrappedListener() {return m_wrappedListener;}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UsedClassesHelper
{
private:
    UsedClassesHelper() {}
public:
    static void NotifyListenerWithUsedClasses(IUsedClassesListener&, ECSchemaHelper const&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IUsedClassesListener&, ECSchemaHelper const&, IRulesPreprocessorR);
    static void NotifyListenerWithUsedClasses(IECDbUsedClassesListener&, ECExpressionsCache&, IConnectionCR, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IECDbUsedClassesListener&, ECExpressionsCache&, IConnectionCR, IRulesPreprocessorR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
