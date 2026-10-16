#include <iostream>
#include <cmath>
#include <complex>
#include <stdexcept>
#include <limits>
#include <random>
#include <type_traits>
#include <cstddef>


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

	void check_square() const {
		if (_rows != _cols) {
			throw std::invalid_argument("Matrix must be square");
		}
	}

	static double magnitude(const T& value) {
		return static_cast<double>(std::abs(value));
	}

	static bool is_zero(const T& value) {
		return magnitude(value) < kEpsilon;
	}

	static bool almost_equal(const T& a, const T& b) {
		return magnitude(a - b) < kEpsilon;
	}
		
	template<typename T>
	struct RandomGenerator {
		static T generate(const T& lower, const T& upper) {
			static std::mt19937 gen(std::random_device{}());
			if constexpr (std::is_integral_v<T>) {
				std::uniform_int_distribution<T> d(lower, upper);
				return d(gen);
			}
			else {
				std::uniform_real_distribution<T> d(lower, upper);
				return d(gen);
			}
		}
	};

	template<typename U>
	struct RandomGenerator<std::complex<U>> {
		static std::complex<U> generate(const std::complex<U>& lower,
			const std::complex<U>& upper) {
			static std::mt19937 gen(std::random_device{}());
			std::uniform_real_distribution<U> dr(lower.real(), upper.real());
			std::uniform_real_distribution<U> di(lower.imag(), upper.imag());
			return std::complex<U>(dr(gen), di(gen));
		}
	};

public:
	Matrix(std::size_t rows, std::size_t cols, const T& value) : _rows(rows), _cols(cols), _data(allocate(rows, cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = value;
		}
	}

	Matrix(std::size_t rows, std::size_t cols, const T& lower, const T& upper): _rows(rows), _cols(cols), _data(allocate(rows, cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = RandomGenerator<T>::generate(lower, upper);
		}
	}

	Matrix(const Matrix& other): _rows(other._rows), _cols(other._cols), _data(allocate(other._rows, other._cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = other._data[i];
		}
	}

	Matrix& operator=(const Matrix& other) {
		if (this == &other) {
			return *this;
		}
		Matrix temp(other);
		swap(temp);
		return *this;
	}

	~Matrix() {
		delete[] _data;
	}

	void swap(Matrix& other) noexcept {
		std::swap(_rows, other._rows);
		std::swap(_cols, other._cols);
		std::swap(_data, other._data);
	}

	std::size_t rows() const { return _rows; }
	std::size_t cols() const { return _cols; }

	const T& operator()(std::size_t row, std::size_t col) const {
		check_index(row, col);
		return _data[row * _cols + col];
	}

	T& operator()(std::size_t row, std::size_t col) {
		check_index(row, col);
		return _data[row * _cols + col];
	}

	Matrix operator+(const Matrix& other) const {
		check_same_size(other);
		Matrix result(_rows, _cols, T{});
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			result._data[i] = _data[i] + other._data[i];
		}
		return result;
	}

	Matrix operator-(const Matrix& other) const {
		check_same_size(other);
		Matrix result(_rows, _cols, T{});
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; ++i) {
			result._data[i] = _data[i] - other._data[i];
		}
		return result;
	}

	Matrix operator*(const Matrix& other) const {
		if (_cols != other._rows) {
			throw std::invalid_argument("Matrix dimensions are incompatible for multiplication");
		}
		Matrix result(_rows, other._cols, T{});
		for (std::size_t i = 0; i < _rows; ++i) {
			for (std::size_t j = 0; j < other._cols; ++j) {
				T sum = T{};
				for (std::size_t k = 0; k < _cols; ++k) {
					sum += (*this)(i, k) * other(k, j);
				}
				result(i, j) = sum;
			}
		}
		return result;
	}

	Matrix operator*(const T& scalar) const {
		Matrix result(_rows, _cols, T{});
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; ++i) {
			result._data[i] = _data[i] * scalar;
		}
		return result;
	}

	Matrix operator/(const T& scalar) const {
		if (is_zero(scalar)) {
			throw std::invalid_argument("Division by zero");
		}
		Matrix result(_rows, _cols, T{});
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; ++i) {
			result._data[i] = _data[i] / scalar;
		}
		return result;
	}

	T trace() const {
		check_square();
		T sum = T{};
		for (std::size_t i = 0; i < _rows; i++) {
			sum += (*this)(i, i);
		}
		return sum;
	}

	bool operator==(const Matrix& other) const {
		if (_rows != other._rows || _cols != other._cols) {
			return false;
		}
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; ++i) {
			if (!almost_equal(_data[i], other._data[i])) {
				return false;
			}
		}
		return true;
	}

	bool operator!=(const Matrix& other) const {
		return !(*this == other);
	}

	friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix) {
		for (std::size_t i = 0; i < matrix._rows; i++) {
			for (std::size_t j = 0; j < matrix._cols; j++) {
				os << matrix(i, j);
				if (j + 1 < matrix._cols) {
					os << ' ';
				}
			}
			os << '\n';
		}
		return os;
	}

};

