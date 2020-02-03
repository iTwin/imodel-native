/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/ExternalTools.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/ExternalTools.h,v 1.1 2010/06/02 13:16:45 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Class : ExternalTool, ExternalTools
//-----------------------------------------------------------------------------
// Defines a container for external commands loaded from a config file.
//-----------------------------------------------------------------------------

#pragma once

namespace ImageHunter
{
    //-----------------------------------------------------------------------------
    // ExternalTool class
    //-----------------------------------------------------------------------------
    ref class ExternalTool
    {
    public:
        ExternalTool(int id, System::String^ executable, System::String^ name, System::String^ arguments);

        System::String^ GetExecutable();
        System::String^ GetArguments();
        System::String^ GetName();
        int             GetID();

        void            SetExecutable(System::String^ exe);
        void            SetArguments(System::String^ args);
        void            SetName(System::String^ name);
        void            SetID(int id);

    private:
        int             m_id;
        System::String^ m_executable;
        System::String^ m_name;
        System::String^ m_arguments;
    };

    //-----------------------------------------------------------------------------
    // ExternalTools class
    //-----------------------------------------------------------------------------
    ref class ExternalTools
    {
    public:
        static ExternalTools^   GetInstance();

        int                                 AddTool(int id, System::String^ executable, System::String^ name, System::String^ arguments);

        int                                 AddTool(ExternalTool^ tool);
        ExternalTool^                       RemoveTool(int index);
        void                                RemoveTool(ExternalTool^ tool);
        ExternalTool^                       GetExternalTool(int index);
        System::Collections::SortedList^    GetExternalTools();
        void                                SaveActualState();
        bool                                RestoreFromSavedState();
        bool                                DropSavedState();

    private:
        ExternalTools();

        static ExternalTools^               m_instance;
        System::Collections::SortedList^    m_tools;
        System::Collections::SortedList^    m_savedTools;
    };

}