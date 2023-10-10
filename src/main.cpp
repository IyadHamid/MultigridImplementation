import std;
import util;

void practice1() {
	auto function = []<typename T, int N>(vec<T, N> x) {
		T out = 1;
		for (auto i : x)
			out *= std::sin(i);
		return out;
	};

	auto c_min = splat<2>(0.0);
	auto c_max = splat<2>(1.0);

	auto c_range = c_max - c_min;
	auto h = 0.1;

	auto total_points = vec_cast<int>(c_range / h) + splat<2>(1);
	auto size = prod(total_points);

	std::vector<double> field;
	field.reserve(size);


	// Should replace with md-span
	for (int i = 0; i < size; ++i) {
		auto ioffset = vec_cast<double>(unflatten(i, total_points));
		auto pos = mul(div(ioffset, vec_cast<double>(total_points)), c_range) + c_min;
		field.push_back(function(pos));
	}

	std::ofstream fout("out.txt");
	for (auto p : field)
		fout << p << "\n";
}



void practice2() {
	auto function = [](vec<double, 2> x) -> vec<double, 2> {
		return { std::sin(x[0]) * std::sin(x[1]), std::cos(x[0]) * std::cos(x[1]) }; 
	};

	auto c_min = splat<2>(0.0);
	auto c_max = splat<2>(1.0);

	auto c_range = c_max - c_min;
	auto h = 0.1;

	auto total_points = vec_cast<int>(c_range / h) + splat<2>(1);
	auto size = prod(total_points);

	std::vector<vec<double, 2>> field;
	field.reserve(size);


	for (int i = 0; i < size; ++i) {
		auto ioffset = vec_cast<double>(unflatten(i, total_points));
		auto pos = mul(div(ioffset, vec_cast<double>(total_points)), c_range) + c_min;
		field.push_back(function(pos));
	}

	std::ofstream fout("out.txt");
	for (auto p : field)
		fout << p[0] << "\t" << p[1] << "\n";

}


// values : f(min_x,min_y); f(min_x,max_y); f(max_x,min_y); f(max_x,max_y)
export template <typename T>
constexpr auto interp(const vec<T, 2>& min_point, const vec<T, 2>& max_point, std::span<T, 4> values, const vec<T, 2>& pos) {
	auto range = max_point - min_point;
	auto ratio = div(pos - min_point, range);

	vec<T, 2> x_mins = { values[0], values[1] };
	vec<T, 2> x_maxs = { values[2], values[3] };
	
	// (lerp at min_y, lerp at max_y)
	auto x_interps = std::lerp(x_mins, x_maxs, ratio[0]);

	return std::lerp(x_interps[0], x_interps[1]);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
	practice2();
	return 0;
}

