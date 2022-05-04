export module vertex;
import allocatedStorage;

import <atomic>;
import <utility>;
import <cassert>;

export namespace nechto
{
	using ushort = unsigned short;
	struct vertex
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
			vertex* operator-> () const
			{
				return allocatedStorage<vertex>::getAllocator(first)->get(second);
			}
			vertex* operator* () const
			{
				return allocatedStorage<vertex>::getAllocator(first)->get(second);
			}
			bool exist() const
			{
				return first != 0;
			}
			operator bool() const 
			{ 
				return exist(); 
			}

			operator const std::pair<ushort, ushort>() const
			{
				return std::pair<ushort, ushort>(first, second);
			}
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

		enum vType
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

	using vertexPtr = vertex::ptr;

	const vertexPtr nullVertexPtr(0, 0);
}