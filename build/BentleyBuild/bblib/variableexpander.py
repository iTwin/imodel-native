#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
from . import utils

# Number of recursions to do before we break and assume infinite recursion
DEPTH_LIMIT     = 512

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class VariableExpanderInfoParen (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self):
        self.VariableStart  = "$("
        self.VariableEnd    = ")"
        self.VariableEscape = "$"

        self.StartLength = len(self.VariableStart)
        self.EndLength = len(self.VariableEnd)
        self.EscapeLength = len(self.VariableEscape)
        # Minimum length necessary for a string to be expandable
        self.MinimumLength = self.StartLength + self.EndLength + 1

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class VariableExpanderInfoCurly (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self):
        self.VariableStart  = "${"
        self.VariableEnd    = "}"
        self.VariableEscape = "$"

        self.StartLength = len(self.VariableStart)
        self.EndLength = len(self.VariableEnd)
        self.EscapeLength = len(self.VariableEscape)
        # Minimum length necessary for a string to be expandable
        self.MinimumLength = self.StartLength + self.EndLength + 1

PAREN_INFO = VariableExpanderInfoParen()
CURLY_INFO = VariableExpanderInfoCurly()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CreateVariableList (obj, bindings):
    vardict = {}
    for (varName, binding) in bindings:
        if binding.startswith ("self.") and None == obj:
            raise utils.FatalError (message='RegisterVariables cannot reference "self" in a binding if no object is specified')

        vardict[varName.lower()] = (obj, binding)
    return vardict

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class VariableExpander (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self):
        pass

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandString (self, varString, expInfo, varDict, stratDefaultExpansionVars, depth=0):
        if not expInfo.VariableStart in varString:
            return varString

        strLen = len (varString)
        if strLen < expInfo.MinimumLength:
            return varString

        if depth > DEPTH_LIMIT:
            raise utils.StrategyError ("Reached the maximum recursion limit ({0}) attempting to expand string \"{1}\"".format (DEPTH_LIMIT, varString))

        # Positions of start sequences we've seen so far
        leftIndices = []

        i = 0
        while (i < strLen):
            if (expInfo.VariableStart == varString [i:(i+expInfo.StartLength)]) and ((i < expInfo.EscapeLength) or (expInfo.VariableEscape != varString [i-expInfo.EscapeLength:i])):
                leftIndices.append (i)

            elif (expInfo.VariableEnd == varString[i:i+expInfo.EndLength]) and (len (leftIndices) > 0):
                leftIndex = leftIndices.pop ()

                originalVar = varString[leftIndex+expInfo.StartLength:i]
                expandedVar = None
                if varDict:
                    expandedVar = varDict.get(originalVar.lower(), None)
                if None == expandedVar:
                    expandedVar = self.GetValue (originalVar, stratDefaultExpansionVars)

                # GetValue returns None if there was no expansion found for this variable.
                if None == expandedVar:
                    expandedMiddle = expInfo.VariableStart + originalVar + expInfo.VariableEnd
                else:
                    expandedMiddle = self.ExpandString (expandedVar, expInfo, varDict, stratDefaultExpansionVars, depth=depth+1)

                varString = varString[:leftIndex] + expandedMiddle + varString[i+1:]

                # adjust i to point to the end of the newly expanded string
                i += (len (expandedMiddle) - (i+1-leftIndex))
                strLen = len (varString)

            i += 1

        return varString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandCurlyBraceString (self, varString, varDict, stratDefaultExpansionVars):
        return self.ExpandString (varString, CURLY_INFO, varDict, stratDefaultExpansionVars)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandParenString (self, varString, varDict, stratDefaultExpansionVars):
        return self.ExpandString (varString, PAREN_INFO, varDict, stratDefaultExpansionVars)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RunBinding (self, bindObject, bindStr):
        bindValue = None
        bindStr = bindStr.strip()

        # If the binding starts with "self.", retrieve that attribute from bindObject
        if bindStr.startswith ("self."):
            bindStr = bindStr[5:]

            if bindStr.endswith ("()"):
                # This supports functions with no arguments right now, which should be enough in
                # the cases we need to call functions for. It could be extended to parse
                # arguments in the future.
                funcAttr = getattr (bindObject, bindStr[:-2])
                bindValue = funcAttr()
            else:
                bindValue = getattr (bindObject, bindStr)

        # Otherwise, bindObject isn't used and we execute bindStr in the global scope
        else:
            # This is unused (and probably unnecessary) and causes pylint warnings. Can't fix it without a test case.
            # The point is to call an arbitrary function as opposed to a class method which will return a value.
            utils.showInfoMsg ('Variable Expander hit a value it could not deal with: "{0}"'.format (bindStr), utils.INFO_LEVEL_Essential)
#            exec ("bindValue = " + bindStr)

        return str (bindValue)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetValue (self, varName, stratDefaultExpansionVars):
        if not stratDefaultExpansionVars:
            return
        varLower = varName.lower()
        if varLower in stratDefaultExpansionVars:
            binding = stratDefaultExpansionVars[varLower]
            try:
                return self.RunBinding (binding[0], binding[1])
            except Exception as error:
                raise utils.StrategyError ("could not resolve variable \"{0}\" - {1}".format (varName, error.args[0]))
        else:
            return None
