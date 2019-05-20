!
! gema patterns to pluck out instance method headers and generate
!    stub methods that are to be included in a wrapper class that has the
!    struct as an instance variable and needs to delegate the method call.
!

@define{@read{@getenv{GEMADIR}g\\domains.g}}
@define{@read{@getenv{GEMADIR}g\\needswork.g}}
@set{indexName;indexNameNotDefined}

\/\*\*\N\* \@struct <I> <T>\*\/=\
            @EmitTarget{$1}\
            @TranslateStructDescription{$0}\
                @EmitLinkToIndex{structs $1}\
            @Delimiter{}

\/\*\*\N\* \@doctext <T>\*\/=\
            @TranslateDocText{$0}\
            @Delimiter{}


PltrFuncFullname:<I>=mdlPlotter_$0

\/\*\*<T>\*\/\G\WPLTRFUNC\W(<RETURNTYPE>\W,\W<I>\W)<OPTIONALCOMMENT>\W(<arglist>)=\
@ProcessHeader{Comment(($1)) ReturnType(($2)) FuncName((@PltrFuncFullname{$3})) ArgList(($5))}

\/\*\*<T>\*\/\G\WsysprtFuncType <RETURNTYPE><FUNCNAME><OPTIONALCOMMENT>\W(<arglist>)=\
@ProcessHeader{Comment(($1)) ReturnType(($2)) FuncName(($3)) ArgList(($5))}

\/\*\*<T>\*\/\G\WPublic <RETURNTYPE><FUNCNAME><OPTIONALCOMMENT>\W(<arglist>)=\
@ProcessHeader{Comment(($1)) ReturnType(($2)) FuncName(($3)) ArgList(($5))}

\/\*\*<T>\*\/\G\WMdlPublic <RETURNTYPE><FUNCNAME><OPTIONALCOMMENT>\W(<arglist>)=\
@ProcessHeader{Comment(($1)) ReturnType(($2)) FuncName(($3)) ArgList(($5))}

\/\*\*<T>\*\/\G\WDLLEXPORT <RETURNTYPE><FUNCNAME><OPTIONALCOMMENT>\W(<arglist>)=\
@ProcessHeader{Comment(($1)) ReturnType(($2)) FuncName(($3)) ArgList(($5))}

ProcessHeader:Comment((<T>))\WReturnType((<T>))\WFuncName((<I>))\WArgList((<arglist>))=\
@set{indexName;$3}\
<a name=\"$3\"></a>\n\
@EnterFunctionNameInIndex{$3}\
<h3>$3</h3>\n\
<code>\n\
$2 $3\
<pre>\
(\
@TranslateParameterList{$4}\
)\
</pre>\
</code>\n\
<h4>Description</h4>\n\
@TranslateMainCommentWithParameterList{\
    @RemoveOldStyleCommentFluff{\
            @RemoveHFill{$1}}}\
@LinkToFunctionCategoryIndex{$3}\
\n</p>\n\
<hr>\n\
\n



?=

RETURNTYPE:<I>\W\*\W=$0@end
RETURNTYPE:<I>\W=$0@end
RETURNTYPE:=@end

! special function name replacment for printer code
FUNCNAME:AdjustNameAW\W(\W<I>\W)=sysprt$1@end
FUNCNAME:<I>=$0@end
FUNCNAME:=@fail

TranslateParameterList:\n\W\*=\n
TranslateParameterList:\IIN\I=
TranslateParameterList:\IOUT\I=
TranslateParameterList:\IINOUT\I=
TranslateParameterList:\I_prtTCHAR\I=char
TranslateParameterList:\IPOINT2D\I=Point2d
TranslateParameterList:\IDPOINT2D\I=DPoint2d
TranslateParameterList:?=?

StripComment:\n\W\*=\n
StripComment:/[-+=][-+=][-+=]*/=
StripComment:?=?

TranslateMainCommentWithParameterList:<T>\@param<T>=@TranslateMainComment{$1}\
    \n<h4>Parameters</h4><ul>@TranslateParams{\@param$2}
TranslateMainCommentWithParameterList:<T>=@TranslateMainComment{$0}
TranslateMainCommentWithParameterList:<T>=$0

