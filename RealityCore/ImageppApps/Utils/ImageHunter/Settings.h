/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/Settings.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/Settings.h,v 1.1 2010/06/02 13:16:46 Jean.Lalande Exp $
//-----------------------------------------------------------------------------------
// Class : ExternalToolElement, ExternalToolsCollection, ExternalToolsSection
//-----------------------------------------------------------------------------------
// Defines all required classes to read the ExternalTools section in the config file.
//-----------------------------------------------------------------------------------

#pragma once

namespace ImageHunter 
{

    //-----------------------------------------------------------------------------
    // ExternalToolElement class
    //-----------------------------------------------------------------------------
    ref class ExternalToolElement : System::Configuration::ConfigurationElement
    {
    public:
        [System::Configuration::ConfigurationProperty("name", IsKey = true, IsRequired = true)]
        property System::String^ Name
        {
            System::String^ get() { return (System::String^)this["name"]; }
            void set(System::String^ value) { this["name"] = value; }
        }

        [System::Configuration::ConfigurationProperty("id")]
        property int ID
        {
            int get() { return (int)this["id"]; }
            void set(int id) { this["id"] = id; }
        }

        [System::Configuration::ConfigurationProperty("exe")]
        property System::String^ Executable
        {
            System::String^ get() { return (System::String^)this["exe"]; }
            void set(System::String^ value) { this["exe"] = value; }
        }

        [System::Configuration::ConfigurationProperty("args")]
        property System::String^ Arguments
        {
            System::String^ get() { return (System::String^)this["args"]; }
            void set(System::String^ value) { this["args"] = value; }
        }
    };

    //-----------------------------------------------------------------------------
    // ExternalToolsCollection class
    //-----------------------------------------------------------------------------
    ref class ExternalToolsCollection : System::Configuration::ConfigurationElementCollection
    {
    protected:
        virtual ConfigurationElement^ CreateNewElement() override
        {
            return gcnew ExternalToolElement();
        }

        virtual Object^ GetElementKey(ConfigurationElement^ tool) override
        {
            return ((ExternalToolElement^) tool)->ID;
        }

    public:
        void Add(ExternalToolElement^ tool)
        {
            BaseAdd(tool);
        }

        void Remove(ExternalToolElement^ tool)
        {
            BaseRemove(tool);
        }

        void RemoveAt(int index)
        {
            BaseRemoveAt(index);
        }

        void Clear()
        {
            BaseClear();
        }
    };

    //-----------------------------------------------------------------------------
    // ExternalToolsSection class
    //-----------------------------------------------------------------------------
    ref class ExternalToolsSection : System::Configuration::ConfigurationSection
    {
    public:
        [System::Configuration::ConfigurationProperty("Tools")]
        property ExternalToolsCollection^ Tools
        {
            ExternalToolsCollection^ get() { return (ExternalToolsCollection^)this["Tools"]; }
        }
    };

}