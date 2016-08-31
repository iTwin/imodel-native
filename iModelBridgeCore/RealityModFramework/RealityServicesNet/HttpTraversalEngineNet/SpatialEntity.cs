namespace HttpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class SpatialEntity
    {
        public SpatialEntity()
        {
            SpatialDataSources = new HashSet<SpatialDataSource>();
        }

        [DatabaseGenerated(DatabaseGeneratedOption.None)]
        public int Id { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual SpatialEntityBas SpatialEntityBas { get; set; }

        public virtual ICollection<SpatialDataSource> SpatialDataSources { get; set; }
    }
}
