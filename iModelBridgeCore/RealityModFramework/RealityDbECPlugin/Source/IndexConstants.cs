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

        //PackageRequestContant
        public const string PRExtendedDataName = "PackageRequest";

        //Australia Constants
        public const string AUBaseUrl = "http://portal.eginger.ninja/server/rest/services/ELVIS_Testing/MapServer/";
        public const string AUDataProviderString = "GeoscienceAUS";
        public const string AUDataProviderNameString = "Geoscience Australia";
        public const string AUSubAPIString = "AU";
        public const string AUServerName = "ELVIS";

        //Earth Explorer Constants
        public const string EEBaseApiURL = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/";
        public const string EEBaseMetadataURL = "https://earthexplorer.usgs.gov/metadata/xml/";
        public const string EEFgdcMetadataURL = "https://earthexplorer.usgs.gov/fgdc/{0}/{1}/save_xml";
        public const string EELegalString = "Data available from the U.S. Geological Survey.";
        public const string EETermsOfUse = "https://lta.cr.usgs.gov/citation";
        public const string EEDownloadURL = "https://earthexplorer.usgs.gov/download/{0}/{1}/STANDARD/EE";
        public const string EESubAPIString = "USGSEE";
        public const string EERegistrationPage = "https://ers.cr.usgs.gov/register/";
        public const string EEOrganisationPage = "https://earthexplorer.usgs.gov/";
        public const string EELoginKey = "EarthExplorer";
        public const string EELoginMethod = "CUSTOM";
        public const string EEServerName = "Earth Explorer";
        public const string EEServerURL = "https://earthexplorer.usgs.gov/";

        //Rds Constants
        //public const string RdsUrlBase = ConfigurationRoot.GetAppSetting("RECPRdsUrlBase"); // "https://s3mxcloudservice.cloudapp.net/v2.3/repositories/S3MXECPlugin--Server/S3MX/";
        public const string RdsRealityDataClass = "RealityData";
        public const string RdsDocumentClass = "Document/";
        public const string RdsSubAPIString = "RDS";
        public const string RdsName = "RealityDataServices";
        public const string RdsSourceName = "RealityDataService";
        public const string RdsLoginKey = "BentleyCONNECT";
        public const string RdsLoginMethod = "CUSTOM";
        public const string RdsRegistrationPage = "https://ims.bentley.com/IMS/Registration/";
        public const string RdsOrganisationPage = "https://www.bentley.com/";
        public const string RdsReadAccessAzureTokenUrlEnd = "/FileAccess.FileAccessKey?$filter=Permissions+eq+\'Read\'&api.singleurlperinstance=true";

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
