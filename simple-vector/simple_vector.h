#pragma once

#include <initializer_list>
#include "array_ptr.h" 
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
    // Конструктор прокси-объекта
    explicit ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve) {}

    // Оператор присваивания для прокси-объекта
    ReserveProxyObj& operator=(const ReserveProxyObj&) = delete;

    // Публичный член данных для емкости
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    // По умолчанию. Создаёт пустой вектор с нулевой вместимостью. Не выделяет динамическую память и не выбрасывает исключений.
    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    // Параметризованный конструктор, создающий вектор заданного размера. 
    // Элементы вектора инициализированы значением по умолчанию для типа Type. 
    // Вектор должен иметь одинаковые размер и вместимость. 
    // Если размер нулевой, динамическая память для его элементов выделяться не должна.
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type())
    {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , capacity_(size)
        , size_(size)
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    // Конструктор из std::initializer_list. Элементы вектора должны содержать копию элементов initializer_list. 
    // Имеет размер и вместимость, совпадающую с размерами и вместимостью переданного initializer_list.
    SimpleVector(std::initializer_list<Type> init)
        : items_(init.size())
        , capacity_(init.size())
        , size_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }

    // Конструктор копирования. Копия вектора должна иметь вместимость, достаточную для хранения копии элементов исходного вектора.
    SimpleVector(const SimpleVector& other)
        : items_(other.size_)
        , capacity_(other.size_)
        , size_(other.size_)
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.capacity_to_reserve_);
    }

    // move конструктор
    SimpleVector(SimpleVector&& other)
        : items_(other.capacity_)
    {
        swap(other);
    }

    // Метод GetSize для получения количества элементов в векторе. Не выбрасывает исключений.
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Метод GetCapacity для получения вместимости вектора. Не выбрасывает исключений.
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    //Метод IsEmpty, сообщающий, пуст ли вектор. Не выбрасывает исключений.
    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Оператор [] для доступа к элементу вектора по его индексу. Имеет две версии — константную и неконстантную. Не выбрасывает исключений. 
    // Для корректной работы оператора индекс элемента массива не должен выходить за пределы массива.
    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Метод At для доступа к элементу вектора по его индексу, аналог метода at класса vector. В случае выхода индекса за пределы массива должен выбросить исключение std::out_of_range.
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index > size_");
        }
        else {
            return items_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index > size_");
        }
        else {
            return items_[index];
        }
    }

    // Метод Clear для очистки массива без изменения его вместимости. Не выбрасывает исключений.
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        /* В методе Resize отдельно обработайте три ситуации:
        1) новый размер меньше или равен текущему
        2) новый размер не превышает его вместимости
        3) новый размер превышает текущую вместимость вектора.

        Если при изменении размера массива новый размер вектора превышает его текущую вместимость, создайте новый массив с нужной вместимостью,
        скопируйте в него прежнее содержимое и заполните остальные элементы значением по умолчанию. Затем старый массив можно удалить и использовать копию.
        После этого не забудьте обновить размер и вместимость вектора.

        Если при увеличении размера массива новый размер вектора не превышает его вместимость, заполните добавленные элементы значением по умолчанию для типа Type.
        При уменьшении размера вектора просто уменьшите его размер. */
        if (new_size <= size_) {
            size_ = new_size;
        }
        if (new_size <= capacity_) {
            Fill(items_.Get() + size_, items_.Get() + size_ + new_size);
        }
        if (new_size > capacity_) {
            size_t new_capacity = std::max(new_size, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            Fill(temp.Get(), temp.Get() + new_capacity);
            std::move(items_.Get(), items_.Get() + capacity_, temp.Get());
            items_.swap(temp);

            size_ = new_size;
            capacity_ = new_capacity;
        }
    }

    // Методы begin, end, cbegin и cend, возвращающие итераторы на начало и конец массива. В качестве итераторов используйте указатели. 
    // Эти методы должны быть объявлены со спецификатором noexcept. В противном случае тренажёр отклонит ваше решение.

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

    // Оператор присваивания. Должен обеспечивать строгую гарантию безопасности исключений.
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector temp(rhs);
            swap(temp);
        }
        return *this;
    }

    // Перемещающий оператор присваивания 
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            items_ = std::move(rhs.items_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);

            rhs.size_ = 0;
            rhs.capacity_ = 0;
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            //Resize(size_ + 1);
            items_[size_++] = item;
        }
        else if (size_ == capacity_) {
            Resize(capacity_ + 1);
            items_[size_ - 1] = item; // size_ - 1 добавляет в конец 
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_++] = std::move(item);
        }
        else if (size_ == capacity_) {
            Resize(capacity_ + 1);
            items_[size_ - 1] = std::move(item); // size_ - 1 добавляет в конец 
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t count = pos - begin();
        if (size_ < capacity_) {
            std::copy_backward(begin() + count, end(), end() + 1);
            *pos = value;
        }
        else {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            auto new_pos = temp.Get() + count;
            std::copy(begin(), pos, temp.Get());
            *new_pos = value;
            std::copy(pos, end(), new_pos + 1);
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;
        return begin() + count;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t count = pos - items_.Get();
        if (capacity_ == 0) {
            ArrayPtr<Type> temp(1);
            temp[count] = std::move(value);
            items_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_) {
            std::move_backward(items_.Get() + count, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[count] = std::move(value);
        }
        else {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            std::move_backward(items_.Get() + count, items_.Get() + size_, temp.Get() + size_ + 1);
            temp[count] = std::move(value);
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;

        return &items_[count];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos != this->end());
        assert(pos >= this->begin());

        size_t count = pos - items_.Get();
        std::move(items_.Get() + count + 1, items_.Get() + size_, items_.Get() + count);
        --size_;
        return &items_[count];
    }


    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Reserve сразу выделяет нужное количество памяти. При добавлении новых элементов в вектор копирование будет происходить или значительно реже или совсем не будет.
    // Если new_capacity больше текущей capacity, память должна быть перевыделена, а элементы вектора скопированы в новый отрезок памяти.
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            if (size_ > 0) {
                std::copy(items_.Get(), items_.Get() + size_, temp.Get());
            }
            items_.swap(temp);
            capacity_ = new_capacity;
        }
    }

private:
    ArrayPtr<Type> items_{};
    size_t capacity_{};
    size_t size_{};

    void Fill(Iterator first, Iterator last) {
        assert(first < last);

        for (; first != last; ++first) {
            *first = std::move(Type());
        }
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (&lhs == &rhs) {
        return true;
    }
    return (lhs.GetSize() == rhs.GetSize()) && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}