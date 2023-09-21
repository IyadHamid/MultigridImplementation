export module util;

import std;

// Requires future version of MSVC to be able to export as a module
#define NDEDUCING_THIS

export template <typename T, int N> requires (std::is_scalar_v<T>&& N > 0)
struct vec {
	T raw[N];

#ifndef NDEDUCING_THIS
	constexpr auto&& data(this auto&& self) noexcept { return self.raw; }
	constexpr auto&& begin(this auto&& self) noexcept { return self.raw; }
	constexpr auto&& end(this auto&& self) noexcept { return self.raw + N; }
	constexpr int size(this auto&&) noexcept { return N; }
	constexpr auto&& operator[](this auto&& self, int i) noexcept { return self.raw[i]; }
#else 
	constexpr T* data() noexcept { return raw; }
	constexpr const T* data() const noexcept { return raw; }

	constexpr T* begin() noexcept { return raw; }
	constexpr const T* begin() const noexcept { return raw; }

	constexpr T* end() noexcept { return raw + N; }
	constexpr const T* end() const noexcept { return raw + N; }

	constexpr int size() const noexcept { return N; }

	constexpr T& operator[](int i) noexcept { return raw[i]; }
	constexpr T operator[](int i) const noexcept { return raw[i]; }
#endif

	friend constexpr vec operator+(vec v) noexcept { return v; }
	friend constexpr vec operator-(vec v) noexcept { std::ranges::transform(v, v.begin(), std::negate{}); return v; }

	friend constexpr vec& operator+=(vec& u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	friend constexpr vec& operator-=(vec& u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	friend constexpr vec operator+(vec u, const vec& v) noexcept { return u += v; }
	friend constexpr vec operator-(vec u, const vec& v) noexcept { return u -= v; }

	friend constexpr vec& operator*=(vec& v, T s) noexcept { for (auto& t : v) t *= s; return v; }
	friend constexpr vec& operator/=(vec& v, T s) noexcept { for (auto& t : v) t /= s; return v; }
	friend constexpr vec operator*(vec v, T s) noexcept { return v *= s; }
	friend constexpr vec operator/(vec v, T s) noexcept { return v /= s; }
	

	friend constexpr vec add(vec u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	friend constexpr vec sub(vec u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	friend constexpr vec mul(vec u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s *= t; return u; }
	friend constexpr vec div(vec u, const vec& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s /= t; return u; }

	friend constexpr std::strong_ordering operator<=>(const vec& u, const vec& v) noexcept { return std::lexicographical_compare_three_way(u.begin(), u.end(), v.begin(), v.end()); }

	friend constexpr T dot(const vec& u, const vec& v) noexcept { return std::transform_reduce(u.begin(), u.end(), v.begin(), 0); }
	friend constexpr T sum(const vec& v) noexcept { return std::accumulate(v.begin(), v.end(), 0); }
	friend constexpr T prod(const vec& v) noexcept { return std::accumulate(v.begin(), v.end(), 1, std::multiplies{}); }

	friend std::ostream& operator<<(std::ostream& os, const vec& v) { os << "( "; std::ranges::copy(v, std::ostream_iterator<T>(os, " ")); return os << ")"; }
};

export template <int N, typename T>
constexpr vec<T, N> splat(T t) noexcept { vec<T, N> u; std::ranges::fill(u, t); return u; }

export template <typename S, typename T, int N>
constexpr vec<S, N> vec_cast(const vec<T, N>& v) noexcept { vec<S, N> u; std::ranges::transform(v, u.begin(), [](T t) { return static_cast<S>(t); }); return u; }

export template <typename T, int N>
constexpr T flatten(const vec<T, N>& pos, const vec<T, N>& dim) noexcept {
	T out = pos[0], uncompressed_dim = 1;
	for (int i = 1; i < N; ++i) {
		uncompressed_dim *= dim[i - 1];
		out += pos[i] * uncompressed_dim;
	}
	return out;
}

export template <typename T, int N>
constexpr vec<T, N> unflatten(T idx, const vec<T, N>& dim) noexcept {
	vec<T, N> out = { idx % dim[0] };
	T uncompressed_dim = 1;
	for (int i = 1; i < N; ++i) {
		uncompressed_dim *= dim[i - 1];
		out[i] = std::modulus{}(idx / uncompressed_dim, dim[i]);
	}	
	return out;
}