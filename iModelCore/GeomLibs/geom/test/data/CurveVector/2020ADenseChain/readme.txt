aecc_alignment.imjs is:
* sorted more-or-less head to tail
* many paths with single arc or linestring
* many paths with many, many, many arcs.
* with gaps
* with many zero-length segments
* with many near-zero-length arcs

two rounds of the imjsTypes file isolates 143 paths
gema -f imjsTypes.g aecc_alignment.imjs | gema -f imjsTypes.g | more