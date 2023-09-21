import std;
import util;

template <typename T, int N> 
T function(vec<T, N> x) {
	T out = 1;
	for (auto i : x)
		out *= std::sin(i);
	return out;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
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

	return 0;
}

