#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include "String.c"
#include "stdlib.h"
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#endif
#ifdef linux
#include <unistd.h>
#define EXPORT
#endif

// This is used for mitigating non-zero values caused by floating point error.
const double MARGIN_OF_ERROR = 1e-6;

/**
 * @brief The metadata associated with a given matrix.
 * @param num_rows: int
 *      The number of rows in the matrix.
 * @param num_cols: int
 *      The number of columns in the matrix.
 * @param augmented_matrix_rank: int
 *      The rank of the augmented matrix. Should be used for determining if the matrix is consistent.
 * @param matrix_rank: int
 *      The rank of the matrix, or the number of linearly independent variables present in the matrix.
 * @param is_consistent: int
 *      A boolean flag that indicates whether the matrix is consistent, meaning there exists some set of values that satisfy all equations in the matrix.
 * @param matrix_determinant: double
 *      The determinant of the matrix. A nonzero determinant indicates the matrix is invertible.
 */
struct MatrixMetadata
{
    int num_rows;
    int num_cols;
    int augmented_matrix_rank;
    int matrix_rank;
    int is_consistent;
    double matrix_determinant;
} MatrixMetadata;

/**
 * @brief Stack two arrays vertically like the diagram below:
 *  ------------------
 *  |                |                     ------------------
 *  |   ARRAY ONE    |                     |                |
 *  |                |                     |                |
 *  ------------------                     | COMBINED ARRAY |
 *          +                 =====>       |                |
 *  ------------------                     |                |
 *  |                |                     |                |
 *  |   ARRAY TWO    |                     ------------------
 *  |                |
 *  ------------------
 *
 *  @param matrix_one: double[ptr]
 *      The data structure of the left-hand side of the vstack operation, and should have dimensions MxN.
 *  @param matrix_two: double[ptr]
 *      The data structure of the right-hand side of the vstack operation, and should have dimensions QxN.
 *  @param result_matrix: double[ptr]
 *      The data structure of the combination of matrix_one and matrix_two. It should have dimensions (Q+M)xN.
 *  @param matrix_one_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the matrix_one data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *  @param matrix_one_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the matrix_two data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *  @param matrix_one_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the result_matrix data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *
 *  @returns None
 *
 **/
void vstack(double *matrix_one, double *matrix_two, double *result_matrix, struct MatrixMetadata *matrix_one_metadata, struct MatrixMetadata *matrix_two_metadata, struct MatrixMetadata *result_matrix_metadata)
{
    int num_cols_total = matrix_one_metadata->num_cols;
    int num_rows_total = matrix_one_metadata->num_rows + matrix_two_metadata->num_rows;
    int num_elements_per_row[2];
    num_elements_per_row[0] = (matrix_one_metadata->num_cols);
    num_elements_per_row[1] = (matrix_two_metadata->num_cols);
    result_matrix_metadata->num_rows = num_rows_total;
    result_matrix_metadata->num_cols = num_cols_total;
    memcpy(&result_matrix[0], &matrix_one[0], sizeof(double) * (matrix_one_metadata->num_rows * matrix_one_metadata->num_cols));
    memcpy(&result_matrix[(matrix_one_metadata->num_rows * matrix_one_metadata->num_cols)], &matrix_two[0], sizeof(double) * ((matrix_two_metadata->num_rows) * matrix_two_metadata->num_cols));
}

/**
 * @brief Stack two arrays horizontally like the diagram below:
 *  ------------------             ------------------               ------------------------------------
 *  |                |             |                |               |                                  |
 *  |   ARRAY ONE    |      +      |   ARRAY TWO    |     =====>    |          COMBINED ARRAY          |
 *  |                |             |                |               |                                  |
 *  ------------------             ------------------               ------------------------------------
 *
 *  @param matrix_one: double[ptr]
 *      The data structure of the left-hand side of the hstack operation, and should have dimensions MxN.
 *  @param matrix_two: double[ptr]
 *      The data structure of the right-hand side of the hstack operation, and should have dimensions QxN.
 *  @param result_matrix: double[ptr]
 *      The data structure of the combination of matrix_one and matrix_two. It should have dimensions (Q+M)xN.
 *  @param matrix_one_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the matrix_one data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *  @param matrix_two_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the matrix_two data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *  @param result_matrix_metadata: struct MatrixMetadata[ptr]
 *      The metadata of the result_matrix data structure.
 *      Must contain the number of rows and columns (i.e., the dimensions) of the matrix.
 *      For more information, consult the MatrixMetadata documentation.
 *
 *  @returns None
 *
 **/
