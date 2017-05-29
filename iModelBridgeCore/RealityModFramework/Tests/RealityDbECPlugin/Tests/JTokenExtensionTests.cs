using IndexECPlugin.Source;
using Newtonsoft.Json.Linq;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    internal class JTokenExtensionTests
        {
        [Test]
        public void TryToGetStringTest ()
            {
            string json = @"
                {
                    CPU: 'Intel',
                    Drives: [
                        'DVD read/writer',
                        '500 gigabyte hard drive'
                    ]
                }";
            JObject obj = JObject.Parse(json);

            Assert.That(obj.TryToGetString("CPU"), Is.EqualTo("Intel"));
            Assert.That(obj.TryToGetString("RAM"), Is.EqualTo(null));
            }
        }
    }
