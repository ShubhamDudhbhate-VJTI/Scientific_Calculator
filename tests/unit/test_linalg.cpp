#include <gtest/gtest.h>
#include "psc/linalg/matrix.h"
#include <cmath>
#include <random>

using namespace psc::linalg;

class MatrixTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

// ============================================================================
// Basic Construction and Access
// ============================================================================

TEST_F(MatrixTest, ConstructorSize) {
    Matrix m(3, 4, 2.0);
    EXPECT_EQ(m.rows(), 3);
    EXPECT_EQ(m.cols(), 4);
    EXPECT_EQ(m.size(), 12);
    EXPECT_DOUBLE_EQ(m(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(m(2, 3), 2.0);
}

TEST_F(MatrixTest, ConstructorFromVector) {
    std::vector<std::vector<double>> data = {{1, 2, 3}, {4, 5, 6}};
    Matrix m(data);
    EXPECT_EQ(m.rows(), 2);
    EXPECT_EQ(m.cols(), 3);
    EXPECT_DOUBLE_EQ(m(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(m(0, 2), 3.0);
    EXPECT_DOUBLE_EQ(m(1, 1), 5.0);
}

TEST_F(MatrixTest, ConstructorFromVectorInvalid) {
    std::vector<std::vector<double>> data = {{1, 2}, {3, 4, 5}};
    EXPECT_THROW(Matrix m(data), std::invalid_argument);
}

TEST_F(MatrixTest, CopyConstructor) {
    Matrix m1(2, 2, 1.0);
    Matrix m2(m1);
    m1(0, 0) = 5.0;
    EXPECT_DOUBLE_EQ(m2(0, 0), 1.0);  // Deep copy
}

TEST_F(MatrixTest, MoveConstructor) {
    Matrix m1(2, 2, 1.0);
    Matrix m2(std::move(m1));
    EXPECT_EQ(m2.rows(), 2);
    EXPECT_EQ(m2.cols(), 2);
    EXPECT_EQ(m1.rows(), 0);  // Moved from
}

TEST_F(MatrixTest, IndexOutOfBounds) {
    Matrix m(2, 2);
    EXPECT_THROW(m(2, 0), std::out_of_range);
    EXPECT_THROW(m(0, 2), std::out_of_range);
}

// ============================================================================
// Basic Operations
// ============================================================================

TEST_F(MatrixTest, Addition) {
    Matrix m1(2, 2, 1.0);
    Matrix m2(2, 2, 2.0);
    Matrix m3 = m1 + m2;
    EXPECT_DOUBLE_EQ(m3(0, 0), 3.0);
    EXPECT_DOUBLE_EQ(m3(1, 1), 3.0);
}

TEST_F(MatrixTest, AdditionDimensionMismatch) {
    Matrix m1(2, 2);
    Matrix m2(2, 3);
    EXPECT_THROW(m1 + m2, std::invalid_argument);
}

TEST_F(MatrixTest, Subtraction) {
    Matrix m1(2, 2, 5.0);
    Matrix m2(2, 2, 3.0);
    Matrix m3 = m1 - m2;
    EXPECT_DOUBLE_EQ(m3(0, 0), 2.0);
}

TEST_F(MatrixTest, ScalarMultiplication) {
    Matrix m(2, 2, 2.0);
    Matrix m2 = m * 3.0;
    EXPECT_DOUBLE_EQ(m2(0, 0), 6.0);
}

TEST_F(MatrixTest, Transpose) {
    Matrix m(2, 3);
    m(0, 0) = 1; m(0, 1) = 2; m(0, 2) = 3;
    m(1, 0) = 4; m(1, 1) = 5; m(1, 2) = 6;
    
    Matrix mt = m.transpose();
    EXPECT_EQ(mt.rows(), 3);
    EXPECT_EQ(mt.cols(), 2);
    EXPECT_DOUBLE_EQ(mt(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(mt(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(mt(2, 1), 6.0);
}

TEST_F(MatrixTest, Trace) {
    Matrix m = Matrix::identity(3);
    m(0, 0) = 2.0;
    m(1, 1) = 3.0;
    m(2, 2) = 4.0;
    EXPECT_DOUBLE_EQ(m.trace(), 9.0);
}

TEST_F(MatrixTest, TraceNonSquare) {
    Matrix m(2, 3);
    EXPECT_THROW(m.trace(), std::invalid_argument);
}

// ============================================================================
// Matrix Multiplication
// ============================================================================

TEST_F(MatrixTest, MultiplicationBasic) {
    Matrix A(2, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 4; A(1, 1) = 5; A(1, 2) = 6;
    
    Matrix B(3, 2);
    B(0, 0) = 7; B(0, 1) = 8;
    B(1, 0) = 9; B(1, 1) = 10;
    B(2, 0) = 11; B(2, 1) = 12;
    
    Matrix C = A * B;
    EXPECT_EQ(C.rows(), 2);
    EXPECT_EQ(C.cols(), 2);
    EXPECT_DOUBLE_EQ(C(0, 0), 58.0);  // 1*7 + 2*9 + 3*11
    EXPECT_DOUBLE_EQ(C(0, 1), 64.0);  // 1*8 + 2*10 + 3*12
    EXPECT_DOUBLE_EQ(C(1, 0), 139.0); // 4*7 + 5*9 + 6*11
    EXPECT_DOUBLE_EQ(C(1, 1), 154.0); // 4*8 + 5*10 + 6*12
}

TEST_F(MatrixTest, MultiplicationDimensionMismatch) {
    Matrix A(2, 3);
    Matrix B(2, 2);
    EXPECT_THROW(A * B, std::invalid_argument);
}

TEST_F(MatrixTest, MultiplicationIdentity) {
    Matrix A(3, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 4; A(1, 1) = 5; A(1, 2) = 6;
    A(2, 0) = 7; A(2, 1) = 8; A(2, 2) = 9;
    
    Matrix I = Matrix::identity(3);
    Matrix C = A * I;
    
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ(C(i, j), A(i, j));
        }
    }
}

// ============================================================================
// Parallel Multiplication Variants
// ============================================================================

TEST_F(MatrixTest, ParallelMultiplyNaive) {
    Matrix A(2, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 4; A(1, 1) = 5; A(1, 2) = 6;
    
    Matrix B(3, 2);
    B(0, 0) = 7; B(0, 1) = 8;
    B(1, 0) = 9; B(1, 1) = 10;
    B(2, 0) = 11; B(2, 1) = 12;
    
    Matrix C = parallel::multiply_naive(A, B, 1);
    EXPECT_DOUBLE_EQ(C(0, 0), 58.0);
    EXPECT_DOUBLE_EQ(C(1, 1), 154.0);
}

TEST_F(MatrixTest, ParallelMultiplyIKJ) {
    Matrix A(2, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 4; A(1, 1) = 5; A(1, 2) = 6;
    
    Matrix B(3, 2);
    B(0, 0) = 7; B(0, 1) = 8;
    B(1, 0) = 9; B(1, 1) = 10;
    B(2, 0) = 11; B(2, 1) = 12;
    
    Matrix C = parallel::multiply_ikj(A, B, 1);
    EXPECT_DOUBLE_EQ(C(0, 0), 58.0);
    EXPECT_DOUBLE_EQ(C(1, 1), 154.0);
}

TEST_F(MatrixTest, ParallelMultiplyTiled) {
    Matrix A(2, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 4; A(1, 1) = 5; A(1, 2) = 6;
    
    Matrix B(3, 2);
    B(0, 0) = 7; B(0, 1) = 8;
    B(1, 0) = 9; B(1, 1) = 10;
    B(2, 0) = 11; B(2, 1) = 12;
    
    Matrix C = parallel::multiply_tiled(A, B, 2, 1);
    EXPECT_DOUBLE_EQ(C(0, 0), 58.0);
    EXPECT_DOUBLE_EQ(C(1, 1), 154.0);
}

TEST_F(MatrixTest, ParallelMultiplyConsistency) {
    // Test that all variants give the same result
    Matrix A(10, 10);
    Matrix B(10, 10);
    
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-10.0, 10.0);
    
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10; ++j) {
            A(i, j) = dis(gen);
            B(i, j) = dis(gen);
        }
    }
    
    Matrix C1 = parallel::multiply_naive(A, B, 1);
    Matrix C2 = parallel::multiply_ikj(A, B, 1);
    Matrix C3 = parallel::multiply_tiled(A, B, 4, 1);
    
    for (size_t i = 0; i < 10; ++i) {
        for (size_t j = 0; j < 10; ++j) {
            EXPECT_NEAR(C1(i, j), C2(i, j), 1e-10);
            EXPECT_NEAR(C1(i, j), C3(i, j), 1e-10);
        }
    }
}

// ============================================================================
// Determinant
// ============================================================================

TEST_F(MatrixTest, Determinant1x1) {
    Matrix m(1, 1);
    m(0, 0) = 5.0;
    EXPECT_DOUBLE_EQ(m.determinant(), 5.0);
}

TEST_F(MatrixTest, Determinant2x2) {
    Matrix m(2, 2);
    m(0, 0) = 1; m(0, 1) = 2;
    m(1, 0) = 3; m(1, 1) = 4;
    EXPECT_DOUBLE_EQ(m.determinant(), -2.0);  // 1*4 - 2*3
}

TEST_F(MatrixTest, DeterminantIdentity) {
    Matrix I = Matrix::identity(4);
    EXPECT_DOUBLE_EQ(I.determinant(), 1.0);
}

TEST_F(MatrixTest, DeterminantNonSquare) {
    Matrix m(2, 3);
    EXPECT_THROW(m.determinant(), std::invalid_argument);
}

// ============================================================================
// Inverse
// ============================================================================

TEST_F(MatrixTest, InverseIdentity) {
    Matrix I = Matrix::identity(3);
    Matrix I_inv = I.inverse();
    
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_NEAR(I_inv(i, j), I(i, j), 1e-10);
        }
    }
}

TEST_F(MatrixTest, InverseProduct) {
    Matrix A(3, 3);
    A(0, 0) = 2; A(0, 1) = -1; A(0, 2) = 0;
    A(1, 0) = -1; A(1, 1) = 2; A(1, 2) = -1;
    A(2, 0) = 0; A(2, 1) = -1; A(2, 2) = 2;
    
    Matrix A_inv = A.inverse();
    Matrix I = A * A_inv;
    
    // Check that A * A^-1 ≈ I
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            EXPECT_NEAR(I(i, j), expected, 1e-9);
        }
    }
}

TEST_F(MatrixTest, InverseSingular) {
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 2; A(1, 1) = 4;  // Singular (rows are linearly dependent)
    
    EXPECT_THROW(A.inverse(), std::runtime_error);
}

TEST_F(MatrixTest, InverseNonSquare) {
    Matrix A(2, 3);
    EXPECT_THROW(A.inverse(), std::invalid_argument);
}

// ============================================================================
// LU Decomposition
// ============================================================================

TEST_F(MatrixTest, LUDecomposition) {
    Matrix A(3, 3);
    A(0, 0) = 2; A(0, 1) = -1; A(0, 2) = -2;
    A(1, 0) = -4; A(1, 1) = 6; A(1, 2) = 3;
    A(2, 0) = -4; A(2, 1) = -2; A(2, 2) = 8;
    
    auto [L, U] = A.lu_decomposition();
    
    // Check that L is lower triangular with ones on diagonal
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(L(i, i), 1.0, 1e-10);
        for (size_t j = i + 1; j < 3; ++j) {
            EXPECT_NEAR(L(i, j), 0.0, 1e-10);
        }
    }
    
    // Check that U is upper triangular
    for (size_t i = 1; i < 3; ++i) {
        for (size_t j = 0; j < i; ++j) {
            EXPECT_NEAR(U(i, j), 0.0, 1e-10);
        }
    }
    
    // Check that A = L * U
    Matrix A_reconstructed = L * U;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_NEAR(A_reconstructed(i, j), A(i, j), 1e-9);
        }
    }
}

TEST_F(MatrixTest, LUNonSquare) {
    Matrix A(2, 3);
    EXPECT_THROW(A.lu_decomposition(), std::invalid_argument);
}

// ============================================================================
// Solve Linear System
// ============================================================================

TEST_F(MatrixTest, SolveLinearSystem) {
    Matrix A(3, 3);
    A(0, 0) = 2; A(0, 1) = -1; A(0, 2) = 0;
    A(1, 0) = -1; A(1, 1) = 2; A(1, 2) = -1;
    A(2, 0) = 0; A(2, 1) = -1; A(2, 2) = 2;
    
    Matrix b(3, 1);
    b(0, 0) = 1;
    b(1, 0) = 0;
    b(2, 0) = 0;
    
    Matrix x = A.solve(b);
    
    // Verify solution
    Matrix Ax = A * x;
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(Ax(i, 0), b(i, 0), 1e-9);
    }
}

