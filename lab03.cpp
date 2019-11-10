#include <iostream>
#include <atomic>
using namespace std;

class Counter // ������� �������������� ����������
{
public:
	atomic_uint count; // ������� ������ (�������������� ����������)
};

template <typename T>
class SharedPtr
{
private:
	T* p; // ���������
	Counter* p_count; // ��������� �� �������
public:
	SharedPtr() // ������ ������
	{
		p = nullptr; p_count = nullptr;
	}
	SharedPtr(T* ptr) // ������� ����� (������) ������ - ����� ���������, �������� - ������� ���������
	{
		//	p = ptr;
			// ���� ���� ���������� ������ � ����� �����
		p = new T; // �������� ������ ��� ����� ���������
		memcpy(p, ptr, sizeof(T)); // �������� �� �������� ��������� � ����� 
		p_count = new Counter; // �������� ������ ��� �������
		p_count->count = 1; // ������ 1 ���������  (�������� ��������)

	}
	// ����������� ����������� (������ ����� �� ��������), �� ������� ����������� �� 1
	SharedPtr(const SharedPtr& r)
	{
		p = r.p; // �������� ����� �� ������������ ������
		if (r.p_count != nullptr)
		{
			++r.p_count->count; // ������� ����������� �� 1 (����� ��������� �� �� �� �������)
			p_count = r.p_count; // ����� ������� �� ��� ������� �������� ����� ��������
		}
		else 
		{ // ����� �������� ������ ������
			p_count = nullptr;
		}
	}
	// ����������� ����������� r - ��������� ������������ ��� ��������� �� ��������� ������
	SharedPtr(SharedPtr&& r)
	{
		p = r.p;
		p_count = r.p_count;
		// ������ r ���� ��������� ������������
		r.p_count = nullptr;
		r.p = nullptr;
	}
	~SharedPtr()
	{
		if (p_count != nullptr && p != nullptr)
			if (p_count->count > 0) // ���� �������������� ���������
			{
				--p_count->count; // ����� ���������� ��������� �� 1 (���� ����������)
				if (p_count->count == 0) { // ���� ��� �������������� ����������, �� ����������� ������
					delete p_count; // ������� �������
					delete p; // ���� ������� ������
					p = nullptr;
					p_count = nullptr;
				}
			}
	}
	// ������� ������������ (��������� ����� ������ ��������� �� ������)
	auto operator = (const SharedPtr& r)->SharedPtr&
	{
		if (p == nullptr) // ��� ������ ���������
		{
			if (r.p != nullptr) // �������� �� ������ ���������
			{
				++(r.p_count->count); // ����� ������� ++
				// ��������� �� ������������ ������, ��������� �� ������� �� ������������ �������
				p = r.p;
				p_count = r.p_count;
			}
			// else ���� ��� ������ ������� ������ �� ������
		}
		else // ��� �� ������ ������ (���������), ���� �� ����� ����������
		{
			if (r.p != p) // ��������� �� ������ ������
			{
				if (r.p != nullptr) // ������ �������� - ��������� �� ������
				{
					--(p_count->count); // ��������� ������� ���� ������ �������� ������, � ������� ��������
					if (p_count->count == 0) // ���� ��� �������������� ���������� ������ ������
					{
						delete p_count;
						delete p;
					}
					++r.p_count->count; // �������� ����� �������������� ��������� (���� ��������)
					// ����������� ������� ������ �� �����, ������ ����������� � �������
					p = r.p;
					p_count = r.p_count;

				}
				else // r.p == nullptr (�������� ������ ������)
				{
					--p_count->count;  // ��������� ������� ���� ������ �������� ������, � ������� ��������
					if (p_count->count == 0) // ���� ��� �������������� ���������� ������ ������
					{
						delete p_count;
						delete p;
					}
					// ������� ������ -������
					p_count = nullptr;
					p = nullptr;
				}
			}
			else // r.p == p
				; // ������ �� ������
		}
		return *this;
	}
	// ������������ � �����������, 
	// ��� ���� ����� ��� � ���� � ������ auto operator = (const SharedPtr& r), 
	// �� ������ r ��������� ������������
	// ����� ����� �������� (����������) �� �����������
	auto operator = (SharedPtr&& r)->SharedPtr&
	{
		if (p == nullptr)
		{
			if (r.p != nullptr)
			{
				++r.p_count->count;
				p = r.p;
				p_count = r.p_count;
				 //������ r ��������� �����������
				r.p = nullptr;
				r.p_count = nullptr;
			}
		}
		else
		{
			if (r.p != p)
			{
				if (r.p != nullptr)
				{
					--p_count->count;
					if (--p_count->count == 0)
					{
						delete p_count;
						delete p;
					}
					++r.p_count->count;
					p = r.p;
					p_count = r.p_count;

				}
				else // r.p == nullptr
				{
					--p_count->count;
					if (--p_count->count == 0)
					{
						delete p_count;
						delete p;
					}
					p_count = nullptr;
					p = nullptr;
				}
			}
			else // r.p == p
				; // ������ �� ������
		}
		return *this;
	}

