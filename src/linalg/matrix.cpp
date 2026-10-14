#include "psc/linalg/matrix.h"
#include <omp.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <iostream>

namespace psc {
namespace linalg {

// ============================================================================
// Constructors and Assignment
// ============================================================================

Matrix::Matrix(size_t rows, size_t cols, double init_val)
    : rows_(rows), cols_(cols), data_(rows * cols, init_val) {
    if (rows == 0 || cols == 0) {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }
}

Matrix::Matrix(const std::vector<std::vector<double>>& data)
    : rows_(data.size()), cols_(data.empty() ? 0 : data[0].size()), 
      data_(rows_ * cols_) {
    if (rows_ == 0) {
        throw std::invalid_argument("Matrix must have at least one row");
    }
    for (size_t i = 0; i < rows_; ++i) {
        if (data[i].size() != cols_) {
            throw std::invalid_argument("All rows must have the same number of columns");
        }
        for (size_t j = 0; j < cols_; ++j) {
            data_[i * cols_ + j] = data[i][j];
        }
    }
}

Matrix::Matrix(const Matrix& other)
    : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {}

Matrix::Matrix(Matrix&& other) noexcept
    : rows_(other.rows_), cols_(other.cols_), data_(std::move(other.data_)) {
    other.rows_ = 0;
    other.cols_ = 0;
}

Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = other.data_;
    }
    return *this;
}

Matrix& Matrix::operator=(Matrix&& other) noexcept {
    if (this != &other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = std::move(other.data_);
        other.rows_ = 0;
        other.cols_ = 0;
    }
    return *this;
}

// ============================================================================
// Accessors
// ============================================================================

double& Matrix::operator()(size_t row, size_t col) {
    if (row >= rows_ || col >= cols_) {
        throw std::out_of_range("Matrix index out of range");
    }
    return data_[row * cols_ + col];
}

const double& Matrix::operator()(size_t row, size_t col) const {
    if (row >= rows_ || col >= cols_) {
        throw std::out_of_range("Matrix index out of range");
    }
    return data_[row * cols_ + col];
}

// ============================================================================
// Basic Operations
// ============================================================================

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for addition");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) {
        throw std::invalid_argument("Matrix dimensions must match for subtraction");
    }
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    if (cols_ != other.rows_) {
        throw std::invalid_argument(
            "Matrix dimensions incompatible for multiplication: "
            "A.cols must equal B.rows");
    }
    // Use naive i-j-k multiplication by default
    return parallel::multiply_naive(*this, other, 1);
}

Matrix Matrix::operator*(double scalar) const {
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

Matrix Matrix::transpose() const {
    Matrix result(cols_, rows_);
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = 0; j < cols_; ++j) {
            result(j, i) = (*this)(i, j);
        }
    }
    return result;
}

double Matrix::trace() const {
    if (!is_square()) {
        throw std::invalid_argument("Trace is only defined for square matrices");
    }
    double sum = 0.0;
    for (size_t i = 0; i < rows_; ++i) {
        sum += (*this)(i, i);
    }
    return sum;
}

// ============================================================================
// Advanced Operations
// ============================================================================

double Matrix::determinant() const {
    if (!is_square()) {
        throw std::invalid_argument("Determinant is only defined for square matrices");
    }
    
    if (rows_ == 1) {
        return data_[0];
    }
    if (rows_ == 2) {
        return data_[0] * data_[3] - data_[1] * data_[2];
    }
    
    // Use LU decomposition for larger matrices
    auto lu = lu_with_pivoting();
    
    // Determinant = product of diagonal of U, adjusted by pivot sign
    double det = 1.0;
    int sign = 1;
    for (size_t i = 0; i < rows_; ++i) {
        det *= lu.U(i, i);
        // Count pivot swaps (odd number of swaps = sign flip)
        if (lu.pivots[i] != i) {
            sign = -sign;
        }
    }
    return sign * det;
}

