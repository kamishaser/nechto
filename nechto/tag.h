#pragma once
#include "node.h"

namespace nechto
{
	namespace tag
	{
		size_t getUniqueID()
		{
			static size_t ID = 0;
			return ++ID;
		}
		enum Type
		{
			Error,
			Comment, //����������� ��������� �����������
			Part,    //����� ��������� ������� ���� ���� � ���� �����
		};
	}
	0) ����
	1) �����

	//std::function<void(nodePtr, size_t)>addTag;
	//std::function<nodePtr(size_t)> GetAddressByID;
	//std::function<>
}