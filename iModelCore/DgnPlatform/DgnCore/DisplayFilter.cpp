/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayFilter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      07/2012
+===============+===============+===============+===============+===============+======*/
struct BentleyApi::Dgn::DisplayFilterState
{
private:
    DisplayFilterHandlerId      m_handlerId;
    bvector<Byte>               m_data;
    bool                        m_state;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterState (DisplayFilterHandlerId handlerId, void const* data, size_t dataSize, bool state) : m_handlerId (handlerId), m_data (dataSize), m_state (state) 
    {
    memcpy (&m_data[0], data, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    Matches (ViewContextR viewContext, ElementHandleCP element) const 
    {
    DisplayFilterHandlerP   handler;

    if (NULL == (handler = DisplayFilterHandlerManager::GetManager().GetHandler (m_handlerId)))
        {
        //BeAssert (false);
        return true;
        }

#if defined (NEEDS_WORK_DGNITEM)
    return  handler->DoConditionalDraw (viewContext, element, &m_data[0], m_data.size()) == m_state;
#endif
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (DisplayFilterState const& rhs)
    {
    if (m_handlerId != rhs.m_handlerId)
        return m_handlerId < rhs.m_handlerId;

    if (m_state != rhs.m_state)
        return m_state < rhs.m_state;

    return m_data < rhs.m_data;
    }

};  // DisplayFilterState

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterKey::~DisplayFilterKey()
    {
    for (T_States::iterator curr = m_states.begin(); curr != m_states.end(); curr++)
        delete *curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayFilterKey::PushState (DisplayFilterHandlerId filterId, void const* data, size_t dataSize, bool state) 
    {
    m_states.push_back (new DisplayFilterState (filterId, data, dataSize, state));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DisplayFilterKey::Matches (ViewContextR viewContext, ElementHandleCP element) const 
    {
    //  TopazMerge_NeedsWork -- this always fails in Graphite 04 and 05 causing us to recreate QvElems, caching
    //  every one that is created.
    for (T_States::const_iterator curr = m_states.begin(); curr != m_states.end(); curr++)
        if (!(*curr)->Matches (viewContext, element))
            return false;

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayFilterKey::operator < (DisplayFilterKey const& rhs)
    {
    if (m_states.size() != rhs.m_states.size())
        return m_states.size() < rhs.m_states.size();

    for (T_States::const_iterator curr0 = m_states.begin(), curr1 = rhs.m_states.begin(), end = m_states.end(); curr0 != end; curr0++, curr1++)
        {
        if (**curr0 < **curr1)
            return true;
        }
            
    return false;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      08/2012
+===============+===============+===============+===============+===============+======*/                                                                        
struct  BooleanOperator : DisplayFilter::Operator
{
protected:

    bvector <Byte>        m_data;

BooleanOperator (DisplayFilterHandlerId id, void const* pData, size_t dataSize) : Operator (id), m_data (dataSize + sizeof (uint16_t))
    {  
    uint16_t        opSize = (uint16_t) dataSize;

    memcpy (&m_data.front(), &opSize, sizeof (opSize));
    memcpy (&m_data[sizeof(opSize)], pData, dataSize); 
    }

virtual void    _GetData (bvector<Byte>& data) const override { data = m_data; }

public:
static  DisplayFilter::OperatorPtr    Create (DisplayFilterHandlerId id, void const* pData, size_t dataSize) { return new BooleanOperator (id, pData, dataSize); }
        
};  // BooleanOperator



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr     DisplayFilter::CreateViewFlagTest (ViewFlag viewFlag, bool testState) 
    { 
    ViewFlagFilterData       data (viewFlag, testState);

    return BooleanOperator::Create (DisplayFilterHandlerId_ViewFlag, &data, sizeof (data)); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilter::OperatorPtr     DisplayFilter::CreateRenderModeTest (DgnRenderMode renderMode, TestMode testMode)
    { 
    ViewParameterFilterData       data (static_cast<uint32_t>(renderMode), testMode, ViewParameterFilterData::Parameter_RenderMode);

    return BooleanOperator::Create (DisplayFilterHandlerId_Parameter, &data, sizeof (data)); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayFilter::If (ViewContextR viewContext, ElementHandleCP element, DisplayFilter::Operator const& filterOperator)
    {
    bvector<Byte>   data;

    filterOperator.GetData (data);

    return viewContext.IfConditionalDraw (filterOperator.GetHandlerId (), element, &data[0], data.size()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayFilter::ElseIf (ViewContextR viewContext, ElementHandleCP element, DisplayFilter::Operator const& filterOperator)
    {
    bvector<Byte>   data;
                                                                                     
    filterOperator.GetData (data);

    return viewContext.ElseIfConditionalDraw (filterOperator.GetHandlerId (), element, &data[0], data.size()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayFilter::Else (ViewContextR viewContext) { return viewContext.ElseConditionalDraw(); }
void DisplayFilter::End (ViewContextR viewContext) { viewContext.EndConditionalDraw(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static DisplayFilterHandlerManager       s_displayFilterHandlerManager;

DisplayFilterHandlerManagerR    DisplayFilterHandlerManager::GetManager () { return s_displayFilterHandlerManager; }
void                            DisplayFilterHandlerManager::RegisterHandler (DisplayFilterHandlerId id, DisplayFilterHandlerR handler)          { m_handlerMap[id] = &handler; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayFilterHandlerP           DisplayFilterHandlerManager::GetHandler (DisplayFilterHandlerId id)
    {
    T_DisplayFilterHandlerMap::iterator   found;
    return ((found = m_handlerMap.find (id)) == m_handlerMap.end()) ? NULL : found->second;
    }


#endif









                                                                                                                                                
