namespace FtpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class SpatialDataSource
    {
        public SpatialDataSource()
        {
            SpatialEntities = new HashSet<SpatialEntity>();
        }

        public int Id { get; set; }

        public string MainURL { get; set; }

        public string CompoundType { get; set; }

        public string LocationInCompound { get; set; }

        public string DataSourceType { get; set; }

        public string SisterFiles { get; set; }

        public int? Server_Id { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public long? FileSize { get; set; }

        public virtual OtherSource OtherSource { get; set; }

        public virtual Server Server { get; set; }

        public virtual WMSSource WMSSource { get; set; }

        public virtual ICollection<SpatialEntity> SpatialEntities { get; set; }
    }
}
