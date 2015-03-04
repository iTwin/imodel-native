/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Properties/RasterFileFormats.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

class RasterFileFormats
    {
    HFC_DECLARE_SINGLETON_DLL(PROPERTIES_EXPORT, RasterFileFormats)
    public:
        typedef map<WString,WString> SupportedExtensionListType;

        PROPERTIES_EXPORT RasterFileFormats(void);
        PROPERTIES_EXPORT ~RasterFileFormats(void);

        PROPERTIES_EXPORT bool IsExtensionSupported(WCharCP extension);
        PROPERTIES_EXPORT void OutputSupportedExtensions();

        PROPERTIES_EXPORT const SupportedExtensionListType& GetSupportedExtensions() const { return m_supportedExtension; }

    private:
        SupportedExtensionListType m_supportedExtension;
    };