TranslateParams:\B=\n<ul>
! Parameter followed by parameter:
TranslateParams:\@param<T>\@param<T>=<li>@TranslateParamBody{$1}</li>\
                                                                    \n@TranslateParams{\@param$2}
! Parameter followed by javadoc tag OTHER THAN a parameter:
TranslateParams:\@param<T>\@<T>=<li>@TranslateParamBody{$1}</li></ul>\n@TranslateMainComment{\@$2}
! Parameter with no following javadoc tag:
TranslateParams:\@param<T>=<li>@TranslateParamBody{$1}</li></ul>
TranslateParams:<T>=$0

TranslateParamBody:\<\=\>=<b>(in/out)</b>
TranslateParamBody:\<\==<b>(out)</b>
TranslateParamBody:\=\>=<b>(in)</b>
TranslateParamBody:?=?


RemoveOldStyleCommentFluff:\n\| \|\N=
RemoveOldStyleCommentFluff:\@instance\I=\@param
RemoveOldStyleCommentFluff:\n\| name <I> \|\N=
RemoveOldStyleCommentFluff:\n\| author <I> <T> \|\N=
RemoveOldStyleCommentFluff:?=?

RemoveHFill:\n\W\* \@bsihdr<T>\G\N=
RemoveHFill:\n\W\* \@bsimethod<T>\G\N=
RemoveHFill:\n/[-=+][-=+][-=+][-=+]+/=
RemoveHFill:\<pre\><T>\<\/pre\>=@FilterPreFormated{$0}
RemoveHFill:\n\W\*=
RemoveHFill:?=?

FilterPreFormated:\<pre\>=$0
FilterPreFormated:\<\/pre\>=$0
FilterPreFormated:\n\*=\n
FilterPreFormated:\>=\&gt\;
FilterPreFormated:\<=\&lt\;
FilterPreFormated:?=?


!TranslateMainComment:\/\*\*=<--- START_COMMENT--->
TranslateMainComment:\@return=<br><br>\n<b>Return</b>
TranslateMainComment:\@commentDisclaimer<T>\n=
TranslateMainComment:\@see\N=
TranslateMainComment:\@see<T>\N=                ! Naming problems for java versus native.
TranslateMainComment:\@param <I>=<br><i>$1</i>
TranslateMainComment:\@deprecated=<br><b><u>Warning\: This is a deprecated function.</u></b><br>
TranslateMainComment:\@remark<T>\n=
TranslateMainComment:\@indexVerb <I>\N=\
    \t<br>See also\: <a href=\"..\\index.html\#$1\" target="indexFrame">$1</a>\n\
    <\!--- indexEntry $1 @var{indexName} --->\n
TranslateMainComment:\@indexVerb\N=
TranslateMainComment:\W\n\*\W\n\*\W\n\*=<br>\n
TranslateMainComment:\W\n\*\W\n\*=<br>\n
TranslateMainComment:\W\n\*=\n!<br>\n


TranslateMainComment:?=?

EnterFunctionNameInIndex:sysprt<I>=\
    <\!--- indexEntry SystemPrinter sysprt$1 --->\n

EnterFunctionNameInIndex:mdlPlotter<I>=\
    <\!--- indexEntry PlotterInfo mdlPlotter$1 --->\n

! Special trap for awkardly named "boolean_" functions.
EnterFunctionNameInIndex:boolean_jmdl<I>_<I>=\
    <\!--- indexEntry $1 $0 --->\n\
    <\!--- indexEntry boolean $0 --->\n

EnterFunctionNameInIndex:jmdl<I>_<I>=\
    <\!--- indexEntry $1 $0 --->\n

EnterFunctionNameInIndex:mdl<I>_<I>=\
    <\!--- indexEntry $1 $0 --->\n

EnterFunctionNameInIndex:<I>_<I>=\
    <\!--- indexEntry $1 $0 --->\n

! No special linkage jumps for mdlXML functions
LinkToFunctionCategoryIndex:mdlXML<I>=

LinkToFunctionCategoryIndex:sysprt<I>=\
    <br>\n\tSee also\: <a href=\"..\\index.html\#SystemPrinter\" target="indexFrame">SystemPrinter</a><br>\n
