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

namespace AnA
{
	template<typename T>
	class Vector
	{
	public:
		Vector()
		{
			
		}
		~Vector()
		{

		}
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
			_str = new char[len];
			memcpy(_str, str, len);
		}
		//reserve_size must be >= len
		String(const char* str, size_t len, size_t reserve_size)
		{
			if (len == -1)
				len = strlen(str);
			_str = new char[_capacity = reserve_size];
			_index = len;
			memcpy(_str, str, len);
		}
		~String()
		{
			delete _str;
		}
		String(const String&) = delete;
		String& operator=(const String&) = delete;
		String(const String&& str) noexcept
		{
			_str = str._str;
			_capacity = str._capacity;
			_index = str._index;
		}
		String& operator=(const String&& str) noexcept
		{
			if (this != &str)
			{
				delete _str;
				_str = str._str;
				_capacity = str._capacity;
				_index = str._index;
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