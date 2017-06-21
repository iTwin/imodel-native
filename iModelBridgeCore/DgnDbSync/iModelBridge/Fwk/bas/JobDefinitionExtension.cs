/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/bas/JobDefinitionExtension.cs $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------+
|
|   Usings
|
+--------------------------------------------------------------------------------------*/
using System;
using System.IO;
using System.Net;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Collections.Generic;
using System.Text;
using Bentley.Automation;
using Bentley.Automation.Extensions;
using Bentley.Automation.JobConfiguration;
using BSI.Orchestration.Utility;
using BSI.Automation;

//https://bsw-wsg.bentley.com/bistro/v2.4/Repositories[DataContract]
//{"instances":[{"instanceId":"iModelHubAdmin--","schemaName":"Repositories","className":"RepositoryIdentifier","properties":{"ECPluginID":"iModelHubAdmin","Location":"","DisplayLabel":"","Description":"DgnDb Repositories Admin","ImageMoniker":""},"eTag":"\"E0BIKX7pWnUHQMorFHDhPHbfPhU=\""},{"instanceId":"iModelHub--BLI_1","schemaName":"Repositories","className":"RepositoryIdentifier","properties":{"ECPluginID":"iModelHub","Location":"BLI_1","DisplayLabel":"BLI_1","Description":"iModelHub","ImageMoniker":""},"eTag":"\"eHDKaUs/HiJ+Lx95yAQLAEMqLns=\""},{"instanceId":"iModelHub--

// *** NEEDS WORK - the REST API seems to have changed
namespace iModelHub_REST_API_v2_4
{
    class GetRepositories
    {
        public static string MakeRequest(string url) { return url + "/v2.4/Repositories"; }

        [DataContract]
        public class InstanceProperties
        {
            [DataMember]
            public string Location { get; set; }
        }

        [DataContract]
        public class Instance
        {
            [DataMember]
            public string instanceId { get; set; }

            [DataMember]
            public InstanceProperties properties { get; set; }

        }

        [DataContract]
        public class Response
        {
            [DataMember]
            public Instance[] instances { get; set; }
        }
    }

    public class Utils
    {
        public static List<string> GetRepositoryNames(string url, string username, string password)
        {
            try
            {
                var request = WebRequest.Create(GetRepositories.MakeRequest(url));
                request.Timeout = 10000;
                request.AuthenticationLevel = System.Net.Security.AuthenticationLevel.None;
                request.Credentials = new System.Net.NetworkCredential(username, password);

                System.Net.ServicePointManager.ServerCertificateValidationCallback += (sender, cert, chain, sslPolicyErrors) =>
                {
                    return true;
                };

                WebResponse webResponse = request.GetResponse ();
                if (null == webResponse)
                    return null;
                Stream objStream;
                objStream = webResponse.GetResponseStream();
                StreamReader objReader = new StreamReader(objStream);
                List<string> repos = new List<string>();
                string sLine;
                while ((sLine = objReader.ReadLine()) != null)
                {
                    var json = new DataContractJsonSerializer(typeof(GetRepositories.Response));
                    var stream = new MemoryStream(Encoding.UTF8.GetBytes(sLine));
                    var response = (GetRepositories.Response)json.ReadObject(stream);
                    for (var i = 0; i < response.instances.Length; ++i)
                    {
                        // repos.Add(response.instances[i].properties.???);         NEEDS WORK: How to get list of repos?
                        //Console.WriteLine(response.instances[i].instanceId + ": " + response.instances[i].properties.Location);
                    }
                }
                return repos;
            }
            catch (WebException exception)
            {
                Console.WriteLine (exception.ToString ());
            }
            catch (System.UriFormatException)
            {
            }
            return null;
        }
    }
}

