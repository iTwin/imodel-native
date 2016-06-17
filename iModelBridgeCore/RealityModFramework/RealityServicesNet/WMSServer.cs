namespace FtpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class WMSServer
    {
        public WMSServer()
        {
            WMSLayers = new HashSet<WMSLayer>();
        }

        [DatabaseGenerated(DatabaseGeneratedOption.None)]
        public int Id { get; set; }

        public string Title { get; set; }

        public string Description { get; set; }

        public int? NumLayers { get; set; }

        public string Version { get; set; }

        public string GetCapabilitiesURL { get; set; }

        public string SupportedFormats { get; set; }

        public string GetMapURL { get; set; }

        public string GetMapURLQuery { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual Server Server { get; set; }

        public virtual ICollection<WMSLayer> WMSLayers { get; set; }
    }
}
