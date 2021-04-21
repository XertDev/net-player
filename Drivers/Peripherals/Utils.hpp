#ifndef UTILS_HPP_
#define UTILS_HPP_

template<typename T, typename A, typename B>
inline bool inRange(T value, A min, B max) {
	return value <= max && value >= min;
}

#endif UTILS_HPP_
