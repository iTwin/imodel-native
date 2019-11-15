/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"
#include <ptedit/editevaluatedstacknode.h>
#include <ptedit/editstackstate.h>

using namespace ptedit;

		
bool EvaluatedStackNode::apply()
{
	return EvaluatedOpStack::Apply( m_file );
}
bool EvaluatedStackNode::readState(const pt::datatree::Branch *b)
{
	return b->getNode("name", m_name ); 
}
bool EvaluatedStackNode::writeState(pt::datatree ::Branch *b) const
{
	return b->addNode("name", m_name ); 
}
