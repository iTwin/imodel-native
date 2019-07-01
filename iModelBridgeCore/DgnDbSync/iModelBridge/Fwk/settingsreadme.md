# iModelBridge Settings #

The persistence of iModel Bridge Settings is in the product settings service
https://dev-connect-productsettingsservice.bentley.com/swagger/index.html 

## Design ##
- The service supports both SAML and JWT tokens. Use the following OIDC scopes: openid profile email org product-settings-service. The service does not support OIDC client credentials flow (yet). JWT tokens must contain a valid IMS user.
- You must decide which partition to store your settings. Each partition can store up to 10 GB of data. In most cases, that should not be a major concern but should be considered. More importantly, the partition strategy you choose will impact who can access your settings.
- You should also avoid getting all settings for this endpoint. Always get specific settings or filter by namespace so that you only get the settings that you care about.

Based on the above guidelines

The model we are using is 
```
    Application/{applicationid}/Context/{ContextId}/iModel/{imodelid}/Setting/{SettingNamespacespace}/{settingname}
```

In general
- applicationid for iModelBridgeFwk == 2661
- ContextId = ProjectGUID

## Types of Settings ##
The different kinds of settings defined for a bridge are

1. iModelBridgeFwk global settings : This will be stored under GPRId of bridgefwk : 2661
2. Bridge specific settings: This will be stored under bridge specific GPRId


### iModelBridgeFwk settings ###

1. DocumentMapping (Namespace)
   1. DmsSource (Setting): This setting describes the location of the files that are input to the bridge service.
	```
	REST API: Application/{applicationid}/Context/{ContextId}/iModel/{imodelid}/Setting/DocumentMapping/DmsSource
	Body:
	{
	  "properties": {
		"Version" : 1.0,
		"DmsSource": [{
			"Id" : "DMSGUID",
			"Type" : "Projectwise",
			"DMSUrl" : "imodeleap-pw.bentley.com:imodeleap-pw-01"
			}
		]
	  }
	}

	``` 
    2. Documents (Setting) : This setting describes the spatial root file and all the master model files that are input to a bridge.
    ```
	REST API: Application/{applicationid}/Context/{ContextId}/iModel/{imodelid}/Setting/DocumentMapping/Documents
	{
	  "properties": {
		"Version" : 1.0,
		"Documents": [ {
			"Id" : "DocumentGUID",
			"DmsId" : "DMSGUID",
			"spatialRootTransform" : null,
			"mappingType": "spatialRoot",
			"documentUrn": "pw://Mstn_IMB.bentley.com:Bridge_DS/Documents/D{483ab7f5-16b0-40a4-bf38-11cad0c21df9}",
			"Bridge Assignment" : "Microstation", //Can be used as an override or a place to store.
			"BriefcaseId" : 2
		}]
	  }
	}
	```

### Bridge Specific Settings ###
eg. Microstation bridge will translate all the contents inside importconfig.xml into here.