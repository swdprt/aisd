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
	static constexpr double kEpsilon = 1e-9; // точность, с которой сравниваем элементы матриц

	std::size_t _rows;
	std::size_t _cols;
	T* _data;

	// проверка размеров матрицы
	static void check_dimensions(std::size_t rows, std::size_t cols) {
		if (rows == 0 || cols == 0) {
			throw std::invalid_argument("Matrix dimensios must be positive");
		}
		if (rows > std::numeric_limits<std::size_t>::max() / cols) {
			throw std::length_error("Matrix is too large");
		}
	}

	// выделение памяти под элементы матрицы
	static T* allocate(std::size_t rows, std::size_t cols) {
		check_dimensions(rows, cols);
		return new T[rows * cols];
	}

	// проверка арг для операций
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

	// функции для работы с числами др типов
	static double magnitude(const T& value) {
		return static_cast<double>(std::abs(value));
	}

	static bool is_zero(const T& value) {
		return magnitude(value) < kEpsilon;
	}

	static bool almost_equal(const T& a, const T& b) {
		return magnitude(a - b) < kEpsilon;
	}
		
	// генератор случайных значений для целых и веществ типов
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

	// для комплексных чисел
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
	// конструктор (размеры и знач для заполнения)
	Matrix(std::size_t rows, std::size_t cols, const T& value) : _rows(rows), _cols(cols), _data(allocate(rows, cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = value;
		}
	}

	// конструктор (случ заполнение)
	Matrix(std::size_t rows, std::size_t cols, const T& lower, const T& upper): _rows(rows), _cols(cols), _data(allocate(rows, cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = RandomGenerator<T>::generate(lower, upper);
		}
	}

	// конструктор копирования
	Matrix(const Matrix& other): _rows(other._rows), _cols(other._cols), _data(allocate(other._rows, other._cols)) {
		std::size_t total = _rows * _cols;
		for (std::size_t i = 0; i < total; i++) {
			_data[i] = other._data[i];
		}
	}

	// оператор присваивания
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

	// доступ к элементу по индексу 
	const T& operator()(std::size_t row, std::size_t col) const {
		check_index(row, col);
		return _data[row * _cols + col];
	}

	T& operator()(std::size_t row, std::size_t col) {
		check_index(row, col);
		return _data[row * _cols + col];
	}

	// сложение и вычитание матриц
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

	// умножение матриц
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

	// умнож и деление на скаляр 
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

	// след
	T trace() const {
		check_square();
		T sum = T{};
		for (std::size_t i = 0; i < _rows; i++) {
			sum += (*this)(i, i);
		}
		return sum;
	}

	// сравнеие матриц с заданной точностью 
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

	// вывод матрицы в поток
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

// умножение скаляра на матрицу
template<typename T> 
Matrix<T> operator*(const T& scalar, const Matrix<T>& matrix) {
	return matrix * scalar;
}

// единичная матрица 
template<typename T>
Matrix<T> identity_matrix(std::size_t size) {
	Matrix<T> result(size, size, T{});
	for (std::size_t i = 0; i < size; ++i) {
		result(i, i) = T{ 1 };
	}
	return result;
}

// обратная матрица
template<typename T>
Matrix<T> inverse(const Matrix<T>& matrix) {
	if (matrix.rows() != matrix.cols()) {
		throw std::invalid_argument("Matrix must be square to invert");
	}
	std::size_t n = matrix.rows();

	// расширенная матрица 
	Matrix<T> augmented(n, 2 * n, T{});
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			augmented(i, j) = matrix(i, j);
		}
		augmented(i, n + i) = T{ 1 };
	}

	for (std::size_t col = 0; col < n; ++col) {
		// поиск ведущего элемента в столбце
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
		// перестановка строк, чтоб вед эл был на диагонали
		if (pivot != col) {
			for (std::size_t j = 0; j < 2 * n; ++j) {
				std::swap(augmented(col, j), augmented(pivot, j));
			}
		}

		// деление вед строки на вед эл
		T divisor = augmented(col, col);
		for (std::size_t j = 0; j < 2 * n; ++j) {
			augmented(col, j) = augmented(col, j) / divisor;
		}

		// обнуление
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

	// обр матрица
	Matrix<T> result(n, n, T{});
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = 0; j < n; ++j) {
			result(i, j) = augmented(i, n + j);
		}
	}
	return result;
}