Matrix Matrix::inverse() const {
    if (!is_square()) {
        throw std::invalid_argument("Inverse is only defined for square matrices");
    }
    
    size_t n = rows_;
    
    // Create augmented matrix [A | I]
    Matrix augmented(n, 2 * n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            augmented(i, j) = (*this)(i, j);
        }
        augmented(i, i + n) = 1.0;  // Identity on the right
    }
    
    // Gaussian elimination with partial pivoting
    for (size_t col = 0; col < n; ++col) {
        // Find pivot
        size_t pivot_row = col;
        double max_val = std::abs(augmented(col, col));
        for (size_t row = col + 1; row < n; ++row) {
            if (std::abs(augmented(row, col)) > max_val) {
                max_val = std::abs(augmented(row, col));
                pivot_row = row;
            }
        }
        
        if (max_val < 1e-10) {
            throw std::runtime_error("Matrix is singular (cannot compute inverse)");
        }
        
        // Swap rows if necessary
        if (pivot_row != col) {
            for (size_t j = 0; j < 2 * n; ++j) {
                std::swap(augmented(col, j), augmented(pivot_row, j));
            }
        }
        
        // Scale pivot row
        double pivot_val = augmented(col, col);
        for (size_t j = 0; j < 2 * n; ++j) {
            augmented(col, j) /= pivot_val;
        }
        
        // Eliminate column
        for (size_t row = 0; row < n; ++row) {
            if (row != col) {
                double factor = augmented(row, col);
                for (size_t j = 0; j < 2 * n; ++j) {
                    augmented(row, j) -= factor * augmented(col, j);
                }
            }
        }
    }
    
    // Extract inverse from right half
    Matrix inv(n, n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            inv(i, j) = augmented(i, j + n);
        }
    }
    
    return inv;
}

Matrix::LUResult Matrix::lu_with_pivoting() const {
    if (!is_square()) {
        throw std::invalid_argument("LU decomposition requires a square matrix");
    }
    
    size_t n = rows_;
    Matrix A = *this;  // Working copy
    Matrix::LUResult result(n);  // Constructor initializes L, U, and pivots
    
    for (size_t i = 0; i < n; ++i) {
        result.pivots[i] = i;
    }
    
    for (size_t k = 0; k < n; ++k) {
        // Partial pivoting: find row with maximum absolute value in column k
        size_t pivot_row = k;
        double max_val = std::abs(A(k, k));
        for (size_t i = k + 1; i < n; ++i) {
            if (std::abs(A(i, k)) > max_val) {
                max_val = std::abs(A(i, k));
                pivot_row = i;
            }
        }
        
        if (max_val < 1e-10) {
            // Singular or nearly singular matrix
            continue;
        }
        
        // Swap rows if necessary
        if (pivot_row != k) {
            for (size_t j = 0; j < n; ++j) {
                std::swap(A(k, j), A(pivot_row, j));
            }
            std::swap(result.pivots[k], result.pivots[pivot_row]);
        }
        
        // Compute multipliers and eliminate
        for (size_t i = k + 1; i < n; ++i) {
            if (std::abs(A(k, k)) > 1e-15) {
                A(i, k) /= A(k, k);
            }
            for (size_t j = k + 1; j < n; ++j) {
                A(i, j) -= A(i, k) * A(k, j);
            }
        }
    }
    
    // Extract L and U
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (i > j) {
                result.L(i, j) = A(i, j);
            } else if (i == j) {
                result.L(i, j) = 1.0;
                result.U(i, j) = A(i, j);
            } else {
                result.U(i, j) = A(i, j);
            }
        }
    }
    
    return result;
}

std::pair<Matrix, Matrix> Matrix::lu_decomposition() const {
    auto lu = lu_with_pivoting();
    return {lu.L, lu.U};
}

Matrix Matrix::solve(const Matrix& b) const {
    if (!is_square()) {
        throw std::invalid_argument("Coefficient matrix must be square");
    }
    if (b.rows() != rows_) {
        throw std::invalid_argument("b must have the same number of rows as A");
    }
    
    size_t n = rows_;
    size_t nrhs = b.cols();
    
    // Solve using LU decomposition with pivoting
    auto lu = lu_with_pivoting();
    
    // Apply pivot permutations to b
    Matrix b_permuted(n, nrhs);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < nrhs; ++j) {
            b_permuted(i, j) = b(lu.pivots[i], j);
        }
    }
    
    // Forward substitution: L y = b_permuted
    Matrix y(n, nrhs);
    for (size_t col = 0; col < nrhs; ++col) {
        for (size_t i = 0; i < n; ++i) {
            y(i, col) = b_permuted(i, col);
            for (size_t j = 0; j < i; ++j) {
                y(i, col) -= lu.L(i, j) * y(j, col);
            }
        }
    }
    
    // Backward substitution: U x = y
    Matrix x(n, nrhs);
    for (size_t col = 0; col < nrhs; ++col) {
        for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
            x(i, col) = y(i, col);
            for (size_t j = i + 1; j < n; ++j) {
                x(i, col) -= lu.U(i, j) * x(j, col);
            }
            if (std::abs(lu.U(i, i)) > 1e-15) {
                x(i, col) /= lu.U(i, i);
            } else {
                throw std::runtime_error("Singular matrix in back substitution");
            }
        }
    }
    
    return x;
}

int Matrix::rank_from_lu(const Matrix::LUResult& lu, double tolerance) const {
    size_t n = rows_;
    int rank = 0;
    
    for (size_t i = 0; i < n; ++i) {
        if (std::abs(lu.U(i, i)) > tolerance) {
            rank++;
        }
    }
    
    return rank;
}

