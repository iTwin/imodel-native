#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import os
import shutil
import json
import argparse

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    12/2018
#-------------------------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--bridge_latest", help="Path of output from Latest Bridge Build run", required=True)
    parser.add_argument("--bridge_prev", help="Path of output from Previous Bridge Build run", required=True)
    parser.add_argument("--bridge_base", help="Path of output from a Baseline Bridge Build run", required=True)    
    parser.add_argument("--json_path", help="Full path of dataset.json file to be updated. If not given, uses the default")
    args = parser.parse_args()

    if (args.json_path):
        json_file = args.json_path
    else:
        json_file = "../config/mstnbridge.dataset.json"
    data_file = open(json_file)
    data = json.load(data_file)
    vals = {"latest": args.bridge_latest, "previous": args.bridge_prev, "baseline": args.bridge_base}
    root_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))), data["root"])
    if not os.path.exists(root_dir):
        os.mkdir(root_dir)
    print root_dir
    for val in vals:
        copy_path = os.path.join(root_dir, val)
        # first cleanup dirs
        if os.path.exists(copy_path):
            shutil.rmtree(copy_path)
        os.mkdir(copy_path)
        # now copy
        print "Copying " + val + " iModels to: " + copy_path
        for root, dirs, files in os.walk(vals[val]):
            for name in files:
                if name.endswith(".bim"):
                    bim_path = (os.path.join(root, name))
                    print "Copying " + bim_path
                    shutil.copy2(bim_path, copy_path)
    

main()