// демонстрация всех операторов и методов на случайных матрицах 
template<typename T>
void demonstrate_operations(const std::string& title, const T& lower,
	const T& upper, const T& scalar) {
	std::cout << "\n[ " << title << " ]\n";

	Matrix<T> a(3, 3, lower, upper);
	Matrix<T> b(3, 3, lower, upper);
	std::cout << "A:\n" << a << "B:\n" << b;

	// ариф операции
	std::cout << "A + B:\n" << (a + b);
	std::cout << "A - B:\n" << (a - b);
	std::cout << "A * B:\n" << (a * b);
	std::cout << "A * s:\n" << (a * scalar);
	std::cout << "s * A:\n" << (scalar * a);
	std::cout << "A / s:\n" << (a / scalar);
	std::cout << "trace(A) = " << a.trace() << '\n';

	// сравнения
	std::cout << "A == B: " << (a == b) << ", A != B: " << (a != b) << '\n';
	std::cout << "A == A: " << (a == a) << ", A != A: " << (a != a) << '\n';

	// копирование, присваивание и запись элемента через оператор ()
	Matrix<T> copy(a);
	copy(0, 0) = b(1, 1);
	std::cout << "Copy of A with (0,0) set to B(1,1):\n" << copy;
	std::cout << "copy == A: " << (copy == a) << '\n';
	copy = a;
	std::cout << "After assignment copy == A: " << (copy == a) << '\n';

	// конструктор с заполнением и методы rows(), cols()
	Matrix<T> filled(2, 3, scalar);
	std::cout << "Filled 2x3: rows = " << filled.rows() << ", cols = " << filled.cols() << '\n' << filled;

	// умножение прямоугольных матриц
	Matrix<T> left(2, 3, lower, upper);
	Matrix<T> right(3, 2, lower, upper);
	std::cout << "(2x3) * (3x2):\n" << (left * right);
}

// выполняет действие и выводит сообщение исключения
template<typename Action>
void expect_exception(const std::string& description, Action action) {
	try {
		action();
		std::cout << description << ": no exception (unexpected)\n";
	}
	catch (const std::exception& e) {
		std::cout << description << ": caught " << e.what() << '\n';
	}
}

int main() {
	try {
		using ComplexFloat = std::complex<float>;
		using ComplexDouble = std::complex<double>;

		demonstrate_operations<int>("int", 1, 9, 2);
		demonstrate_operations<float>("float", 1.0F, 9.0F, 2.0F);
		demonstrate_operations<double>("double", 1.0, 9.0, 2.0);
		demonstrate_operations<ComplexFloat>("std::complex<float>",
			ComplexFloat(1.0F, 1.0F), ComplexFloat(9.0F, 9.0F), ComplexFloat(2.0F, 1.0F));
		demonstrate_operations<ComplexDouble>("std::complex<double>",
			ComplexDouble(1.0, 1.0), ComplexDouble(9.0, 9.0), ComplexDouble(2.0, 1.0));

		std::cout << "\n[ Task: inverse of a 3x3 matrix ]\n";

		// заданная матрица, ответ: (1/4) * [[3,2,1],[2,4,2],[1,2,3]]
		Matrix<double> given(3, 3, 0.0);
		given(0, 0) = 2.0;  given(0, 1) = -1.0; given(0, 2) = 0.0;
		given(1, 0) = -1.0; given(1, 1) = 2.0;  given(1, 2) = -1.0;
		given(2, 0) = 0.0;  given(2, 1) = -1.0; given(2, 2) = 2.0;
		Matrix<double> given_inverse = inverse(given);
		std::cout << "A:\n" << given << "A^-1:\n" << given_inverse;
		std::cout << "A * A^-1 == E: " << (given * given_inverse == identity_matrix<double>(3)) << '\n';

		// случайная вещественная матрица
		Matrix<double> random_real(3, 3, -9.0, 9.0);
		Matrix<double> random_real_inverse = inverse(random_real);
		std::cout << "Random A:\n" << random_real << "A^-1:\n" << random_real_inverse;
		std::cout << "A * A^-1 == E: " << (random_real * random_real_inverse == identity_matrix<double>(3)) << '\n';

		// случайная комплексная матрица
		Matrix<ComplexDouble> random_complex(3, 3,
			ComplexDouble(-9.0, -9.0), ComplexDouble(9.0, 9.0));
		Matrix<ComplexDouble> random_complex_inverse = inverse(random_complex);
		std::cout << "Random complex A:\n" << random_complex << "A^-1:\n" << random_complex_inverse;
		std::cout << "A * A^-1 == E: " << (random_complex * random_complex_inverse == identity_matrix<ComplexDouble>(3)) << '\n';

		// некорректные ситуации
		std::cout << "\n[ Exceptions ]\n";
		expect_exception("Zero dimension", [] { Matrix<int> m(0, 3, 1); });
		expect_exception("Index out of range", [] {
			Matrix<int> m(2, 2, 1);
			std::cout << m(5, 5);
			});
		expect_exception("Add different sizes", [] {
			Matrix<int> x(2, 2, 1);
			Matrix<int> y(3, 3, 1);
			static_cast<void>(x + y);
			});
		expect_exception("Multiply incompatible", [] {
			Matrix<int> x(2, 3, 1);
			Matrix<int> y(2, 3, 1);
			static_cast<void>(x * y);
			});
		expect_exception("Divide by zero", [] {
			Matrix<double> x(2, 2, 1.0);
			static_cast<void>(x / 0.0);
			});
		expect_exception("Trace of non-square", [] {
			Matrix<int> x(2, 3, 1);
			static_cast<void>(x.trace());
			});
		expect_exception("Inverse of non-square", [] {
			Matrix<double> x(2, 3, 1.0);
			static_cast<void>(inverse(x));
			});
		expect_exception("Inverse of singular", [] {
			Matrix<double> x(3, 3, 1.0);
			static_cast<void>(inverse(x));
			});
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal error: " << e.what() << '\n';
		return 1;
	}
	return 0;
}