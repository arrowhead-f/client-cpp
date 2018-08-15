#pragma once

template<typename T>
vector<T>
split(const T &str, const T &delimiters) {
	vector<T> v;

	typename T::size_type start = 0;

	auto pos = str.find_first_of(delimiters, start);

	while (pos != T::npos) {
		if (pos != start)
			v.emplace_back(str, start, pos - start);
		start = pos + 1;
		pos = str.find_first_of(delimiters, start);
	}

	if (start < str.length()) {
		v.emplace_back(str, start, str.length() - start);
	}
		
	return v;
}