void hstack(double *matrix_one, double *matrix_two, double *result_matrix, struct MatrixMetadata *matrix_one_metadata, struct MatrixMetadata *matrix_two_metadata, struct MatrixMetadata *result_matrix_metadata)
{
    int num_rows_total = matrix_one_metadata->num_rows;
    int num_elements_per_row[2];
    num_elements_per_row[0] = matrix_one_metadata->num_cols;
    num_elements_per_row[1] = matrix_two_metadata->num_cols;
    int num_cols_total = num_elements_per_row[0] + num_elements_per_row[1];
    if (num_rows_total < 1 || num_cols_total < 1)
    {
        // This array cannot exist
        result_matrix_metadata->num_cols = -1;
        result_matrix_metadata->num_rows = -1;
        return;
    }
    result_matrix_metadata->num_rows = num_rows_total;
    result_matrix_metadata->num_cols = num_cols_total;
    for (int i = 0; i < num_rows_total; i++)
    {
        memcpy(&result_matrix[(i * result_matrix_metadata->num_cols)], &matrix_one[(i * num_elements_per_row[0])], sizeof(double) * num_elements_per_row[0]);
        memcpy(&result_matrix[(i * result_matrix_metadata->num_cols) + num_elements_per_row[0]], &matrix_two[(i * num_elements_per_row[1])], sizeof(double) * num_elements_per_row[1]);
    }
}

/**
 * @brief Generate a square Identity matrix (i.e., 1 along main diagonal elements, 0 otherwise).
 *
 * @param num_rows: int
 *      The number of rows that the identity matrix should have. This must be equal to the num_cols value provided.
 * @param num_cols: int
 *      The number of columns that the identity matrix should have. This must be equal to the num_cols value provided.
 * @return identity_matrix: double[ptr]
 *      The identity matrix, as a 1-D array.
 */
static inline double *generate_square_identity_matrix(int num_rows, int num_cols)
{
    double *identity_matrix = (double *)malloc(sizeof(double) * (num_rows * num_cols));
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            if (row == col)
            {
                identity_matrix[(row * num_cols) + col] = 1;
            }
            else
            {
                identity_matrix[(row * num_cols) + col] = 0;
            }
        }
    }
    return identity_matrix;
}

/**
 * @brief Check if a row in a matrix contains only zeros. This is used for determining things like matrix rank, linear independence, etc.
 *
 * @param matrix_to_check: double[ptr]
 *      The matrix of values to check.
 * @param row_to_check: int
 *      The row (0-indexed) to check for all-zeros. Obviously should be in the range [0, (N-1)] where N is the number of rows in matrix_to_check.
 * @param num_cols: int
 *      The number of columns in matrix_to_check. Used for indexing the matrix_to_check data structure, as it is in a 1-D array format.
 * @param not_augmented_matrix: int
 *
 * @return contains_all_zeros: int
 *      A boolean flag indicating whether or not the row contains all zeros.
 */
