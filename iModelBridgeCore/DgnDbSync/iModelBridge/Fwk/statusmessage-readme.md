# Status and Progress Messages #

iModelBridgeFwk.exe takes the following argument:

```
--fwk-status-message-sink-url=<URL>
```

This an optional command-line argument. If supplied, it specifies the URL of a WebServer that will process progress meter and status messages.

The framework will POST to the supplied URL. The requests will have JSON bodies. There are two kinds: status messages and progress messages. Their layouts are as follows:

## Status message JSON body:
```ts
{
	messageType: “status”,
	message: string; // The brief message
	details: string; // The message details
	isError: boolean; // Set to true if the message is about an error. Otherwise, the message is just for information.
}
```

## Progress message JSON body:
```ts
{
  messageType: “progress”,
  jobRequestId: string; // The value passed in the --fwk-jobrequest-guid= command line argument
  jobRunCorrelationId: string;  // The value passed in the --fwk-jobrun-guid= command line argument
  phase: string; // the name of the phase currently in progress
  step: string; // the name of the step currently in progress
  task: string; // the name of the task currently in progress
  spinCount: number; // an increasing counter that indicates activity within the current task
  phasesPct: number; // phases completed so far, as a percentage, that is, a number between 0 and 100
  stepsPct: number; // steps completed so far, as a percentage, that is, a number between 0 and 100
  tasksPct: number; // tasks completed so far, as a percentage, that is, a number between 0 and 100
  lastUpdateTime: number; // time of last POST
  phaseCount?: number; // the total number of phases
  stepCount?: number; // the total number of steps
  taskCount?: number; // the total number of tasks  
}
```