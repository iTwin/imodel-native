\B=@set{Indent1;\n\S\S\S\S}

PRIMITIVE <I>=\
@set{FieldNumber;0}\
message $1\
\n\{

END_PRIMITIVE=\n\}\n

\<field <u>\/\>=${Indent1}@ProcessField{$0}
\<list <u>\/\>=${Indent1}@ProcessList{$0}
\<child <u>\/\>=${Indent1}@ProcessChild{$0}
\<value<u>\/\>=${Indent1}@ProcessValue{$0}
?=

ProcessField:\A=@incr{FieldNumber}
ProcessField:ItemType\=<I>=required @MapType{$1}
ProcessField:ItemParamName\=<I>= $1 \=\ ${FieldNumber}\;
ProcessField:?=

ProcessList:\A=@incr{FieldNumber}
ProcessList:ItemType\=<I>=repeated @MapType{$1}
ProcessList:ItemSemanticName\=<I>= $1 \=\ ${FieldNumber}\;
ProcessList:?=

ProcessChild:\A=@incr{FieldNumber}
ProcessChild:ItemType\=<I>=required @MapType{$1}
ProcessChild:ItemSemanticName\=<I>= $1 \=\ ${FieldNumber}\;
ProcessChild:?=

ProcessValue:\A=@incr{FieldNumber}
ProcessValue:ItemType\=<I>=required @MapType{$1}
ProcessValue:ItemSemanticName\=<I>= $1 \=\ ${FieldNumber}\;
ProcessValue:?=



\E=\n

MapType:\Iint\I=int32
MapType:\IString\I=string
MapType:?=?
