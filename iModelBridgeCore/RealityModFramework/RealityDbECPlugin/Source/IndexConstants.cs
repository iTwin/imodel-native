/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/IndexConstants.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source
    {
    internal static class IndexConstants
        {

        public const int USGSIdLenght = 24;

        //Rds Constants
        //public const string RdsUrlBase = ConfigurationRoot.GetAppSetting("RECPRdsUrlBase"); // "https://s3mxcloudservice.cloudapp.net/v2.3/repositories/S3MXECPlugin--Server/S3MX/";
        public const string RdsRealityDataClass = "RealityData";
        public const string RdsDocumentClass = "Document/";
        public const string RdsSubAPIString = "RDS";
        public const string RdsSourceName = "RealityDataService";

        //Usgs
        public const string UsgsTermsOfUse = "https://www2.usgs.gov/laws/info_policies.html";
        public const string UsgsLegalString = " courtesy of the U.S. Geological Survey";
        public const string UsgsSubAPIString = "USGS";
        public const string UsgsRawMetadataFormatString = "FGDC";
        public const string UsgsRawMetadataURLEnding = "?format=fgdc";
        public const string UsgsDataProviderString = "USGS";
        public const string UsgsDataProviderNameString = "United States Geological Survey";

        //Product constants
        public const string ProductGUID = "614a68c4-c15d-46b9-98a9-9d4d10893623";
        public const int ProductId = 2537;


        //Features GUIDs

        public const string PackageFeatureGuid = "496da74d-b86b-45a7-bbd3-6f2ea025bdd8";
        }
    }