	// ���������, ��������� �� ��������� �� ������
	operator bool() const
	{
		if (p != nullptr) return true;
		else return false;
	}
	// ��������� �� ������ (����������)
	auto operator*() const->T&
	{
		return *p;
	}
	// ��������� ����� ��������� (����������)
	auto operator->() const->T*
	{
		return p;
	}

	// �������� ��������� �� ������� ������
	auto get()->T*
	{
		return p;
	}

	// ���������� ������� ��������� (������ ��� ������)
	void reset()
	{
		--p_count->count; // ������� �����������
		if (p_count->count == 0) { // ���� ��� �������������� ���������� ������� ������
			delete p;
			delete p_count;
		}
		// ������ ������ ������
		p = nullptr;
		p_count = nullptr;
	}

	// ���������� ��������� � ����������� �� ����� ������
	void reset(T* ptr)
	{
		if (p_count != nullptr)
		{
			--p_count->count;
			if (p_count->count == 0) {
				delete p;
				delete p_count;
			}
		}
		// �������� ����� ������ ��� �������� � ������������, ���� ��� ��������
		if (p == nullptr) p = new T;
		memcpy(p, ptr, sizeof(T));
		//	p = ptr; // �� �������� ����� ������
		if (p_count == nullptr) p_count = new Counter;
		p_count->count = 1;
	}

	// 
	void swap(SharedPtr& r) // ������ 2 ������� �������
	{
		T* pbuf = r.p;
		Counter* p_buf = r.p_count;
		r.p = p;
		r.p_count = p_count;
		p = pbuf;
		p_count = p_buf;

	}
	// ���������� ���������� �������� SharedPtr, ������� ��������� �� ��� �� ����������� ������
	auto use_count() const->size_t
	{
		return p_count->count;
	}
};

void main()
{
	SharedPtr<int> P1, P2;
	int a = 10;
	int b = 20;
	P1.reset(&a);
	P2.reset(&b);
	// cout << *P.p << endl;
	cout << "P1.count=" << P1.use_count() << endl;
	cout << "P2.count=" << P2.use_count() << endl << endl;
	{
		SharedPtr<int> P1_1 = P1, P1_2 = P1;
		cout << "P1.count=" << P1.use_count() << endl;
		cout << "P2.count=" << P2.use_count() << endl;
		P1.swap(P2);
		cout << "P1.count=" << P1.use_count() << endl;
		cout << "P2.count=" << P2.use_count() << endl;
		cout << "P1.p=" << *P1 << endl;
		cout << "P2.p=" << *P2 << endl;
		*P1_1 = 11111111;
		cout << "P1.p=" << *P1 << endl;
		cout << "P2.p=" << *P2 << endl;

	}
	cout << "P1.count=" << P1.use_count() << endl;
	cout << "P2.count=" << P2.use_count() << endl;

	system("pause");
	//cout << "Hello World";

}