TEST_F(MatrixTest, SolveMultipleRHS) {
    Matrix A(3, 3);
    A(0, 0) = 2; A(0, 1) = -1; A(0, 2) = 0;
    A(1, 0) = -1; A(1, 1) = 2; A(1, 2) = -1;
    A(2, 0) = 0; A(2, 1) = -1; A(2, 2) = 2;
    
    Matrix B(3, 2);
    B(0, 0) = 1; B(0, 1) = 0;
    B(1, 0) = 0; B(1, 1) = 1;
    B(2, 0) = 0; B(2, 1) = 0;
    
    Matrix X = A.solve(B);
    
    // Verify solution
    Matrix AX = A * X;
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            EXPECT_NEAR(AX(i, j), B(i, j), 1e-9);
        }
    }
}

TEST_F(MatrixTest, SolveSingular) {
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 2; A(1, 1) = 4;  // Singular
    
    Matrix b(2, 1);
    b(0, 0) = 1;
    b(1, 0) = 2;
    
    EXPECT_THROW(A.solve(b), std::runtime_error);
}

// ============================================================================
// Rank
// ============================================================================

TEST_F(MatrixTest, RankFullRank) {
    Matrix A = Matrix::identity(3);
    EXPECT_EQ(A.rank(), 3);
}

TEST_F(MatrixTest, RankSingular) {
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 2; A(1, 1) = 4;  // Singular
    EXPECT_EQ(A.rank(), 1);
}

