#pragma once
#include "node.h"
namespace nechto
{
	const nodePtr newNode();
	
	namespace hub
	{
		enum Type
		{
			Error,
			Extender,
			Array
		};
		void add(nodePtr vertex, nodePtr lastHub)
		{//���������� ���� � ��������
			assert(vertex.exist());
			assert(lastHub.exist());
			nodePtr hub = newNode();
			setTypeAndSubtype(hub, node::Hub, Extender);
			hub->setData(std::pair<nodePtr, nodePtr>(vertex, lastHub));
			//������ ������������ ��������
			nodePtr temp = nullNodePtr;
			if (!lastHub->hubConnection.compare_exchange_strong(temp, hub))
				deleteNode(hub);
			//���� ������������ ��� �� �������, ������ �� ��� ����
		}
		const nodePtr getParrent(const nodePtr hub)
		{
			assert(hub->getType() == node::Hub);
			return hub->getData<std::pair<nodePtr, nodePtr>>().first;
		}
	}
}