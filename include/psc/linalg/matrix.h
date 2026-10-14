#pragma once

#include <vector>
#include <stdexcept>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace psc {
namespace linalg {

class Matrix {
public:
    // Constructors
    Matrix(size_t rows, size_t cols, double init_val = 0.0);
    Matrix(const std::vector<std::vector<double>>& data);
    Matrix(const Matrix& other);
    Matrix(Matrix&& other) noexcept;
    ~Matrix() = default;

    // Assignment
    Matrix& operator=(const Matrix& other);
    Matrix& operator=(Matrix&& other) noexcept;

    // Accessors
    double& operator()(size_t row, size_t col);
    const double& operator()(size_t row, size_t col) const;
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t size() const { return rows_ * cols_; }

    // Basic operations
    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;  // Matrix multiplication
    Matrix operator*(double scalar) const;
    
    Matrix transpose() const;
    double trace() const;
    
    // Advanced operations
    double determinant() const;
    Matrix inverse() const;
    std::pair<Matrix, Matrix> lu_decomposition() const;
    LUResult lu_with_pivoting() const;  // Returns L, U, and pivot information
    Matrix solve(const Matrix& b) const;  // Solve Ax = b
    int rank(double tolerance = 1e-10) const;

    // Static factory methods
    static Matrix identity(size_t n);
    static Matrix zeros(size_t rows, size_t cols);
    static Matrix ones(size_t rows, size_t cols);

    // Utility
    void print() const;
    bool is_square() const { return rows_ == cols_; }
    bool is_symmetric(double tolerance = 1e-10) const;

private:
    size_t rows_;
    size_t cols_;
    std::vector<double> data_;  // Row-major storage

    // Forward declaration for helper struct
    struct LUResult;
    
    // Helper for rank computation
    int rank_from_lu(const LUResult& lu, double tolerance) const;
};

// Helper for LU decomposition (defined after Matrix is complete)
struct Matrix::LUResult {
    Matrix L;
    Matrix U;
    std::vector<size_t> pivots;
    
    // Constructor to initialize matrices with size n
    LUResult(size_t n) : L(Matrix::identity(n)), U(n, n), pivots(n) {}
};

// Free functions for parallel operations
namespace parallel {

// Matrix multiplication variants
Matrix multiply_naive(const Matrix& A, const Matrix& B, int threads = 1);
Matrix multiply_ijk(const Matrix& A, const Matrix& B, int threads = 1);
Matrix multiply_ikj(const Matrix& A, const Matrix& B, int threads = 1);
Matrix multiply_tiled(const Matrix& A, const Matrix& B, size_t tile_size = 64, int threads = 1);

// Parallel Gaussian elimination for LU decomposition
std::pair<Matrix, Matrix> lu_decomposition(const Matrix& A, int threads = 1);

} // namespace parallel

} // namespace linalg
} // namespace psc
