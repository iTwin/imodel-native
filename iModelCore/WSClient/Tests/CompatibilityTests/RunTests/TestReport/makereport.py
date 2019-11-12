#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import lxml.etree as ET
import sys
import os

if len(sys.argv) != 2:
    print("Usage: in.xml")
    exit(1)

xmlFilePath = sys.argv[1]
dom = ET.parse(xmlFilePath)
xslt = ET.parse(os.path.dirname(os.path.realpath(__file__)) + "/gtest.xsl")
transform = ET.XSLT(xslt)
newdom = transform(dom)
print(ET.tostring(newdom, pretty_print=True))