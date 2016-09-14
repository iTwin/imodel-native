/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/lib/Bentley/BimTiles.js $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//=======================================================================================
//! Interface for working with BIM tiles in Cesium.
// @bsistruct                                                   Paul.Connelly   09/16
//=======================================================================================
var BimTiles = {

};

/*---------------------------------------------------------------------------------**//**
* Fixes up sandbox attribute on cesium info box iframes to enable loading of json tilesets etc.
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BimTiles.fixupSandboxAttributes = function() {
    // They want to disallow scripts, but need to be able to load .json tilesets...
    var iframes = document.getElementsByClassName('cesium-infoBox-iframe');
    for (var i = 0; i < iframes.length; i++) {
        iframes[i].removeAttribute('sandbox');
    }
};

/*---------------------------------------------------------------------------------**//**
* Adjust tileset to terrain height based on ground point.
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BimTiles.adjustTilesetHeight = function(viewer, tileset, groundPoint) {
    var terrainProvider = viewer.terrainProvider;
    if (!Cesium.isDefined(terrainProvider)) {
        return;
    }

    var carto = Cesium.Cartographic.fromCartesian(groundPoint);
    var positions = [ carto ];
    var currentHeight = carto.height;

    Cesium.SampleTerrain(terrainProvider, 15, positions).then(function() {
        var targetHeight = positions[0].height;
        var heightDelta = targetHeight - currentHeight;

        var deltaMagnitude = heightDelta / Cesium.Cartesian3.magnitude(groundPoint);
        var delta = new Cesium.Cartesian3();
        Cesium.Cartesian3.multiplyByScalar(groundPoint, deltaMagnitude, delta);

        tileset.modelMatrix[12] += delta.x;
        tileset.modelMatrix[13] += delta.y;
        tileset.modelMatrix[14] += delta.z;
    });
};

/*---------------------------------------------------------------------------------**//**
* Set up default options to pass to Cesium.Viewer ctor based on bim view.
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BimTiles.createCesiumViewerOptions = function(view) {
    var options = {
        infoxBo: true,
        shadows: false,
        contextOptions: {
            webgl: {
                alpha: true
            }
        }
    };

    if (!view.geolocated) {
        options.globe = false;
        options.scene3DOnly = true;
        options.skyBox = false;
        options.skyAtmosphere = false;
    }

    return options;
};

/*---------------------------------------------------------------------------------**//**
* Loads a tileset into a Cesium.Viewer.
* @param {Viewer} viewer The viewer into which to add the tileset
* @param {Object} view The description of the tileset
* @param {Function} callback Callback invoked when tileset is ready. Receives no arguments.
* @param {String} [view.url] The tileset's URL (e.g. "MyTileset.json")
* @param {Object} [view.dest] view destination coords as {x,y,z}
* @param {Object} [view.dir] view direction vector as {x,y,z}
* @param {Object} [view.up] view up vector as {x,y,z}
* @param {Boolean} [view.geolocated=false] Whether the view is geolocated or not.
* @param {Object} [view.groundPoint=undefined] Ground point coords as {x,y,z}
* @param {Number} [view.maximumScreenSpaceError=2] Max screen space error
* @param {Number} [view.maximumNumberOfLoadedTiles=1000]
* @param {Boolean} [view.debugShowBoundingVolumes=false]
* @param {Boolean} [view.refineToVisible=true]
* @return The tileset. The tileset is not fully ready for use until the callback is invoked.
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
BimTiles.loadTileset = function(viewer, view, callback) {
    var scene = viewer.scene;
    var tileset = scene.primitives.add(new Cesium.Cesium3DTileset({
        url: view.url,
        maximumScreenSpaceError: Cesium.defaultValue(view.maximumScreenSpaceError, 2),
        maximumNumberOfLoadedTiles: Cesium.defaultValue(view.maximumNumberOfLoadedTiles, 1000),
        debugShowBoundingVolumes: Cesium.defaultValue(view.debugShowBoundingVolumes, false),
        refineToVisible: Cesium.defaultValue(view.refineToVisible, true)
    }));

    Cesium.when(tileset.readyPromise).then(function(cbTileset) {
        var tf = undefined;
        if (!view.geolocated) {
            var boundingSphere = tileset.boundingSphere;
            var mtx = new Cesium.Matrix4();
            tf = Cesium.Transforms.eastNorthUpToFixedFrame(boundingSphere.center, Cesium.Ellipsoid.WGS84, mtx);
        }

        viewer.camera.setView({
            destination: new Cesium.Cartesian3(view.dest.x, view.dest.y, view.dest.z),
            orientation: {
                direction: new Cesium.Cartesian3(view.dir.x, view.dir.y, view.dir.z),
                up: new Cesium.Cartesian3(view.up.x, view.up.y, view.up.z)
            },
            endTransform: tf
        });

        if (Cesium.isDefined(view.groundPoint)) {
            scene.globe.depthTestAgainstTerrain = false;

            var groundPoint = new Cesium.Cartesian3(view.groundPoint.x, view.groundPoint.y, view.groundPoint.z);
            BimTiles.adjustTilesetHeight(viewer, tileset, groundPoint);

            scene.terrainProviderChanged.addEventListener(function() {
                BimTiles.adjustTilesetHeight(viewer, tileset, groundPoint);
            });
        }

        if (Cesium.isDefined(callback)) {
            callback();
        }
    });

    return tileset;
};