static inline int row_has_all_zeros(const double *matrix_to_check, int row_to_check, int num_cols)
{
    // The minus one is to account for the augmented matrix
    for (int col = 0; col < num_cols; ++col)
    {
        double value_to_check = matrix_to_check[(row_to_check * num_cols) + col];
        // The second condition is to try and ameliorate floating point errors.
        if (value_to_check != 0 && (value_to_check < (-1 * MARGIN_OF_ERROR) || value_to_check > MARGIN_OF_ERROR))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Check if a column in a matrix contains only zeros. This is used for determining things like matrix rank, linear independence, etc.
 *
 * @param matrix_to_check: double[ptr]
 *      The matrix of values to check.
 * @param column_to_check: int
 *      The column (0-indexed) to check for all-zeros. Obviously should be in the range [0, (N-1)] where N is the number of columns in matrix_to_check.
 * @param num_rows: int
 *      The number of rows in matrix_to_check. Used for indexing the matrix_to_check data structure, as it is in a 1-D array format.
 * @param not_augmented_matrix: int
 *
 * @return contains_all_zeros: int
 *      A boolean flag indicating whether or not the column contains all zeros.
 */
static inline int column_has_all_zeros(const double *matrix_to_check, int column_to_check, int num_rows, int num_cols)
{
    for (int row = 0; row < num_rows; row++)
    {
        double value_to_check = matrix_to_check[(row * num_cols) + column_to_check];
        if (value_to_check != 0 && (value_to_check < (-1 * MARGIN_OF_ERROR) || value_to_check > MARGIN_OF_ERROR))
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Calculate the row rank of the matrix.
 *
 * @param matrix_to_check: double[ptr]
 *      The matrix to calculate the rank for.
 * @param metadata: struct MatrixMetadata[ptr]
 *      The metadata associated with the matrix_to_check data structure. Should contain the dimensions of the matrix.
 * @return int The rank of the matrix.
 */
static inline int calculate_matrix_row_rank(double *matrix_to_check, struct MatrixMetadata *metadata)
{
    int row_rank = 0;
    for (int row = 0; row < metadata->num_rows; row++)
    {
        if (!row_has_all_zeros(matrix_to_check, row, metadata->num_cols))
        {
            row_rank++;
        }
    }
    return row_rank;
}

/**
 * @brief Calculate the column rank of the matrix. Used for determining if a matrix has full rank, and if all variables are linearly independent.
 *
 * @param matrix_to_check: double[ptr]
 *      The matrix to calculate the rank for.
 * @param metadata: struct MatrixMetadata[ptr]
 *      The metadata associated with the matrix_to_check data structure. Should contain the dimensions of the matrix.
 * @return int The column rank of the matrix.
 */
static inline int calculate_matrix_column_rank(double *matrix_to_check, struct MatrixMetadata *metadata)
{
    int column_rank = 0;
    for (int col = 0; col < metadata->num_cols; col++)
    {
        if (!column_has_all_zeros(matrix_to_check, col, metadata->num_rows, metadata->num_cols))
        {
            column_rank++;
        }
    }
    return column_rank;
}

/**
 *  @brief Check if a matrix is consistent and how many solutions it has using the Rouché–Capelli theorem. Note that since row and column rank are equivalent, only row rank is considered by the theorem.
 * 
 *  @param matrix_to_check: double[ptr]
 *      The matrix to determine consistency for.
 *  @param augmented_matrix_to_check double[ptr]
 *      The augmented matrix used to determine if the matrix_to_check is consistent.
 *  @param matrix_to_check_metadata struct MatrixMetadata[ptr]
 *      The metadata of the matrix_to_check data structure. Should contain the dimensions of the matrix, and the consistency value will be written to it.
 *  @param augmented_matrix_to_check_metadata struct MatrixMetadata[ptr]
 *      The metadata of the augmented_matrix_to_check data structure. Should contain the dimensions of the matrix.
 * 
 *  @return None
 */
static inline void is_matrix_consistent_rouche_capelli(double *matrix_to_check, double *augmented_matrix_to_check, struct MatrixMetadata *matrix_to_check_metadata, struct MatrixMetadata *augmented_matrix_to_check_metadata, struct String *message_buffer)
{
    // NOTE: It appears that you can prove that the column and row rank are equivalent, and since row rank is simpler to calculate we'll use only that.
    int matrix_row_rank = calculate_matrix_row_rank(matrix_to_check, matrix_to_check_metadata);
    int augmented_matrix_row_rank = calculate_matrix_row_rank(augmented_matrix_to_check, augmented_matrix_to_check_metadata);
    if (matrix_row_rank < augmented_matrix_row_rank)
    {
        if (!message_buffer)
        {
            printf("System of Equations is not Consistent. No solution exists.\n");
        }
        else
        {
            writeNulTerminatedString("System of Equations is not Consistent. No solution exists.\n", message_buffer);
        }
        matrix_to_check_metadata->is_consistent = 0;
    }
    else if (matrix_row_rank == augmented_matrix_row_rank)
    {
        if (matrix_row_rank < matrix_to_check_metadata->num_rows)
        {
            if (!message_buffer)
            {
                printf("System of Equations is Consistent. Infinite solutions exist.\n");
            }
            else
            {
                writeNulTerminatedString("System of Equations is Consistent. Infinite solutions exist.\n", message_buffer);
            }
            matrix_to_check_metadata->is_consistent = 1;
        }
        else if (matrix_row_rank == matrix_to_check_metadata->num_rows)
        {
            if (!message_buffer)
            {
                printf("System of Equations is Consistent. Unique solution exists.\n");
            }
            else
            {
                writeNulTerminatedString("System of Equations is Consistent. Unique solution exists.\n", message_buffer);
            }
            matrix_to_check_metadata->is_consistent = 1;
        }
        else
        {
            if (!message_buffer)
            {
                printf("System of Equations is Consistent. Row Rank exceeds number of rows in matrix.\n");
            }
            else
            {
                writeNulTerminatedString("System of Equations is Consistent. Row Rank exceeds number of rows in matrix.\n", message_buffer);
            }
            matrix_to_check_metadata->is_consistent = 1;
        }
    }
    else
    {
        if (!message_buffer)
        {
            printf("Somehow rank(A|b) > n. Don't know what to do.\n");
        }
        else
        {
            writeNulTerminatedString("Somehow rank(A|b) > n. Don't know what to do.\n", message_buffer);
        }
        matrix_to_check_metadata->is_consistent = 1;
    }
}

/**********************************************************************************
 *                                                                                *
 *                                                                                *
 *                                                                                *
 *                               PRINT FUNCTIONS                                  *
 *                                                                                *
 *                                                                                *
 *                                                                                *
 **********************************************************************************/

/**
 * @brief Helper function to print out the matrix metadata.
 * 
 * @param matrix_metadata_to_print: struct MatrixMetadata[ptr]
 *      The matrix metadata structure to print out.
 * 
 * @returns None. 
 */
static inline void print_matrix_metadata(struct MatrixMetadata *matrix_metadata_to_print)
{
    printf(
        "Num Rows: %3d\nNum Columns: %3d\nAugmented Matrix Rank: %d\nMatrix Rank: %d\nIs Consistent? %d\nMatrix Determinant: %03.6f\n",
        matrix_metadata_to_print->num_rows,
        matrix_metadata_to_print->num_cols,
        matrix_metadata_to_print->augmented_matrix_rank,
        matrix_metadata_to_print->matrix_rank,
        matrix_metadata_to_print->is_consistent,
        matrix_metadata_to_print->matrix_determinant);
}

/**
 *  @brief Print the matrix. If a buffer is not provided, it prints to STDOUT instead.
 * 
 *  @param matrix_to_print: double[ptr]
 *      The matrix to print the values of.
 *  @param num_rows: int
 *      The number of rows in the matrix_to_print data structure.
 *  @param num_cols: int
 *      The number of columns in the matrix_to_print data structure.
 *  @param message_buffer: struct String[ptr]
 *      A string buffer that, if initialized, will house messages to be displayed to the Python GUI component. Otherwise, values will be printed out to STDOUT.
 * 
 *  @return None
 * 
 */
static inline void print_matrix(double *matrix_to_print, int num_rows, int num_cols, struct String *message_buffer)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            if (!message_buffer)
            {
                printf("% f\t", matrix_to_print[(row * num_cols) + col]);
            }
            else
            {
                writeDecimalNumber((int64_t)(matrix_to_print[(row * num_cols) + col] * 1e6), 6, message_buffer);
                writeStringNoNullTerminator("\t", message_buffer);
            }
        }
        if (!message_buffer)
        {
            printf("\n");
        }
        else
        {
            writeNulTerminatedString("\n", message_buffer);
        }
    }
}

/**
 *  @brief Print the augmented matrix with a dividing line. If a buffer is not provided, it prints to STDOUT instead.
 * 
 *  @param matrix_to_print: double[ptr]
 *      The matrix to print the values of.
 *  @param num_rows: int
 *      The number of rows in the matrix_to_print data structure.
 *  @param num_cols: int
 *      The number of columns in the matrix_to_print data structure.
 *  @param message_buffer: struct String[ptr]
 *      A string buffer that, if initialized, will house messages to be displayed to the Python GUI component. Otherwise, values will be printed out to STDOUT.
 * 
 *  @return None
 * 
 */
static inline void print_augmented_matrix(double *matrix_to_print, int num_rows, int num_cols, int num_augmented_cols, struct String *message_buffer)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            if (!message_buffer)
            {
                printf("% f\t", matrix_to_print[(row * num_cols) + col]);
            }
            else
            {
                writeDecimalNumber((int64_t)(matrix_to_print[(row * num_cols) + col] * 1e6), 6, message_buffer);
                writeStringNoNullTerminator("\t", message_buffer);
            }
            if (col == ((num_cols - num_augmented_cols) - 1))
            {
                if (!message_buffer)
                {
                    printf("|\t");
                }
                else
                {
                    writeStringNoNullTerminator("|\t", message_buffer);
                }
            }
        }
        if (!message_buffer)
        {
            printf("\n");
        }
        else
        {
            writeNulTerminatedString("\n", message_buffer);
        }
    }
}

