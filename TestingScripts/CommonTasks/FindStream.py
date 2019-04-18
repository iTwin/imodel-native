#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------

import os, sys
import xml.etree.ElementTree as ET

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin      10/2017
#-------------------------------------------------------------------------------------------
def GetStreamName():
    TREE_CONFIG_PATH = os.path.join(os.getenv('SrcRoot'), 'teamConfig', 'treeConfiguration.xml')
    Stream = FindStreamDetails(TREE_CONFIG_PATH)
    return Stream

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
def FindStreamDetails(TREE_CONFIG_PATH):
    Buildconfig=""
    Stream=""
    tree = ET.parse(TREE_CONFIG_PATH)
    root = tree.getroot()
    for child in root.iter('Stream'):
        Stream=child.attrib
    Stream=Stream['Name']
    return Stream

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    
    srcpath=sys.argv[1]
    RELATIVE_TREE_CONFIG_PATH = 'teamConfig' + os.sep + 'treeConfiguration.xml'
    TREE_CONFIG_PATH = os.path.join(srcpath,RELATIVE_TREE_CONFIG_PATH)
    Stream=FindStreamDetails(TREE_CONFIG_PATH)
    print Stream
    exit(0)