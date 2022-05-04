export module staticAllocator;
import <atomic>;

export namespace nechto
{
	using ushort = unsigned short;
	template <class TCon>
	class staticAllocator
	{
	private:
		TCon content[65536]; //������ ���������
		ushort occupancy[65536]; //������ ���������
		std::atomic<ushort> gflag = 0;
		//����� ����������� ��������
		std::atomic<ushort> sflag = 0;
		//����� �������������� ��������
		std::atomic<ushort> freespace = 65535; //��������� �����

	public:

		staticAllocator()
		{//���������� ������ ���������
			for (int i = 0; i <= 65535; i++)
				occupancy[i] = i;
		}
		TCon* get(const ushort number)
		{
			return &content[number];
		}
		TCon* operator[](const ushort number)
		{
			return &content[number];
		}
		

		void deallocate(ushort number) //������������ ��������
		{
			++freespace;
			occupancy[sflag.fetch_add(1)] = number;
		}
		ushort allocate() //������ ��������
		{
			--freespace;
			return occupancy[gflag.fetch_add(1)];
		}
		ushort freeSpace() //���������� ��������� � �������
		{
			return freespace.load();
		}
		void release()
		{//������ ����� ���������. ������� ��������!
			gflag = 0;
			sflag = 0;
			freespace = 65535;
			for (int i = 0; i <= 65535; i++)
				occupancy[i] = i;
		}
	};
}