/**********************************************************************************
 *                                                                                *
 *                                                                                *
 *                                                                                *
 *                          ELEMENTARY ROW OPERATIONS                             *
 *                                                                                *
 *                                                                                *
 *                                                                                *
 **********************************************************************************/

/**
 *  @brief Multiply a row of values in a matrix by some scalar value.
 * 
 *  @param matrix_to_scale: double[ptr]
 *      The matrix whose row needs scaling. Note that the matrix is assumed to be in a 1-D format.
 *  @param row_to_scale: int
 *      The row of the matrix to scale by the scalar value.
 *  @param num_cols: int
 *      The number of columns in the matrix. Used for iterating through the array.
 *  @param scalar: double
 *      The value that the row of values in the matrix will be scaled by.
 * 
 *  @returns None.
 * 
 */
static inline void multiply_row_by_scalar(double *matrix_to_scale, int row_to_scale, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_scale[(row_to_scale * num_cols) + col] *= scalar;
    }
}

/**
 *  @brief Subtract a row of values, potentially multiplied by some scalar value, from another row in the matrix.
 * 
 *  @param matrix_to_modify: double[ptr]
 *      The matrix to perform the operation on. Note that the matrix is assumed to be in a 1-D format.
 *  @param row_index_to_modify: int
 *      The row of the matrix whose value will be modified by the operation.
 *  @param row_to_use_for_subtraction: int
 *      The row of the matrix whose value will be the subtractor. Values might be modified by some scalar value.
 *  @param num_cols: int
 *      The number of columns in the matrix. Used for iterating through the array.
 *  @param scalar: double
 *      The value that the row_to_use_for_subtraction in the matrix will be scaled by.
 * 
 *  @returns None.
 * 
 */