namespace BentleyB0200.Dgn.DgnV8Mirror.ICS
    {
    using iModelHubRESTApi = iModelHub_REST_API_v2_4;

    /// <summary>
    /// DgnV8MirrorICSPlugin job definition extension.
    /// </summary>
    public class JobDefinitionExtension: ASJobDefinitionExtension
        {
        private string[] m_steps;

        /// <summary>
        /// Constructor
        /// </summary>
        public JobDefinitionExtension 
        (
        ) : base (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
                   new Guid (DgnV8MirrorICSPluginConstants.DocumentProcessorGuid),
                   DgnV8MirrorICSPluginConstants.DocumentProcessorDescription)
            {
            m_steps = new String[]
                {
                    DgnV8MirrorICSPluginConstants.Steps.DelegateExample
                };

            }

        /// *****************************************************************************
        /// <summary>
        /// Identifies the job types in a single job type per job def workflow.
        /// </summary>
        /// @author                                             AnthonyFalcone 03/08
        /// *****************************************************************************
        public override JobType[] GetJobTypes
        (
        )
            {
            return new JobType[] { new JobType (DgnV8MirrorICSPluginConstants.DocumentProcessorName,
                    DgnV8MirrorICSPluginConstants.DocumentProcessorGuid, DocumentProcessorGuid.ToString (), true, true) };
            }

        /// <summary>
        /// Provides a chance to load the current job definition.
        /// It is called when the document processor tab is initialized
        /// </summary>
        /// <param name="jd">current job definition</param>
        public override void OnLoad (JobDefinition jd)
            {
            System.Windows.Forms.Control myDocProcessorControl = null;

            // If the DgnV8MirrorICSPluginControls was already constructed use it.
            // The ExtendedProperties is a in memory hash map for application development.
            // It persists as long as the job defintion exists.  It is a good place to
            // store the controls for the document processors.
            if (jd.ExtendedProperties.Contains (DocumentProcessorGuid))
                myDocProcessorControl = (System.Windows.Forms.Control)jd.ExtendedProperties[DocumentProcessorGuid];

            /* Bridge job does not have user-supplied input parameters 
             
            // First time, construct a DgnV8MirrorICSPluginControls and store it in the ExtendedProperties
            // using the DocumentProcessorGuid as a unique key.
            if (null == myDocProcessorControl)
                {
                myDocProcessorControl = new DgnV8MirrorICSPluginControls (jd, DocumentProcessorGuid);
                jd.ExtendedProperties.Add (DocumentProcessorGuid, myDocProcessorControl);
                }

            */
            }

        /// <summary>
        /// Provides a customized windows control a user can populate
        /// with values used in DgnV8MirrorICSPlugin processing.  It is
        /// displayed in the Document Processor tab of the Job Builder dialog.
        /// </summary>
        /// <param name="jd">current job definition</param>
        /// <returns>DgnV8MirrorICSPlugin's window control</returns>
        public override System.Windows.Forms.Control GetWindowsControl (JobDefinition jd)
            {
			return null;
            // return (System.Windows.Forms.Control)jd.ExtendedProperties[DocumentProcessorGuid];
            }



        /// *****************************************************************************
        /// <summary>
        /// Called to validate a job definition before it is saved.
        /// </summary>
        /// <param name="jd">job definition</param>
        /// <param name="errorMessage">error found in job definition.</param>
        /// <returns>true, job definition will progree to save.
        /// false, the error found message will get displayed and the job definition will not get saved.</returns>
        /// @author                                             AnthonyFalcone 04/09
        /// *****************************************************************************
        public override bool OnValidate
        (
        JobDefinition jd,
        ref string errorMessage
        )
            {
            return true;
            }

        /// <summary>
        /// Provides a chance to persist the current job defintion.
        /// It is called when a job is saved through the Save As dialog.
        /// </summary>
        /// <param name="jd"></param>
        public override void OnSave (JobDefinition jd)
            {
            /*
             
            DgnV8MirrorICSPluginControls myDocProcessorControls;
            // Getting your window control.
            myDocProcessorControls = (DgnV8MirrorICSPluginControls)jd.ExtendedProperties[DocumentProcessorGuid];

            if (null != myDocProcessorControls)
                {
                // Permanently store the configuration data collected in the
                // DgnV8MirrorICSPluginConfigData in the job definition.
                jd.SetCustomData (DocumentProcessorGuid, myDocProcessorControls.ToXmlElement ());
                }

            */
            }



        /// <summary>
        /// Called when a job is about to get started
        /// </summary>
        /// <param name="jd">Job definition</param>
        /// <param name="stopJobMessage">If job is stop, message describing why</param>
        /// <returns></returns>
        public override bool OnJobStart (JobDefinition jd, ref string stopJobMessage)
            {
            return true;
            }

        /// <summary>
        /// Called when an array of all the steps for
        /// DgnV8MirrorICSPlugin is needed.
        /// </summary>
        /// <returns></returns>
        public override string[] GetSteps ()
            {
            return m_steps;
            }
        }
    }