// ============================================================================
// Symmetry Check
// ============================================================================

TEST_F(MatrixTest, IsSymmetric) {
    Matrix A(3, 3);
    A(0, 0) = 1; A(0, 1) = 2; A(0, 2) = 3;
    A(1, 0) = 2; A(1, 1) = 5; A(1, 2) = 6;
    A(2, 0) = 3; A(2, 1) = 6; A(2, 2) = 9;
    EXPECT_TRUE(A.is_symmetric());
}

TEST_F(MatrixTest, IsNotSymmetric) {
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 3; A(1, 1) = 4;
    EXPECT_FALSE(A.is_symmetric());
}

TEST_F(MatrixTest, IsSymmetricNonSquare) {
    Matrix A(2, 3);
    EXPECT_FALSE(A.is_symmetric());
}

// ============================================================================
// Factory Methods
// ============================================================================

TEST_F(MatrixTest, Identity) {
    Matrix I = Matrix::identity(4);
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            double expected = (i == j) ? 1.0 : 0.0;
            EXPECT_DOUBLE_EQ(I(i, j), expected);
        }
    }
}

TEST_F(MatrixTest, Zeros) {
    Matrix Z = Matrix::zeros(3, 4);
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            EXPECT_DOUBLE_EQ(Z(i, j), 0.0);
        }
    }
}

TEST_F(MatrixTest, Ones) {
    Matrix O = Matrix::ones(2, 3);
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ(O(i, j), 1.0);
        }
    }
}
