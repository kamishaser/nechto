#pragma once
#include "node.h"

#include <utility>
#include <cassert>

namespace nechto
{
	//��������� �����
	bool typeCompare(nodePtr v1, ushort type);
	//�������� ������� ����������
	bool isConnectionForNumberExist(nodePtr v1, ushort number);
	bool isHubExist(nodePtr v1);
	bool isNodeHasConnections(nodePtr v1);
	//��������
	const nodePtr newNode();
	
	//�������� � ������
	void addHub(nodePtr v1);
	const nodePtr getHubParrent(const nodePtr hub);
	//�������� �������������� ����������
	bool NumConnect(nodePtr v1, nodePtr v2, ushort conNumber);
	void HubConnect(nodePtr v1, nodePtr v2);
	//�������� �������������� ���������
	bool NumNumConnect(nodePtr v1, nodePtr v2, ushort number1, ushort number2);
	bool NumHubConnect(nodePtr v1, nodePtr v2, ushort number1);
	void HubHubConnect(nodePtr v1, nodePtr v2);
	//������ ����������
	void oneSideDisconnect(nodePtr v1, nodePtr v2);
	void disconnect(nodePtr v1, nodePtr v2);
	//��������
	void deleteNode(nodePtr v);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//��������� �����
	bool typeCompare(nodePtr v1, ushort type)
	{
		return v1->type.load() == type;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� ������� ����������
	bool isConnectionForNumberExist(nodePtr v1, ushort number)
	{
		assert(number < 4);
		return v1->connection[number].load().exist();
	}
	bool isHubExist(nodePtr v1)
	{
		return v1->hubConnection.load().exist();
	}
	bool isNodeHasConnections(nodePtr v1)
	{
		for (int i = 0; i < 4; i++)
			if (isConnectionForNumberExist(v1, i))
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
		v.first  = temp.first;
		v.second = temp.second;
		return v;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� � ������

	void addHub(nodePtr v1)
	{//���������� ���� � ��������
		nodePtr hub = newNode();
		hub->type.store(node::Hub);
		hub->type.store(0);
		hub->setData<nodePtr>(v1);
		//������ ������������ ��������
		nodePtr temp = nullNodePtr;//����� ����, ��� compare_excha
		if (!v1->hubConnection.compare_exchange_strong(temp, hub))
			deleteNode(hub);
		//���� ������������ ��� �� �������, ������ �� ��� ����
	}
	const nodePtr getHubParrent(const nodePtr hub)
	{
		assert(hub->type.load() == node::Hub);
		return hub->getData<nodePtr>();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� �������������� ����������
	bool NumConnect(nodePtr v1, nodePtr v2, ushort conNumber)
	{//���������� ���������� ������������� �����
		//�������������� �������� v2 � v1 �� ������ conNumber
		nodePtr temp = nullNodePtr;
		return v1->hubConnection.compare_exchange_strong(temp, v2);
	}//���������� true, ���� ����� �������� ������� � false � ��������� ������

	void HubConnect(nodePtr v1, nodePtr v2)
	{//���������� ����� � ������ ���������� ��������� ����� � ����
		if (!v1->hubConnection.load().exist())
			addHub(v1);
		v1=v1->hubConnection;

		while (true)
		{
			if (NumConnect(v1, v2, 0))
				return;
			if (NumConnect(v1, v2, 1))
				return;
			if (NumConnect(v1, v2, 2))
				return;
			if (NumConnect(v1, v2, 3))
				return;
			addHub(v1);
			v1 = v1->hubConnection;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� �������������� ����������

	bool NumNumConnect(nodePtr v1, nodePtr v2, ushort number1, ushort number2)
	{
		if (!NumConnect(v1, v2, number1))
			return false;
		if (!NumConnect(v2, v1, number2))
		{
			v1->connection[number1] = nullNodePtr;
			return false;
		}
		return true;

	}
	bool NumHubConnect(nodePtr v1, nodePtr v2, ushort number1)
	{
		if (!NumConnect(v1, v2, number1))
			return false;
		HubConnect(v2, v1);
		return true;
	}
	void HubHubConnect(nodePtr v1, nodePtr v2)
	{
		HubConnect(v1, v2);
		HubConnect(v2, v1);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//������ ����������

	void oneSideDisconnect(nodePtr v1, nodePtr v2)
	{//������������ ������������ v2 �� v1 (v1 �� ����� ����� � v2)
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
		oneSideDisconnect(v1, v2);
		oneSideDisconnect(v2, v1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// ��������
	void deleteNode(nodePtr v1)
	{
		assert(!isNodeHasConnections(v1));
		nodePtr vTemp = v1;
		while (true)
		{//���� �������� ���� �� ����� ������
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
}