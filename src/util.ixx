export module util;

import std;

// Requires future version to be able to export as a module
#define NDEDUCING_THIS

export template <typename T, int N> requires (std::is_scalar_v<T>&& N > 0)
struct vec {
	T raw[N];

#ifndef NDEDUCING_THIS
	constexpr auto&& data(this auto&& self) { return self.raw; }
	constexpr auto&& begin(this auto&& self) { return self.raw; }
	constexpr auto&& end(this auto&& self) { return self.raw + N; }
	constexpr int size(this auto&&) { return N; }
	constexpr auto&& operator[](this auto&& self, int i) { return self.raw[i]; }
#else 
	constexpr T* data() { return raw; }
	constexpr const T* data() const { return raw; }

	constexpr T* begin() { return raw; }
	constexpr const T* begin() const { return raw; }

	constexpr T* end() { return raw + N; }
	constexpr const T* end() const { return raw + N; }

	constexpr int size() const { return N; }

	constexpr T& operator[](int i) { return raw[i]; }
	constexpr T operator[](int i) const { return raw[i]; }
#endif

	friend constexpr vec operator+(vec a) { return a; }
	friend constexpr vec operator-(vec a) { std::ranges::transform(a, a.begin(), std::negate{}); return a; }

	friend constexpr vec& operator+=(vec& a, const vec& b) { for (auto&& [u, v] : std::views::zip(a, b)) u += v; return a; }
	friend constexpr vec& operator-=(vec& a, const vec& b) { for (auto&& [u, v] : std::views::zip(a, b)) u -= v; return a; }
	friend constexpr vec operator+(vec a, const vec& b) { return a += b; }
	friend constexpr vec operator-(vec a, const vec& b) { return a -= b; }

	friend constexpr vec& operator*=(vec& a, T b) { for (auto& v : a) v *= b; return a; }
	friend constexpr vec& operator/=(vec& a, T b) { for (auto& v : a) v /= b; return a; }
	friend constexpr vec operator*(vec a, T b) { return a *= b; }
	friend constexpr vec operator/(vec a, T b) { return a /= b; }

	friend constexpr T dot(const vec& a, const vec& b) { return std::transform_reduce(a.begin(), a.end(), b.begin(), 0); }
	friend constexpr T sum(const vec& a) { return std::accumulate(a.begin(), a.end(), 0); }
	friend constexpr T prod(const vec& a) { return std::accumulate(a.begin(), a.end(), 1, std::multiplies{}); }

	friend std::ostream& operator<<(std::ostream& os, const vec& v) { os << "( "; std::ranges::copy(v, std::ostream_iterator<T>(os, " ")); return os << ")"; }

};

export template <int N, typename T>
constexpr vec<T, N> splat(T a) { vec<T, N> b; std::ranges::fill(b, a); return b; }

export template <typename S, typename T, int N>
constexpr vec<S, N> vec_cast(vec<T, N> a) { vec<S, N> b; std::ranges::transform(a, b.begin(), [](T t) { return static_cast<S>(t); }); return b; }

