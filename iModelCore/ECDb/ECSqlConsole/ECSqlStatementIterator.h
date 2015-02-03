/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlStatementIterator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#pragma once
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjectsAPI.h>


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
struct IECSqlIteratorCallback
    {
public:
    enum class Event
        {
        Begin, End
        };
    virtual void OnPrimitive (BeSQLite::EC::ECSqlPropertyPath const& propertyPath, int rowId, ECN::ECValueCR value) = 0;
    virtual void OnRow (BeSQLite::EC::ECSqlPropertyPath const* propertyPath, int rowId, Event event, bool isNull) = 0;
    virtual void OnRows (BeSQLite::EC::ECSqlPropertyPath const* propertyPath, Event event) = 0;
    virtual void OnRest() {};
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
struct ECSqlStatementIterator
    {
private:
    static BeSQLite::EC::ECSqlStatus IterateCell (IECSqlIteratorCallback& callback, BeSQLite::EC::IECSqlValue const& ecsqlValue, int rowId);
    static BeSQLite::EC::ECSqlStatus IteratePrimitve (IECSqlIteratorCallback& callback, BeSQLite::EC::IECSqlValue const& value, int rowId, BeSQLite::EC::ECSqlPropertyPath const* propertyPath);

public:
    static BeSQLite::EC::ECSqlStatus Iterate (BeSQLite::EC::ECSqlStatement& statement, IECSqlIteratorCallback& callback);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
struct ListDataWriter : IECSqlIteratorCallback
    {
    size_t m_indent;
    Utf8String m_linePrefix;
    size_t m_indexTabSize;
public:
    virtual void OnRest() override;
    virtual void OnPrimitive (BeSQLite::EC::ECSqlPropertyPath const& propertyPath, int rowId, ECN::ECValueCR value) override;
    virtual void OnRow (BeSQLite::EC::ECSqlPropertyPath const* propertyPath, int rowId, Event event, bool isNull) override;
    virtual void OnRows (BeSQLite::EC::ECSqlPropertyPath const* propertyPath, Event event) override;
    void Indent();
    void Unindent();
    };

    