static inline void subtract_scaled_row(double *matrix_to_modify, int row_index_to_modify, int row_to_use_for_subtraction, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_modify[(row_index_to_modify * num_cols) + col] -= (scalar * matrix_to_modify[(row_to_use_for_subtraction * num_cols) + col]);
    }
}

/**
 *  @brief Add a row of values, potentially multiplied by some scalar value, to another row in the matrix.
 * 
 *  @param matrix_to_modify: double[ptr]
 *      The matrix to perform the operation on. Note that the matrix is assumed to be in a 1-D format.
 *  @param row_index_to_modify: int
 *      The row of the matrix whose value will be modified by the operation.
 *  @param row_to_use_for_addition: int
 *      The row of the matrix whose value will be the subtractor. Values might be modified by some scalar value.
 *  @param num_cols: int
 *      The number of columns in the matrix. Used for iterating through the array.
 *  @param scalar: double
 *      The value that the row_to_use_for_addition in the matrix will be scaled by.
 * 
 *  @returns None.
 * 
 */
static inline void add_scaled_row(double *matrix_to_modify, int row_index_to_modify, int row_to_use_for_addition, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_modify[(row_index_to_modify * num_cols) + col] += (scalar * matrix_to_modify[(row_to_use_for_addition * num_cols) + col]);
    }
}

/**
 *  @brief Swap two rows in a matrix.
 * 
 *  @param matrix_to_swap_rows: double[ptr]
 *      The matrix to swap rows of. Note that the matrix is assumed to be in a 1-D format.
 *  @param row_to_swap_index_a: int
 *      The row of the matrix that serves as one half of the swap operation.
 *  @param row_to_swap_index_b: int
 *      The row of the matrix that serves as one half of the swap operation.
 *  @param num_cols: int
 *      The number of columns in the matrix. Used for iterating through the array.
 * 
 *  @returns None.
 * 
 */
static inline void swap_rows(double *matrix_to_swap_rows, int row_to_swap_index_a, int row_to_swap_index_b, int num_cols)
{
    double row_holder[16];
    size_t num_bytes_per_row = sizeof(double) * num_cols;
    memcpy(&row_holder, &matrix_to_swap_rows[(row_to_swap_index_a * num_cols) + 0], num_bytes_per_row);
    memcpy(&matrix_to_swap_rows[(row_to_swap_index_a * num_cols) + 0], &matrix_to_swap_rows[(row_to_swap_index_b * num_cols) + 0], num_bytes_per_row);
    memcpy(&matrix_to_swap_rows[(row_to_swap_index_b * num_cols) + 0], &row_holder, num_bytes_per_row);
}

/**
 *  @brief Attempt row reduction using the Gauss-Jordan algorithm.
 * 
 *  @param matrix_to_reduce: double[ptr]
 *      The matrix to reduce into reduced row echelon form. Note that the matrix is assumed to be in a 1-D format.
 *  @param matrix_augment: double[ptr]
 *      The augment portion of the matrix (i.e., the b portion of Ax = b). Used in row reduction to solve for x.
 *  @param message_buffer: struct String[ptr]
 *      A string buffer that, if initialized, will house messages to be displayed to the Python GUI component.
 *  @param metadata: struct MatrixMetadata[ptr]
 *      The metadata associated with the matrix_to_reduce data structure. Should contain the dimensions of the matrix.
 *  @param matrix_augment_metadata: struct MatrixMetadata[ptr]
 *      The metadata associated with the matrix_augment data structure. Should contain the dimensions of the matrix.
 *  
 *  @return None
 * 
 */
