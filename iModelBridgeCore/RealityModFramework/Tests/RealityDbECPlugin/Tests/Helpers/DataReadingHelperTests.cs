using System.Collections.Generic;
using System.Linq;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {

    [TestFixture]
    class DataReadingHelperTests
        {
        private DataReadingHelper dataReadingHelper;
        private MockRepository mocks;

        [SetUp]
        public void SetUp ()
            {
            dataReadingHelper = new DataReadingHelper();
            mocks = new MockRepository();
            }

        [Test]
        public void GetPropertiesTest ()
            {
            IECProperty instanceDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty instanceDataECPropertyStub2 = mocks.Stub<IECProperty>();
            IECProperty spatialInstanceDataECPropertyStub = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.instanceData, instanceDataECPropertyStub);
            dataReadingHelper.AddColumn(ColumnCategory.instanceData, instanceDataECPropertyStub2);
            dataReadingHelper.AddColumn(ColumnCategory.spatialInstanceData, spatialInstanceDataECPropertyStub);

            IEnumerable<IECProperty> properties = dataReadingHelper.GetProperties();

            Assert.That(properties.Count(), Is.EqualTo(3));
            Assert.That(properties.Contains(instanceDataECPropertyStub));
            Assert.That(properties.Contains(instanceDataECPropertyStub2));
            Assert.That(properties.Contains(spatialInstanceDataECPropertyStub));
            }

        [Test]
        public void AddColumnCurrentIndexIncreasetest ()
            {
            IECProperty instanceDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty spatialInstanceDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty streamDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty relatedInstanceIdECPropertStub = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.instanceData, instanceDataECPropertyStub);
            dataReadingHelper.AddColumn(ColumnCategory.spatialInstanceData, spatialInstanceDataECPropertyStub, 2);
            dataReadingHelper.AddColumn(ColumnCategory.streamData, streamDataECPropertyStub, 1000);
            dataReadingHelper.AddColumn(ColumnCategory.relatedInstanceId, relatedInstanceIdECPropertStub);

            Assert.That(dataReadingHelper.getInstanceDataColumn(instanceDataECPropertyStub), Is.EqualTo(0));
            Assert.That(dataReadingHelper.getInstanceDataColumn(spatialInstanceDataECPropertyStub), Is.EqualTo(1));
            Assert.That(dataReadingHelper.getStreamDataColumn(), Is.EqualTo(3));
            Assert.That(dataReadingHelper.getRelatedInstanceIdColumn(), Is.EqualTo(1003));
            }

        [Test]
        public void AddColumnInstanceDataTest ()
            {
            IECProperty instanceDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty spatialInstanceDataECPropertyStub = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.instanceData, instanceDataECPropertyStub);
            dataReadingHelper.AddColumn(ColumnCategory.spatialInstanceData, spatialInstanceDataECPropertyStub);

            Assert.That(dataReadingHelper.getInstanceDataColumn(instanceDataECPropertyStub), Is.EqualTo(0));
            Assert.That(dataReadingHelper.getInstanceDataColumn(spatialInstanceDataECPropertyStub), Is.EqualTo(1));
            }

        [Test]
        public void AddColumnNonPropertyDataExceptionTest ()
            {
            IECProperty ECPropertyStub = mocks.Stub<IECProperty>();
            Assert.That(() => dataReadingHelper.AddColumn(ColumnCategory.nonPropertyData, ECPropertyStub), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddColumnStreamDataColumnTest ()
            {
            IECProperty propertyStub = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.streamData, propertyStub);

            Assert.That(dataReadingHelper.getStreamDataColumn(), Is.EqualTo(0));
            Assert.That(() => dataReadingHelper.AddColumn(ColumnCategory.streamData, propertyStub), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddColumnRelatedInstanceIdTest ()
            {
            IECProperty propertyStub = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.relatedInstanceId, propertyStub);

            Assert.That(dataReadingHelper.getRelatedInstanceIdColumn(), Is.EqualTo(0));
            Assert.That(() => dataReadingHelper.AddColumn(ColumnCategory.relatedInstanceId, propertyStub), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddNonPropertyDataColumnTest ()
            {
            string firstColumnName = "first column name";
            string secondColumnName = "Second column name";

            dataReadingHelper.AddNonPropertyDataColumn(firstColumnName);
            dataReadingHelper.AddNonPropertyDataColumn(secondColumnName);

            Assert.That(dataReadingHelper.getNonPropertyDataColumn(firstColumnName), Is.EqualTo(0));
            Assert.That(dataReadingHelper.getNonPropertyDataColumn(secondColumnName), Is.EqualTo(1));
            }

        [Test]
        public void GetInstanceDataColumnTest ()
            {
            IECProperty instanceDataECPropertyStub = mocks.Stub<IECProperty>();
            IECProperty instanceDataECPropertyStub2 = mocks.Stub<IECProperty>();

            dataReadingHelper.AddColumn(ColumnCategory.instanceData, instanceDataECPropertyStub);

            Assert.That(dataReadingHelper.getInstanceDataColumn(instanceDataECPropertyStub), Is.EqualTo(0));
            Assert.That(dataReadingHelper.getInstanceDataColumn(instanceDataECPropertyStub2), Is.Null);
            }
        }
    }
