# MicroStation Bridge smoke tests

This contains tests for MicroStation Bridge smoke dataset to compare results from separate runs. The actual comparison uses the data-validation-api.

## How to run

1. First clone the tooling-scripts repository.

2. Once you have source, cd to data-validation/mstn-bridge directory.

3. Install the dependencies
  ```sh
  npm install
  ```

4. Build the source and tests
  ```sh
  npm run build
  ```
5. Run tests that run Mstn Bridge smoke data validationtests
  ```sh
  npm test
  ```
6. It will produce an html report in lib that shows results of comparison. This report shows results of Latest, Previous and Baseline iModels for a pre-defined datasets.

A command npm run test:multip is WIP for multi-bridge testing workflow.

## Helper scripts

To setup test data coming from bridge runs, there is a helper Python script in src/scripts.

For setting up data from Bridge output:
  ```sh
  setup-data.py --bridge_latest=D:\\bridge-data\\FromBridge1 --bridge_prev=D:\\bridge-date\\FromBridge2 --bridge_base=D:\\bridge-data\\base
  ```

## Datasets

The datasets are managed through a json file. This contains a list of datasets to be identified from bridge runs and it contains a name, description and list of .dgn or .dwg files to be located in the iModel. Since runs from a Bridge gives a GUID as name of the iModels, this helps in finding which datasets are to be compared.

Currently two separate dataset.json files are being maintained for MSTN-bridge and multi-bridge testing.