export module node;
import staticAllocator;

import <memory>;
import <vector>;
import <atomic>;
import <mutex>;
import <thread>;
import <cassert>;
import <iostream>;

namespace nechto
{
	using ushort = unsigned short;
	struct node //���� ���� nechto
 	{
		struct ptr
		{
			ushort first;
			ushort second;

			ptr(ushort f = 0, ushort s = 0){
				first = f;
				second = s;
			}
			ptr(std::pair<ushort, ushort> address)
			{
				first = address.first;
				second = address.second;
			}
			bool exist() const
			{
				return first != 0;
			}
			operator bool() const
			{
				return exist();
			}
			node* operator-> () const;
			node* operator* () const;
		};
		
		std::atomic<size_t> data = 0;
		std::atomic<std::pair<ushort, ushort>> type;
		std::atomic<ptr> connection[4];
		std::atomic<ptr> hubConnection;

		template <class TCon>
		const TCon getData() const
		{
			assert(sizeof(TCon) <= sizeof(size_t));
			size_t temp = data.load();
			return *static_cast<TCon*>(static_cast<void*>(&temp));
		}
		template <class TCon>
		void setData(TCon Data)
		{
			assert(sizeof(TCon) <= sizeof(size_t));
			size_t temp = *static_cast<size_t*>(static_cast<void*>(&Data));
			data.store(temp);
		}

		enum Type
		{
			Empty,					//������ ������
			Hub,					//������������
			Tag,					//��������� ����� ������ (������������� ����� ����� ���� ������ � ����� 
									//(� ����� ����� ������ ���� ������))
			Variable,				//������-���������� �������� ����, ���������� ������ ��������� (�������� ��� ���� ������������)
			Pointer,				//��������� �� ������
			Assignment,				//�������� ������������ (���������� �� �������� ���������������)
			Function,				//�������, �� ���������� ������ nechto
			MathOperator,			//�������������� ��������
			ConditionalBranching,	//if
			BranchingMerge,			//������� ������
			Segment,				//����������� ������� ���������, ���� � ����� �� �������� �������������� ������ � ����� �����
			TagCall,				//����� ��������� �� ����
			NumberOfTypes			//�� ������. �����, ������������ ���������� �����
		};
	};

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////
	const int maxNumOfAllocators = 65536;

	namespace nodeStorage
	{
		static_assert(maxNumOfAllocators <= 65536, "65536 is maximum");
		std::unique_ptr<staticAllocator<node>> content[maxNumOfAllocators]; //������ �����������
		ushort occupancy[maxNumOfAllocators]; //������ ���������
		std::atomic<ushort> sflag; //����� ����������� ����������
		std::atomic<ushort> freeSpace; //���������� ��������� �����������

		std::mutex changeAllocatorBlock;



		void toFreeAllocator(ushort number)//������������ ����������
		{
			changeAllocatorBlock.lock();
			occupancy[sflag.fetch_add(-1)] = number;
			++freeSpace;
			changeAllocatorBlock.unlock();
		}
		ushort getFreeAllocator()//������ ����������
		{
			changeAllocatorBlock.lock();
			--freeSpace;
			ushort number;
			while (true)
			{
				number = sflag.fetch_add(1);
				if (!content[occupancy[number]])
				{
					changeAllocatorBlock.unlock();
					content[occupancy[number]] = std::make_unique<staticAllocator<node>>();
					return number;
				}
				if (content[occupancy[number]]->freeSpace() > 256)
				{
					changeAllocatorBlock.unlock();
					return number;
				}
			}

		}

		staticAllocator<node>* getAllocator(const ushort number)
		{
			assert((number > 0) && (number < maxNumOfAllocators));
			assert(content[number]);
			return content[number].get();
		}

		ushort getFreeSpace()
		{
			return freeSpace;
		}
		void reset()
		{//������ ����� ���������. ������� ��������!
			changeAllocatorBlock.lock();
			for (int i = 0; i < maxNumOfAllocators; i++)
				occupancy[i] = i;
			sflag = 1;
			freeSpace = maxNumOfAllocators - 1;
			changeAllocatorBlock.unlock();
		}

		//////////////////////////////////////////////////////////////////////////////////////////

		class Terminal
		{//����������� ��������� ������� ��� ���������� ������ ������������ � �����������
			std::vector<ushort> localAllocatorSet;//������ ������� ����������������� �����������
			ushort currentAllocatorNumber;//����� �������� ����������
			staticAllocator<node>* currentAllocator;//������� ���������

			//����� ������������� � ������ ������ ����������
			void changeCurrentAllocator()
			{
				for (auto i = localAllocatorSet.begin(); i != localAllocatorSet.end(); ++i)
				{
					if (allocatedStorage<node>::getAllocator(*i)->freeSpace() > 256)
					{
						currentAllocatorNumber = *i;

						currentAllocator = allocatedStorage<node>::getAllocator(*i);
						return;
					}
				}

				currentAllocatorNumber = allocatedStorage<node>::getFreeAllocator();
				std::cout << currentAllocatorNumber << std::endl;
				currentAllocator = allocatedStorage<node>::getAllocator(currentAllocatorNumber);
				localAllocatorSet.push_back(currentAllocatorNumber);
			}//������� ������� ���������
			static bool isFistTerminal;//������ �������� ������������� ���������
		public:

			explicit Terminal()
			{
				if (isFistTerminal)
				{
					allocatedStorage<node>::reset();
					isFistTerminal = false;
				}
				changeCurrentAllocator();
			}
			~Terminal()
			{
				for (auto i = localAllocatorSet.begin(); i != localAllocatorSet.end(); ++i)
				{
					allocatedStorage<node>::toFreeAllocator(*i);//������������ ������������ ����������, ����� �� ���������� ��� ������ ����������
				}
			}
			const std::pair<ushort, ushort> allocate()
			{
				assert(currentAllocator != nullptr);
				if (currentAllocator->freeSpace() <= static_cast<ushort>(256))
					changeCurrentAllocator();
				std::pair<ushort, ushort> id;
				id.first = currentAllocatorNumber;
				id.second = currentAllocator->allocate();
				return id;
			}
			void deallocate(const std::pair<ushort, ushort> id)
			{
				allocatedStorage<node>::getAllocator(id.first)->deallocate(id.second);
			}
		};

		thread_local Terminal terminal;

	};
	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////

	node* vertex::ptr::operator-> () const
	{
		return nodeStorage::getAllocator(first)->get(second);
	}
	node* vertex::ptr::operator* () const
	{
		return nodeStorage::getAllocator(first)->get(second);
	}

	using nodePtr = node::ptr;

	const nodePtr nullNodePtr(0, 0);
}