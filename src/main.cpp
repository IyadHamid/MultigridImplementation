import std;
import util;

template <typename T, int N> 
T function(vec<T, N> x) {
	T out = 1;
	for (auto i : x)
		out *= sin(i);
	return out;
}


int main() {
	auto c_min = splat<2>(0.0), c_max = splat<2>(1.0);
	 
	auto c_range = c_max - c_min;
	auto h = 0.1;
	auto total_points = vec_cast<int>(c_range / h) + splat<2>(1);

	std::vector<double> points(prod(total_points));

	std::cout << points.size() << std::endl;
	return 0;
}

