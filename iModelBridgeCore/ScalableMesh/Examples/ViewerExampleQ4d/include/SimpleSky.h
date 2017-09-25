#pragma once

#pragma once
#include <gl/glew.h>
#include "ExampleApp.h"
// simple sky rendering

class SimpleSky : public Tool
{
public:
	SimpleSky();

	void preDrawSetup();
	void drawPreDisplay();
	void drawPostDisplay();

private:

	void	setup();
};

