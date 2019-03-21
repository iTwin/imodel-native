# Integration testing module for iModelBridges

## Feature Requiremnents

1. Ability to talk to iModel Bank and iModel Hub
2. Ability to run multi bridges in parallel
3. Ability to control which bridge gets tested.

## Plan

1. Write an orchestrator using nodejs and mocha
2. Initialize the set 
3. Extract information using custom logging xml

## Sequence

1. Read environment configuration from a yaml file. (TODO)
2. Download the required bridge application to the staging folder  (Done)
3. Start up iModel Bank if necessary  (Done)
4. General logging configuration file that will provide structured IPC between orchestrator and bridges (TODO)
6. We will use log4cxx xml format to collect the information. (TODO)
7. Invoke required bridges with correct config (Done)
8. Run applicable validation routines (TODO)
