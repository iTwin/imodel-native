/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/environment/embedded.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Environment from "./Environment";

/**
 * An environment delivered by an embedded instance of a JavaScript runtime.
 */
export class EmbeddedEnvironment extends Environment {

private EmbeddedEnvironmentTypeId;

}

/**
 * A Google V8 engine environment.
 */
export class V8Environment extends EmbeddedEnvironment {

private V8EnvironmentTypeId;

}

/**
 * A Microsoft Chakra engine environment.
 */
export class ChakraEnvironment extends EmbeddedEnvironment {

private ChakraEnvironmentTypeId;

}

/**
 * A WebKit JavaScriptCore engine environment.
 */
export class JscEnvironment extends EmbeddedEnvironment {

private JscEnvironmentTypeId;

}
