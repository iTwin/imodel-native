/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/Environment.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Host from "../host/Host";

/**
 * An iModel.js runtime environment.
 */
export default class Environment {

private EnvironmentTypeId;
private m_host : Host;

constructor (host : Host)
    {
    this.m_host = host;
    }

}
