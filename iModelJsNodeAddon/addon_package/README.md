# @bentley/imodeljs-nodeaddonapi

Represents the default iModelJs node addon.

imodeljs-backend requires an imodeljs addon, and it relies on the app to provide the addon. Electron apps will need to acquire, deliver, and load an addon built for the electron environment, while node apps will need a nodejs-compatible addon. 

Apps that want to deliver the *default* imodeljs node addon should have a dependency on the imodeljs-nodeaddon package, along with their dependency on imodeljs-backend, like this:

`"@bentley/imodeljs-backend": "~3.1.6",`
`"@bentley/imodeljs-nodeaddon": "~1.2.2",`

The app must specify a version of the addon that meets the requirements of the specified version of imodeljs-backend. To find the minimum acceptable version, look at the version of imodeljs-nodeaddonapi that requires. To find that, run the following command in your app's folder:

`npm view @bentley/imodeljs-backend dependencies | findstr nodeaddonapi` 

Note that apps should use a ~ range to depend on imodeljs-backend and on imodeljs-addon. When an app wants to move up to a newer minor version of imodeljs-backend, it should change the minor version of its dependencies on both imodeljs-backend and imodeljs-nodeaddon in its package.json file, being careful to specify a version of the addon that is compatible with the chosen version of imodeljs-backend. To require a bug fix to the addon, the app should change the patch version of its dependency on imodeljs-nodeaddon. Note that bug fixes to the addon may be published independently of changes to imodeljs-backend.

The app must load and register the addon in its initialization logic, like this:

``` ts
import { NodeAddonLoader } from "@bentley/imodeljs-nodeaddon/NodeAddonLoader";
import { NodeAddonRegistry } from "@bentley/imodeljs-backend/lib/backend/NodeAddonRegistry";

NodeAddonRegistry.registerAddon(NodeAddonLoader.loadAddon());
```