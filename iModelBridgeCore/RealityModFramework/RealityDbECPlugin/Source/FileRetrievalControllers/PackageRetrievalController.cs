using Bentley.EC.Persistence;
using Bentley.EC.Persistence.FileSystemResource;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
//using Newtonsoft.Json;
//using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.FileRetrievalControllers
{
    internal class PackageRetrievalController : FileResourceRetrievalController
    {
        string m_packagesLocation;

        public PackageRetrievalController
            (
            IECInstance instance,
            FileResourceManager manager,
            RetrieveBackingFileOperation operation,
            string packagesLocation
            )
            : base(instance, manager, operation)
        {
            m_packagesLocation = packagesLocation;
        }

        public override void DoRetrieveFile(bool transferFile)
        {

            string location = Path.Combine(m_packagesLocation, Instance.InstanceId);

            FileInfo fileInfo = new FileInfo(location);
            if (!fileInfo.Exists)
            {
                throw new UserFriendlyException("There is no such file associated to this instance");
            }

            DateTime WriteTime = fileInfo.LastWriteTimeUtc;

            var fileBackedDescriptor = new FileBackedDescriptor(ResourceManager.Connection.RepositoryIdentifier.ECPluginID, fileInfo.FullName, WriteTime);

            FileBackedDescriptorAccessor.SetIn(Instance, fileBackedDescriptor);
            ResourceManager.SetAsSynchronizedWithRepository(Instance, ResourceManager.Connection.RepositoryIdentifier.ECPluginID, fileBackedDescriptor.RelativePath, WriteTime, transferFile);

        }

        public override bool InstanceRequiresLockForLocalFileModifications
        {
            get
            {
                return false;
            }
        }

        public override bool ObtainingLockDuringRetrieval
        {
            get
            {
                return false;
            }
        }
    }
}

