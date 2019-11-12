/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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