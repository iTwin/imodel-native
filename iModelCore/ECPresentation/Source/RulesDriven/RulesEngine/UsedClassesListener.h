/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/UsedClassesListener.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct IUsedClassesListener
    {
    virtual ~IUsedClassesListener() {}
    virtual void _OnClassUsed(ECClassCR, bool polymorphically) = 0;
    };

/*=============================================================================**//**
* @bsiclass                                     Grigas.Petraitis            07/2016
+===============+===============+===============+===============+===============+==*/
struct ECDbUsedClassesListenerWrapper : IUsedClassesListener
    {
    IConnectionCR m_connection;
    IECDbUsedClassesListener& m_wrappedListener;
    ECDbUsedClassesListenerWrapper(IConnectionCR connection, IECDbUsedClassesListener& wrappedListener)
        : m_connection(connection), m_wrappedListener(wrappedListener)
        {}
    void _OnClassUsed(ECClassCR ecClass, bool polymorphically) override {m_wrappedListener.NotifyClassUsed(m_connection.GetECDb(), ecClass, polymorphically);}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct UsedClassesHelper
{
private:
    UsedClassesHelper() {}
public:
    static void NotifyListenerWithUsedClasses(IUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IUsedClassesListener&, ECSchemaHelper const&, ECExpressionsCache&, PresentationRuleSetCR);
    static void NotifyListenerWithUsedClasses(IECDbUsedClassesListener&, ECExpressionsCache&, IConnectionCR, Utf8StringCR ecexpression);
    static void NotifyListenerWithRulesetClasses(IECDbUsedClassesListener&, ECExpressionsCache&, IConnectionCR, PresentationRuleSetCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
