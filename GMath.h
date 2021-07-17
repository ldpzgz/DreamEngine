#ifndef _GMATH_H_
#define _GMATH_H_
#include <array>

template<typename T,int N>
class VecHelper {
public:
	static void copy(T out[N], T const rhs[N]) noexcept
	{
		out[0] = rhs[0];
		VecHelper<T, N - 1>::copy(out + 1, rhs + 1);
	}

	static void add(T out[N], T const left[N], T const right[N]) noexcept
	{
		out[0] = left[0] + right[0];
		VecHelper<T, N - 1>::add(out + 1, left + 1,right + 1);
	}

	static void sub(T out[N], T const left[N], T const right[N]) noexcept
	{
		out[0] = left[0] - right[0];
		VecHelper<T, N - 1>::sub(out + 1, left + 1, right + 1);
	}

	static void mul(T out[N], T temp[N],const T& t) noexcept
	{
		out[0] = temp[0] * t;
		VecHelper<T, N - 1>::mul(out + 1, temp + 1, t);
	}

	static void dot(T& out, T temp1[N], T temp2[N]) noexcept
	{
		out += temp1[0] * temp2[0];
		VecHelper<T, N - 1>::dot(out, temp1 + 1, temp2 + 1);
	}
};

template <typename T>
struct VecHelper<T, 1>
{
	static void DoCopy(T out[1], T const rhs[1]) noexcept
	{
		out[0] = rhs[0];
	}

	static void add(T out[1], T const left[1], T const right[1]) noexcept
	{
		out[0] = left[0] + right[0];
	}

	static void sub(T out[1], T const left[1], T const right[1]) noexcept
	{
		out[0] = left[0] - right[0];
	}

	static void mul(T out[1], T temp[1], const T& t) noexcept
	{
		out[0] = temp[0] * t;
	}

	static void dot(T& out, T temp1[1], T temp2[1]) noexcept
	{
		out += temp1[0] * temp2[0];
	}
};

template<typename T,int N>
class Vec {
public:
	Vec<T, N>() {

	}

	explicit Vec<T, N>(const T* data) {
		VecHelper::copy(mData.data(),data);
	}

	explicit Vec<T, N>(const Vec<T, N>& other) {
		VecHelper::copy(mData.data(), other.data());
	}

	Vec<T, N>(const T& x, const T& y) : mData{x,y}
	{
		static_assert(N == 2, "declare Vec<T, 2>,but n not 2");
	}

	Vec<T, N>(const T& x, const T& y, const T& z ) : mData{ x,y,z }
	{
		static_assert(N == 3, "declare Vec<T, 3>,but n not 3");
	}

	Vec<T, N>(const T& x, const T& y, const T& z, const T& w) : mData{ x,y,z,w}
	{
		static_assert(N == 4, "declare Vec<T, 4>,but n not 4");
	}

	Vec<T, N> operator+(const Vec<T, N>& other) {
		Vec<T, N> temp;
		VecHelper::add(temp.data(),mData.data(), other.data());
		return temp;
	}

	Vec<T, N>& operator+=(const Vec<T, N>& other) {
		VecHelper::add(mData.data(), mData.data(), other.data());
		return mData;
	}

	Vec<T, N> operator-(const Vec<T, N>& other) {
		Vec<T, N> temp;
		VecHelper::sub(temp.data(), mData.data(), other.data());
		return temp;
	}

	Vec<T, N>& operator-=(const Vec<T, N>& data) {
		VecHelper::sub(mData.data(), mData.data(), other.data());
		return mData;
	}

	Vec<T, N> operator*(const T& t) {
		Vec<T, N> temp;
		VecHelper::mul(temp.data(), mData.data(), t);
		return mData;
	}

	Vec<T, N>& operator*=(const T& t) {
		VecHelper::mul(mData.data(), mData.data(), t);
		return mData;
	}

	Vec<T, N> operator/(const T& t)
	{
		Vec<T, N> temp;
		VecHelper::mul(temp.data(), mData.data(), T(1) / t);
		return mData;
	}

	Vec<T, N>& operator/=(const T&) {
		VecHelper::mul(temp.data(), mData.data(), T(1) / t);
		return mData;
	}

	T dot(const Vec<T, N>& temp) {
		T out = T(0);
		VecHelper::dot(out, mData.data(), temp.mData())j;
		return out;
	}

	/*Vec<T, N> cross(const Vec<T, N>&) {

	}*/

	std::array<T, N> mData{};
};

#endif