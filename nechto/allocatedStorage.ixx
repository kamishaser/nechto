export module allocatedStorage;
import staticAllocator;

import <memory>;
import <vector>;
import <atomic>;
import <mutex>;
import <thread>;
import <utility>;
import <cassert>;
import <iostream>;


export namespace nechto
{
	using ushort = unsigned short;
	const int maxNumOfAllocators = 65536;

	template <class TCon>//��� ������������ �������� ����� 3 ����� ������ � ����������� ��������
	class storageTerminal;//���� ����� ����� ��� ������ ��������� ������� � ����� ����������

	template <class TCon>
	class allocatedStorage
	{
		static_assert(maxNumOfAllocators <= 65536, "65536 is maximum");
		static std::unique_ptr<staticAllocator<TCon>> content[maxNumOfAllocators]; //������ �����������
		static ushort occupancy[maxNumOfAllocators]; //������ ���������
		static std::atomic<ushort> sflag; //����� ����������� ����������
		static std::atomic<ushort> freeSpace; //���������� ��������� �����������

		static std::mutex changeAllocatorBlock;

		

		static void toFreeAllocator(ushort number)//������������ ����������
		{
			changeAllocatorBlock.lock();
			occupancy[sflag.fetch_add(-1)] = number;
			++freeSpace;
			changeAllocatorBlock.unlock();
		}
		static ushort getFreeAllocator()//������ ����������
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
					content[occupancy[number]] = std::make_unique<staticAllocator<TCon>>();
					return number;
				}
				if (content[occupancy[number]]->freeSpace() > 256)
				{
					changeAllocatorBlock.unlock();
					return number;
				}
			}

		}

	public:

		static staticAllocator<TCon>* getAllocator(const ushort number)
		{
			assert((number > 0) && (number < maxNumOfAllocators));
			assert(content[number]);
			return content[number].get();
		}

		static ushort getFreeSpace()
		{
			return freeSpace;
		}
		static void reset()
		{//������ ����� ���������. ������� ��������!
			changeAllocatorBlock.lock();
			for (int i = 0; i < maxNumOfAllocators; i++)
				occupancy[i] = i;
			sflag = 1;
			freeSpace = maxNumOfAllocators - 1;
			changeAllocatorBlock.unlock();
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////
		friend class storageTerminal<TCon>;
		static thread_local storageTerminal<TCon> terminal;

	};

	//////////////////////////////////////////////////////////////////////////////////////////////
	//��������� ����������� https://www.cyberforum.ru/cpp-beginners/thread1143095.html
	template<class TCon>
	std::unique_ptr<staticAllocator<TCon>> allocatedStorage<TCon>::content[maxNumOfAllocators]; 
	//������ �����������
	template<class TCon>
	ushort allocatedStorage<TCon>::occupancy[maxNumOfAllocators]; //������ ���������
	template<class TCon>
	std::atomic<ushort> allocatedStorage<TCon>::sflag = 1; //����� ����������� ����������
	template<class TCon>
	std::atomic<ushort> allocatedStorage<TCon>::freeSpace
		= maxNumOfAllocators - 1; //���������� ��������� �����������
	template<class TCon>
	std::mutex allocatedStorage<TCon>::changeAllocatorBlock;
	

	template<class TCon>
	class storageTerminal
	{//����������� ��������� ������� ��� ���������� ������ ������������ � �����������
		std::vector<ushort> localAllocatorSet;//������ ������� ����������������� �����������
		ushort currentAllocatorNumber;//����� �������� ����������
		staticAllocator<TCon>* currentAllocator;//������� ���������

		//����� ������������� � ������ ������ ����������
		void changeCurrentAllocator()
		{
			for (auto i = localAllocatorSet.begin(); i != localAllocatorSet.end(); ++i)
			{
				if (allocatedStorage<TCon>::getAllocator(*i)->freeSpace() > 256)
				{
					currentAllocatorNumber = *i;
					
					currentAllocator = allocatedStorage<TCon>::getAllocator(*i);
					return;
				}
			}

			currentAllocatorNumber = allocatedStorage<TCon>::getFreeAllocator();
			std::cout << currentAllocatorNumber << std::endl;
			currentAllocator = allocatedStorage<TCon>::getAllocator(currentAllocatorNumber);
			localAllocatorSet.push_back(currentAllocatorNumber);
		}//������� ������� ���������
		static bool isFistTerminal;//������ �������� ������������� ���������
	public:
		
		explicit storageTerminal()
		{
			if (isFistTerminal)
			{
				allocatedStorage<TCon>::reset();
				isFistTerminal = false;
			}
			changeCurrentAllocator();			
		}
		~storageTerminal()
		{
			for (auto i = localAllocatorSet.begin(); i != localAllocatorSet.end(); ++i)
			{
				allocatedStorage<TCon>::toFreeAllocator(*i);//������������ ������������ ����������, ����� �� ���������� ��� ������ ����������
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
			allocatedStorage<TCon>::getAllocator(id.first)->deallocate(id.second);
		}
	};
	
	template<class TCon>
	bool storageTerminal<TCon>::isFistTerminal = true;

	template<class TCon>
	thread_local storageTerminal<TCon> allocatedStorage<TCon>::terminal;
}