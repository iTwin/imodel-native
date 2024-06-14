#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
from __future__ import absolute_import
import os
from . import globalvars, utils

def getLanguageSpecificationsFile ():
    if hasattr (globalvars.programOptions, "languageSpecificationsFile"):
        return globalvars.programOptions.languageSpecificationsFile
    else:
        return os.path.join (os.path.dirname(os.path.abspath(__file__)), "languageSpecifications.xml")

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LanguageSettings (object):
    def __init__ (self, languageName):
        languageSpecificationsFile = getLanguageSpecificationsFile ()
        languageNode = None

        if not os.access (languageSpecificationsFile, os.R_OK):
            raise utils.BuildError ("Language Specification file cannot be Found : {0}".format (languageSpecificationsFile))

        try:
            xmlDom = utils.parseXml (languageSpecificationsFile)
            languageNode = next ((language for language in xmlDom.getElementsByTagName ("LanguageSpecification") if language.getAttribute ("Name").lower () == languageName.lower ()), None)
        except:
            raise utils.BuildError ("Language Specification file cannot be loaded : {0}".format (languageSpecificationsFile))

        if not languageNode:
            raise utils.BuildError ("Language Specifications cannot found for language {0} in file {1}".format (languageName, languageSpecificationsFile))


        self.m_name             = languageName.lower ()
        self.m_languageName     = utils.getDomElementValue (languageNode, "LanguageName")
        self.m_LCID             = utils.getDomElementValue (languageNode, "LCID")
        self.m_culture          = utils.getDomElementValue (languageNode, "Culture")
        self.m_codePage         = utils.getDomElementValue (languageNode, "Codepage")
        self.m_threeLetterWindowsName   = utils.getDomElementValue (languageNode, "ThreeLetterWindowsName")
        self.m_threeLetterISOName       = utils.getDomElementValue (languageNode, "ThreeLetterISOName")
        self.m_prgLanguageShortName = utils.getEnvOrDomElementValue (languageNode, "PRGLanguageShortName")
        self.m_wixCultureCode = utils.getEnvOrDomElementValue (languageNode, "WixCultureCode")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsNeutral (self):
        return "en" == self.m_culture.lower ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsPseudoLocalized (self):
        return "mr" == self.m_culture.lower ()