/*--------------------------------------------------------------------------------------+
|
|     $Source: src/LeakDetector.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <boost/foreach.hpp>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <class OTYPE> struct LeakDetector : ILeakDetector
{
private:
typedef std::map<OTYPE const*, Int32> ObjectsMap;

Int32       m_totalAllocs;
Int32       m_totalFrees;
Int32       m_currentLive;
WString     m_singularTypeName;
WString     m_pluralTypeName;
bool        m_buildMap;
ObjectsMap  m_objectsMap;

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void        AddToMap (OTYPE const& object, Int32 index)
    {
    if (m_buildMap)
        m_objectsMap[&object] = index; // record this so we know if it was the 1st, 2nd allocation
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void        RemoveFromMap (OTYPE const& object)
    {
    if (m_buildMap)
        m_objectsMap.erase(&object);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
LeakDetector (WCharCP singular, WCharCP plural, bool buildMap)
    :
    m_singularTypeName (singular), m_pluralTypeName (plural), m_buildMap (buildMap),
    m_totalAllocs(0), m_totalFrees(0), m_currentLive(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     CaroleMacDonald  03/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~LeakDetector()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void        ObjectCreated(OTYPE const& object)   { m_totalAllocs++; m_currentLive++; AddToMap(object, m_totalAllocs); }
void        ObjectDestroyed(OTYPE const& object) { m_totalFrees++;  m_currentLive--; RemoveFromMap (object); };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    GetStats(Int32& currentLive, Int32& totalAllocs, Int32& totalFrees) const override
    {
    currentLive = m_currentLive;
    totalAllocs = m_totalAllocs;
    totalFrees  = m_totalFrees;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    ResetStats() override
    {
    m_currentLive = m_totalAllocs = m_totalFrees = 0;

    if (m_buildMap)
        m_objectsMap.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    ReportStats (WCharCP prefix) const override
    {
    if (!prefix)
        prefix = L"";

    ECObjectsLogger::Log()->debugv (L"%ls Live %ls: %d, Total Allocs: %d, TotalFrees: %d", prefix, m_pluralTypeName.c_str(), m_currentLive, m_totalAllocs, m_totalFrees);

    CheckForLeaks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Int32   CheckForLeaks () const override
    {
    if ( ! m_buildMap)
        return m_currentLive;

    FOR_EACH (typename ObjectsMap::value_type leak, m_objectsMap)
        {
        OTYPE const*    leakedObject = leak.first;
        UInt32          orderOfAllocation = leak.second;
        
        WString name = leakedObject->GetName();
        
        ECObjectsLogger::Log()->errorv (L"Leaked the %dth %ls that was allocated: %ls", orderOfAllocation, m_singularTypeName.c_str(), name.c_str());
        }

    return m_currentLive;
    }

};

END_BENTLEY_EC_NAMESPACE