EXPORT void python_perform_gauss_jordan_reduction(double *matrix_to_reduce, double *matrix_augment, struct String *message_buffer, struct MatrixMetadata *metadata, struct MatrixMetadata *matrix_augment_metadata)
{
    // Generate the augmented matrix from the matrix to reduce and its augment
    double *augmented_matrix = (double *)malloc(sizeof(double) * (metadata->num_rows * (metadata->num_cols + matrix_augment_metadata->num_cols)));
    struct MatrixMetadata augmented_matrix_metadata;
    augmented_matrix_metadata.num_rows = metadata->num_rows;
    augmented_matrix_metadata.num_cols = metadata->num_cols + matrix_augment_metadata->num_cols;
    hstack(matrix_to_reduce, matrix_augment, augmented_matrix, metadata, matrix_augment_metadata, &augmented_matrix_metadata);

    int size_main_diagonal;
    int swap_rows_flag = 0;
    if ((augmented_matrix_metadata.num_cols - matrix_augment_metadata->num_cols) <= augmented_matrix_metadata.num_rows)
    {
        size_main_diagonal = (augmented_matrix_metadata.num_cols - matrix_augment_metadata->num_cols);
    }
    else
    {
        size_main_diagonal = augmented_matrix_metadata.num_rows;
    }

    // Iterate down the main diagonal to begin conversion to echelon form
    double product_of_diagonal_elements = 1.0;
    double denominator_value = 1;
    int swap_multiplier = 1;
    for (int i = 0; i < size_main_diagonal; i++)
    {
        double pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
        if (pivot_element == 0)
        {
            // See if there is a nonzero element in the same column (below it) and swap the rows.
            swap_rows_flag = 1;
        }
        double value_below_pivot_element;
        for (int row = (i + 1); row < augmented_matrix_metadata.num_rows; row++)
        {
            value_below_pivot_element = augmented_matrix[(row * augmented_matrix_metadata.num_cols) + i];
            if (value_below_pivot_element != 0)
            {
                if (swap_rows_flag == 1)
                {
                    // Swap the rows instead.
                    if (!message_buffer)
                    {
                        printf("[SWP] Row %d = (R%d) <=> (R%d)\n", (row + 1), (row + 1), (i + 1));
                    }
                    else
                    {
                        writeStringNoNullTerminator("[SWP] Row ", message_buffer);
                        writeNumber((row + 1), message_buffer);
                        writeStringNoNullTerminator(" = (R", message_buffer);
                        writeNumber((row + 1), message_buffer);
                        writeStringNoNullTerminator(") <=> (R", message_buffer);
                        writeNumber((i + 1), message_buffer);
                        writeNulTerminatedString(")\n", message_buffer);
                    }
                    swap_rows(augmented_matrix, row, i, augmented_matrix_metadata.num_cols);
                    swap_rows_flag = 0;
                    pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
                    if (!message_buffer)
                    {
                        printf("New Pivot Element: % .6f\n", pivot_element);
                    }
                    else
                    {
                        writeStringNoNullTerminator("New Pivot Element: ", message_buffer);
                        writeDecimalNumber((int64_t)(pivot_element * 1e9), 9, message_buffer);
                        writeNulTerminatedString("\n", message_buffer);
                    }
                    swap_multiplier *= -1;
                }
                else
                {
                    double reciprocal_fraction_scalar;
                    if (value_below_pivot_element < 0)
                    {
                        reciprocal_fraction_scalar = (-1.0) * (value_below_pivot_element / pivot_element);
                        if (!message_buffer)
                        {
                            printf("[ADD] Row %d = (R%d) + % .6f*(R%d)\n", row, row, reciprocal_fraction_scalar, i);
                        }
                        else
                        {
                            writeStringNoNullTerminator("[ADD] Row ", message_buffer);
                            writeNumber(row, message_buffer);
                            writeStringNoNullTerminator(" = (R", message_buffer);
                            writeNumber(row, message_buffer);
                            writeStringNoNullTerminator(") + ", message_buffer);
                            writeDecimalNumber((int64_t)(reciprocal_fraction_scalar * 1e9), 9, message_buffer);
                            writeStringNoNullTerminator("*(R", message_buffer);
                            writeNumber(i, message_buffer);
                            writeNulTerminatedString(")\n", message_buffer);
                        }
                        add_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciprocal_fraction_scalar);
                    }
                    else
                    {
                        reciprocal_fraction_scalar = (value_below_pivot_element / pivot_element);
                        if (!message_buffer)
                        {
                            printf("[SUB] Row %d = (R%d) + % .6f*(R%d)\n", row, row, reciprocal_fraction_scalar, i);
                        }
                        else
                        {
                            writeStringNoNullTerminator("[SUB] Row ", message_buffer);
                            writeNumber(row, message_buffer);
                            writeStringNoNullTerminator(" = (R", message_buffer);
                            writeNumber(row, message_buffer);
                            writeStringNoNullTerminator(") - ", message_buffer);
                            writeDecimalNumber((int64_t)(reciprocal_fraction_scalar * 1e9), 9, message_buffer);
                            writeStringNoNullTerminator("*(R", message_buffer);
                            writeNumber(i, message_buffer);
                            writeNulTerminatedString(")\n", message_buffer);
                        }
                        subtract_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciprocal_fraction_scalar);
                    }
                }
            }
            print_augmented_matrix(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
        }
        product_of_diagonal_elements *= pivot_element;
    }

    // Next perform the second half
    if (!message_buffer)
    {
        printf("Shifting to Reduced Row Echelon Portion of Algorithm.\n");
    }
    else
    {
        writeNulTerminatedString("Shifting to Reduced Row Echelon Portion of Algorithm\n", message_buffer);
    }
    for (int i = (size_main_diagonal - 1); i > -1; i--)
    {
        // Try to convert the pivot element to 1
        double pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
        double pivot_reciprocal;
        if (pivot_element == 0)
        {
        }
        else
        {
            if (pivot_element != 1)
            {
                pivot_reciprocal = (1.0 / pivot_element);
                if (!message_buffer)
                {
                    printf("[SCL] Row %d = % .6f*(R%d)\n", (i + 1), pivot_reciprocal, (i + 1));
                }
                else
                {
                    writeStringNoNullTerminator("[SCL] Row ", message_buffer);
                    writeNumber((i + 1), message_buffer);
                    writeStringNoNullTerminator(" = ", message_buffer);
                    writeDecimalNumber((int64_t)(pivot_reciprocal * 1e9), 9, message_buffer);
                    writeStringNoNullTerminator(" * (R", message_buffer);
                    writeNumber((i + 1), message_buffer);
                    writeNulTerminatedString(")\n", message_buffer);
                }
                multiply_row_by_scalar(augmented_matrix, i, augmented_matrix_metadata.num_cols, pivot_reciprocal);
                print_augmented_matrix(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
                pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
            }
            for (int row = (i - 1); row > -1; row--)
            {
                double value_above_pivot_element = augmented_matrix[(row * augmented_matrix_metadata.num_cols) + i];
                if (value_above_pivot_element != 0)
                {
                    double reciprocal_fraction_scalar;
                    reciprocal_fraction_scalar = (value_above_pivot_element / pivot_element);
                    if (!message_buffer)
                    {
                        printf("Reciprocal Fraction Scalar: % .6f\n", reciprocal_fraction_scalar);
                        printf("[SUB] Row %d = (R%d) + % .6f*(R%d)\n", (row + 1), (row + 1), reciprocal_fraction_scalar, (i + 1));
                    }
                    else
                    {
                        writeStringNoNullTerminator("Reciprocal Fraction Scalar: ", message_buffer);
                        writeDecimalNumber((int64_t)(value_above_pivot_element * 1e9), 9, message_buffer);
                        writeStringNoNullTerminator(" / ", message_buffer);
                        writeDecimalNumber((int64_t)(value_above_pivot_element * 1e9), 9, message_buffer);
                        writeStringNoNullTerminator(" = ", message_buffer);
                        writeDecimalNumber((int64_t)(value_above_pivot_element * 1e9), 9, message_buffer);
                        writeNulTerminatedString("\n", message_buffer);
                        writeStringNoNullTerminator("[SUB] Row ", message_buffer);
                        writeNumber((row + 1), message_buffer);
                        writeStringNoNullTerminator(" = (R", message_buffer);
                        writeNumber((row + 1), message_buffer);
                        writeStringNoNullTerminator(") - ", message_buffer);
                        writeDecimalNumber((int64_t)(reciprocal_fraction_scalar * 1e9), 9, message_buffer);
                        writeStringNoNullTerminator("*(R", message_buffer);
                        writeNumber((i + 1), message_buffer);
                        writeNulTerminatedString(")\n", message_buffer);
                    }
                    subtract_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciprocal_fraction_scalar);
                }
                print_augmented_matrix(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
            }
        }
    }
    is_matrix_consistent_rouche_capelli(matrix_to_reduce, augmented_matrix, metadata, &augmented_matrix_metadata, message_buffer);
    print_matrix_metadata(metadata);
    if (metadata->is_consistent == 1)
    {
        if (!message_buffer)
        {
            printf("Product of Diagonal Elements is: % .6f\n", product_of_diagonal_elements);
            printf("Denominator Value is: % .6f\n", denominator_value);
            printf("Swap Multiplier is: %d\n", swap_multiplier);
        }
        else
        {
            writeStringNoNullTerminator("Product of Diagonal Elements is: ", message_buffer);
            writeDecimalNumber((int64_t)(product_of_diagonal_elements * 1e9), 9, message_buffer);
            writeNulTerminatedString("\n", message_buffer);
            // BUG?: Denominator value is product of all scalar multiplications performed during gaussian elimination (reduced echelon); Currently remains at default value 1
            writeStringNoNullTerminator("Denominator Value is: ", message_buffer);
            writeDecimalNumber((int64_t)(denominator_value * 1e9), 9, message_buffer);
            writeNulTerminatedString("\n", message_buffer);

            writeStringNoNullTerminator("Swap Multiplier is: ", message_buffer);
            writeNumber(swap_multiplier, message_buffer);
            writeNulTerminatedString("\n", message_buffer);
        }
        metadata->matrix_determinant = (product_of_diagonal_elements / denominator_value) * swap_multiplier;
        if (!message_buffer)
        {
            printf("Determinant of matrix A is: % .6f\n", metadata->matrix_determinant);
        }
        else
        {
            writeStringNoNullTerminator("Determinant of non-augmented matrix A is: ", message_buffer);
            writeDecimalNumber((int64_t)(metadata->matrix_determinant * 1e9), 9, message_buffer);
            writeNulTerminatedString("\n", message_buffer);
        }
    }
    // Free allocated resources, end of function
    free(augmented_matrix);
}

