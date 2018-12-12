/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/ContentDescriptor.hpp $
|    $RCSfile: ContentDescriptor.hpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/08/05 00:12:57 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/

inline TypeDescriptor::operator DataType () const 
    { return m_type; }  

inline const DataType& TypeDescriptor::GetType () const 
    { return m_type; } 

inline bool TypeDescriptor::operator< (const TypeDescriptor& rhs) const 
    { return GetType() < rhs.GetType(); }

inline bool TypeDescriptor::operator== (const TypeDescriptor& rhs) const 
    { return GetType() == rhs.GetType(); }


inline bool operator== (const TypeDescriptor&   lhs,
                        const DataType&         rhs)
    { return lhs.GetType() == rhs; }

inline bool operator== (const DataType&         lhs,
                        const TypeDescriptor&   rhs)
    { return lhs == rhs.GetType(); }

inline bool operator== (const TypeDescriptor&   lhs,
                        const DataTypeFamily&   rhs)
    { return lhs.GetType() == rhs; }

inline bool operator== (const DataTypeFamily&   lhs,
                        const TypeDescriptor&   rhs)
    { return lhs == rhs.GetType(); }


inline bool operator<  (const TypeDescriptor&   lhs,
                        const DataType&         rhs)
    { return lhs.GetType() < rhs; }

inline bool operator<  (const DataType&         lhs,
                        const TypeDescriptor&   rhs)
    { return lhs < rhs.GetType(); }

inline bool operator<  (const TypeDescriptor&   lhs,
                        const DataTypeFamily&   rhs)
    { return lhs.GetType() < rhs; }

inline bool operator<  (const DataTypeFamily&   lhs,
                        const TypeDescriptor&   rhs)
    { return lhs < rhs.GetType(); }