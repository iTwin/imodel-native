using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class IECInstanceExtensionTests
        {
        [Test]
        public void InitializePropertiesToNullWithNullClassDefinitionTest ()
            {
            MockRepository mocks = new MockRepository();
            IECInstance instanceStub = mocks.Stub<IECInstance>();

            using ( mocks.Record() )
                {
                SetupResult.For(instanceStub.ClassDefinition).Return(null);
                }
            using ( mocks.Playback() )
                {
                instanceStub.InitializePropertiesToNull();
                Assert.That(instanceStub.ClassDefinition, Is.Null);
                }
            }

        [Test]
        public void InitializePropertiesToNullWithNonNullClassDefinitionTest ()
            {
            IECSchema schema = SetupHelpers.PrepareSchema();
            IECInstance instance = SetupHelpers.CreateIndexSEWDV(schema);

            instance.InitializePropertiesToNull();

            foreach ( IECProperty property in instance.ClassDefinition )
                {
                Assert.That(instance[property.Name].IsNull);
                }
            }
        }
    }
