# @bentley/imodeljs-nodeaddonapi

Represents the default iModelJs node addon.

imodeljs-backend uses an imodeljs addon. 

An app that depends on and uses imodeljs-backenod must load the addon and then register it. A nodejs service or agent app do the following logic in its initialization code:

``` ts
import { NodeAddonLoader } from "@bentley/imodeljs-nodeaddon/NodeAddonLoader";
import { NodeAddonRegistry } from "@bentley/imodeljs-backend/lib/backend/NodeAddonRegistry";

NodeAddonRegistry.registerAddon(NodeAddonLoader.loadAddon());
```