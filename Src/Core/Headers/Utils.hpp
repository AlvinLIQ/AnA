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

namespace AnA
{
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
			_str = new char[len];
			memcpy(_str, str, len);
		}
		~String()
		{
			delete _str;
		}
		String(const String&) = delete;
		String& operator=(const String&) = delete;
		String& operator=(const String&& str) noexcept
		{
			if (this != &str)
			{
				delete _str;
				_str = str._str;
			}
			return *this;
		}
	protected:
		char* _str = nullptr;
	};
}