LinkToFunctionCategoryIndex:mdlPlotter<I>=\
    <br>\n\tSee also\: <a href=\"..\\index.html\#PlotterInfo\" target="indexFrame">PlotterInfo</a><br>\n
LinkToFunctionCategoryIndex:jmdl<I>_<I>=\
    <br>\n\tSee also\: <a href=\"..\\index.html\#$1\" target="indexFrame">$1</a><br>\n
LinkToFunctionCategoryIndex:mdl<I>_<I>=\
    <br>\n\tSee also\: <a href=\"..\\index.html\#$1\" target="indexFrame">$1</a><br>\n
LinkToFunctionCategoryIndex:<I>_<I>=\
    <br>\n\tRelatedFunctions\: <a href=\"..\\index.html\#$1\" target="indexFrame">$1</a><br>\n

! No special linkage jumps for bare name
LinkToFunctionCategoryIndex:<I>=

! NewLine function -- Turn each character into 4 blanks on a new line, with tabs for 8
NewLine:????=\n\t\t
NewLine:???=\n\t\ \ \ \
NewLine:??=\n\t
NewLine:?=\n\ \ \ \


TranslateStructDescription:\@struct <I>=<H2>$1<\/H2>\n
TranslateStructDescription:\@fields \*<u> \@endfields==\n<h4>Fields</h4>\n<UL>\n\
    @TranslateFields{$1\@}\n<\/UL><P>
TranslateStructDescription:\/\*=
TranslateStructDescription:\/\*\*=
TranslateStructDescription:\n\*=
TranslateStructDescription:\@bsihdr<T>\n=
TranslateStructDescription:\@bsimethod<T>\n=
TranslateStructDescription:\@bsistruct<T>\n=
TranslateStructDescription:/[-+=]*\*+/\/=
TranslateStructDescription:?=?

TranslateDocText:\@doctext=
TranslateDocText:\n\*\W\<pre\><T>\<\/pre\>=@TranslatePreformatted{$0}
TranslateDocText:\/\*=
TranslateDocText:\/\*\*=
TranslateDocText:\n\*=
TranslateDocText:\@bsihdr<T>\n=
TranslateDocText:\@bsimethod<T>\n=
TranslateDocText:\@bsistruct<T>\n=
TranslateDocText:/[-+=]*\*+/\/=
TranslateDocText:?=?

TranslateFields:\@field <U>\P\@=\<li\>@TranslateFieldBody{$1}\n\</li\>\n
TranslateFields:\W=
TranslateFields:\@\Z=
TranslateFields:?=($0)

TranslateFieldBody:<FieldType>\W<FieldName><U>=@OutputType{$1} @OutputVarName{$2}  @RemoveHFill{$3}
TranslateFieldBody:*=$0

FieldType:<FieldBasicType>\W\[<u>\]=$0@end
FieldType:<FieldBasicType>\W\*=$0@end
FieldType:<FieldBasicType>\W\*\W\*=$0@end
FieldType:<FieldBasicType>=$0@end
FieldType:=@end

FieldBasicType:unsigned <I>=$0@end
FieldBasicType:<I>=$0@end
FieldBasicType:=@end

FieldName:<I>.<I>=$0@end
FieldName:<I>\W\[\]=$0@end
FieldName:<I>\W\[<U>\]=$0@end
FieldName:<I>=$0@end
FieldName:=@end

OutputType:<U>=@BeginElement{b}$0@EndElement{b}

OutputVarName:<U>=@BeginElement{b}$0@EndElement{b}
OutputVarName:?=(?)

BeginElement:<U>=\<$0\>

EndElement:<U>=\</$0\>


TranslatePreformatted:\n\*=\n
TranslatePreformatted:?=?

EmitLinkToIndex:<I> <I>=\n\
    <\!--- indexEntry $1 $2 --->\n\
    <br>\tSee also\: <a href=\"..\\index.html\#$1\" target="indexFrame">$1</a><br>\n

EmitTarget:<I>=\
        <a name="$1"><\/a>\n

Delimiter:=<hr>\n