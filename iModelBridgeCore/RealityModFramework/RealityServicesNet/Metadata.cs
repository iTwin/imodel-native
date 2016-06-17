namespace FtpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class Metadata
    {
        public Metadata()
        {
            SpatialEntityBases = new HashSet<SpatialEntityBas>();
        }

        public int Id { get; set; }

        public string RawMetadata { get; set; }

        [StringLength(50)]
        public string RawMetadataFormat { get; set; }

        public string DisplayStyle { get; set; }

        public string Description { get; set; }

        public string ContactInformation { get; set; }

        public string Keywords { get; set; }

        public string Legal { get; set; }

        public string Lineage { get; set; }

        public string Provenance { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual ICollection<SpatialEntityBas> SpatialEntityBases { get; set; }
    }
}
