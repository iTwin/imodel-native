/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/Bentley.DTM.Commands.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#using <mscorlib.dll>
#using <System.dll>
#using <System.Xml.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
#using <System.Data.dll>

#using <bentley.GeometryNET.Structs.dll>
#using <Bentley.Exceptions.dll>
#using <Bentley.DgnPlatformNET.dll>
#using <bentley.UI.dll>

#using <bentley.microstation.interfaces.1.0.dll>
#using <ustation.dll>

#ifndef Intellisense
#using <Bentley.Microstation.dll>
#using <bentley.microstation.templates.dll>
#using <bentley.microstation.templates.support.dll>
#endif
//#using <bentley.microstation.winforms.controls.dll>

//=======================================================================================
// @bsiclass                                                   Steve.Jones 09/10
//=======================================================================================
private ref class StringLocalizer
    {
    private: System::Resources::ResourceManager^ _resourceManager;
    private: System::Globalization::CultureInfo^ _cultureInfo;
    private: static StringLocalizer^ m_stringLocalizer;

    public: property static StringLocalizer^ Instance
        {
        StringLocalizer^ get()
            {
            if (m_stringLocalizer == nullptr)
                {
                m_stringLocalizer = gcnew StringLocalizer();
                }
            return m_stringLocalizer;
            }
        }

    /// <summary>
    /// Initializes a new instance of the StringLocalizer class.
    /// </summary>
    /// <author>Andy.Farr</author>                              <date>01/2007</date>
    private: StringLocalizer ()
        {
        _resourceManager = gcnew System::Resources::ResourceManager("LocalizableStrings", System::Reflection::Assembly::GetExecutingAssembly());
        _cultureInfo = gcnew System::Globalization::CultureInfo (System::Globalization::CultureInfo::CurrentUICulture->Name);
        }
    /// <summary>
    /// Retrieves translated string for given Resource file ID
    /// </summary>
    /// <param name="resourceID"></param>
    /// <returns>The translated string.</returns>
    /// <remarks>If the string was not found the method return the resource id within brackets</remarks>
    public: System::String^ GetLocalizedString (System::String^ resourceID)
        {
        System::String^ cReturn = nullptr;

        if (_resourceManager != nullptr && _cultureInfo != nullptr)
            {
            cReturn = _resourceManager->GetString (resourceID, _cultureInfo);
            }

        if (cReturn == nullptr)
            {
            cReturn = "<- " + resourceID + " ->";
            }

        return cReturn;
        }

}; // End StringLocalizer class

#define GET_LOCALIZED(_STR_) StringLocalizer::Instance->GetLocalizedString ((_STR_))

#if defined (NDEBUG)
#   define TRACE __noop
#elif defined (DEBUG)
#   pragma warning (push)
#   pragma warning (disable : 4793)

inline void TRACE (System::String ^msg)
    {
    System::Diagnostics::Debug::WriteLine (msg);
    }

    inline void TRACE (char const formatBuf[], ...)
        {
        va_list     va;
        int         size;
        char        *buf;

        va_start (va, formatBuf);
        size = _vscprintf (formatBuf, va);
        va_end (va);
        BeAsset (size > 0);
        size += 10;
        buf = reinterpret_cast<char*>(_malloca (size * sizeof (char)));
        BeAsset (buf);
        va_start (va, formatBuf);
        vsprintf_s (buf, size, formatBuf, va);
        va_end (va);
        strcat (buf, "\n");
        OutputDebugStringA (buf);
        }

    inline void TRACE (wchar_t const formatBuf[], ...)
        {
        va_list     va;
        int         size;
        wchar_t     *buf;

        va_start (va, formatBuf);
        size = _vscwprintf (formatBuf, va);
        va_end (va);
        BeAsset (size > 0);
        size += 10;
        buf = reinterpret_cast<wchar_t*>(_malloca (size * sizeof (wchar_t)));
        BeAsset (buf);
        va_start (va, formatBuf);
        vswprintf_s (buf, size, formatBuf, va);
        va_end (va);
        wcscat (buf, L"\n");
        OutputDebugStringW (buf);
        }
#   pragma warning ( pop )
#else
#   define TRACE __noop
#endif
