#pragma once
#include "node.h"

#include <utility>
#include <cassert>
#include <set>

#include "baseValueTypes.h"
#include "mathOperator.h"
#include "tag.h"
#include "pointer.h"
#include "externalFunction.h"

namespace nechto
{
	//��������� �����
	bool typeCompare(nodePtr v1, ushort type);
	//�������� ������� ����������
	bool isHubExist(nodePtr v1);
	bool isNodeHasConnections(nodePtr v1);
	bool hasConnection(nodePtr v1, nodePtr v2);
	bool hasMultipleConnection(nodePtr v1);
	//��������
	const nodePtr newNode();
	const nodePtr newNode(char type, char subtype = 0, size_t data = 0);
	
	//�������� � ������
	void addHub(nodePtr v1);
	const nodePtr getHubParrent(const nodePtr hub);
	//�������� �������������� ����������
	void NumConnect(nodePtr v1, nodePtr v2, ushort conNumber);
	void HubConnect(nodePtr v1, nodePtr v2);
	//�������� �������������� ���������
	void NumHubConnect(nodePtr v1, nodePtr v2, ushort number1);
	void HubHubConnect(nodePtr v1, nodePtr v2);
	//������ ����������
	void oneSideDisconnect(nodePtr v1, nodePtr v2);
	void disconnect(nodePtr v1, nodePtr v2);
	//����� ����
	void reset(nodePtr v1);
	void setTypeAndSubtype(nodePtr v1, char type, char subtype = 0);
	
