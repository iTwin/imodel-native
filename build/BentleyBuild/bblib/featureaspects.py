#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, xml

from . import symlinks, utils

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class FeatureAspect (object):
    def Disable(self):
        self.m_allow = False
        if None != self.m_subAspects:
            for subaspect in self.m_subAspects.values():
                subaspect.Disable()

    def Enable(self):
        self.m_allow = True
        if None != self.m_subAspects:
            for subaspect in self.m_subAspects.values():
                subaspect.Enable()

    def AddIfDisabled (self, omitSet, parentName):
        thisName = parentName + "_" + self.m_name
        if self.m_allow == False:
            omitSet.add (thisName)
        if None != self.m_subAspects:
            for aspect in self.m_subAspects.values():
                aspect.AddIfDisabled (omitSet, thisName)

    def __init__(self, dom, name):
        self.m_allow = True
        self.m_subAspects = None
        self.m_name = name

        subAspects = utils.getDomElementsByName (dom, "FeatureAspect")
        if len(subAspects) > 0:
            self.m_subAspects = dict()
            for subAspect in subAspects:
                name = subAspect.getAttribute("name")
                self.m_subAspects[name] = FeatureAspect (subAspect, name)

        defaultVal = dom.getAttribute ("default")
        if "" != defaultVal and defaultVal.lower() == "disallowed":
            self.Disable()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def setAllowedFlag (featureAspects, aspect, faFile):
    for node in aspect.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        value = node.getAttribute("value").lower()
        if not node.nodeName in featureAspects:
            raise utils.BuildError("Feature Aspect {0} not found in existing Feature Aspects while processing {1}".format(node.nodeName, faFile))
        thisAspect = featureAspects[node.nodeName]
        if "" != value:
            if value == "disallowed":
                thisAspect.Disable()
            elif value == "allowed" or value == "hidden":
                thisAspect.Enable()
            else:
                msg = "Warning: FeatureAspect {0} has value '{1}' which is not disallowed, allowed, or hidden. \n Enabling in bentleybuild. Please check {2} in case it is incorrect.".format(node.tagName, value, faFile)
                # The feature aspect should still be symlinked to the output product and handled accordingly.
                thisAspect.Enable() 
                utils.showInfoMsg (msg, utils.INFO_LEVEL_RarelyUseful, utils.YELLOW)

        if None != thisAspect.m_subAspects:
            setAllowedFlag (thisAspect.m_subAspects, node, faFile)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def processProductAspects (faFile, featureAspects):

    faFile = symlinks.normalizePathName(faFile)
    if not os.path.isfile (faFile):
        raise utils.BuildError("FeatureAspects File not found {0}: ".format(faFile))

    try:
        faDom = utils.parseXml (faFile)
    except xml.parsers.expat.ExpatError as errIns:
        raise utils.BuildError("Parse Error\n    {0}: ".format(faFile) + errIns.__str__())

    product = utils.getDomElementsByName (faDom, "PowerProduct")
    aspects = utils.getDomElementsByName (product[0], "FeatureAspects")
    for aspect in aspects:
        setAllowedFlag (featureAspects, aspect, faFile)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def interpretFeatureAspects (omitSet, templateFile, faFile):
    templateFile = symlinks.normalizePathName(templateFile)
    if not os.path.isfile (templateFile):
        raise utils.BuildError("FeatureAspects TemplateFile not found {0}: ".format(templateFile))

    try:
        templateDom = utils.parseXml (templateFile)
    except xml.parsers.expat.ExpatError as errIns:
        raise utils.BuildError("Parse Error\n    {0}: ".format(templateFile) + errIns.__str__())

    featureAspects = dict()

    faNode = utils.getDomElementsByName (templateDom, "FeatureAspects")
    fAspects = utils.getDomElementsByName (faNode[0], "FeatureAspect")
    for aspect in fAspects:
        name = aspect.getAttribute("name")
        featureAspects[name] = FeatureAspect(aspect, name)

    processProductAspects (faFile, featureAspects)

    for name,aspect in featureAspects.items():
        aspect.AddIfDisabled (omitSet, "PP_ID_FeatureAspects")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():
    omitSet = set()
    templateFile = "D:\\work\\prostructures\\FeatureAspectsTemplate.xml"
    instanceFile = "D:\\work\\prostructures\\ProStructures.xml"
    interpretFeatureAspects (omitSet, templateFile, instanceFile)

if __name__ == '__main__':
    main()
