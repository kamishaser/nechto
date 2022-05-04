export module lowLevelGraphOperations;
import vertex;

import <utility>;
import <cassert>;

export namespace nechto
{
	//��������� �����
	bool firstTypeCompare(vertexPtr v1, ushort type);
	//�������� ������� ����������
	bool isConnectionForNumberExist(vertexPtr v1, ushort number);
	bool isHubExist(vertexPtr v1);
	bool isVertexHasConnections(vertexPtr v1);
	//�������� / ��������
	const vertexPtr newVertex();
	void deleteVertex(vertexPtr v);
	//�������� � ������
	void addHub(vertexPtr v1);
	const vertexPtr getHubParrent(const vertexPtr hub);
	//�������� �������������� ����������
	bool NumConnect(vertexPtr v1, vertexPtr v2, ushort conNumber);
	void HubConnect(vertexPtr v1, vertexPtr v2);
	//�������� �������������� ���������
	bool NumNumConnect(vertexPtr v1, vertexPtr v2, ushort number1, ushort number2);
	bool NumHubConnect(vertexPtr v1, vertexPtr v2, ushort number1);
	void HubHubConnect(vertexPtr v1, vertexPtr v2);
	//������ ����������
	void oneSideDisconnect(vertexPtr v1, vertexPtr v2);
	void disconnect(vertexPtr v1, vertexPtr v2);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//��������� �����
	bool firstTypeCompare(vertexPtr v1, ushort type)
	{
		return v1->type.load().first == type;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� ������� ����������
	bool isConnectionForNumberExist(vertexPtr v1, ushort number)
	{
		assert(number < 4);
		return v1->connection[number].load().exist();
	}
	bool isHubExist(vertexPtr v1)
	{
		return v1->hubConnection.load().exist();
	}
	bool isVertexHasConnections(vertexPtr v1)
	{
		for (int i = 0; i < 4; i++)
			if (isConnectionForNumberExist(v1, i))
				return true;
		if (isHubExist(v1)) return true;
		return false;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� / ��������
	const vertexPtr newVertex()
	{
		vertexPtr v;
		std::pair<ushort, ushort> temp = allocatedStorage<vertex>::terminal.allocate();
		v.first  = temp.first;
		v.second = temp.second;
		return v;
	}
	void deleteVertex(vertexPtr v1)
	{
		assert(!isVertexHasConnections(v1));
		allocatedStorage<vertex>::terminal.deallocate(std::pair<ushort, ushort>(v1.first, v1.second));
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� � ������

	void addHub(vertexPtr v1)
	{//���������� ���� � ��������
		vertexPtr hub = newVertex();
		std::pair<ushort, ushort> type(static_cast<ushort>(vertex::Hub), 0);
		hub->type.store(type);
		//hub->setData<vertexPtr>(v1); ��������������
		//������ ������������ ��������
		vertexPtr temp = nullVertexPtr;//����� ����, ��� compare_excha
		if (!v1->hubConnection.compare_exchange_strong(temp, hub))
			deleteVertex(hub);
		//���� ������������ ��� �� �������, ������ �� ��� ����
	}
	const vertexPtr getHubParrent(const vertexPtr hub)
	{
		assert(hub->type.load().first == vertex::Hub);
		return nullVertexPtr;//hub->getData<vertexPtr>(); ��������������
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//�������� �������������� ����������
	bool NumConnect(vertexPtr v1, vertexPtr v2, ushort conNumber)
	{//���������� ���������� ������������� �����
		//�������������� �������� v2 � v1 �� ������ conNumber
		vertexPtr temp = nullVertexPtr;
		return v1->hubConnection.compare_exchange_strong(temp, v2);
	}//���������� true, ���� ����� �������� ������� � false � ��������� ������

	void HubConnect(vertexPtr v1, vertexPtr v2)
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

	bool NumNumConnect(vertexPtr v1, vertexPtr v2, ushort number1, ushort number2)
	{
		if (!NumConnect(v1, v2, number1))
			return false;
		if (!NumConnect(v2, v1, number2))
		{
			v1->connection[number1] = nullVertexPtr;
			return false;
		}
		return true;

	}
	bool NumHubConnect(vertexPtr v1, vertexPtr v2, ushort number1)
	{
		if (!NumConnect(v1, v2, number1))
			return false;
		HubConnect(v2, v1);
		return true;
	}
	void HubHubConnect(vertexPtr v1, vertexPtr v2)
	{
		HubConnect(v1, v2);
		HubConnect(v2, v1);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//������ ����������

	void oneSideDisconnect(vertexPtr v1, vertexPtr v2)
	{//������������ ������������ v2 �� v1 (v1 �� ����� ����� � v2)
		vertexPtr temp = v2;
		while (true)
		{
			if (v1->connection[0].compare_exchange_strong(temp, nullVertexPtr)) return;
			else temp = v2;
			if (v1->connection[1].compare_exchange_strong(temp, nullVertexPtr)) return;
			else temp = v2;
			if (v1->connection[2].compare_exchange_strong(temp, nullVertexPtr)) return;
			else temp = v2;
			if (v1->connection[3].compare_exchange_strong(temp, nullVertexPtr)) return;
			else temp = v2;

			if (!(v1->hubConnection.load()))
				return;
			else
				v1 = v1->hubConnection;
		}
	}
	void disconnect(vertexPtr v1, vertexPtr v2)
	{
		oneSideDisconnect(v1, v2);
		oneSideDisconnect(v2, v1);
	}
}

