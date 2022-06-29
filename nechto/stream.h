#pragma once
#include "node.h"
#include "nodeTypeProperties.h"

#include "externalFunction.h"
#include "tag.h"
#include <functional>
#include <map>
#include <set>

namespace nechto
{
	//���� ������� �� 2 ������:
	//����, (���� � �����)
	//�������������� ������ (��� � ��� ���������, ������������ ��� ������)

	/*
	* ������� ������� ������ ��������� ���������� � json �����
	"nodeList":
	[
		"node":
		{
			"address": ... ...
			"type":    ...
			"subtype": ...
			"data":    ...

			"additionalDataSize": ... //uint32_t
			//�������������� ������ ����. ��� ��� �������� ��� �������� �� �����
			"addeteionalData": "...",

			"numberedConnections":
			[
				"con0":
				{
				"address":  ... ...
				"backNumber":   ... 
				},
				"con1": ...  
				"con2": ...
				"con3": ...
			],			
			"hubConnections":
			//���������� ������ � ���� ���������, ������� ��� ���������
			[
				"hubcon":
				{
				"address":  ... ...
				"backNumber":   ... 
				},
				"hubcon":       ...
				...
				nullNodePtr //�����������
			]
		}
		...
	]
	}
	*/



	class stream
	{//����� ��������� ���������� �����

		//��� ����� �������� ��� �� ����������, ��� � �� ��������, 
		// ��������� solid,
		// �� � �� ���� ������� �������� ��� ����������
		//


		//�������� �������� ���������� ����� ����������� � ���������� ���� ������
		//����� ���� ��� ������ �������� ��������� �� ���������
		//��� ��� ���� ����������� ��������������� ��� ������������ �������
		//���������� ����������� ������ � ��� ������, ���� ��� ���� ���������
		//�� ���� �� ������ ����������� ����
	private:
		const char hubBack = -1;

		std::set<nodePtr> savedNodes;//������ ����������� ���
		std::map<nodePtr, nodePtr> loadedNodes;//map ����������� ���
		//���� ����������� � ���� ��������, ������� �������� �� ������ ����������
		//�������������� � ����� ��������� �� ���
		//������ ��� �������� �� ������ ����� ������, �� ������� � ���� ������������
		
		//std::ofstream out;//����� ��������� ����������
		
		
		char NumConnectionNumber(nodePtr v1, nodePtr v2)
		{//���� ���������� ������������ ��������� �����, ����� -1
			for (char i = 0; i < 4; i++)
				if (v1->connection[i].load() == v2)
					return i;
			return hubBack;
		}
		/*template<class T>
		void write(const T* data)
		{
			out.write(reinterpret_cast<const char*>(data), sizeof(T));
		}*/

		template<class T>
		void writeElement(const T* data)
		{
			write(reinterpret_cast<const char*>(data), sizeof(T));
		}
		nodePtr readAddress()
		{
			nodePtr temp;
			read(reinterpret_cast<char*>(&temp), sizeof(temp));
			return temp;
		}
		char readBackNumber()
		{
			char backNumber;
			read(reinterpret_cast<char*>(&backNumber), sizeof(backNumber));
			return backNumber;
		}

		bool saveMode  = false; //saveMode - true | loadMode - false
		bool inProcess = false;
	public:
		using saveFunType = std::function<void(const char*, uint32_t size)>;
		using loadFunType =  std::function<void(char*, uint32_t size)>;
	private:
		saveFunType write = nullptr;
		loadFunType  read = nullptr;
	public:	
		stream(saveFunType sf, loadFunType lf)
			:write(sf), read(lf) {}

		bool isSaved(nodePtr v1)
		{//�������� ��������� �� ����
			return savedNodes.contains(v1);
		}
		void saveStart()
		{
			assert(!inProcess);
			inProcess = true;
			saveMode = true;
		}
		void loadStart()
		{
			assert(!inProcess);
			inProcess = true;
			saveMode = false;
		}
		void end()
		{
			assert(inProcess);
			inProcess = false;
			if (saveMode)
			{
				writeElement(&nullNodePtr);
				savedNodes.clear();
			}
			else
			{
				inProcess = false;
				loadedNodes.clear();
			}
		}

