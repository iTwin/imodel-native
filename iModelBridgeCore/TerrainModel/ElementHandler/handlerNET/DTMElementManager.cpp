/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handlerNET/DTMElementManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"

#include <TerrainModel\ElementHandler\DTMDataRef.h>
#include <TerrainModel/ElementHandler/DTMElementHandler.h>
#include <TerrainModel/ElementHandler/DTMElementSubHandler.h>
#include "DTMElement.h"
#include "DTMElementManager.h"
#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
bool DTMElementManager::IsDTMElement(ElementHandleCR elemHandle)
    {
    return DTMElement::IsDTMElement (elemHandle);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
bool DTMElementManager::IsDTMElement (Bentley::Internal::MicroStation::Elements::Element^ element)
    {
    return DTMElement::IsDTMElement (element);
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMElement^ DTMElementManager::FromElemHandle (ElementHandleCR elemHandle)
    {
    DTMElement^ ret = DTMElement::FromElemHandle (elemHandle);

    if (ret && ret->DTMDataReference.IsValid())
        return ret;

    if (ret)
        delete ret;

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMElement^ DTMElementManager::FromEditElemHandle (System::IntPtr elemHandle)
    {
    DTMElement^ ret = DTMElement::FromEditElemHandle(static_cast<EditElementHandleP>(elemHandle.ToPointer()));

    if (ret && ret->DTMDataReference.IsValid())
        return ret;

    if (ret)
        delete ret;

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
DTMElement^ DTMElementManager::FromElement (Bentley::Internal::MicroStation::Elements::Element^ element)
    {
    DTMElement^ ret = DTMElement::FromElement (element);

    if (ret && ret->DTMDataReference.IsValid())
        return ret;

    if (ret)
        delete ret;

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
void DTMElementManager::ScheduleFromDtm (EditElementHandleR elemHandle, BCDTM::DTM^ dtm, bool disposeDTM)
    {
    if (dtm == nullptr)
        throw gcnew System::ArgumentNullException("dtm");

    IBcDTM* cDTM = (IBcDTM*)dtm->ExternalHandle.ToPointer();
    if (DTMElementHandlerManager::ScheduleFromDtm (elemHandle, cDTM->GetIDTM(), disposeDTM) != SUCCESS)
        throw gcnew System::Exception ("Failed to create DTM probably out of memory");
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
DTMElement^ DTMElementManager::FromDtm (BCDTM::DTM^ dtm)
    {
    if (dtm == nullptr)
        throw gcnew System::ArgumentNullException("dtm");

    EditElementHandle eh;
    eh.SetModelRef (ACTIVEMODEL);

    IBcDTM* cDTM = (IBcDTM*)dtm->ExternalHandle.ToPointer();
    if (DTMElementHandlerManager::ScheduleFromDtm (eh, cDTM->GetIDTM(), false) != SUCCESS)
        throw gcnew System::Exception ("Failed to create DTM probably out of memory");

    eh.AddToModel (ACTIVEMODEL);
    DTMElement^ dtmelement = DTMElement::FromElemHandle (eh);
    return dtmelement;
    }

public ref class DTMElementIterator : public System::Collections::Generic::IEnumerator<DTMElement^>
    {
    private:
        DTMElementIterator* m_iterator;
    public:
        DTMElementIterator (Bentley::Internal::MicroStation::ModelReference^ modelRef)
            {
            m_iterator = new DTMElementIterator (modelRef->DgnModelRefPtr, true);
            }

        ~DTMElementIterator()
            {
            this->!DTMElementIterator();
            }

        !DTMElementIterator()
            {
            if (m_iterator)
                delete m_iterator;
            }


        virtual property DTMElement^ Current
            {
            virtual DTMElement^ get()
                {
                return DTMElement::FromElemHandle (m_iterator->Current());
                }
            }

    virtual property System::Object^ Current2
            {
            virtual System::Object^ get() = System::Collections::IEnumerator::Current::get
                {
                return DTMElement::FromElemHandle (m_iterator->Current());
                }
            }
    public:
        virtual bool MoveNext()
            {
            return m_iterator->MoveNext();
            }

        virtual void Reset()
            {
            m_iterator->Reset();
            }
    };


private ref class DTMElementElements : System::Collections::Generic::IEnumerable<DTMElement^>
    {
    private:
        Bentley::Internal::MicroStation::ModelReference^ m_modelRef;
    public:
        DTMElementElements (Bentley::Internal::MicroStation::ModelReference^ modelRef)
            {
            m_modelRef = modelRef;
            }
        virtual System::Collections::Generic::IEnumerator<DTMElement^>^ GetEnumerator() = System::Collections::Generic::IEnumerable<DTMElement^>::GetEnumerator
            {
            return gcnew DTMElementIterator (m_modelRef);
            }
        virtual System::Collections::IEnumerator^ System_Collections_IEnumerable_GetEnumerator() = System::Collections::IEnumerable::GetEnumerator
            {
            return gcnew DTMElementIterator (m_modelRef);
            }
    };

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
System::Collections::Generic::IEnumerable<DTMElement^>^ DTMElementManager::GetEnumerator (Bentley::Internal::MicroStation::ModelReference^ modelRef)
    {
    return gcnew DTMElementElements (modelRef);
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

