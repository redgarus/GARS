#include <iostream>
#include <stdexcept> // для исключений

template<typename T>
class MyVector {
public:
    // Конструктор по умолчанию
    MyVector() : data(nullptr), _size(0), _capacity(0) {}

    // Конструктор с начальным размером
    MyVector(size_t size) : data(new T[size]), _size(size), _capacity(size) {}

    // Деструктор
    ~MyVector() {
        delete[] data;
    }

    // Метод для получения размера вектора
    size_t size() const {
        return _size;
    }

    // Метод для получения емкости вектора
    size_t capacity() const {
        return _capacity;
    }

    // Добавление элемента в конец
    void push_back(const T& value) {
        if (_size >= _capacity) {
            resize();
        }
        data[_size++] = value;
    }

    // Оператор доступа по индексу
    T& operator[](size_t index) {
        if (index >= _size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    // Константная версия оператора доступа по индексу
    const T& operator[](size_t index) const {
        if (index >= _size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

private:
    T* data;          // Указатель на динамический массив
    size_t _size;     // Текущий размер вектора
    size_t _capacity; // Емкость (сколько элементов можно разместить)

    // Увеличение емкости
    void resize() {
        _capacity = (_capacity == 0) ? 1 : _capacity * 2;
        T* newData = new T[_capacity];
        for (size_t i = 0; i < _size; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }
};

// Пример использования
int main() {
    MyVector<int> vec;

    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    std::cout << "Vector size: " << vec.size() << std::endl;
    std::cout << "Vector capacity: " << vec.capacity() << std::endl;

    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "Element " << i << ": " << vec[i] << std::endl;
    }

    return 0;
}
