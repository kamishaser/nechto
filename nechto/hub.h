#pragma once
namespace nechto::hub
{
	enum Type
	{
		Error,
		Extender,
		Array
	};
	//void add(nodePtr vertex, nodePtr lastHub)
	//{//���������� ���� � ��������
	//	assert(vertex.exist());
	//	assert(lastHub.exist());
	//	nodePtr hub = newNode();
	//	setTypeAndSubtype(hub, node::Hub, 0);
	//	hub->setData(std::pair<nodePtr, nodePtr>(vertex, lastHub));
	//	//������ ������������ ��������
	//	nodePtr temp = nullNodePtr;//����� ����, ��� compare_excha
	//	if (!lastHub->hubConnection.compare_exchange_strong(temp, hub))
	//		deleteNode(hub);
	//	//���� ������������ ��� �� �������, ������ �� ��� ����
	//}
	//const nodePtr getHubParrent(const nodePtr hub)
	//{
	//	assert(hub->getType() == node::Hub);
	//	return hub->getData<std::pair<nodePtr, nodePtr>>().first;
	//}
}