namespace HttpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class SpatialEntityDataset
    {
        public SpatialEntityDataset()
        {
            SpatialEntityBases = new HashSet<SpatialEntityBas>();
            SpatialEntityDatasets1 = new HashSet<SpatialEntityDataset>();
        }

        [DatabaseGenerated(DatabaseGeneratedOption.None)]
        public int Id { get; set; }

        public int? AlternateDataset_Id { get; set; }

        public bool Processable { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual ICollection<SpatialEntityBas> SpatialEntityBases { get; set; }

        public virtual SpatialEntityBas SpatialEntityBas { get; set; }

        public virtual ICollection<SpatialEntityDataset> SpatialEntityDatasets1 { get; set; }

        public virtual SpatialEntityDataset SpatialEntityDataset1 { get; set; }
    }
}
