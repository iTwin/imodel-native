/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/DTMAddIn.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

using namespace System;

#define AddInID_Options   L"TMCommands"

BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE

[Bentley::MstnPlatformNET::AddInAttribute (MdlTaskID = ADDIN_NAME)]
public ref class AddIn : public Bentley::MstnPlatformNET::AddIn
    {
    private: static AddIn^ s_app;
    private: System::IntPtr m_mdlDesc;
    private: AddIn (System::IntPtr mdlDescIn);
    protected: virtual int Run (array<System::String^>^ commandLine) override;

    public: static AddIn^ GetInstance()
            {
                return s_app;
            }

    public: static void AnnotateContours (WCharCP unparsed);
    public: static void AnnotateSpots (WCharCP unparsed);
    public: static void ImportLandXML (WCharCP unparsed);
    public: static void ImportDTM (WCharCP unparsed);
    };

END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE
