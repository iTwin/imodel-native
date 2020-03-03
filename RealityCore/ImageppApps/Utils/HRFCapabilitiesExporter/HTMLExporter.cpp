/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HRFCapabilitiesExporter/HTMLExporter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "HTMLExporter.h"
#include <Imagepp/all/h/HRFHMRFile.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HTMLExporter::HTMLExporter(void)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
HTMLExporter::~HTMLExporter(void)
{
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintGenericHeader(otstream& out)
{
    // Simple HTML header with basic CSS support
    out << _T("<html>\n");
    out << _T("\t<head>\n");
    out << _T("\t\t<title>Image++ Capabilities</title>\n");
    out << _T("\t\t<link rel='stylesheet' type='text/css' href='style.css' media='screen' />\n");
    out << _T("\t\t<link rel='stylesheet' type='text/css' href='print.css' media='print' />\n");
    out << _T("\t</head>\n");
    out << _T("\t<body>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintGenericFooter(otstream& out)
{
    out << _T("\t</body>\n");
    out << _T("</html>");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintHeader(otstream& out)
{
    out << _T("<div id='header'><h1>Image++ Capabilities</h1></div>\n");
    out << _T("<div id='documentBody'>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintTableOfContent(ImageFormatMap const& imageFormats, otstream& out, bool usingFrames = false)
{
    out << _T("<!-- Begin of ###HEADER### -->\n");
    out << _T("<div id='toc'>\n");
    out << _T("<h3>Image formats</h3>\n");
    out << _T("<ul>\n");
    
    // Adding links to table of content
    for (ImageFormatMap::const_iterator itr = imageFormats.begin(); itr != imageFormats.end(); ++itr)
    {
        out << _T("<li>");
        out << _T("<a href='");
        if (usingFrames)
            out << CONTENT_PATH.c_str() << _T("/") << itr->second.id << _T(".html");
        else
            out << _T("#") << itr->second.id;
        out << _T("' title='");
        out << itr->second.label.c_str()  << _T(" (Extensions: ") << itr->second.extensions.c_str() << _T(")'");
        if (usingFrames)
            out << _T(" target='content'");
        out << _T(">");
        out << itr->second.label.c_str() << _T("</a>");
        out << _T("</li>\n");
    }
    
    out << _T("</ul>\n");
    out << _T("</div>\n\n");
    out << _T("<!-- End of ###HEADER### -->\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintFooter(otstream& out)
{
    out << _T("<!-- Begin of ###FOOTER### -->\n");
    
    // Querying timezone if it is not set
    _tzset();

    // Current date
    WChar currentDate[9];
    _tstrdate(currentDate);
    
    // Current time
    WChar currentTime[9];
    _tstrtime(currentTime);
    
    out << _T("<div id='footer'>\n");
    out << _T("<div id='copyright'><p>All rights reserved © 2012 Bentley Systems, Incorporated</p></div>\n");
    out << _T("<div id='createdDate'><p>Report generated at ") << currentTime << _T(" on ") << currentDate << _T("</p></div>\n");
    out << _T("</div>\n");
    
    out << _T("<!-- End of ###FOOTER### -->\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::ConvertAccessModesToString(const BooleanMap& accessModes, WString& output)
{
    output.clear();
    
    // Looping through available access modes
    for (BooleanMap::const_iterator accessItr = accessModes.begin();
         accessItr != accessModes.end();
         ++accessItr)
    {
        if ((AccessModes) accessItr->second == true)
        {
            // A dash is added for each access mode except for the first one
            if (!output.empty())
                output += _T("-");
                
            // Letters are used to represent access modes
            switch (accessItr->first)
            {
            case Create:
                output += _T("C");
                break;
            case Read:
                output += _T("R");
                break;
            case Write:
                output += _T("W");
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::ConvertBlockObjectToString(const BlockObject& blockObject, WString& output)
{
    output += blockObject.label;
    
    // Only Strip & Tile block type can support min/max height and increment
    switch (blockObject.ID)
    {
    case HRFBlockType::STRIP:
    case HRFBlockType::TILE:
        if (!(blockObject.minimumHeight == 0 && blockObject.maximumHeight == INT_MAX))
        {
            output += _T(" (");
            // Minimum height
            WChar minHeight[256];
            _ltot(blockObject.minimumHeight, minHeight, 10);
            output += minHeight;
            
            output += _T("-");
            // Maximum height
            WChar maxHeight[256] = _T("INF");
            if (blockObject.maximumHeight != INT_MAX)
                _ltot(blockObject.maximumHeight, maxHeight, 10);
            output += maxHeight;
            
            output += _T("-");
            // Height increment
            WChar increment[256];
            _ltot(blockObject.heightIncrement, increment, 10);
            output += increment;
            
            output += _T(" )");
        }
        break;
    }
    
    if (blockObject.isUnlimitedResolution)
    {
        output += _T(", Progressive");
    }
    else
    if (blockObject.isMultiResolution)
    {
        output += _T(", Multi-Resolution");
    }

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::PrintPixelTypeCapabilities(otstream& out, ImageFormatMap::const_iterator& itr)
{
    // Table header
    out << _T("<div class='PixelType'>\n");
    out << _T("<h4>Pixel Type Capabilities</h4>\n");
    out << _T("<table>\n");
    out << _T("<thead>\n");
    out << _T("<tr>\n");
    out << _T("<th>Profile ID</th>\n");
    out << _T("<th>Color Space</th>\n");
    out << _T("<th>Block Type</th>\n");
    out << _T("<th>Compression</th>\n");
    out << _T("<th>Access Mode</th>\n");
    out << _T("</tr>\n");
    out << _T("</thead>\n");
    
    out << _T("<tbody>\n");
    
    // Loop through all supported color spaces
    for (ColorSpaceMap::const_iterator colItr = itr->second.supportedColorSpace.begin();
         colItr != itr->second.supportedColorSpace.end();
         ++colItr)
    {
        // Concatenating Access Modes
        WString accessModeColorSpace;
        ConvertAccessModesToString(colItr->second.accessMode, accessModeColorSpace);
        
        // Compression Codecs
        for (CodecMap::const_iterator compressionItr = colItr->second.supportedCompressions.begin();
             compressionItr != colItr->second.supportedCompressions.end();
             ++compressionItr)
        {
            // Concatenating Access Modes
            WString accessModeCodec;
            ConvertAccessModesToString(compressionItr->second.accessMode, accessModeCodec);

            // Block types
            for (BlockMap::const_iterator blockItr = compressionItr->second.blockTypes.begin();
                 blockItr != compressionItr->second.blockTypes.end();
                 ++blockItr)
            {
                WString blockTypeInfo;
                ConvertBlockObjectToString(blockItr->second, blockTypeInfo);
                
                // Concatenating Access Modes
                WString accessModeBlockType;
                ConvertAccessModesToString(blockItr->second.accessMode, accessModeBlockType);

                // Concatenating the ColorSpace, Codec and Block types access modes
                WChar concatenatedAccessMode[6];
                if (_wcsicmp(accessModeColorSpace.c_str(), accessModeCodec.c_str()) == 0 &&
                    _wcsicmp(accessModeColorSpace.c_str(), accessModeBlockType.c_str()) == 0)
                {
                    wcscpy (concatenatedAccessMode, accessModeColorSpace.c_str());
                }
                else
                {
                    bool allStringHaveC = false;
                    bool allStringHaveW = false;
                    bool allStringHaveR = false;

                    if (wcschr(accessModeColorSpace.c_str(),'C') != NULL &&
                        wcschr(accessModeCodec.c_str(),'C') != NULL &&
                        wcschr(accessModeBlockType.c_str(),'C') != NULL)
                        allStringHaveC = true;

                    if (wcschr(accessModeColorSpace.c_str(),'W') != NULL &&
                        wcschr(accessModeCodec.c_str(),'W') != NULL &&
                        wcschr(accessModeBlockType.c_str(),'W') != NULL)
                        allStringHaveW = true;

                    if (wcschr(accessModeColorSpace.c_str(),'R') != NULL &&
                        wcschr(accessModeCodec.c_str(),'R') != NULL &&
                        wcschr(accessModeBlockType.c_str(),'R') != NULL)
                        allStringHaveR = true;

                    if (allStringHaveC && allStringHaveW && allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"C-R-W");
                    else if (allStringHaveC && !allStringHaveW && !allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"C");
                    else if (!allStringHaveC && !allStringHaveW && allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"R");
                    else if (!allStringHaveC && allStringHaveW && !allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"W");
                    else if (allStringHaveC && allStringHaveW && !allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"C-W");
                    else if (allStringHaveC && !allStringHaveW && allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"C-R");
                    else if (!allStringHaveC && allStringHaveW && allStringHaveR)
                        wcscpy (concatenatedAccessMode, L"R-W");
                }

                WString accessMode = concatenatedAccessMode;

                // If available, we use the DownSampling Methods map.
                // Otherwise, we simply use a fake object with the Z flag (None).
                if (colItr->second.downSamplingMethods.size() > 0)
                {
                    for (DownSamplingMap::const_iterator dsItr = colItr->second.downSamplingMethods.begin();
                         dsItr != colItr->second.downSamplingMethods.end();
                         ++dsItr)
                    {
    
                        // Getting the Profile ID
                        WString profileID;
                        BuildProfileID(itr->second, colItr->second, compressionItr->second, 
                                       blockItr->second, dsItr->second, profileID);
                        
                        // Adding the element to the table
                        out << _T("<tr>\n");
                        out << _T("<td>") << profileID.c_str() << _T("</td>\n");
                        out << _T("<td>") << colItr->second.label.c_str() << _T("</td>\n");
                        out << _T("<td>") << blockTypeInfo.c_str() << _T("</td>\n");
                        out << _T("<td>") << compressionItr->second.label.c_str() << _T("</td>\n");
                        out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
                        out << _T("</tr>\n");
                    }
                }
                else
                {
                    // Getting the Profile ID
                    WString profileID;
                    DownSamplingObject dsObj;   // Fake DownSampling Object
                    dsObj.ID = HRFDownSamplingMethod::NONE;
                    BuildProfileID(itr->second, colItr->second, compressionItr->second, 
                                   blockItr->second, dsObj, profileID);
                    
                    // Adding the element to the table
                    out << _T("<tr>\n");
                    out << _T("<td>") << profileID.c_str() << _T("</td>\n");
                    out << _T("<td>") << colItr->second.label.c_str() << _T("</td>\n");
                    out << _T("<td>") << blockTypeInfo.c_str() << _T("</td>\n");
                    out << _T("<td>") << compressionItr->second.label.c_str() << _T("</td>\n");
                    out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
                    out << _T("</tr>\n");
                }
            }
        }
    }
    out << _T("</tbody>\n");
    
    // End of table
    out << _T("</table>\n");
    out << _T("</div>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::PrintTransfoModel(otstream& out, ImageFormatMap::const_iterator& itr)
{
    if (itr->second.transfoModels.size() > 0)
    {
        // Table header
        out << _T("<div class='TransfoModel'>\n");
        out << _T("<h4>Transformation Models</h4>\n");
        out << _T("<table>\n");
        out << _T("<thead>\n");
        out << _T("<tr>\n");
        out << _T("<th>TransfoModel</th>\n");
        out << _T("<th>Access Mode</th>\n");
        out << _T("</tr>\n");
        out << _T("</thead>\n");
        out << _T("<tbody>\n");
        
        for (TransfoModelMap::const_iterator tmItr = itr->second.transfoModels.begin();
             tmItr != itr->second.transfoModels.end();
             ++tmItr)
        {
            WString accessMode;
            ConvertAccessModesToString(tmItr->second.accessMode, accessMode);
            
            out << _T("<tr>\n");
            
            out << _T("<td>") << tmItr->second.label.c_str() << _T("</td>\n");
            out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
            
            out << _T("</tr>\n");
        }
        
        out << _T("</tbody>\n");
        
        // End of Table
        out << _T("</table>\n");
        out << _T("</div>");
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::PrintImageFormatProperties(otstream& out, ImageFormatMap::const_iterator& itr)
{   
    WString accessMode;
    
    out << _T("<div class='ImageProperties'>\n");
    out << _T("<h4>Image Properties</h4>\n");
    out << _T("<table>\n");
    out << _T("<thead>\n");
    out << _T("<tr>\n");
    out << _T("<th>Property</th>\n");
    out << _T("<th>Value</th>\n");
    out << _T("<th>Access Mode</th>\n");
    out << _T("</tr>\n");
    out << _T("</thead>\n");
    out << _T("<tbody>\n");
    
    // Geocoding Support
    accessMode.clear();
    ConvertAccessModesToString(itr->second.geocodingSupport.accessMode, accessMode);
    out << _T("<tr>\n");
    out << _T("<td>Geocoding Support</td>\n");
    if (itr->second.geocodingSupport.isSupported)
    {
        out << _T("<td>Yes</td>\n");
        out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
    }
    else
    {
        out << _T("<td>No</td>\n");
        out << _T("<td>---</td>\n");
    }
    out << _T("</tr>\n");
    
    // Histogram Support
    accessMode.clear();
    ConvertAccessModesToString(itr->second.histogramSupport.accessMode, accessMode);
    out << _T("<tr>\n");
    out << _T("<td>Histogram Support</td>\n");
    if (itr->second.histogramSupport.isSupported)
    {
        out << _T("<td>Yes</td>\n");
        out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
    }
    else
    {
        out << _T("<td>No</td>\n");
        out << _T("<td>---</td>\n");
    }
    out << _T("</tr>\n");
    
    
    // Interleave Type
    accessMode.clear();
    ConvertAccessModesToString(itr->second.interleaveType.accessMode, accessMode);
    out << _T("<tr>\n");
    out << _T("<td>Interleave Type</td>\n");
    out << _T("<td>") << itr->second.interleaveType.label.c_str() << _T("</td>\n");
    out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
    out << _T("</tr>\n");
    
    // Multipage Support
    accessMode.clear();
    ConvertAccessModesToString(itr->second.multiPageSupport.accessMode, accessMode);
    out << _T("<tr>\n");
    out << _T("<td>MultiPage Support</td>\n");
    if (itr->second.multiPageSupport.isSupported)
    {
        out << _T("<td>Yes</td>\n");
        out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
    }
    else
    {
        out << _T("<td>No</td>\n");
        out << _T("<td>---</td>\n");
    }
    out << _T("</tr>\n");
    
    // Thumbnail Support
    accessMode.clear();
    ConvertAccessModesToString(itr->second.thumbnailSupport.accessMode, accessMode);
    out << _T("<tr>\n");
    out << _T("<td>Thumbnail Support</td>\n");
    if (itr->second.thumbnailSupport.isSupported)
    {
        out << _T("<td>Yes</td>\n");
        out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
    }
    else
    {
        out << _T("<td>No</td>\n");
        out << _T("<td>---</td>\n");
    }
    out << _T("</tr>\n");
    
    // Unlimited Resolution
    accessMode.clear();
    out << _T("<tr>\n");
    out << _T("<td>Unlimited Resolution</td>\n");
    if (itr->second.unlimitedResolution.isUnlimited)
    {
        out << _T("<td>Yes</td>\n");
        out << _T("<td>---</td>\n");
    }
    else
    {
        out << _T("<td>No</td>\n");
        out << _T("<td>---</td>\n");
    }
    out << _T("</tr>\n");

    out << _T("</tbody>\n");
    out << _T("</table>\n");
    out << _T("</div>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::PrintTags(otstream& out, ImageFormatMap::const_iterator& itr)
{
    if (itr->second.tags.size() > 0)
    {
        // Table header
        out << _T("<div class='Tags'>\n");
        out << _T("<h4>Tags Capabilities</h4>\n");
        out << _T("<table>\n");
        out << _T("<thead>\n");
        out << _T("<tr>");
        out << _T("<th>Tag</th>");
        out << _T("<th>Type</th>");
        out << _T("<th>Access Mode</th>");
        out << _T("</tr>\n");
        out << _T("</thead>\n");
        out << _T("<tbody>\n");
        
        for (TagMap::const_iterator tagItr = itr->second.tags.begin();
             tagItr != itr->second.tags.end();
             ++tagItr)
        {
            WString accessMode;
            ConvertAccessModesToString(tagItr->second.accessMode, accessMode);
            
            
            out << _T("<tr>\n");
            
            out << _T("<td>") << tagItr->second.tag.c_str() << _T("</td>\n");
            out << _T("<td>") << accessMode.c_str() << _T("</td>\n");
            
            out << _T("</tr>\n");
        }
        out << _T("</tbody>\n");
        
        // End of table
        out << _T("</table>\n");
        out << _T("</div>\n");
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Jonathan.Bernier      01/2012
+---------------+---------------+---------------+---------------+---------------+------*/ 
void HTMLExporter::PrintGeocodingCapabilities(otstream& out, ImageFormatMap::const_iterator& itr)
{
    if (itr->second.geocodingSupport.isSupported)
    {
        // Table header
        out << _T("<div class='GeocodingCapabilities'>\n");
        out << _T("<h4>Geocoding Capabilities</h4>\n");
        out << _T("<table>\n");
        out << _T("<thead>\n");
        out << _T("<tr>");
        out << _T("<td><b>Supported GeoKeys</b></td>");
        out << _T("</tr>\n");
        out << _T("</thead>\n");
        out << _T("<tbody>\n");

        for (StringList::const_iterator geocodingVectorItr = itr->second.geokeys.begin();
            geocodingVectorItr != itr->second.geokeys.end();
            ++geocodingVectorItr)
        {
            out << _T("<tr>\n");
            out << _T("<td>") << *(geocodingVectorItr) << _T("</td>\n");
            out << _T("</tr>\n");
        }

        out << _T("</tr>\n");
        out << _T("</tbody>\n");

        // End of table
        out << _T("</table>\n");
        out << _T("</div>\n");
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintContentHeader(otstream& out)
{
    // Begin of content
    out << _T("<!-- Begin of ###CONTENT### -->\n");
    out << _T("<div id='content'>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintContentFooter(otstream& out)
{
    // End of content
    out << _T("</div>\n");
    out << _T("</div>\n");
    out << _T("<!-- End of ###CONTENT### -->\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::PrintImageFormat(ImageFormatMap::const_iterator& itr, otstream& out, bool usingFrames = false)
{
    out << _T("<div class='ImageFormat'>");
    // Element header
    out << _T("<div class='FormatHeader' id='") << itr->second.id << _T("'>\n");
    out << _T("<h2>") << itr->second.label.c_str() << _T(" (Extensions: ") << itr->second.extensions.c_str() << _T(")</h2>\n");
    out << _T("</div>\n");
    
    out << _T("<div class='FormatContent'>\n");
    PrintImageFormatProperties(out, itr);
    PrintTransfoModel(out, itr);
    PrintPixelTypeCapabilities(out, itr);
    PrintTags(out, itr);
    PrintGeocodingCapabilities(out, itr);
    out << _T("</div>\n");
    
    if (!usingFrames)
    {
        out << _T("<div class='FormatFooter'><a href='#toc' title='Table of Content'>Top</a></div>\n");
    }
    out << _T("</div>\n\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
void HTMLExporter::PrintContent(ImageFormatMap const& imageFormats, otstream& out)
{
    PrintContentHeader(out);
    for (ImageFormatMap::const_iterator itr = imageFormats.begin(); itr != imageFormats.end(); ++itr)
    {
        PrintImageFormat(itr, out);
    }
    PrintContentFooter(out);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::PrintContent(ImageFormatMap const& imageFormats, WString pathToDirectory)
{
    for (ImageFormatMap::const_iterator itr = imageFormats.begin(); itr != imageFormats.end(); ++itr)
    {
        WChar id[10];
        _itot(itr->second.id, id, 10);
        id[9] = L'\0';
        WChar path[256];
        _tcscpy(path, pathToDirectory.c_str());
        _tcscat(path, id);
        _tcscat(path, _T(".html"));
        
        tfstream out;
        out.open(path, ios_base::out);
        
        if (out.is_open())
        {
            PrintGenericHeader(out);
            out << _T("<div id='documentBody'>\n");
            PrintContentHeader(out);
            PrintImageFormat(itr, out, true);
            PrintContentFooter(out);
            out << _T("</div>\n");
            PrintGenericFooter(out);
        }
        else
            return;
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::PrintFrameset (otstream& out)
{
    out << _T("<html>\n");
    out << _T("\t<head><title>Image++ Capabilities</title></head>\n");
    out << _T("\t<frameset rows='80, *, 50'>\n");
    out << _T("\t\t<frame src='header.html' name='header' />\n");
    out << _T("\t\t<frameset cols='250, *'>\n");
    out << _T("\t\t\t<frame src='toc.html' name='toc' />\n");
    out << _T("\t\t\t<frame src='content.html' name='content' />\n");
    out << _T("\t\t</frameset>\n");
    out << _T("\t\t<frame src='footer.html' name='footer' />\n");
    out << _T("\t</frameset>\n");
    out << _T("</html>\n");
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::PrintDefaultContent(otstream& out)
{
    PrintGenericHeader(out);
    out << _T("<div id='defaultContent'>\n");
    out << _T("<div>\n");
    out << _T("<h2>Welcome to Image++ Capabilities Exporter.</h2>\n");
    out << _T("<p>This document contains the supported capabilities of each available image format in Image++.</p>");
    out << _T("<p>These capabilities are automatically generated by querying the library.</p>\n");
    out << _T("<p>Click one of the links in the menu bar on your left to continue.</p>\n");
    out << _T("</div>\n");
    PrintGenericFooter(out);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetColorSpaceCode(long const colorSpaceID, WString& output)
{
    output.clear();
    output += WString(HUTClassIDDescriptor::GetInstance()->GetClassCodePixelType(colorSpaceID).c_str(), BentleyCharEncoding::Utf8);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetCompressionCode(long const codecID, WString& output)
{
    output.clear();
    output += WString(HUTClassIDDescriptor::GetInstance()->GetClassCodeCodec(codecID).c_str(), BentleyCharEncoding::Utf8);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetOrganizationCode(BlockObject const& blockType, WString& output)
{
    output.clear();
    switch (blockType.ID)
    {
    case HRFBlockType::AUTO_DETECT:
        output += _T("Z");
        break;
    case HRFBlockType::IMAGE:
        output += _T("I");
        break;
    case HRFBlockType::LINE:
        output += _T("L");
        break;
    case HRFBlockType::STRIP:
        if ((blockType.minimumHeight == 16 || blockType.minimumHeight == 32) && blockType.heightIncrement == 16)
            output += _T("S");
        else
        if (blockType.minimumHeight == 1 && blockType.heightIncrement == 1)
            output += _T("C");
        else
            output += _T("Z");
        break;
    case HRFBlockType::TILE:
        if (blockType.minimumHeight == 256 && blockType.maximumHeight == 256 &&
            blockType.minimumWidth == 256 && blockType.maximumWidth == 256 &&
            blockType.heightIncrement == 0)
            output += _T("T");
        else
        if (blockType.minimumHeight == 32 && blockType.minimumWidth == 32 &&
            blockType.heightIncrement == 16)
            output += _T("X");
        else
        if (blockType.minimumHeight == 32 && blockType.minimumWidth == 32 &&
            blockType.heightIncrement == 32)
            output += _T("M");
        else
        if (blockType.minimumHeight == 64 && blockType.minimumWidth == 64 &&
            blockType.maximumHeight == 64 && blockType.maximumWidth == 64 &&
            blockType.heightIncrement == 0)
            output += _T("R");
        else
            output += _T("Z");
        
        break;
    default:
        output += _T("?");
    }
}

/*---------------------------------------------------------------------------------**//**
* Returns the following codes:
*       1 :     Multi-Resolution
*       2 :     Multi-Resolution with 2x scale factor (not supported for now since it is
*                                                      not possible to check the scaling
*                                                      factor from the creator)
*       3 :     Progressive Resolution (Actually, we check if the format has unlimited 
*                                       resolution but it could wrong)
*       ? :     Unknown value
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetResolutionCode(BlockObject const& blockType, WString& output)
{
    output.clear();
    if (blockType.isUnlimitedResolution)
        output += _T("3");
    else
    if (blockType.isMultiResolution)
        output += _T("1");
    else
        output += _T("0");
        
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetDownSamplingCode(long const downSamplingMethodID, WString& output)
{
    output.clear();
    switch ((HRFDownSamplingMethod::DownSamplingMethod) downSamplingMethodID)
    {
    case HRFDownSamplingMethod::AVERAGE:
        output += _T("A");
        break;
    case HRFDownSamplingMethod::NEAREST_NEIGHBOUR:
        output += _T("N");
        break;
    case HRFDownSamplingMethod::NONE:
        output += _T("Z");
        break;
    case HRFDownSamplingMethod::ORING4:
        output += _T("4");
        break;
    case HRFDownSamplingMethod::UNKOWN:
        output += _T("U");
        break;
    case HRFDownSamplingMethod::VECTOR_AWARENESS:
        output += _T("V");
        break;
    default:
        output += _T("?");
    }
}

/*---------------------------------------------------------------------------------**//**
* Writes the ScanlineOrientation code for all image formats but HMR.
*
* There is actually no way to know the HMR file version supported by the creator.
* Since the first versions have only been used for a short period of time, the 
* default version used here is the last one. Of course, it means that the exported
* report is incomplete, missing 2 profileID.
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::GetScanlineOrientation(ImageFormat const& image, WString& output)
{
    output.clear();
    if (image.id == HRFHMRFile::CLASS_ID)
    {
        output += HMR_CURRENT_VERSION;
    }
    else
    {
        // Converting the SLO directly to string
        WChar slo[10];
        _ltot(image.SLO.ID, slo, 10);
        output += slo;
    }
}

/*---------------------------------------------------------------------------------**//**
* This method generates the profile ID used to identify images in the test dataset. 
* 
* The profile ID usually has the following form:
*       AA NN A N A N .EXT
* where
*       AA :    Color Space code
*       NN :    Compression code
*       A  :    Organization code
*       N  :    Multi-Resolution code
*       A  :    DownSampling Method code
*       N  :    Version/SLO - It is the Scanline Orientation of the image for all formats
*                             but HMR. See the documentation of GetScanlineOrientation 
*                             for more information.
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::BuildProfileID(ImageFormat const&            image,
                                  ColorSpaceObject const&       colorSpace,
                                  CodecObject const&            codecObject,
                                  BlockObject const&            blockType,
                                  DownSamplingObject const&     downSamplingMethod,
                                  WString&                      profileID)
{
    WString str;

    // Color space
    GetColorSpaceCode(colorSpace.ID, str);
    profileID += str;

    // Compression codec
    GetCompressionCode(codecObject.ID, str);
    profileID += str;

    // Image Organization (block, line, strip, etc.)
    GetOrganizationCode(blockType, str);
    profileID += str;

    // Single/Multi Resolution
    GetResolutionCode(blockType, str);
    profileID += str;

    // DownSampling method (iTiff only)
    GetDownSamplingCode(downSamplingMethod.ID, str);
    profileID += str;

    // Scanline Orientation
    GetScanlineOrientation(image, str);
    profileID += str;
}

/*---------------------------------------------------------------------------------**//**
* This method creates an one-page HTML document containing Image++ Capabilities.
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::Export(ImageFormatMap const& imageFormats, otstream& output)
{   
    // Header & ToC
    PrintGenericHeader(output);
    PrintHeader(output);
    PrintTableOfContent(imageFormats, output);
    
    // Content
    PrintContent(imageFormats, output);
    
    // Footer
    PrintFooter(output);
    PrintGenericFooter(output);
}        

/*---------------------------------------------------------------------------------**//**
* Using an HTML template, this method exports the Image++ Capabilities by doing a find & 
* replace of special tags located in the template file.
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::Export(ImageFormatMap const& imageFormats,  itstream& templateInput, otstream& output)
{
    // Parsing the template file
    TCHAR line[MAX_LINE_LENGTH];
    TCHAR tmpStr[MAX_LINE_LENGTH];
    TCHAR* str = NULL;
    int pos;
    templateInput.getline(line, MAX_LINE_LENGTH);
    while (!templateInput.eof())
    {
        pos = 0;
        tmpStr[0] = _T('\n');
        // Looking for the header tag
        if ((str = _tcsstr(line, TEMPLATE_TAG_HEADER)) != NULL)
        {
            pos = (int) (_tcslen(line) - _tcslen(str));
            // Outputting the string before the tag
            _tcsncpy_s(tmpStr, MAX_LINE_LENGTH, line, pos);
            tmpStr[pos] = _T('\0');
            output << tmpStr;
            // Exporting the header
            PrintTableOfContent(imageFormats, output);

            // Outputting the end of the string
            str += _tcslen(TEMPLATE_TAG_HEADER);
            output << str;
        }
        else
        // Looking for the content tag
        if ((str = _tcsstr(line, TEMPLATE_TAG_CONTENT)) != NULL)
        {
            pos = (int) (_tcslen(line) - _tcslen(str));
            // Outputting the string before the tag
            _tcsncpy_s(tmpStr, MAX_LINE_LENGTH, line, pos);
            tmpStr[pos] = _T('\0');
            output << tmpStr;
            // Exporting the content
            PrintContentHeader(output);
            PrintContent(imageFormats, output);
            PrintContentFooter(output);
            
            // Outputting the end of the string
            str += _tcslen(TEMPLATE_TAG_CONTENT);
            output << str;
        }
        else
        // Looking for the footer tag
        if ((str = _tcsstr(line, TEMPLATE_TAG_FOOTER)) != NULL)
        {
            pos = (int) (_tcslen(line) - _tcslen(str));
            // Outputting the string before the tag
            _tcsncpy_s(tmpStr, MAX_LINE_LENGTH, line, pos);
            tmpStr[pos] = _T('\0');
            output << tmpStr;
            // Exporting the footer
            PrintFooter(output);
            
            // Outputting the end of the string
            str += _tcslen(TEMPLATE_TAG_FOOTER);
            output << str;
        }
        else
        {
            // No tag found, we just print the line to the output stream
            output << line << _T("\n");
        }
        
        // Next line
        templateInput.getline(line, MAX_LINE_LENGTH);
    }
}

/*---------------------------------------------------------------------------------**//**
* Creates a multiple pages document using HTML frames. All the files are saved under the
* specified path. The user only has to open index.html in this folder.
*
* @bsimethod                                                    Jean.Lalande      07/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HTMLExporter::Export(ImageFormatMap const& imageFormats, WString const& exportPath)
{
    // Frameset
    WString framePath;
    framePath = exportPath + FRAMESET_FILENAME;
    tfstream frameFile;
    frameFile.open(framePath.c_str(), ios_base::out);
    
    if (!frameFile.is_open())
        return;
        
    PrintFrameset(frameFile);
    frameFile.close();
    
    // Header
    WString headerPath;
    headerPath = exportPath + HEADER_FILENAME;
    tfstream headerFile;
    headerFile.open(headerPath.c_str(), ios_base::out);
    
    if (!headerFile.is_open())
        return;
    
    PrintGenericHeader(headerFile);
    PrintHeader(headerFile);
    headerFile << _T("</div>\n");   // Needed since we do not call the PrintFooter method.
                                    // Otherwise, the HTML document won't be valid.
    PrintGenericFooter(headerFile);
    headerFile.close();
    
    // Table of Content
    WString tocPath;
    tocPath = exportPath + TOC_FILENAME;
    tfstream tocFile;
    tocFile.open(tocPath.c_str(), ios_base::out);
    
    if (!tocFile.is_open())
        return;
        
    PrintGenericHeader(tocFile);
    PrintTableOfContent(imageFormats, tocFile, true);
    PrintGenericFooter(tocFile);
    tocFile.close();
    
    // Default content page
    WString defaultPath;
    defaultPath = exportPath + DEFAULT_CONTENT_FILENAME;
    tfstream defaultFile;
    defaultFile.open(defaultPath.c_str(), ios_base::out);
    
    if (!defaultFile.is_open())
        return;
        
    PrintDefaultContent(defaultFile);
    defaultFile.close();
    
    // Content
    WString contentPath;
    contentPath = exportPath + CONTENT_PATH + _T("\\");
    if (!PathIsDirectory(contentPath.c_str()))
    {
        CreateDirectory(contentPath.c_str(), NULL); 
    }
    PrintContent(imageFormats, contentPath);
    
    // Footer
    WString footerPath;
    footerPath = exportPath + FOOTER_FILENAME;
    tfstream footerFile;
    footerFile.open(footerPath.c_str(), ios_base::out);
    
    if (!footerFile.is_open())
        return;
        
    PrintGenericHeader(footerFile);
    PrintFooter(footerFile);
    PrintGenericFooter(footerFile);
    footerFile.close();
}