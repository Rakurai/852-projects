#include <string>
#include <exception>

class DieException : public std::exception {
public:
	DieException(const char *s) : str(s) { }
	~DieException() throw () { }

	virtual const char* what() const throw() {
		return str.c_str();
	}
private:
	std::string str;
};
