!!  Run gema with xyzrangetree.h as inputs -- place output in a tempoarary file.
!!  The temporary h file should match xyrangetree.h character by character.
!!  The two cpp files include from a common "h" file that contains highly macro-ized source.
!!  If you like templates (the cpp variety), you can consider templatizing so that there is
!!    one master for both source and .h files, templatized across both (a) dimension (2d/3d)
!!    and user data type (currently a painful void*).  If you do so, try to minimize the number
!!    of infernal <T> entries by coders writing application-specific tree traversers !!!!
!!
XYZRangeTree=XYRangeTree
DRange3d=DRange2d
DPoint3d=DPoint2d
?=?
