/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/requestAnalyser.cs $
|    $RCSfile: requestAnalyser.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/23 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Globalization;

namespace Geo_Web_Publisher_Unit_Testing_App
{

    #region enums

    /// <summary>Enum containing all service possible for a request</summary>
    /// <author>Julien Rossignol</author>
    public enum Service
    {
        WMS ,
        WFS ,
        ERROR
    }

    /// <summary>Enum containing all possible request type</summary>
    /// <author>Julien Rossignol</author>
    public enum RequestType
    {
        GETMAP ,
        GETCAPABILITIES ,
        GETFEATUREINFO ,
        DESCRIBEFEATURETYPE ,
        GETFEATURE ,
        ERROR
    }

    /// <summary>Struct containing all image format a getmap request can ask for</summary>
    /// <author>Julien Rossignol</author>
    public enum Format
    {
        JPEG ,
        PNG ,
        PNG8BIT ,
        GIF ,
        ERROR
    }

    #endregion

    static public class RequestAnalyser
    {
        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Analyse every aspect of requestToAnalyse and returns a Request instance containing all informations</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Request AnalyseRequest(string requestToAnalyse)
        {
            Request request = new Request();
            if( requestToAnalyse != null )
            {
                request.RequestString = requestToAnalyse;
                request.Service = AnalyseService(requestToAnalyse);
                request.Type = AnalyseRequestType(requestToAnalyse);
                request.Format = AnalyseFormat(requestToAnalyse);
                request.LayerList = AnalyseLayer(requestToAnalyse);
                request.version = AnalyseVersion(requestToAnalyse);
                request.BBox = AnalyseExtent(requestToAnalyse);
                request.Map = AnalyseMap(requestToAnalyse);
                request.SRS = AnalyseSRS(requestToAnalyse);
                request.IsPlausible = CheckPlausibility(request);
            }
            return request;

        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Analyse the srs of the request to analyse</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <returns>String containing the srs information</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public static string AnalyseSRS(string requestToAnalyse)
        {
            int position = requestToAnalyse.IndexOf("SRS=" , StringComparison.CurrentCultureIgnoreCase); // get the position of the srs in the request
            if( position <= 0 )
                return "None";
            string srs = requestToAnalyse.Substring(position+4); //cut everything before the srs
            srs = srs.Remove(srs.IndexOf('&' , 0)); //cut everything after
            return srs;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Analyse the map of the request to analyse</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <returns>Returns the map GUID</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public static string AnalyseMap(string requestToAnalyse)
        {
            try
            {
                return requestToAnalyse.Remove(requestToAnalyse.IndexOf(".")); //cut everything after the map and returns it
            }
            catch
            {
                return "";
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the service (WMS or WFS) of the request, if both are there, returns an error</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Service AnalyseService(string requestToAnalyse)
        {
            Service service = Service.ERROR;
            if( requestToAnalyse != string.Empty )
            {
                bool wms = requestToAnalyse.IndexOf("SERVICE=WMS" , StringComparison.CurrentCultureIgnoreCase) != -1; //indicates if wms service is found
                bool wfs = requestToAnalyse.IndexOf("SERVICE=WFS" , StringComparison.CurrentCultureIgnoreCase) != -1; // indicates if wfs service is found

                if( wfs && !wms ) //if wfs is there but not wms
                    service = Service.WFS;
                else if( wms && !wfs ) // if wms is there and not wfs
                    service = Service.WMS;
            }
            return service;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the type of the request, if more than one is there, returns an error</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public RequestType AnalyseRequestType(string requestToAnalyse)
        {
            RequestType type = RequestType.ERROR;
            if( requestToAnalyse != string.Empty )
            {
                //try to fin every possible string indicating request type
                bool getMap = requestToAnalyse.IndexOf("request=getmap&" , StringComparison.CurrentCultureIgnoreCase) != -1;
                if( !getMap )
                    getMap = requestToAnalyse.IndexOf("request=map&" , StringComparison.CurrentCultureIgnoreCase) != -1;

                bool getCapabilities = requestToAnalyse.IndexOf("request=getcapabilities&" , StringComparison.CurrentCultureIgnoreCase) != -1;
                if( !getCapabilities )
                    getCapabilities = requestToAnalyse.IndexOf("request=capabilities&" , StringComparison.CurrentCultureIgnoreCase) != -1;

                bool getFeatureInfo = requestToAnalyse.IndexOf("request=getfeatureinfo&" , StringComparison.CurrentCultureIgnoreCase) != -1;

                bool describeFeatureType = requestToAnalyse.IndexOf("request=DescribeFeatureType&" , StringComparison.CurrentCultureIgnoreCase) != -1;

                bool getFeature = requestToAnalyse.IndexOf("request=GetFeature&" , StringComparison.CurrentCultureIgnoreCase) != -1;

                //then checked which were found
                int numberOfType = 0;
                if( getMap )
                {
                    type = RequestType.GETMAP;
                    numberOfType++;
                }
                if( getCapabilities )
                {
                    type = RequestType.GETCAPABILITIES;
                    numberOfType++;
                }
                if( getFeatureInfo )
                {
                    type = RequestType.GETFEATUREINFO;
                    numberOfType++;
                }
                if( describeFeatureType )
                {
                    type = RequestType.DESCRIBEFEATURETYPE;
                    numberOfType++;
                }
                if( getFeature )
                {
                    type = RequestType.GETFEATURE;
                    numberOfType++;
                }
                if( numberOfType != 1 ) // if there is more than one request type found
                    type = RequestType.ERROR;
            }
            return type;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the image format of the request</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Format AnalyseFormat(string requestToAnalyse)
        {
            Format format = Format.ERROR;
            if( requestToAnalyse != string.Empty )
            {
                //try to find each possible format 
                bool jpeg = requestToAnalyse.IndexOf("format=image/jpeg" , StringComparison.CurrentCultureIgnoreCase) != -1;
                if( !jpeg )
                    jpeg = requestToAnalyse.IndexOf("format=jpeg" , StringComparison.CurrentCultureIgnoreCase) != -1;

                bool png8bit = requestToAnalyse.IndexOf("format=image/png;" , StringComparison.CurrentCultureIgnoreCase) != -1;
                bool png = false;
                if( !png8bit )
                {
                    png8bit = requestToAnalyse.IndexOf("format=8-bit" , StringComparison.CurrentCultureIgnoreCase) != -1;

                    png = requestToAnalyse.IndexOf("format=image/png" , StringComparison.CurrentCultureIgnoreCase) != -1;
                    if( !png )
                        png = requestToAnalyse.IndexOf("format=png" , StringComparison.CurrentCultureIgnoreCase) != -1;
                }
                bool gif = requestToAnalyse.IndexOf("format=image/gif" , StringComparison.CurrentCultureIgnoreCase) != -1;

                //then checked which were found
                int numberOfType = 0;
                if( jpeg )
                {
                    format = Format.JPEG;
                    numberOfType++;
                }
                if( png8bit )
                {
                    format = Format.PNG8BIT;
                    numberOfType++;
                }
                if( png )
                {
                    format = Format.PNG;
                    numberOfType++;
                }
                if( gif )
                {
                    format = Format.GIF;
                    numberOfType++;
                }

                if( numberOfType != 1 )//if there is more than one type
                    format = Format.ERROR;
            }

            return format;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the list of layers of the request</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public List<string> AnalyseLayer(string requestToAnalyse)
        {
            List<string> layerList = new List<string>();
            if( requestToAnalyse != string.Empty )
            {
                int layerPosition = requestToAnalyse.IndexOf("layers=" , StringComparison.CurrentCultureIgnoreCase); //find the beginning of the layer list
                if( layerPosition >= 0 )
                {
                    string layerString = requestToAnalyse.Substring(layerPosition); //cut the beginning
                    int endLayerPosition = layerString.IndexOf("&"); //find the end of the layer list
                    layerString = layerString.Remove(endLayerPosition); // cut the end
                    layerString = layerString.Substring(7); //remove the "layers="
                    string[] layerArray = layerString.Split(','); // split what remains in multiple string
                    foreach( string layer in layerArray )
                        layerList.Add(layer);
                }
            }

            return layerList;

        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Return the version use for the request</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public string AnalyseVersion(string requestToAnalyse)
        {
            string version = "";
            if( requestToAnalyse != string.Empty )
            {
                int versionPosition = requestToAnalyse.IndexOf("version=" , StringComparison.CurrentCultureIgnoreCase); // find the beginning of the version
                if( versionPosition >= 0 )
                {
                    version = requestToAnalyse.Substring(versionPosition); //cut the beginning
                    int endLayerPosition = version.IndexOf("&"); //find the end
                    version = version.Remove(endLayerPosition); // removes the end
                    version = version.Substring(8); //removes "version="
                }
            }

            return version;
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Check if the request seems plausible</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public bool CheckPlausibility(Request requestToAnalyse)
        {
            bool IsPlausible = true;
            if( requestToAnalyse.Service == Service.WMS && ( requestToAnalyse.Type == RequestType.GETFEATURE || requestToAnalyse.Type == RequestType.DESCRIBEFEATURETYPE ) ) // check if services and request type are plausible
                IsPlausible = false;
            if( requestToAnalyse.Service == Service.WFS && ( requestToAnalyse.Type == RequestType.GETFEATUREINFO || requestToAnalyse.Type == RequestType.GETMAP ) )// check if services and request type are plausible
                IsPlausible = false;
            if( ( requestToAnalyse.Type == RequestType.GETMAP ) == ( requestToAnalyse.Format == Format.ERROR ) ) //check if image format fits the request type
                IsPlausible = false;
            if( requestToAnalyse.Service == Service.ERROR || requestToAnalyse.Type == RequestType.ERROR ) //check if there is an error in service and type
                IsPlausible = false;

            return IsPlausible;
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Returns the extent of a request</summary>  
        /// <param name="requestToAnalyse">Request to analyse</param>
        /// <returns>Extent of the request</returns>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Extent AnalyseExtent(string requestToAnalyse)
        {
            Extent bBox = new Extent();
            if( requestToAnalyse != string.Empty )
            {
                int bBoxPosition = requestToAnalyse.IndexOf("BBOX=" , StringComparison.CurrentCultureIgnoreCase); //find the beginning of the bboc
                if( bBoxPosition >= 0 )
                {
                    string bBoxString = requestToAnalyse.Substring(bBoxPosition); // delete the beginning
                    int endbBoxPosition = bBoxString.IndexOf("&"); // find the end
                    bBoxString = bBoxString.Remove(endbBoxPosition); //delete the end 
                    bBoxString = bBoxString.Substring(5); // delete "BBOX="
                    string[] bBoxArray = bBoxString.Split(','); //split what's left into multiple string
                    bBox.XMin = Double.Parse(bBoxArray[0] , CultureInfo.InvariantCulture); //save data in corresponding variable
                    bBox.YMin = Double.Parse(bBoxArray[1] , CultureInfo.InvariantCulture);
                    bBox.XMax = Double.Parse(bBoxArray[2] , CultureInfo.InvariantCulture);
                    bBox.YMax = Double.Parse(bBoxArray[3] , CultureInfo.InvariantCulture);
                }
            }
            return bBox;
        }

        #endregion
    }
}
