#ifndef DUCKOS_RESULT_HPP
#define DUCKOS_RESULT_HPP

#include <common/shared_ptr.hpp>

class Result {
public:
	static const int Success = 0;

	Result(int code);

	bool is_success() const;
	bool is_error() const;
private:
	int _code;
};

template<typename T>
class ResultRet {
public:
	ResultRet(int error):  _result(ResultRet(error)) {};
	ResultRet(Result error): _result(error) {};
	ResultRet(T ret): _ret(ret), _result(0) {};
	bool is_error() const {return _result.is_error();}
	T value() const {
		ASSERT(!is_error());
		return _ret;
	};
private:
	Result _result;
	T _ret;
};

#endif //DUCKOS_RESULT_HPP
