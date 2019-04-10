/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handlerNET/DTMElementManager.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

ref class DTMElement;

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/11
//=======================================================================================		
public ref class DTMElementManager
    {
    public:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static DTMElement^ FromElemHandle (ElementHandleCR elem);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static DTMElement^ FromEditElemHandle (System::IntPtr elemHandle);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static DTMElement^ FromElement (Bentley::Internal::MicroStation::Elements::Element^ element);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static bool IsDTMElement (ElementHandleCR elem);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static bool IsDTMElement (Bentley::Internal::MicroStation::Elements::Element^ element);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static void ScheduleFromDtm (EditElementHandleR editHandlePtr, BCDTM::DTM^ dtm, bool disposeDTM);

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 06/11
        //=======================================================================================		
        static DTMElement^ FromDtm (BCDTM::DTM^ dtm);        

        static System::Collections::Generic::IEnumerable<DTMElement^>^ GetEnumerator(Bentley::Internal::MicroStation::ModelReference^ modelRef);

    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