template<typename T> 
Matrix<T> operator*(const T& scalar, const Matrix<T>& matrix) {
	return matrix * scalar;
}

template<typename T>
Matrix<T> inverse(const Matrix<T>& matrix) {
	if (matrix.rows() != matrix.cols()) {
		throw std::invalid_argument("Matrix must be square to invert");
	}
	std::size_t n = matrix.rows();

	Matrix<T> augmented(n, 2 * n, T{});
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			augmented(i, j) = matrix(i, j);
		}
		augmented(i, n + i) = T{ 1 };
	}

	for (std::size_t col = 0; col < n; ++col) {
		std::size_t pivot = col;
		double best = static_cast<double>(std::abs(augmented(col, col)));
		for (std::size_t row = col + 1; row < n; ++row) {
			double current = static_cast<double>(std::abs(augmented(row, col)));
			if (current > best) {
				best = current;
				pivot = row;
			}
		}
		if (best < 1e-12) {
			throw std::runtime_error("Matrix is singular and cannot be inverted");
		}
		if (pivot != col) {
			for (std::size_t j = 0; j < 2 * n; ++j) {
				std::swap(augmented(col, j), augmented(pivot, j));
			}
		}

		T divisor = augmented(col, col);
		for (std::size_t j = 0; j < 2 * n; ++j) {
			augmented(col, j) = augmented(col, j) / divisor;
		}

		for (std::size_t row = 0; row < n; ++row) {
			if (row == col) {
				continue;
			}
			T factor = augmented(row, col);
			for (std::size_t j = 0; j < 2 * n; ++j) {
				augmented(row, j) = augmented(row, j) - factor * augmented(col, j);
			}
		}
	}

	Matrix<T> result(n, n, T{});
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			result(i, j) = augmented(i, n + j);
		}
	}
	return result;
}

int main() {
	try {
		std::cout << "[1] Integer matrix\n";
		Matrix<int> a(3, 3, 1, 9);
		std::cout << "A:\n" << a;

		Matrix<int> b(3, 3, 1, 9);
		std::cout << "B:\n" << b;

		std::cout << "A + B:\n" << (a + b);
		std::cout << "A - B:\n" << (a - b);
		std::cout << "A * B:\n" << (a * b);
		std::cout << "A * 2:\n" << (a * 2);
		std::cout << "2 * A:\n" << (2 * a);
		std::cout << "A / 2:\n" << (a / 2);
		std::cout << "trace(A) = " << a.trace() << '\n';
		std::cout << "A == B: " << (a == b) << '\n';
		std::cout << "A != B: " << (a != b) << '\n';

		std::cout << "\n[2] Double matrix and inverse (3x3)\n";
		Matrix<double> c(3, 3, 1.0, 9.0);
		std::cout << "C:\n" << c;

		Matrix<double> inv_c = inverse(c);
		std::cout << "C^-1:\n" << inv_c;

		Matrix<double> identity = c * inv_c;
		std::cout << "C * C^-1 (should be identity):\n" << identity;

		std::cout << "\n[3] Complex matrix\n";
		using Complex = std::complex<double>;
		Matrix<Complex> d(2, 2, Complex(1.0, 0.0));
		std::cout << "D:\n" << d;
		std::cout << "trace(D) = " << d.trace() << '\n';

		std::cout << "\n[4] Exception handling\n";
		try {
			Matrix<int> bad(2, 3, 1);
			Matrix<int> another(2, 3, 1);
			std::cout << (bad * another);
		}
		catch (const std::exception& e) {
			std::cout << "Caught: " << e.what() << '\n';
		}

		try {
			Matrix<int> small(2, 2, 1);
			std::cout << small(5, 5);
		}
		catch (const std::exception& e) {
			std::cout << "Caught: " << e.what() << '\n';
		}

	}
	catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}