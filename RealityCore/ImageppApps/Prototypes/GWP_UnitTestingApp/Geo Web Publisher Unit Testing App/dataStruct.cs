/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/dataStruct.cs $
|    $RCSfile: dataStruct.cs, $
|   $Revision: 1 $
|       $Date: 2013/05/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Collections.Generic;
using System.Drawing;

/// <summary>Contain multiple data struct to allow easy information sharing between class</summary>
/// <author>Julien Rossignol</author>
namespace Geo_Web_Publisher_Unit_Testing_App
{
    #region struct

    /// <summary>Struct containing all information of a request</summary>
    /// <author>Julien Rossignol</author>
    public struct Request
    {
        public string RequestString; //request url
        public Service Service; // wmf or wfs
        public RequestType Type;  //get map, get info, get feature etc
        public Format Format;  // gif, jpeg, png
        public string version; // wmf or wfs protocol version
        public List<string> LayerList; // list of layers used in the request
        public bool IsPlausible; // indicate weither the record is plausible or not
        public Extent BBox; //BBOX of the request (string format to simplify extraction)
        public string Map; //GUID of the map
        public string SRS; //SRS of the map
    }

    /// <summary>Struct containing all information about one test, a request is associate with the test</summary>
    /// <author>Julien Rossignol</author>
    public struct Record
    {
        public int Id; //database index of the record
        public string Name; //name given by user
        public string Description; // description given by user
        public int CategoryId; // id of the linked category 
        public Request RecordRequest; //request associate with the record
        public byte[] BaselineBitmap; //byte[] containing the baseline image or xml/error string
        public int MRBaselineId; // id of the baseline in the database
        public bool BaselineIsUpToDate; // tell if the baseline is up to date (if the request as change since last update)
        public bool BaselineIsXML; // tell if the baseline is an xml file
    }

    /// <summary>Struct containing the extent of a map (BBOX)</summary>
    /// <author>Julien Rossignol</author>
    public struct Extent
    {
        public double XMin;
        public double YMin;
        public double XMax;
        public double YMax;
    }

    /// <summary>Struct containing the result of a record</summary>
    /// <author>Julien Rossignol</author>
    public struct ResultRequest
    {
        public int Id; // database index of the result
        public byte[] ResultBitmap; //result bitmap or string
        public Bitmap DifferenceBitmap; //bitmap of all difference between baseline and output
        public Record ResultRecord;  // record associatie with the result
        public RequestStatus Status; //status of the comparison
        public int TestId; //database index of the test associate with the result
        public Test LinkedTest;//test linked with the result, also containt the index, but since the test is not always load in memory testId was needed
        public bool ExecuteTest; //tell if the testmanager must execute or not the request
        public int FailureRasterId; //database index of the failure raster
        public int InvalidPixels; // number of invalid pixels
        public int BigDifferencePixels; //number of pixels with a large difference
    }

    /// <summary>Struct containing multiple results processed at the same time</summary>
    /// <author>Julien Rossignol</author>
    public struct Test
    {
        public int Id; //database index of the test
        public int DateTime; //time of the test
        public string Description; //desciption given by the user
        public string Prefix; //protocol, server name and port number of the test
    }

    /// <summary>Struct containing a category</summary>
    /// <author>Julien Rossignol</author>
    public struct Category
    {
        public string Name; //name of the category
        public int Id; // database index of the category
    }

    #endregion

    #region enum

    /// <summary>Enum containing all database table</summary>
    /// <author>Julien Rossignol</author>
    public enum DBTable
    {
        RECORD ,
        CATEGORY ,
        BASELINE ,
        REQUESTRESULT ,
        FAILURERASTER ,
        TEST ,
    }

    /// <summary>Enum containing every possible status for a record</summary>
    /// <author>Julien Rossignol</author>
    public enum RequestStatus
    {
        SUCCESS , //Output is the same as baseline
        FAILED , // output is not the same as baseline
        WARNING , //Output bitmap is almost the same
        ERROR , // output are not the same format
        NONE // no comparison was done yet
    }

    #endregion
}
