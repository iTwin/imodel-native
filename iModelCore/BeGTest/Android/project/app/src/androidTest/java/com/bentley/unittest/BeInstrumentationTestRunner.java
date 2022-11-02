/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
package com.bentley.unittest;

import android.test.AndroidTestRunner;
import android.test.InstrumentationTestRunner;

public class BeInstrumentationTestRunner extends InstrumentationTestRunner
	{
	BeAndroidTestRunner testRunner;
	
	@Override
	protected AndroidTestRunner getAndroidTestRunner() 
		{
		if (null == testRunner)
			testRunner = new BeAndroidTestRunner ();
		return testRunner;
		}
	}
