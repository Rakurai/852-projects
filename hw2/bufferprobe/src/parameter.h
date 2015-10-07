#include <string>

template <class T>
class Parameter {
public:
	Parameter(const char *name, const T value) :
		_name(name),
		_min_value(0),
		_max_value(0),
		_value(value)
	{ }

	Parameter(const char *name, const T mn, const T mx, const T value) :
		_name(name),
		_min_value(mn),
		_max_value(mx),
		_value(0)
	{
		this->value(value);
	}

	void value(const T value) {
		if (_min_value != _max_value && (value < _min_value || value > _max_value))
			throw DieException("Parameter out of range");

		_value = value;
	}
	T value() const { return _value; }

	const char *name() { return name.c_str(); }
private:
	std::string _name;
	T _min_value;
	T _max_value;
	T _value;
};