/**
 *  @brief Attempt to invert a square matrix.
 * 
 *  @param matrix_to_invert: double[ptr]
 *      The matrix to invert. Note that the matrix is assumed to be in a 1-D format.
 *  @param matrix_to_invert_metadata: struct MatrixMetadata[ptr]
 *      The metadata associated with the matrix_to_reduce data structure. Should contain the dimensions of the matrix.
 *  @param message_buffer: struct String[ptr]
 *      A string buffer that, if initialized, will house messages to be displayed to the Python GUI component.
 * 
 *  @return None
 * 
 */
EXPORT void python_perform_square_matrix_inversion_gaussian_reduction(double *matrix_to_invert, struct MatrixMetadata *matrix_to_invert_metadata, struct String *message_buffer)
{
    int matrix_column_rank = calculate_matrix_column_rank(matrix_to_invert, matrix_to_invert_metadata);
    int matrix_row_rank = calculate_matrix_row_rank(matrix_to_invert, matrix_to_invert_metadata);
    // This also covers if there is a row or column of zero values
    if (matrix_to_invert_metadata->matrix_determinant == 0)
    {
        if (!message_buffer)
        {
            printf("The matrix provided has a determinant of 0, meaning it is not invertible.\n");
        }
        else
        {
            writeNulTerminatedString("The matrix provided has a determinant of 0, meaning it is not invertible.", message_buffer);
        }
        return;
    }
    else if ((matrix_column_rank != matrix_to_invert_metadata->num_cols) || (matrix_row_rank != matrix_to_invert_metadata->num_rows) || (matrix_column_rank != matrix_row_rank))
    {
        if (!message_buffer)
        {
            printf("The matrix provided does not have full rank and thus it is not invertible.\n");
        }
        else
        {
            writeNulTerminatedString("The matrix provided does not have full rank and thus it is not invertible.", message_buffer);
        }
        return;
    }
    else
    {
        double *identity_matrix = generate_square_identity_matrix(matrix_to_invert_metadata->num_rows, matrix_to_invert_metadata->num_cols);
        struct MatrixMetadata identity_matrix_metadata;
        identity_matrix_metadata.num_rows = matrix_to_invert_metadata->num_rows;
        identity_matrix_metadata.num_cols = matrix_to_invert_metadata->num_rows;
        python_perform_gauss_jordan_reduction(matrix_to_invert, identity_matrix, message_buffer, matrix_to_invert_metadata, &identity_matrix_metadata);
        // Free the matrices
        free(identity_matrix);
    }
}

