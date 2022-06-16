#pragma once
#include "node.h"
#include <filesystem>
#include <fstream>
#include <map>
#include <set>

namespace nechto
{
	class nechtoOutFileStream
	{//����� ��������� ���������� �����

		//�������� �������� ���������� ����� ����������� � ���������� ���� ������
		//����� ���� ��� ������ �������� ��������� �� ���������
		//��� ��� ���� ����������� ��������������� ��� ������������ �������
		//���������� ����������� ������ � ��� ������, ���� ��� ���� ���������
		//�� ���� �� ������ ����������� ����
	private:
		std::set<nodePtr> savedNodes;//������ ���������� ���
		std::ofstream out;//����� ��������� ����������

		char NumConnectionNumber(nodePtr v1, nodePtr v2)
		{//���� ���������� ������������ ��������� �����, ����� -1
			for (char i = 0; i < 4; i++)
				if (v1->connection[i].load() == v2)
					return i;
			return -1;
		}
		template<class T>
		void write(T* data)
		{
			out.write(reinterpret_cast<char*>(data), sizeof(T));
		}

	public:
		bool start(std::filesystem::path path)
		{//�������� �����
			out.open(path, std::ios::binary | std::ios::trunc | std::ios::out);
			return out.is_open();
		}
		bool isOpen()
		{//��������
			return out.is_open();
		}
		void end()
		{
			savedNodes.clear();
			out.close();
		}
		void clear()
		{//������ �������� ������ ���������� ���
			savedNodes.clear();
			out.clear();
		}
		bool isSaved(nodePtr v1)
		{//�������� ��������� �� ����
			return savedNodes.contains(v1);
		}
		nechtoOutFileStream& operator<<(const nodePtr v1)
		{//���������� ����
			savedNodes.insert(v1);//���������� � ������ ����������
			
			//1)���������� ������ ����
			auto type    = v1->type.load();
			auto subtype = v1->subtype.load();
			auto data    = v1->data.load();
			write(&v1);
			write(&type);
			write(&subtype);
			write(&data);
			//2)���������� ������������ ����������
			for (int i = 0; i < 4; i++)
			{
				nodePtr temp;
				char backNumber;
				//��� ��� ��� ���������� ������������ ����� ����� ����� ��������� ����������
				if (savedNodes.contains(v1->connection[i].load()))
				{
					temp = v1->connection[i].load();
					backNumber = NumConnectionNumber(temp, v1);
				}
				else
				{
					temp = nullNodePtr;
					backNumber = -1;
				}
				//���������� ������������ ���� ���� ��� ����������
				write(&temp);
				write(&backNumber);
			}
			//3)���������� ���������� �� �����
			nodePtr hubIterator = v1;
			while(hubIterator->hasHub())
			{
				hubIterator = hubIterator->hubConnection;
				for(int i = 0; i < 4; i++)
					if (hubIterator->hasConnection(i))
					{
						nodePtr temp = hubIterator->connection[i].load();
						if (savedNodes.contains(temp))
						{
							char backNumber = NumConnectionNumber(temp, v1);
							write(&temp);
							write(&backNumber);
						}
					}
			}
			out.write(reinterpret_cast<const char*>(&nullNodePtr), sizeof(nullNodePtr));
			return *this;
		}
		
	};

	bool nechtoFileLoad(std::filesystem::path path, std::function<void(nodePtr)> nodeLoad)
	{
		std::ifstream in;
		in.open(path, std::ios::binary);
		if (!in.is_open())
			return false;

		return true;
	}
	class nechtoInFileStream
	{
		std::map<nodePtr, nodePtr> loadedNodes;//map ����������� ���
		//���� ����������� � ���� ��������, ������� �������� �� ������ ����������
		//�������������� � ����� ��������� �� ���
		//������ ��� �������� �� ������ ����� ������, �� ������� � ���� ������������
		

	public:

		
	};
}
