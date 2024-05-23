#pragma once
#include <cstdlib>
#include <cstring>

inline double random_double() {
	// Returns a random real in [0,1).
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}

#define BIG_AMOUNT_OF_MEMORY_SIZE 512
#define SMALL_AMOUNT_OF_MEMORY_SIZE 16

namespace AnA
{
	template<typename T, const size_t count>
	class Array
	{
	public:
		Array();
		T* Data()
		{
			return data;
		}
		const T& operator[](size_t i) const
		{
			return data[i];
		}
	protected:
		T data[count];
	};
	template<typename T>
	class Vector
	{
	public:
		Vector()
		{
			
		}
		Vector(size_t n)
		{
			data = new T[n];
		}
		~Vector()
		{
			delete[] data;
		}
		Vector(const Vector&) = delete;
		Vector& operator=(const Vector&) = delete;
		Vector(const Vector&& v) noexcept
		{
			data = v.data;
			size = v.size;
			capacity = v.capacity;
		}
		Vector& operator=(const Vector&& v) noexcept
		{
			if (&v != this)
			{
				if (data)
					delete[] data;
				data = v.data;
				size = v.size;
				capacity = v.capacity;
				v.data = nullptr;
				v.capacity = 0;
				v.size = 0;
			}
			return *this;
		}
		T* Data()
		{
			return data;
		}
		const T& operator[](size_t i) const
		{
			return data[i];
		}
		const size_t Size() const
		{
			return size;
		}
		void Resize(size_t newSize)
		{
			if (newSize > capacity || capacity - newSize > SMALL_AMOUNT_OF_MEMORY_SIZE)
				Reserve(newSize);
			size = newSize;
		}
		void Reserve(size_t newSize)
		{
			auto newData = new T[newSize];
			if (data)
			{
				memcpy(newData, data, newSize > size ? size : newSize);
				delete[] data;
			}
			data = newData;
			capacity = newSize;
		}
		void Insert(size_t pos, T& newData)
		{
			if (pos >= capacity)
			{
				Reserve(pos + SMALL_AMOUNT_OF_MEMORY_SIZE);
			}
			if (pos != size)
			{
				memcpy(data, &data[pos], size - pos);
			}
			data[pos] = newData;
			++size;
		}
		void Insert(size_t pos, Vector<T>& newData)
		{
			if (pos >= capacity)
			{
				Reserve(pos + SMALL_AMOUNT_OF_MEMORY_SIZE);
			}
			if (pos + newData.size != size)
			{
				memcpy(data, &data[pos + newData.size], size - pos - newData.size);
			}
			memcpy(&data[pos], newData.data, newData.size);
			size += newData.size;
		}
		Vector& operator+=(T& newData)
		{
			Insert(size, newData);
		}
		Vector& operator+=(Vector<T>& newData)
		{
			Insert(size, newData);
		}
	protected:
		size_t size = 0, capacity = 0;
		T* data = nullptr;
	};
	class String
	{
	public:
		String()
		{

		}
		String(const char* str, size_t len = -1)
		{
			if (len == -1)
				len = strlen(str);
			_index = _capacity = len;
			_str = new char[len + 1];
			memcpy(_str, str, len);
			_str[len] = '\0';
		}
		//reserve_size must be >= len
		String(const char* str, size_t len, size_t reserve_size)
		{
			if (len == -1)
				len = strlen(str);
			_str = new char[(_capacity = reserve_size) + 1];
			_index = len;
			memcpy(_str, str, len);
			_str[_index] = '\0';
		}
		~String()
		{
			delete _str;
		}
		String(const String&) = delete;
		String& operator=(const String&) = delete;
		String(String&& str) noexcept
		{
			_str = str._str;
			_capacity = str._capacity;
			_index = str._index;
			str._str = nullptr;
			str._capacity = 0;
			str._index = 0;
		}
		String& operator=(String&& str) noexcept
		{
			if (this != &str)
			{
				delete _str;
				_str = str._str;
				_capacity = str._capacity;
				_index = str._index;
				str._str = nullptr;
				str._capacity = 0;
				str._index = 0;
			}
			return *this;
		}
		String& operator+=(const String&& str) noexcept
		{
			size_t len = _index + str._index;
			TryResize(len);
			memcpy(&_str[_index], str._str, str._index);
			_index = len;
			return *this;
		}
		operator const char*() const
		{
			return _str;
		}
		const char* Str() const
		{
			return _str;
		}
		const size_t Length() const
		{
			return _index;
		}
		const size_t Size() const
		{
			return _capacity;
		}
		void TryResize(size_t len)
		{
			if (len > _capacity || _capacity - len >= BIG_AMOUNT_OF_MEMORY_SIZE)
			{
				Resize(len);
			}
		}
		void Resize(size_t len)
		{
			if (len == _capacity)
				return;
			char* newStr = new char[len];
			memcpy(&newStr[_index], _str, _index);
			char* tmp = _str;
			_str = newStr;
			delete tmp;
			_capacity = len;
		}
		void Copy(const String& str, size_t offset = 0)
		{
			Copy(str._str, str._index, offset);
		}
		void Copy(const char* str, const size_t len, size_t offset = 0)
		{
			size_t endPos = len + offset;
			if (endPos > _capacity)
				Resize(endPos);
			memcpy(&_str[offset], str, len);
			if (endPos > _index)
				_index = endPos;
		}
	protected:
		char* _str = nullptr;
		size_t _index = 0;
		size_t _capacity = 0;
	};
}