// int main()
// {
//     double matrix_to_reduce[9] = {
//         2, 1, -1,
//         -3, -1, 2,
//         -2, 1, 2};
//     double matrix_to_reduce_augment[3] = {8, -11, -3};
//     struct String *buffer = 0x0;
//     struct MatrixMetadata matrix_to_reduce_metadata;
//     matrix_to_reduce_metadata.num_rows = 3;
//     matrix_to_reduce_metadata.num_cols = 3;
//     struct MatrixMetadata matrix_to_reduce_augment_metadata;
//     matrix_to_reduce_augment_metadata.num_rows = 3;
//     matrix_to_reduce_augment_metadata.num_cols = 1;
//     python_perform_gauss_jordan_reduction(matrix_to_reduce, matrix_to_reduce_augment, buffer, &matrix_to_reduce_metadata, &matrix_to_reduce_augment_metadata);
//     printf("\n\n\n");
//     printf("Performing First Matrix Inversion\n");
//     python_perform_square_matrix_inversion_gaussian_reduction(matrix_to_reduce, &matrix_to_reduce_metadata, buffer);
//     printf("\n\n\n");
//     // printf("Performing Second Matrix Reduction\n");
//     // double two_matrix_to_reduce[12] = {
//     //     0, 1, 5, -4,
//     //     1, 4, 3, -2,
//     //     2, 7, 1, -2};
//     // perform_gauss_jordan_reduction(two_matrix_to_reduce, 3, 4, 1);
//     // printf("\n\n\n");
//     // double two_matrix_to_invert[9] = {
//     //     0, 1, 5,
//     //     1, 4, 3,
//     //     2, 7, 1};
//     // printf("Performing Second Matrix Inversion\n");
//     // perform_square_matrix_inversion_gaussian_reduction(two_matrix_to_invert, 3, 3);
//     // printf("\n\n\n");
//     // double three_matrix_to_reduce[12] = {
//     //     2, 0, -6, -8,
//     //     0, 1, 2, 3,
//     //     3, 6, -2, -4};
//     // perform_gauss_jordan_reduction(three_matrix_to_reduce, 3, 4, 1);
//     // printf("\n\n\n");
//     // printf("Performing Third Matrix Inversion\n");
//     // double three_matrix_to_invert[9] = {
//     //     2, 0, -6,
//     //     0, 1, 2,
//     //     3, 6, -2};
//     // perform_square_matrix_inversion_gaussian_reduction(three_matrix_to_invert, 3, 3);
//     // printf("\n\n\n");

//     printf("Press Any Key to Continue\n");
//     getch();
// }