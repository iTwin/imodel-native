#--------------------------------------------------------------------------------------
#
#     $Source: mki/SplitEdmx.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, glob, sys, string

from xml.dom.minidom import parse
import xml.dom.minidom
# xml.tree is a mess with namespaces it generates ns0, ns1... unless we register all the namespaces. too much trouble we simply want to copy the node content.
#import xml.etree.ElementTree as ElementTree
#from lxml import etree

#-------------------------------------------------------------------------------------------
# bsimethod                                             Mathieu.Marchand  6/2015
#-------------------------------------------------------------------------------------------
def ExtractNodeFromDocument(outFilename, document, ParentName, ChildName):
    
    node = document.getElementsByTagName(ParentName)
    if node.length != 1: 
        print ("ERROR: Cannot find node '{0}'".format (ParentName))
        exit(1)

    childNode = node[0].getElementsByTagName(ChildName)
    if childNode.length != 1: 
        print ("ERROR: Cannot find node '{0}'".format (ChildName))
        exit(1)
    
    file_handle = open(outFilename,"wb")
    data = '<?xml version="1.0" encoding="utf-8"?>\n' + childNode[0].toxml(encoding="utf-8")
    file_handle.write(data)
    file_handle.close()
    
#-------------------------------------------------------------------------------------------
# bsimethod                                             Mathieu.Marchand  6/2015
#-------------------------------------------------------------------------------------------
def main():
 
    if (len (sys.argv) < 3):
        print "ERROR: Usage: ", os.path.basename(sys.argv[0]), " source outputdir"
        exit(1)

    edmxFilename = sys.argv[1]
    outputDir = sys.argv[2]

    edmxName = os.path.basename(edmxFilename)
    baseName = os.path.splitext (edmxName)[0]
             
    document = xml.dom.minidom.parse(edmxFilename)
	
    # ssdl
    ssdlFilename = os.path.join (outputDir, baseName + ".ssdl")
    ExtractNodeFromDocument(ssdlFilename, document, 'edmx:StorageModels', 'Schema')
    
    # csdl
    csdlFilename = os.path.join (outputDir, baseName + ".csdl")
    ExtractNodeFromDocument(csdlFilename, document, 'edmx:ConceptualModels', 'Schema')
    
    # msl
    mslFilename = os.path.join (outputDir, baseName + ".msl")
    ExtractNodeFromDocument(mslFilename, document, 'edmx:Mappings', 'Mapping')
    
if __name__ == '__main__':
    main()