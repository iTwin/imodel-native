namespace FtpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class WMSLayer
    {
        public WMSLayer()
        {
            WMSLayers1 = new HashSet<WMSLayer>();
        }

        public int Id { get; set; }

        public string Name { get; set; }

        public string Title { get; set; }

        public string Description { get; set; }

        public DbGeometry BoundingBox { get; set; }

        public string CoordinateSystems { get; set; }

        public int? ParentLayer_Id { get; set; }

        public int WMSServer_Id { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual ICollection<WMSLayer> WMSLayers1 { get; set; }

        public virtual WMSLayer WMSLayer1 { get; set; }

        public virtual WMSServer WMSServer { get; set; }
    }
}
