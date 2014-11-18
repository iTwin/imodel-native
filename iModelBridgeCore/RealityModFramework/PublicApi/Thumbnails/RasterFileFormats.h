/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Thumbnails/RasterFileFormats.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

class RasterFileFormats
    {
    HFC_DECLARE_SINGLETON_DLL(THUMBNAILS_EXPORT, RasterFileFormats)
    public:
        typedef map<WString,WString> SupportedExtensionListType;

        THUMBNAILS_EXPORT RasterFileFormats(void);
        THUMBNAILS_EXPORT ~RasterFileFormats(void);

        THUMBNAILS_EXPORT bool IsExtensionSupported(WCharCP extension);
        THUMBNAILS_EXPORT void OutputSupportedExtensions();

        THUMBNAILS_EXPORT const SupportedExtensionListType& GetSupportedExtensions() const { return m_supportedExtension; }

    private:
        SupportedExtensionListType m_supportedExtension;
    };

