namespace HttpTraversalEngineNet
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel.DataAnnotations;
    using System.ComponentModel.DataAnnotations.Schema;
    using System.Data.Entity.Spatial;

    public partial class Server
    {
        public Server()
        {
            SpatialDataSources = new HashSet<SpatialDataSource>();
        }

        public int Id { get; set; }

        public string CommunicationProtocol { get; set; }

        public string Name { get; set; }

        public string URL { get; set; }

        public string ServerContactInformation { get; set; }

        public string Fees { get; set; }

        public string Legal { get; set; }

        public string AccessConstraints { get; set; }

        public bool Online { get; set; }

        public DateTime LastCheck { get; set; }

        public DateTime LastTimeOnline { get; set; }

        public double Latency { get; set; }

        public int MeanReachabilityStats { get; set; }

        public string State { get; set; }

        public string Type { get; set; }

        [DatabaseGenerated(DatabaseGeneratedOption.Computed)]
        [StringLength(50)]
        public string IdStr { get; set; }

        public virtual ICollection<SpatialDataSource> SpatialDataSources { get; set; }
    }
}
