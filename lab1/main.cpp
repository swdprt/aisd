#include <iostream>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <limits>


template<typename T> class Matrix {
private:
	static constexpr double kEpsilon = 1e-9;

	std::size_t _rows;
	std::size_t _cols;
	T* _data;

	static void check_dimensions(std::size_t rows, std::size_t cols) {
		if (rows == 0 || cols == 0) {
			throw std::invalid_argument("Matrix dimensios must be positive");
		}
		if (rows > std::numeric_limits<std::size_t>::max() / cols) {
			throw std::length_error("Matrix is too large");
		}
	}

	static T* allocate(std::size_t rows, std::size_t cols) {
		check_dimensions(rows, cols);
		return new T[rows * cols];
	}

	void check_index(std::size_t row, std::size_t col) const {
		if (row >= _rows || col >= _cols) {
			throw std::out_of_range("Matrix index out of range");
		}
	}

	void check_same_size(const Matrix& other) const {
		if (_rows != other._rows || _cols != other._cols) {
			throw std::invalid_argument("Matrix dimensions must match");
		}
	}

	static double magnitude(const T& value) {
		if constexpr (std::is_complex<T>::value) {
			return static_cast<double>(std::abs(value));
		}
		else {
			return static_cast<double>(std::abs(value));
		}
	}

	static bool is_zero(const T& value) {
		return magnitude(value) < kEpsilon;
	}

	static bool almost_equal(const T& a, const T& b) {
		return magnitude(a - b) < kEpsilon;
	}

public:
	Matrix(std::size_t rows, std::size_t cols, const T& value) : _rows(rows), _cols(cols), _data(allocate(rows, cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = value;
		}
	}

	Matrix(std::size_t rows, std::size_t cols, const T& lower, const T& upper): _rows(rows), _cols(cols), _data(allocate(rows, cols)) {

	}

	Matrix(const Matrix& other): _rows(other._rows), _cols(other._cols), _data(allocate(other._rows, other._cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = other._data[i];
		}
	}
};