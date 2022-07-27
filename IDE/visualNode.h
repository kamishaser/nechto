#pragma once
#include "GLM/glm.hpp"
#include "externalConnection.h"
#include "visual.h"
#include "visualAttribute.h"

namespace nechto::ide
{
	
	struct visualNode
	{
		glm::vec2 position{ 500,500 };
		glm::vec2 stepPosExchange{ 0,0 };
		glm::vec2 size{10,10};

		color shapeColor = color::White;
		color lightColor = color(0, 0, 0);
		geometricShape nShape;
		std::wstring nodeText = L"� ��������� ��� ��������?";

		externalConnection exCon;		
		visualNode()
			:exCon(nullNodePtr, L"nechto.ide.visualNode") {}

	};
}