int Matrix::rank(double tolerance) const {
    if (!is_square()) {
        // For non-square matrices, use a different approach
        // Compute rank via SVD approximation (simplified here)
        // For now, return min(rows, cols) as an upper bound
        return static_cast<int>(std::min(rows_, cols_));
    }
    auto lu = lu_with_pivoting();
    return rank_from_lu(lu, tolerance);
}

bool Matrix::is_symmetric(double tolerance) const {
    if (!is_square()) {
        return false;
    }
    for (size_t i = 0; i < rows_; ++i) {
        for (size_t j = i + 1; j < cols_; ++j) {
            if (std::abs((*this)(i, j) - (*this)(j, i)) > tolerance) {
                return false;
            }
        }
    }
    return true;
}

// ============================================================================
// Static Factory Methods
// ============================================================================

Matrix Matrix::identity(size_t n) {
    Matrix result(n, n);
    for (size_t i = 0; i < n; ++i) {
        result(i, i) = 1.0;
    }
    return result;
}

Matrix Matrix::zeros(size_t rows, size_t cols) {
    return Matrix(rows, cols, 0.0);
}

Matrix Matrix::ones(size_t rows, size_t cols) {
    return Matrix(rows, cols, 1.0);
}

// ============================================================================
// Utility
// ============================================================================

void Matrix::print() const {
    std::cout << "Matrix (" << rows_ << "x" << cols_ << "):" << std::endl;
    for (size_t i = 0; i < rows_; ++i) {
        std::cout << "  [";
        for (size_t j = 0; j < cols_; ++j) {
            std::cout << std::setw(10) << std::setprecision(4) << (*this)(i, j);
            if (j < cols_ - 1) std::cout << " ";
        }
        std::cout << "]" << std::endl;
    }
}

// ============================================================================
// Parallel Operations
// ============================================================================

namespace parallel {

Matrix multiply_naive(const Matrix& A, const Matrix& B, int threads) {
    if (A.cols() != B.rows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
    size_t M = A.rows();
    size_t N = B.cols();
    size_t K = A.cols();
    
    Matrix C(M, N);
    
    #pragma omp parallel for num_threads(threads) if(threads > 1)
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < K; ++k) {
                sum += A(i, k) * B(k, j);
            }
            C(i, j) = sum;
        }
    }
    
    return C;
}

Matrix multiply_ijk(const Matrix& A, const Matrix& B, int threads) {
    // Same as naive (i-j-k order)
    return multiply_naive(A, B, threads);
}

Matrix multiply_ikj(const Matrix& A, const Matrix& B, int threads) {
    if (A.cols() != B.rows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
    size_t M = A.rows();
    size_t N = B.cols();
    size_t K = A.cols();
    
    Matrix C(M, N, 0.0);
    
    #pragma omp parallel for num_threads(threads) if(threads > 1)
    for (size_t i = 0; i < M; ++i) {
        for (size_t k = 0; k < K; ++k) {
            double aik = A(i, k);
            for (size_t j = 0; j < N; ++j) {
                C(i, j) += aik * B(k, j);
            }
        }
    }
    
    return C;
}

Matrix multiply_tiled(const Matrix& A, const Matrix& B, size_t tile_size, int threads) {
    if (A.cols() != B.rows()) {
        throw std::invalid_argument("Matrix dimensions incompatible for multiplication");
    }
    
    size_t M = A.rows();
    size_t N = B.cols();
    size_t K = A.cols();
    
    Matrix C(M, N, 0.0);
    
    #pragma omp parallel for num_threads(threads) if(threads > 1) schedule(dynamic)
    for (size_t ii = 0; ii < M; ii += tile_size) {
        for (size_t jj = 0; jj < N; jj += tile_size) {
            for (size_t kk = 0; kk < K; kk += tile_size) {
                // Process tile
                size_t i_end = std::min(ii + tile_size, M);
                size_t j_end = std::min(jj + tile_size, N);
                size_t k_end = std::min(kk + tile_size, K);
                
                for (size_t i = ii; i < i_end; ++i) {
                    for (size_t k = kk; k < k_end; ++k) {
                        double aik = A(i, k);
                        for (size_t j = jj; j < j_end; ++j) {
                            C(i, j) += aik * B(k, j);
                        }
                    }
                }
            }
        }
    }
    
    return C;
}

std::pair<Matrix, Matrix> lu_decomposition(const Matrix& A, int threads) {
    (void)threads;  // Parameter reserved for future parallel implementation
    // For now, use serial LU decomposition
    // Parallel LU is complex and requires careful synchronization
    return A.lu_decomposition();
}

} // namespace parallel

} // namespace linalg
} // namespace psc