	//��������
	void deleteNode(nodePtr v);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//��������� �����
	bool typeCompare(nodePtr v1, ushort type)
	{
		assert(v1.exist());
		return v1->getType() == type;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� ������� ����������
	bool isHubExist(nodePtr v1)
	{
		assert(v1.exist());
		return v1->hubConnection.load().exist();
	}
	bool isNodeHasConnections(nodePtr v1)
	{
		assert(v1.exist());
		for (int i = 0; i < 4; i++)
			if (v1->hasConnection(i))
				return true;
		if (isHubExist(v1)) return true;
		return false;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//��������
	const nodePtr newNode()
	{
		nodePtr v;
		nodePtr temp = nodeStorage::terminal.allocate();
		v = temp;
		return v;
	}
	const nodePtr newNode(char type, char subtype, size_t data)
	{
		nodePtr v;
		nodePtr temp = nodeStorage::terminal.allocate();
		v = temp;
		setTypeAndSubtype(v, type, subtype);
		v->setData(data);
		return v;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� � ������

	void addHub(nodePtr vertex, nodePtr lastHub)
	{//���������� ���� � ��������
		assert(vertex.exist());
		assert(lastHub.exist());

		nodePtr hub = newNode();
		setTypeAndSubtype(hub, node::Hub, 0);
		hub->setData(std::pair<nodePtr, nodePtr>(vertex, lastHub));
		//������ ������������ ��������
		nodePtr temp = nullNodePtr;//����� ����, ��� compare_excha
		if (!lastHub->hubConnection.compare_exchange_strong(temp, hub))
			deleteNode(hub);
		//���� ������������ ��� �� �������, ������ �� ��� ����
	}
	const nodePtr getHubParrent(const nodePtr hub)
	{
		assert(hub->getType() == node::Hub);
		return hub->getData<std::pair<nodePtr, nodePtr>>().first;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� �������������� ����������
	void NumConnect(nodePtr v1, nodePtr v2, ushort conNumber)
	{//���������� ���������� ������������� �����
		//�������������� �������� v2 � v1 �� ������ conNumber
		nodePtr temp = nullNodePtr;
		if (v1->connection[conNumber].compare_exchange_strong(temp, v2))
			return;
		v1->connection[conNumber] = v2;
		oneSideDisconnect(temp, v1);
	}//���������� true, ���� ����� �������� ������� � false � ��������� ������

	void HubConnect(nodePtr v1, nodePtr v2)
	{//���������� ����� � ������ ���������� ��������� ����� � ����
		assert(v1.exist() && v2.exist());
		if (!v1->hubConnection.load().exist())
			addHub(v1, nullNodePtr);
		nodePtr hubIterator = v1->hubConnection;
		nodePtr temp;
		while (true)
		{
			temp = nullNodePtr;
			if (hubIterator->connection[0].compare_exchange_strong(temp, v2))
				return;
			temp = nullNodePtr;
			if (hubIterator->connection[1].compare_exchange_strong(temp, v2))
				return;
			temp = nullNodePtr;
			if (hubIterator->connection[2].compare_exchange_strong(temp, v2))
				return;
			temp = nullNodePtr;
			if (hubIterator->connection[3].compare_exchange_strong(temp, v2))
				return;
			addHub(v1, hubIterator);
			hubIterator = hubIterator->hubConnection;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� �������������� ����������
	void NumHubConnect(nodePtr v1, nodePtr v2, ushort number1)
	{
		assert(v1.exist() && v2.exist());
		assert(v1->getType() != node::Hub);
		assert(v2->getType() != node::Hub);
		NumConnect(v1, v2, number1);
		HubConnect(v2, v1);
	}
	void HubHubConnect(nodePtr v1, nodePtr v2)
	{
		assert(v1.exist() && v2.exist());
		assert(v1->getType() != node::Hub);
		assert(v2->getType() != node::Hub);
		HubConnect(v1, v2);
		HubConnect(v2, v1);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//������ ����������

	void oneSideDisconnect(nodePtr v1, nodePtr v2)
	{//������������ ������������ v2 �� v1 (v1 �� ����� ����� � v2)
		assert(v1.exist() && v2.exist());
		nodePtr temp = v2;
		while (true)
		{
			if (v1->connection[0].compare_exchange_strong(temp, nullNodePtr)) return;
			else temp = v2;
			if (v1->connection[1].compare_exchange_strong(temp, nullNodePtr)) return;
			else temp = v2;
			if (v1->connection[2].compare_exchange_strong(temp, nullNodePtr)) return;
			else temp = v2;
			if (v1->connection[3].compare_exchange_strong(temp, nullNodePtr)) return;
			else temp = v2;

			if (!(v1->hubConnection.load()))
				return;
			else
				v1 = v1->hubConnection;
		}
	}
	void disconnect(nodePtr v1, nodePtr v2)
	{
		if (!v1.exist() || !v2.exist())
			return;
		oneSideDisconnect(v1, v2);
		oneSideDisconnect(v2, v1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//����� ����
	void reset(nodePtr v1)
	{
		switch (v1->getType())
		{
		case node::Tag:
			tag::deleteData(v1);
			break;
		case node::Pointer:
			pointer::deletePointer(v1);
			break;
		default:
			break;
		}
	}
	void setTypeAndSubtype(nodePtr v1, char type, char subtype)
	{
		reset(v1);
		v1->type = type;
		v1->subtype = subtype;

		switch (v1->getType())
		{
		case node::Tag:
		case node::ExteralFunction:
			v1->setData(nullptr);
			break;
		case node::Variable:
			switch (v1->getSubtype())
			{
			case baseValueType::Int64:
				v1->setData<int64_t>(0);
				break;
			case baseValueType::Double:
				v1->setData<double>(0);
				break;
			default:
				break;
			}
		default:
			break;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// ��������
	void deleteNode(nodePtr v1)
	{
		assert(v1.exist());
		nodePtr vTemp = v1;
		reset(v1);
		while (true)
		{//���� �������� ���� �� ����� ������
			setTypeAndSubtype(vTemp, node::Error, 0);
			for (int i = 0; i < 4; i++)//������ ����������
				if (vTemp->connection[i].load().exist())
				{
					oneSideDisconnect(vTemp->connection[i], v1);
					vTemp->connection[i] = nullNodePtr;
				}
			if (!vTemp->hubConnection.load().exist())
				break;

			nodePtr vHub = vTemp->hubConnection;
			nodeStorage::terminal.deallocate(vTemp);
			vTemp = vHub;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//������ ����������
}