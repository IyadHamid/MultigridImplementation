export module util;

import std;

// Requires future version of MSVC to be able to export as a module
#define NDEDUCING_THIS


template <typename T, int N, int M> requires (std::is_scalar_v<T>&& N > 0 && M > 0)
class mat;

export template <typename T, int N>
using vec = mat<T, N, 1>;

#pragma region VectorUtility
export template <int N, typename T>
constexpr auto splat(T t) noexcept { vec<T, N> u; std::ranges::fill(u, t); return u; }

export template <typename S, typename T, int N>
constexpr auto vec_cast(const vec<T, N>& v) noexcept { vec<S, N> u; std::ranges::transform(v, u.begin(), [](T t) { return static_cast<S>(t); }); return u; }

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
#pragma endregion

// Row ordered matrix
export template <typename T, int N, int M = N> requires (std::is_scalar_v<T>&& N > 0 && M > 0)
class mat {
public:
	using value_type = T;

	T raw[N * M];

#pragma region Range
	static constexpr int size() noexcept { return N * M; }
	static constexpr mat<int, 2, 1> dim() { return { N, M }; }
#ifndef NDEDUCING_THIS
	constexpr auto&& data(this auto&& self) noexcept { return self.raw; }
	constexpr auto&& begin(this auto&& self) noexcept { return self.raw; }
	constexpr auto&& end(this auto&& self) noexcept { return self.raw + size(); }
	constexpr auto&& operator[](this auto&& self, int i) noexcept { return self.raw[i]; }
#else 
	constexpr T* data() noexcept { return raw; }
	constexpr const T* data() const noexcept { return raw; }

	constexpr T* begin() noexcept { return raw; }
	constexpr const T* begin() const noexcept { return raw; }

	constexpr T* end() noexcept { return raw + size(); }
	constexpr const T* end() const noexcept { return raw + size(); }

	constexpr T& operator[](int i) noexcept { return raw[i]; }
	constexpr T operator[](int i) const noexcept { return raw[i]; }
#endif
#pragma endregion

#pragma region MatrixViews
#define assert(X) if (!(X)) std::terminate();  
	constexpr auto nth_row(int n) const noexcept { assert(n < N);  return (*this) | std::views::drop(n * M) | std::views::take(M); }
	constexpr auto rows() const noexcept { return (*this) | std::views::chunk(M); }

	constexpr auto nth_col(int n) const noexcept { assert(n < M);  return (*this) | std::views::drop(n) | std::views::stride(M); }
	constexpr auto cols() const noexcept { return std::views::iota(0, M) | std::views::transform([&](int n) { return (*this) | std::views::drop(n) | std::views::stride(M); }); }
#pragma endregion

#pragma region VectorSpace
	// Inverse identity
	friend constexpr mat operator+(mat v) noexcept { return v; }
	friend constexpr mat operator-(mat v) noexcept { std::ranges::transform(v, v.begin(), std::negate{}); return v; }

	// Vector addition
	friend constexpr mat& operator+=(mat& u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	friend constexpr mat& operator-=(mat& u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	friend constexpr mat operator+(mat u, const mat& v) noexcept { return u += v; }
	friend constexpr mat operator-(mat u, const mat& v) noexcept { return u -= v; }

	// Scalar multiplication
	friend constexpr mat& operator*=(mat& v, std::convertible_to<T> auto s) noexcept { for (auto& t : v) t *= s; return v; }
	friend constexpr mat& operator/=(mat& v, std::convertible_to<T> auto s) noexcept { for (auto& t : v) t /= s; return v; }
	friend constexpr mat operator*(mat v, std::convertible_to<T> auto s) noexcept { return v *= s; }
	friend constexpr mat operator*(std::convertible_to<T> auto s, mat v) noexcept { return v *= s; }
	friend constexpr mat operator/(mat v, std::convertible_to<T> auto s) noexcept { return v /= s; }
#pragma endregion

#pragma region UtilityOperations
	// Matrix multiplication
	template <int O>
	friend constexpr mat<T, N, O> operator*(const mat<T, N, M>& u, const mat<T, M, O>& v) noexcept {
		mat<T, N, O> w;
		std::ranges::transform(std::views::cartesian_product(u.rows(), v.cols()), w.begin(), [](auto uv) { return std::transform_reduce(std::get<0>(uv).begin(), std::get<0>(uv).end(), std::get<1>(uv).begin(), T{}); });
		return w;
	}

	// Comparison
	friend constexpr std::strong_ordering operator<=>(const mat& u, const mat& v) noexcept { return std::lexicographical_compare_three_way(u.begin(), u.end(), v.begin(), v.end()); }

	// Element-wise operations
	friend constexpr auto add(mat u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	friend constexpr auto sub(mat u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	friend constexpr auto mul(mat u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s *= t; return u; }
	friend constexpr auto div(mat u, const mat& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s /= t; return u; }

	// Misc
	friend constexpr auto dot(const mat& u, const mat& v) noexcept requires (M == 1) { return std::transform_reduce(u.begin(), u.end(), v.begin(), 0); }
	friend constexpr auto sum(const mat& v) noexcept { return std::accumulate(v.begin(), v.end(), T{}); }
	friend constexpr auto prod(const mat& v) noexcept { return std::accumulate(v.begin(), v.end(), T{ 1 }, std::multiplies{}); }

	// Printing
	friend std::ostream& operator<<(std::ostream& os, const mat& v) { os << "( "; std::ranges::copy(v, std::ostream_iterator<T>(os, " ")); return os << ")"; }
#pragma endregion

#pragma region SquareMatrix
private:
	constexpr auto triangularize(auto&& F) requires (N == M) {
		mat out = {};
		for (int i = 0; i < N * N; ++i)
			if (auto x = unflatten(i, dim()); F(x[0], x[1]))
				out[i] = (*this)[i];
		return out;
	}

public:
	constexpr auto diagonal() requires (N == M) { return triangularize(std::equal_to{}); }

	constexpr auto lower() requires (N == M) { return triangularize(std::less_equal{}); }

	constexpr auto lowerStrict() requires (N == M) { return triangularize(std::less{}); }

	constexpr auto upper() requires (N == M) { return triangularize(std::greater_equal{}); }

	constexpr auto upperStrict() requires (N == M) { return triangularize(std::greater{}); }

#pragma endregion
};

#pragma region StdOverload
export namespace std {
	template <typename T, int N, int M>
	constexpr auto lerp(const mat<T, N, M>& a, const mat<T, N, M>& b, const auto& t) {
		return a + t * (b - a);
	}
}
#pragma endregion

