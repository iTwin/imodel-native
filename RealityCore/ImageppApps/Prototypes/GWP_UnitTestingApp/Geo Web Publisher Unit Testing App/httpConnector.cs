/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/httpConnector.cs $
|    $RCSfile: httpConnector.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.IO;
using System.Net;

/// <summary>This class execute an http request and convert it to a usable string</summary>
/// <author>Julien Rossignol</author>
namespace Geo_Web_Publisher_Unit_Testing_App
{
    class HttpConnector
    {
        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute the http request sent as a parameter and return the server's response as a string</summary>  
        /// <param name="request">Request to be execute</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public byte[] ExecuteRequest(string request)
        {
            WebRequest requestToSend = HttpWebRequest.Create(request); //create the request
            try
            {
                WebResponse response = requestToSend.GetResponse(); //get and wait for the response of the server
                return ConvertWebResponseToByte(response); // convert it to byte[] then return it
            }
            catch( Exception exception ) // if browser call an exeption (404 per exemple)
            {
                return System.Text.Encoding.GetEncoding(28591).GetBytes(exception.Message); //return the exception message
            }
        }

        /*------------------------------------------------------------------------------------**/
        /// <summary>Convert a WebResponse to a usable byte[]</summary>  
        /// <param name="response">Response to be convert</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public byte[] ConvertWebResponseToByte(WebResponse response)
        {
            Stream responseStream = response.GetResponseStream();  // open the response
            MemoryStream memStream = new MemoryStream();
            responseStream.CopyTo(memStream);  //copy the response in the memstream
            return memStream.ToArray(); //convert it to byte[]
        }


        /*------------------------------------------------------------------------------------**/
        /// <summary>Execute a request witouth catching any exception, used to see if the servername/protocol/port number is ok</summary>  
        /// <param name="request">Request to be execute</param>
        /// <author>Julien Rossignol</author>                            <date>05/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public byte[] ExecuteRequestWithoutProtection(string request)
        {
            WebRequest requestToSend = HttpWebRequest.Create(request); //the absence of protection allow catching the error outside of the function
            WebResponse response = requestToSend.GetResponse();
            return ConvertWebResponseToByte(response);
        }

        #endregion
    }
}
