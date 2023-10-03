export module util;

import std;

// Requires future version of MSVC to be able to export as a module
#define NDEDUCING_THIS

export {
	// vec utility
	template <typename T, int N> requires (std::is_scalar_v<T>&& N > 0)
		struct vec {
		using value_type = T;

		T raw[N];

#ifndef NDEDUCING_THIS
		constexpr auto&& data(this auto&& self) noexcept { return self.raw; }
		constexpr auto&& begin(this auto&& self) noexcept { return self.raw; }
		constexpr auto&& end(this auto&& self) noexcept { return self.raw + N; }
		constexpr auto&& operator[](this auto&& self, int i) noexcept { return self.raw[i]; }
#else 
		constexpr T* data() noexcept { return raw; }
		constexpr const T* data() const noexcept { return raw; }

		constexpr T* begin() noexcept { return raw; }
		constexpr const T* begin() const noexcept { return raw; }

		constexpr T* end() noexcept { return raw + N; }
		constexpr const T* end() const noexcept { return raw + N; }

		constexpr T& operator[](int i) noexcept { return raw[i]; }
		constexpr T operator[](int i) const noexcept { return raw[i]; }
#endif
		static constexpr int size() noexcept { return N; }

	};

	template <int N, typename T>
		constexpr auto splat(T t) noexcept { vec<T, N> u; std::ranges::fill(u, t); return u; }

	template <typename S, typename T, int N>
		constexpr auto vec_cast(const vec<T, N>& v) noexcept { vec<S, N> u; std::ranges::transform(v, u.begin(), [](T t) { return static_cast<S>(t); }); return u; }

	template <typename T, int N>
		constexpr T flatten(const vec<T, N>& pos, const vec<T, N>& dim) noexcept {
		T out = pos[0], uncompressed_dim = 1;
		for (int i = 1; i < N; ++i) {
			uncompressed_dim *= dim[i - 1];
			out += pos[i] * uncompressed_dim;
		}
		return out;
	}

	template <typename T, int N>
		constexpr vec<T, N> unflatten(T idx, const vec<T, N>& dim) noexcept {
		vec<T, N> out = { idx % dim[0] };
		T uncompressed_dim = 1;
		for (int i = 1; i < N; ++i) {
			uncompressed_dim *= dim[i - 1];
			out[i] = std::modulus{}(idx / uncompressed_dim, dim[i]);
		}
		return out;
	}


	// mat utility

	// A column ordered matrix
	template <typename T, int N>
	struct mat : vec<T, N*N> {

		static constexpr vec<int, 2> dim() { return { N, N }; }

	private:
		constexpr auto triangularize(auto&& F) {
			mat out = {};
			for (int i = 0; i < N; ++i)
				if (auto x = unflatten(i, dim(); F(x[0], x[1]));
					out[i] = (*this)[i];
			return out;
		}

	public:
		constexpr auto diagonal() { return triangularize(std::equal{}); }

		constexpr auto lower() { return triangularize(std::less_equal{}); }

		constexpr auto lowerStrict() { return triangularize(std::less{}); }

		constexpr auto upper() { return triangularize(std::greater_equal{}); }

		constexpr auto upperStrict() { return triangularize(std::greater{}); }
	};



	// operations

	template <typename V>
	concept VecLike = std::is_base_of_v<std::decay_t<V>, vec<typename V::value_type, V::size()>>;

	constexpr auto operator+(VecLike auto v) noexcept { return v; }
	constexpr auto operator-(VecLike auto v) noexcept { std::ranges::transform(v, v.begin(), std::negate{}); return v; }

	constexpr auto& operator+=(VecLike auto& u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	constexpr auto& operator-=(VecLike auto& u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	constexpr auto operator+(VecLike auto u, const VecLike auto& v) noexcept { return u += v; }
	constexpr auto operator-(VecLike auto u, const VecLike auto& v) noexcept { return u -= v; }

	constexpr VecLike auto& operator*=(VecLike auto& v, auto s) noexcept { for (auto& t : v) t *= s; return v; }
	constexpr VecLike auto& operator/=(VecLike auto& v, auto s) noexcept { for (auto& t : v) t /= s; return v; }
	constexpr VecLike auto operator*(VecLike auto v, auto s) noexcept { return v *= s; }
	constexpr VecLike auto operator*(auto s, VecLike auto v) noexcept { return v *= s; }
	constexpr VecLike auto operator/(VecLike auto v, auto s) noexcept { return v /= s; }

	constexpr std::strong_ordering operator<=>(const VecLike auto& u, const VecLike auto& v) noexcept { return std::lexicographical_compare_three_way(u.begin(), u.end(), v.begin(), v.end()); }

	constexpr auto add(VecLike auto u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s += t; return u; }
	constexpr auto sub(VecLike auto u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s -= t; return u; }
	constexpr auto mul(VecLike auto u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s *= t; return u; }
	constexpr auto div(VecLike auto u, const VecLike auto& v) noexcept { for (auto&& [s, t] : std::views::zip(u, v)) s /= t; return u; }

	constexpr auto dot(const VecLike auto& u, const VecLike auto& v) noexcept { return std::transform_reduce(u.begin(), u.end(), v.begin(), 0); }
	constexpr auto sum(const VecLike auto& v) noexcept { return std::accumulate(v.begin(), v.end(), 0); }
	constexpr auto prod(const VecLike auto& v) noexcept { return std::accumulate(v.begin(), v.end(), 1, std::multiplies{}); }

	std::ostream& operator<<(std::ostream& os, const VecLike auto& v) { os << "( "; std::ranges::copy(v, std::ostream_iterator<decltype(v)::value_type>(os, " ")); return os << ")"; }
}