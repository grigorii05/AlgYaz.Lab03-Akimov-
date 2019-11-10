#include <iostream>
#include <atomic>
using namespace std;

class Counter // Счетчик действительных указателей
{
public:
	atomic_uint count; // Счетчик ссылок (действительных указателей)
};

template <typename T>
class SharedPtr
{
private:
	T* p; // Указатель
	Counter* p_count; // Указатель на счетчик
public:
	SharedPtr() // Пустой объект
	{
		p = nullptr; p_count = nullptr;
	}
	SharedPtr(T* ptr) // Создаем новый (первый) объект - умный указатель, параметр - обычный указатель
	{
		//	p = ptr;
			// Ниже если копировать память в новый объет
		p = new T; // Выделяем память под умный указатель
		memcpy(p, ptr, sizeof(T)); // Копируем из обычного указателя в умный 
		p_count = new Counter; // Выделяем память под счетчик
		p_count->count = 1; // Создан 1 указатель  (значение счетчика)

	}
	// Конструктор копирования (память новую не выделяем), но счетчик увеличиваем на 1
	SharedPtr(const SharedPtr& r)
	{
		p = r.p; // Копируем адрес на существующую память
		if (r.p_count != nullptr)
		{
			++r.p_count->count; // Счетчик увеличиваем на 1 (новый указатель на ту же область)
			p_count = r.p_count; // Общий счетчик на все объекты копируем адрес счетчика
		}
		else 
		{ // Когда копируем пустой объект
			p_count = nullptr;
		}
	}
	// Конструктор перемещения r - перестает существовать как указатель на некоторую память
	SharedPtr(SharedPtr&& r)
	{
		p = r.p;
		p_count = r.p_count;
		// Объект r ниже перестает существовать
		r.p_count = nullptr;
		r.p = nullptr;
	}
	~SharedPtr()
	{
		if (p_count != nullptr && p != nullptr)
			if (p_count->count > 0) // Есть действительные указатели
			{
				--p_count->count; // Число указателей уменьшает на 1 (один уничтожаем)
				if (p_count->count == 0) { // Если нет действительных указателей, то освобождаем память
					delete p_count; // Удаляем счетчик
					delete p; // Саму область памяти
					p = nullptr;
					p_count = nullptr;
				}
			}
	}
	// Простое присваивание (создается новый объект указатель на память)
	auto operator = (const SharedPtr& r)->SharedPtr&
	{
		if (p == nullptr) // Был пустой указатель
		{
			if (r.p != nullptr) // Копируем не пустой указатель
			{
				++(r.p_count->count); // Общий счетчик ++
				// Указатель на существующую память, указатель на счетчик на существующий счетчик
				p = r.p;
				p_count = r.p_count;
			}
			// else Если оба пустых объекта ничего не делаем
		}
		else // Был не пустой объект (указатель), куда мы хотим копировать
		{
			if (r.p != p) // Указатели на разную память
			{
				if (r.p != nullptr) // Откуда копируем - указатель не пустой
				{
					--(p_count->count); // Уменьшаем счетчик куда раньше указывал объект, в который копируем
					if (p_count->count == 0) // Если нет действительных указателей чистим память
					{
						delete p_count;
						delete p;
					}
					++r.p_count->count; // Появился новый действительный указатель (куда копируем)
					// Настраиваем текущий объект на новый, котрый скопировали в текущий
					p = r.p;
					p_count = r.p_count;

				}
				else // r.p == nullptr (копируем пустой объект)
				{
					--p_count->count;  // Уменьшаем счетчик куда раньше указывал объект, в который копируем
					if (p_count->count == 0) // если нет действительных указателей чистим память
					{
						delete p_count;
						delete p;
					}
					// Текущий объект -пустой
					p_count = nullptr;
					p = nullptr;
				}
			}
			else // r.p == p
				; // Ничего не делаем
		}
		return *this;
	}
	// Присваивание с перемещение, 
	// все тоже самое что и выше в методе auto operator = (const SharedPtr& r), 
	// но объект r перестает существовать
	// Общее число объектов (указателей) не увеличилось
	auto operator = (SharedPtr&& r)->SharedPtr&
	{
		if (p == nullptr)
		{
			if (r.p != nullptr)
			{
				++r.p_count->count;
				p = r.p;
				p_count = r.p_count;
				 //Объект r перестает сущестовать
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
				; // Ничего не делаем
		}
		return *this;
	}

	// проверяет, указывает ли указатель на объект
	operator bool() const
	{
		if (p != nullptr) return true;
		else return false;
	}
	// Обращение по адресу (перегрузка)
	auto operator*() const->T&
	{
		return *p;
	}
	// Обращение через указатель (перегрузка)
	auto operator->() const->T*
	{
		return p;
	}

	// Получить указатель на область памяти
	auto get()->T*
	{
		return p;
	}

	// Сбрасывает текущий указатель (делает его пустым)
	void reset()
	{
		--p_count->count; // Счетчик уменьшается
		if (p_count->count == 0) { // Если нет действительных указателей удаляем память
			delete p;
			delete p_count;
		}
		// Делаем объект пустым
		p = nullptr;
		p_count = nullptr;
	}

	// Сбрасываем указатель и настраиваем на новую память
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
		// Выделяем новую память или копируем в существующую, если она выделена
		if (p == nullptr) p = new T;
		memcpy(p, ptr, sizeof(T));
		//	p = ptr; // Не выделяем новую память
		if (p_count == nullptr) p_count = new Counter;
		p_count->count = 1;
	}

	// 
	void swap(SharedPtr& r) // Меняем 2 объекта местами
	{
		T* pbuf = r.p;
		Counter* p_buf = r.p_count;
		r.p = p;
		r.p_count = p_count;
		p = pbuf;
		p_count = p_buf;

	}
	// возвращает количество объектов SharedPtr, которые ссылаются на тот же управляемый объект
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