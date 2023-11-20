#ifndef _UTILS_H_
#define _UTILS_H_

#include <ctime>

template<typename T>
size_t get_random_data(T* data, T lo, T hi, const size_t count) {
	if ((data != nullptr) && (count != 0) && (lo != hi)) {
		std::srand(std::time(nullptr));
		const double f = double(hi - lo) / double(RAND_MAX - 0);
		for (size_t i = 0; i < count; i++) {
			data[i] = static_cast<T>(static_cast<double>(std::rand()) * f) + lo;
		}
		return count;
	}
	return 0;
}

float norm_24bit_msb(unsigned char* src);
float norm_24bit_lsb(unsigned char* src);
void mon(void* buf, size_t bytes, size_t byte_width);
void mon_mfcc(float* buf, size_t len);

#endif // _UTILS_H_

