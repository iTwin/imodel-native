namespace FtpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class Thumbnail
    {
        public Thumbnail()
        {
            SpatialEntityBases = new HashSet<SpatialEntityBas>();
        }

        public int Id { get; set; }

        public string ThumbnailProvenance { get; set; }

        public string ThumbnailFormat { get; set; }

        public string ThumbnailWidth { get; set; }

        public string ThumbnailHeight { get; set; }

        public string ThumbnailStamp { get; set; }

        public byte[] ThumbnailData { get; set; }

        public string ThumbnailGenerationDetails { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual ICollection<SpatialEntityBas> SpatialEntityBases { get; set; }
    }
}