		void save(const nodePtr v1)
		{//���������� ����
			assert(inProcess);
			assert(saveMode);

			assert(v1->getType() != node::Hub);
			if (isSaved(v1))
				return;
			savedNodes.insert(v1);//���������� � ������ �����������
			
			//1)���������� ������ ����
			auto type    = v1->getType();
			auto subtype = v1->getSubtype();
			auto data    = v1->getData<size_t>();
			writeElement(&v1);		//�����
			writeElement(&type);
			if(hasSubType(v1))
				writeElement(&subtype);
			if(hasStaticData(v1))
				writeElement(&data);
			if (hasStaticAdData(v1))
			{
				std::string adData;
				if(v1->getType() == node::Tag)
					adData = tag::getData(v1);
				if(v1->getType() == node::ExteralFunction)
					if (v1->getData<externalFunction*>() == nullptr)
						adData = "error";
					else
						adData = v1->getData<externalFunction*>()->name;
				const uint32_t adDataSize = static_cast<uint32_t>(adData.size());
				writeElement(&adDataSize);
				for (uint32_t i = 0; i < adDataSize; i++)
				{
					char temp = adData[i];
					writeElement(&temp);
				}
			}
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
				writeElement(&temp);
				writeElement(&backNumber);
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
							writeElement(&temp);
							writeElement(&backNumber);
						}
					}
			}
			writeElement(&nullNodePtr);
			return;
		}
		nodePtr load()
		{	
			assert(inProcess);
			assert(!saveMode);
			char typeBuffer, subtypeBuffer;
			size_t dataBuffer = 0;
			nodePtr oldAddress = readAddress();
			if (!oldAddress.exist())
				return nullNodePtr;
			nodePtr vload = newNode();
			loadedNodes.emplace(oldAddress, vload);
			read(reinterpret_cast<char*>(&typeBuffer), sizeof(typeBuffer));
			if (hasSubType(vload))
			{
				read(reinterpret_cast<char*>(&subtypeBuffer), sizeof(subtypeBuffer));
				setTypeAndSubtype(vload, typeBuffer, subtypeBuffer);
			}
			else
			{
				setTypeAndSubtype(vload, typeBuffer);
			}
			if (hasStaticData(vload))
			{
				read(reinterpret_cast<char*>(&dataBuffer), sizeof(dataBuffer));
				vload->setData(dataBuffer);
			}
			std::string adData;
			if (hasStaticAdData(vload))
			{
				uint32_t adDataSize = 0;
				read(reinterpret_cast<char*>(&adDataSize), sizeof(adDataSize));

				if (adDataSize != 0)
				{
					adData.resize(adDataSize);
					for (uint32_t i = 0; i < adDataSize; i++)
					{
						char temp;
						read(&temp, 1);
						adData[i] = temp;
					}
				}
				if (vload->getType() == node::Tag)
					tag::setData(vload, adData);
				if (vload->getType() == node::ExteralFunction)
				{
					if (!isExternalFunctionExist(adData))//���� ������� ���, �������� �������, ������� ����� ����� ���������
						addExternalFunction(externalFunction(adData, [](nodePtr v1) {return false; }, nullptr));
					vload->setData(getExternalFunction(adData));
				}
			}
			for (int i = 0; i < 4; i++)
			{
				nodePtr conAddress = readAddress();
				char backNumber = readBackNumber();

				if (conAddress.exist())
				{
					assert(loadedNodes.contains(conAddress));
					conAddress = loadedNodes.at(conAddress);

					if (conAddress.exist())
						if (backNumber == hubBack)
							NumHubConnect(vload, conAddress, i);
						else
							assert(false);
				}

			}
			while (true)
			{
				nodePtr conAddress = readAddress();
				if (!conAddress.exist())
					break;
				assert(loadedNodes.contains(conAddress));
				conAddress = loadedNodes.at(conAddress);
				char backNumber = readBackNumber();
				if (backNumber != hubBack)
					NumHubConnect(conAddress, vload, backNumber);
				else
					HubHubConnect(conAddress, vload);
			}
			return vload;
		}
	};
}
