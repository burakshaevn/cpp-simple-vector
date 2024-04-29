#pragma once
#include <cassert>
#include <cstdlib>
#include <algorithm>

template <typename Type>
class ArrayPtr {
public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr()
        : raw_ptr_(nullptr)
    {}

    // конструктор копирования 
    ArrayPtr(ArrayPtr&& other) noexcept {
        raw_ptr_ = std::move(other.raw_ptr_);
    }

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        if (size == 0) {
            raw_ptr_ = nullptr;
        }
        else {
            raw_ptr_ = new Type[size];
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
        //raw_ptr_ = exchange(raw_ptr, nullptr);
    }

    // Запрещаем копирование
    ArrayPtr(const ArrayPtr&) = delete;

    // При разрушении ArrayPtr должно происходить удаление массива в куче при помощи delete[].
    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Запрещаем присваивание
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    // перемещение
    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        if (this != &other) {
            delete[] raw_ptr_; // Удаляем существующий массив в случае, если он существует
            raw_ptr_ = other.raw_ptr_; // Переносим сырой указатель из другого объекта
            other.raw_ptr_ = nullptr; // Устанавливаем нулевой указатель в объекте other, чтобы избежать удаления того же массива при разрушении
        }
        return *this;
    }

    // Метод Release позволит передавать указатель на массив из одной области видимости в другую без копирования элементов.
    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* released_ptr = raw_ptr_; // Сохраняем указатель на массив
        raw_ptr_ = nullptr;            // Обнуляем указатель в объекте умного указателя
        return released_ptr;           // Возвращаем указатель на массив
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return raw_ptr_[index];
    }

    // Чтобы ArrayPtr мог предоставлять доступ к элементам массива по их индексу, подобно обычным массивами и vector, перегрузим операцию []. 
    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return raw_ptr_[index];
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {
        return raw_ptr_ != nullptr;
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        // Сохраняем значение указателя other.raw_ptr_ во временной переменной
        auto temp = raw_ptr_;
        // Обновляем значение нашего текущего указателя
        raw_ptr_ = other.raw_ptr_;
        // Обновляем значение указателя other
        other.raw_ptr_ = temp;
    }

private:
    // Внутри ArrayPtr должен содержаться сырой указатель, хранящий адрес нулевого элемента массива в динамической памяти.
    Type* raw_ptr_ = nullptr;
};

//int main() {
//    ArrayPtr<int> numbers(10);
//    const auto& const_numbers = numbers;
//
//    numbers[2] = 42;
//    assert(const_numbers[2] == 42);
//    assert(&const_numbers[2] == &numbers[2]);
//
//    assert(numbers.Get() == &numbers[0]);
//
//    ArrayPtr<int> numbers_2(5);
//    numbers_2[2] = 43;
//
//    numbers.swap(numbers_2);
//
//    assert(numbers_2[2] == 42);
//    assert(numbers[2] == 43);
//}
