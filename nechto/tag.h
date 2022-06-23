#pragma once
#include "node.h"
#include <map>

namespace nechto
{
	namespace tag
	{
		//��� - ���������������, �� ���������� ������ ������������ ��������� ����������
		enum Type
		{
			Error,
			Comment, //����������� ��������� �����������
			Name,

			ExternalConnection, //������� �����������. ������ ���� ����� �������������� �����.
			//���� ������� ������� ����������� ������ �������.
			//��������� ���������, �� ������� �������� ����������� ��������� ��������� � 
			//����� ���� ������� ��������� ������, (����� �� ����� ����������)
		};
		bool isCorrect(nodePtr tag)
		{
			assert(tag->type == node::Tag);
			
			switch (tag->subtype.load())
			{
			case Comment:
				if (!tag->hasConnection(0))//������� ����������)
					return false;
				break;
			default:
				return false;
			}
			return true;
		}


		void setData(nodePtr tag, std::string& data) noexcept
		{
			std::cout << tag->getData<std::string*>() << std::endl;
			if (tag->getData<std::string*>() == nullptr)
				tag->setData(new std::string(data));
			else
				*tag->getData<std::string*>() = data;
		}
		std::string getData(nodePtr tag) noexcept
		{
			if (tag->getData<std::string*>() == nullptr)
				return std::string();
			else
				return *tag->getData<std::string*>();
		}
		void deleteData(nodePtr tag) noexcept
		{
			if (tag->getData<std::string*>() != nullptr)
				delete tag->getData<std::string*>();
			tag->setData<std::string*>(nullptr);
		}
	}
	//std::function<void(nodePtr, size_t)>addTag;
	//std::function<nodePtr(size_t)> GetAddressByID;
	//std::function<>
	/*class externalConnection
	{
		nodePtr exConTag;
	public:
		externalConnection(nodePtr conNode)
		{
			if (conNode->type == node::Tag && conNode->subtype == tag::ExternalConnection)
				exConTag = conNode;
		}
	};*/
}