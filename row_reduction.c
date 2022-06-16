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

const double MARGIN_OF_ERROR = 1e-6;

struct MatrixMetadata
{
    int num_rows;
    int num_cols;
    int augmented_matrix_rank;
    int matrix_rank;
    int is_consistent;
    double matrix_determinant;
} MatrixMetadata;

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

/** Stack two arrays vertically like the diagram below.
 *
 *  Arguments
 *  ---------
 *
 *  Returns
 *  -------
 *  None.
 *
 *
 * ------------------
 * |                |                     ------------------
 * |   ARRAY ONE    |                     |                |
 * |                |                     |                |
 * ------------------                     | COMBINED ARRAY |
 *         +                 =====>       |                |
 * ------------------                     |                |
 * |                |                     |                |
 * |   ARRAY TWO    |                     ------------------
 * |                |
 * ------------------

 **/
void vstack(double *matrix_one, double *matrix_two, double *result_matrix, struct MatrixMetadata *matrix_one_metadata, struct MatrixMetadata *matrix_two_metadata, struct MatrixMetadata *result_matrix_metadata)
{

    int num_cols_total = matrix_one_metadata->num_cols;
    int num_rows_total = matrix_one_metadata->num_rows + matrix_two_metadata->num_rows;
    int num_elements_per_row[2];
    num_elements_per_row[0] = (matrix_one_metadata->num_cols);
    num_elements_per_row[1] = (matrix_two_metadata->num_cols);
    // result_matrix = (int16_t *)malloc(sizeof(int16_t) * (num_rows_total * num_cols_total));
    result_matrix_metadata->num_rows = num_rows_total;
    result_matrix_metadata->num_cols = num_cols_total;
    memcpy(&result_matrix[0], &matrix_one[0], sizeof(double) * (matrix_one_metadata->num_rows * matrix_one_metadata->num_cols));
    memcpy(&result_matrix[(matrix_one_metadata->num_rows * matrix_one_metadata->num_cols)], &matrix_two[0], sizeof(double) * ((matrix_two_metadata->num_rows) * matrix_two_metadata->num_cols));
}

/** Stack two arrays horizontally like the diagram below.
 *
 *  Arguments
 *  ---------
 *
 *  Returns
 *  -------
 *  None.
 *
 *
 * ------------------             ------------------               ------------------------------------
 * |                |             |                |               |                                  |
 * |   ARRAY ONE    |      +      |   ARRAY TWO    |     =====>    |          COMBINED ARRAY          |
 * |                |             |                |               |                                  |
 * ------------------             ------------------               ------------------------------------
 *
 **/
void hstack(double *matrix_one, double *matrix_two, double *result_matrix, struct MatrixMetadata *matrix_one_metadata, struct MatrixMetadata *matrix_two_metadata, struct MatrixMetadata *result_matrix_metadata)
{
    int num_rows_total = matrix_one_metadata->num_rows;
    printf("Num Rows in Matrix One: %d\n", num_rows_total);
    int num_elements_per_row[2];
    num_elements_per_row[0] = matrix_one_metadata->num_cols;
    num_elements_per_row[1] = matrix_two_metadata->num_cols;
    printf("Number of columns in matrix one: %d\tNumber of columns in matrix two: %d\n", num_elements_per_row[0], num_elements_per_row[1]);
    int num_cols_total = num_elements_per_row[0] + num_elements_per_row[1];
    if (num_rows_total < 1 || num_cols_total < 1)
    {
        // This array cannot exist
        result_matrix_metadata->num_cols = 0;
        result_matrix_metadata->num_rows = 0;
        return;
    }
    result_matrix_metadata->num_rows = num_rows_total;
    result_matrix_metadata->num_cols = num_cols_total;
    print_matrix_metadata(result_matrix_metadata);
    for (int i = 0; i < num_rows_total; i++)
    {
        memcpy(&result_matrix[(i * result_matrix_metadata->num_cols)], &matrix_one[(i * num_elements_per_row[0])], sizeof(double) * num_elements_per_row[0]);
        memcpy(&result_matrix[(i * result_matrix_metadata->num_cols) + num_elements_per_row[0]], &matrix_two[(i * num_elements_per_row[1])], sizeof(double) * num_elements_per_row[1]);
    }
}

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

static inline int row_has_all_zeros(double *matrix_to_check, int row_to_check, int num_cols, int not_augmented_matrix)
{
    // The minus one is to account for the augmented matrix
    for (int col = 0; col < (num_cols - not_augmented_matrix); ++col)
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

static inline int column_has_all_zeros(double *matrix_to_check, int column_to_check, int num_rows, int num_cols)
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
 * @brief Calculate the rank of the matrix.
 *
 * @param matrix_to_check [double*] The matrix to calculate the rank for.
 * @param num_rows [int] The number of rows in the unaugmented matrix.
 * @param num_cols [int] The number of columns in the unaugmented matrix.
 * @param num_augmented_columns How many columns there are in the augmented matrix, if there is one. If this value is 0, then the matrix is assumed to be unaugmented.
 * @return int The rank of the matrix.
 */
static inline int calculate_matrix_rank(double *matrix_to_check, int num_rows, int num_cols, int num_augmented_columns)
{
    // This only calculates row rank
    // Heuristic: The rank is determined by whichever dimension is the limiting dimension
    // int base_rank = (num_rows <= (num_cols - num_augmented_columns)) ? num_rows : (num_cols - num_augmented_columns);
    int actual_rank = 0;
    for (int row = 0; row < num_rows; row++)
    {
        if (!row_has_all_zeros(matrix_to_check, row, num_cols, num_augmented_columns))
        {
            actual_rank++;
        }
    }
    if (actual_rank <= (num_cols - num_augmented_columns))
    {
        return actual_rank;
    }
    else
    {
        return (num_cols - num_augmented_columns);
    }
}

static inline int is_matrix_consistent(double *matrix_to_check, int num_rows, int num_cols, int num_augmented_columns)
{
    // NOTE: This is actually confusing given how I've documented the calculate_matrix_rank function. I'll need to re-think the design.
    int augmented_rank = calculate_matrix_rank(matrix_to_check, num_rows, num_cols, 0);
    int base_matrix_rank = calculate_matrix_rank(matrix_to_check, num_rows, num_cols, num_augmented_columns);
    if (base_matrix_rank != augmented_rank)
    {
        printf("System of Equations is not Consistent. No solution exists.\n");
        return 0;
    }
    else if (base_matrix_rank == augmented_rank && augmented_rank == (num_cols - 1))
    {
        printf("System of Equations is Consistent. Unique solution exists.\n");
        return 1;
    }
    else if (base_matrix_rank == augmented_rank && augmented_rank < (num_cols - 1))
    {
        printf("System of Equations is Consistent. Infinite solutions exist.\n");
        return 1;
    }
    else
    {
        printf("Somehow rank(A|b) > n. Don't know what to do.\n");
        return 1;
    }
}

static inline void python_is_matrix_consistent(double *matrix_to_check, int num_rows, int num_cols, struct String *message_buffer, struct MatrixMetadata *metadata)
{
    metadata->augmented_matrix_rank = calculate_matrix_rank(matrix_to_check, num_rows, num_cols, 0);
    metadata->matrix_rank = calculate_matrix_rank(matrix_to_check, num_rows, num_cols, 1);

    if (metadata->matrix_rank != metadata->augmented_matrix_rank)
    {
        writeNulTerminatedString("System of Equations is not Consistent. No solution exists.\n", message_buffer);
        metadata->is_consistent = 0;
    }
    else if (metadata->matrix_rank == metadata->augmented_matrix_rank && metadata->augmented_matrix_rank == (num_cols - 1))
    {
        writeNulTerminatedString("System of Equations is Consistent. Unique solution exists.\n", message_buffer);
        metadata->is_consistent = 1;
    }
    else if (metadata->matrix_rank == metadata->augmented_matrix_rank && metadata->augmented_matrix_rank < (num_cols - 1))
    {
        writeNulTerminatedString("System of Equations is Consistent. Infinite solutions exist.\n", message_buffer);
        metadata->is_consistent = 1;
    }
    else
    {
        writeNulTerminatedString("Somehow rank(A|b) > n. Don't know what to do.\n", message_buffer);
        metadata->is_consistent = 1;
    }
}

static inline void print_matrix(double *matrix_to_print, int num_rows, int num_cols)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            printf("% f\t", matrix_to_print[(row * num_cols) + col]);
        }
        printf("\n");
    }
}

static inline void print_augmented_matrix(double *matrix_to_print, int num_rows, int num_cols, int num_augmented_cols)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            printf("% f\t", matrix_to_print[(row * num_cols) + col]);
            if (col == ((num_cols - num_augmented_cols) - 1))
            {
                printf("|\t");
            }
        }
        printf("\n");
    }
}

static inline void print_matrix_to_buffer(double *matrix_to_print, int num_rows, int num_cols, struct String *message_buffer)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            writeDecimalNumber((int64_t)(matrix_to_print[(row * num_cols) + col] * 1e6), 6, message_buffer);
            writeStringNoNullTerminator("\t", message_buffer);
        }
        writeNulTerminatedString("\n", message_buffer);
    }
}

static inline void print_augmented_matrix_to_buffer(double *matrix_to_print, int num_rows, int num_cols, int num_augmented_cols, struct String *message_buffer)
{
    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_cols; col++)
        {
            writeDecimalNumber((int64_t)(matrix_to_print[(row * num_cols) + col] * 1e6), 6, message_buffer);
            writeStringNoNullTerminator("\t", message_buffer);
            if (col == ((num_cols - num_augmented_cols) - 1))
            {
                writeStringNoNullTerminator("|\t", message_buffer);
            }
        }
        writeNulTerminatedString("\n", message_buffer);
    }
}

static inline void multiply_by_nonzero_scalar(double *matrix_to_scale, int row_index, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_scale[(row_index * num_cols) + col] *= scalar;
    }
}

static inline void subtract_scaled_row(double *matrix_to_scale, int row_index_to_modify, int row_to_use_for_subtraction, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_scale[(row_index_to_modify * num_cols) + col] -= (scalar * matrix_to_scale[(row_to_use_for_subtraction * num_cols) + col]);
    }
}

static inline void add_scaled_row(double *matrix_to_scale, int row_index_to_modify, int row_to_use_for_addition, int num_cols, double scalar)
{
    for (int col = 0; col < num_cols; col++)
    {
        matrix_to_scale[(row_index_to_modify * num_cols) + col] += (scalar * matrix_to_scale[(row_to_use_for_addition * num_cols) + col]);
    }
}

static inline void swap_rows(double *matrix_to_swap_rows, int row_to_swap_index_a, int row_to_swap_index_b, int num_cols)
{
    double row_holder[16];
    size_t num_bytes_per_row = sizeof(double) * num_cols;
    printf("Num Bytes Per Row: %zd\n", num_bytes_per_row);
    memcpy(&row_holder, &matrix_to_swap_rows[(row_to_swap_index_a * num_cols) + 0], num_bytes_per_row);
    memcpy(&matrix_to_swap_rows[(row_to_swap_index_a * num_cols) + 0], &matrix_to_swap_rows[(row_to_swap_index_b * num_cols) + 0], num_bytes_per_row);
    memcpy(&matrix_to_swap_rows[(row_to_swap_index_b * num_cols) + 0], &row_holder, num_bytes_per_row);
}

static inline void perform_gauss_jordan_reduction(double *matrix_to_reduce, int num_rows, int num_cols, int num_augmented_cols)
{
    int size_main_diagonal;
    int swap_rows_flag = 0;
    // Account for the extra column (the augmented matrix)
    if ((num_cols - num_augmented_cols) <= num_rows)
    {
        size_main_diagonal = (num_cols - num_augmented_cols);
    }
    else
    {
        size_main_diagonal = num_rows;
    }

    // Iterate down the main diagonal to begin conversion to echelon form
    double product_of_diagonal_elements = 1.0;
    double denominator_value = 1;
    int swap_multiplier = 1;
    for (int i = 0; i < size_main_diagonal; i++)
    {
        double pivot_element = matrix_to_reduce[(i * num_cols) + i];
        if (pivot_element == 0)
        {
            // See if there is a nonzero element in the same column (below it) and swap the rows.
            swap_rows_flag = 1;
        }
        double value_below_pivot_element;
        for (int row = (i + 1); row < num_rows; row++)
        {
            value_below_pivot_element = matrix_to_reduce[(row * num_cols) + i];
            if (value_below_pivot_element != 0)
            {
                if (swap_rows_flag == 1)
                {
                    // Swap the rows instead.
                    printf("[SWP] Row %d = (R%d) <=> (R%d)\n", (row + 1), (row + 1), (i + 1));
                    swap_rows(matrix_to_reduce, row, i, num_cols);
                    swap_rows_flag = 0;
                    pivot_element = matrix_to_reduce[(i * num_cols) + i];
                    printf("New Pivot Element: %.3f\n", pivot_element);
                    swap_multiplier *= -1;
                }
                else
                {
                    double reciporical_fraction_scalar;
                    if (value_below_pivot_element < 0)
                    {
                        reciporical_fraction_scalar = (-1.0) * (value_below_pivot_element / pivot_element);
                        printf("[ADD] Row %d = (R%d) + %.3f*(R%d)\n", row, row, reciporical_fraction_scalar, i);
                        add_scaled_row(matrix_to_reduce, row, i, num_cols, reciporical_fraction_scalar);
                    }
                    else
                    {
                        reciporical_fraction_scalar = (value_below_pivot_element / pivot_element);
                        printf("[SUB] Row %d = (R%d) - %.3f*(R%d)\n", row, row, reciporical_fraction_scalar, i);
                        subtract_scaled_row(matrix_to_reduce, row, i, num_cols, reciporical_fraction_scalar);
                    }
                }
            }
            print_augmented_matrix(matrix_to_reduce, num_rows, num_cols, num_augmented_cols);
        }
        product_of_diagonal_elements *= pivot_element;
    }

    // Next perform the second half
    printf("Shifting to Reduced Row Echelon Portion of Algorithm\n");
    for (int i = (size_main_diagonal - 1); i > -1; i--)
    {
        // Try to convert the pivot element to 1
        double pivot_element = matrix_to_reduce[(i * num_cols) + i];
        double pivot_reciporical;
        if (pivot_element == 0)
        {
        }
        else
        {
            if (pivot_element != 1)
            {
                pivot_reciporical = (1.0 / pivot_element);
                printf("[SCL] Row %d = %.3f*(R%d)\n", (i + 1), pivot_reciporical, (i + 1));
                multiply_by_nonzero_scalar(matrix_to_reduce, i, num_cols, pivot_reciporical);
                // denominator_value *= pivot_reciporical;
                print_matrix(matrix_to_reduce, num_rows, num_cols);
                pivot_element = matrix_to_reduce[(i * num_cols) + i];
            }
            double value_above_pivot_element;
            for (int row = (i - 1); row > -1; row--)
            {
                value_above_pivot_element = matrix_to_reduce[(row * num_cols) + i];
                if (value_above_pivot_element != 0)
                {
                    double reciporical_fraction_scalar;
                    reciporical_fraction_scalar = (value_above_pivot_element / pivot_element);
                    printf("Reciporical Fraction Scalar: %.3f / %.3f = %.3f\n", value_above_pivot_element, pivot_element, reciporical_fraction_scalar);
                    printf("[SUB] Row %d = (R%d) - %.3f*(R%d)\n", (row + 1), (row + 1), reciporical_fraction_scalar, (i + 1));
                    subtract_scaled_row(matrix_to_reduce, row, i, num_cols, reciporical_fraction_scalar);
                    // printf("[ADD] Row %d = (R%d) + %.3f*(R%d)\n", (row + 1), (row + 1), reciporical_fraction_scalar, (i + 1));
                    // add_scaled_row(matrix_to_reduce, row, i, num_cols, reciporical_fraction_scalar);
                    // reciporical_fraction_scalar = (value_above_pivot_element / pivot_element);
                    // subtract_scaled_row(matrix_to_reduce, row, i, num_cols, reciporical_fraction_scalar);
                }
                print_matrix(matrix_to_reduce, num_rows, num_cols);
            }
        }
    }

    if (is_matrix_consistent(matrix_to_reduce, num_rows, num_cols, num_augmented_cols))
    {
        printf("Product of Diagonal Elements is %.6f\n", product_of_diagonal_elements);
        printf("Denominator value is %.6f\n", denominator_value);
        printf("Swap Multiplier is %d\n", swap_multiplier);
        double determinant = (product_of_diagonal_elements / denominator_value) * swap_multiplier;
        printf("Determinant of non-augmented matrix A is %.6f\n", determinant);
    }
}

static inline void perform_square_matrix_inversion_gaussian_reduction(double *matrix_to_invert, int num_rows, int num_cols)
{
    struct MatrixMetadata matrix_to_invert_metadata;
    matrix_to_invert_metadata.num_rows = num_rows;
    matrix_to_invert_metadata.num_cols = num_cols;
    printf("Matrix To Invert Metadata\n\n\n");
    print_matrix_metadata(&matrix_to_invert_metadata);
    double *identity_matrix = generate_square_identity_matrix(num_rows, num_cols);
    struct MatrixMetadata identity_matrix_metadata;
    identity_matrix_metadata.num_rows = num_rows;
    identity_matrix_metadata.num_cols = num_cols;
    printf("Identity Matrix Metadata\n\n\n");
    print_matrix_metadata(&identity_matrix_metadata);
    double *augmented_matrix = (double *)malloc(sizeof(double) * (num_rows * (matrix_to_invert_metadata.num_cols + identity_matrix_metadata.num_cols)));
    struct MatrixMetadata augmented_matrix_metadata;
    augmented_matrix_metadata.num_rows = num_rows;
    augmented_matrix_metadata.num_cols = matrix_to_invert_metadata.num_cols + identity_matrix_metadata.num_cols;
    printf("Horizontal Stack of matrix with identity matrix\n");
    // Create the augmented matrix of the input matrix and its identity and start the Gaussian reduction.
    hstack(matrix_to_invert, identity_matrix, augmented_matrix, &matrix_to_invert_metadata, &identity_matrix_metadata, &augmented_matrix_metadata);
    print_augmented_matrix(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, identity_matrix_metadata.num_cols);
    perform_gauss_jordan_reduction(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, identity_matrix_metadata.num_cols);
    // Free the matrices
    free(identity_matrix);
    free(augmented_matrix);
}

EXPORT void python_perform_gauss_jordan_reduction(double *matrix_to_reduce, double *matrix_augment, struct String *message_buffer, struct MatrixMetadata *metadata, struct MatrixMetadata *matrix_augment_metadata)
{

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
                    writeStringNoNullTerminator("[SWP] Row ", message_buffer);
                    writeNumber((row + 1), message_buffer);
                    writeStringNoNullTerminator(" = (R", message_buffer);
                    writeNumber((row + 1), message_buffer);
                    writeStringNoNullTerminator(") <=> (R", message_buffer);
                    writeNumber((i + 1), message_buffer);
                    writeNulTerminatedString(")\n", message_buffer);
                    swap_rows(augmented_matrix, row, i, augmented_matrix_metadata.num_cols);
                    swap_rows_flag = 0;
                    pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
                    writeStringNoNullTerminator("New Pivot Element: ", message_buffer);
                    writeDecimalNumber((int64_t)(pivot_element * 1e9), 9, message_buffer);
                    writeNulTerminatedString("\n", message_buffer);
                    swap_multiplier *= -1;
                }
                else
                {
                    double reciporical_fraction_scalar;
                    if (value_below_pivot_element < 0)
                    {
                        reciporical_fraction_scalar = (-1.0) * (value_below_pivot_element / pivot_element);

                        writeStringNoNullTerminator("[ADD] Row ", message_buffer);
                        writeNumber(row, message_buffer);
                        writeStringNoNullTerminator(" = (R", message_buffer);
                        writeNumber(row, message_buffer);
                        writeStringNoNullTerminator(") + ", message_buffer);
                        writeDecimalNumber((int64_t)(reciporical_fraction_scalar * 1e9), 9, message_buffer);
                        writeStringNoNullTerminator("*(R", message_buffer);
                        writeNumber(i, message_buffer);
                        writeNulTerminatedString(")\n", message_buffer);
                        add_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciporical_fraction_scalar);
                    }
                    else
                    {
                        reciporical_fraction_scalar = (value_below_pivot_element / pivot_element);
                        writeStringNoNullTerminator("[SUB] Row ", message_buffer);
                        writeNumber(row, message_buffer);
                        writeStringNoNullTerminator(" = (R", message_buffer);
                        writeNumber(row, message_buffer);
                        writeStringNoNullTerminator(") - ", message_buffer);
                        writeDecimalNumber((int64_t)(reciporical_fraction_scalar * 1e9), 9, message_buffer);
                        writeStringNoNullTerminator("*(R", message_buffer);
                        writeNumber(i, message_buffer);
                        writeNulTerminatedString(")\n", message_buffer);
                        subtract_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciporical_fraction_scalar);
                    }
                }
            }
            print_augmented_matrix_to_buffer(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
        }
        product_of_diagonal_elements *= pivot_element;
    }

    // Next perform the second half
    writeNulTerminatedString("Shifting to Reduced Row Echelon Portion of Algorithm\n", message_buffer);
    for (int i = (size_main_diagonal - 1); i > -1; i--)
    {
        // Try to convert the pivot element to 1
        double pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
        double pivot_reciporical;
        if (pivot_element == 0)
        {
        }
        else
        {
            if (pivot_element != 1)
            {
                pivot_reciporical = (1.0 / pivot_element);
                writeStringNoNullTerminator("[SCL] Row ", message_buffer);
                writeNumber((i + 1), message_buffer);
                writeStringNoNullTerminator(" = ", message_buffer);
                writeDecimalNumber((int64_t)(pivot_reciporical * 1e9), 9, message_buffer);
                writeStringNoNullTerminator(" * (R", message_buffer);
                writeNumber((i + 1), message_buffer);
                writeNulTerminatedString(")\n", message_buffer);
                multiply_by_nonzero_scalar(augmented_matrix, i, augmented_matrix_metadata.num_cols, pivot_reciporical);
                // denominator_value *= pivot_reciporical;
                print_augmented_matrix_to_buffer(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
                pivot_element = augmented_matrix[(i * augmented_matrix_metadata.num_cols) + i];
            }
            double value_above_pivot_element;
            for (int row = (i - 1); row > -1; row--)
            {
                value_above_pivot_element = augmented_matrix[(row * augmented_matrix_metadata.num_cols) + i];
                if (value_above_pivot_element != 0)
                {
                    double reciporical_fraction_scalar;
                    reciporical_fraction_scalar = (value_above_pivot_element / pivot_element);
                    writeStringNoNullTerminator("Reciporical Fraction Scalar: ", message_buffer);
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
                    writeDecimalNumber((int64_t)(reciporical_fraction_scalar * 1e9), 9, message_buffer);
                    writeStringNoNullTerminator("*(R", message_buffer);
                    writeNumber((i + 1), message_buffer);
                    writeNulTerminatedString(")\n", message_buffer);
                    subtract_scaled_row(augmented_matrix, row, i, augmented_matrix_metadata.num_cols, reciporical_fraction_scalar);
                }
                print_augmented_matrix_to_buffer(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, matrix_augment_metadata->num_cols, message_buffer);
            }
        }
    }
    python_is_matrix_consistent(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, message_buffer, metadata);
    if (metadata->is_consistent == 1)
    {
        writeStringNoNullTerminator("Product of Diagonal Elements is: ", message_buffer);
        writeDecimalNumber((int64_t)(product_of_diagonal_elements * 1e9), 9, message_buffer);
        writeNulTerminatedString("\n", message_buffer);

        writeStringNoNullTerminator("Denominator Value is: ", message_buffer);
        writeDecimalNumber((int64_t)(denominator_value * 1e9), 9, message_buffer);
        writeNulTerminatedString("\n", message_buffer);

        writeStringNoNullTerminator("Swap Multiplier is: ", message_buffer);
        writeNumber(swap_multiplier, message_buffer);
        writeNulTerminatedString("\n", message_buffer);

        metadata->matrix_determinant = (product_of_diagonal_elements / denominator_value) * swap_multiplier;

        writeStringNoNullTerminator("Determinant of non-augmented matrix A is: ", message_buffer);
        writeDecimalNumber((int64_t)(metadata->matrix_determinant * 1e9), 9, message_buffer);
        writeNulTerminatedString("\n", message_buffer);
    }
}

EXPORT void python_perform_square_matrix_inversion_gaussian_reduction(double *matrix_to_invert, int num_rows, int num_cols, struct MatrixMetadata matrix_to_invert_metadata, struct String *message_buffer)
{
    // This also covers if there is a row or column of zero values
    if (matrix_to_invert_metadata.matrix_determinant == 0)
    {
        writeNulTerminatedString("The matrix provided has a determinant of 0, meaning it is not invertible.", message_buffer);
        return;
    }
    else if ((matrix_to_invert_metadata.matrix_rank != matrix_to_invert_metadata.num_cols) || (matrix_to_invert_metadata.matrix_rank != matrix_to_invert_metadata.num_rows))
    {
        writeNulTerminatedString("The matrix provided does not have full rank and thus it cannot be invertible.", message_buffer);
        return;
    }
    matrix_to_invert_metadata.num_rows = num_rows;
    matrix_to_invert_metadata.num_cols = num_cols;
    // print_matrix_metadata(&matrix_to_invert_metadata);
    double *identity_matrix = generate_square_identity_matrix(num_rows, num_cols);
    struct MatrixMetadata identity_matrix_metadata;
    identity_matrix_metadata.num_rows = num_rows;
    identity_matrix_metadata.num_cols = num_cols;
    // printf("Identity Matrix Metadata\n\n\n");
    // print_matrix_metadata(&identity_matrix_metadata);
    double *augmented_matrix = (double *)malloc(sizeof(double) * (num_rows * (matrix_to_invert_metadata.num_cols + identity_matrix_metadata.num_cols)));
    struct MatrixMetadata augmented_matrix_metadata;
    augmented_matrix_metadata.num_rows = num_rows;
    augmented_matrix_metadata.num_cols = matrix_to_invert_metadata.num_cols + identity_matrix_metadata.num_cols;
    // Create the augmented matrix of the input matrix and its identity and start the Gaussian reduction.
    hstack(matrix_to_invert, identity_matrix, augmented_matrix, &matrix_to_invert_metadata, &identity_matrix_metadata, &augmented_matrix_metadata);
    perform_gauss_jordan_reduction(augmented_matrix, augmented_matrix_metadata.num_rows, augmented_matrix_metadata.num_cols, identity_matrix_metadata.num_cols); //, message_buffer, augmented_matrix_metadata);
    // Free the matrices
    free(identity_matrix);
    free(augmented_matrix);
}

// int main()
// {
//     double matrix_to_reduce[12] = {
//         2, 1, -1, 8,
//         -3, -1, 2, -11,
//         -2, 1, 2, -3};
//     perform_gauss_jordan_reduction(matrix_to_reduce, 3, 4, 1);
//     printf("\n\n\n");
//     printf("Performing First Matrix Inversion\n");
//     double matrix_to_invert[9] = {
//         2, 1, -1,
//         -3, -1, 2,
//         -2, 1, 2};
//     perform_square_matrix_inversion_gaussian_reduction(matrix_to_invert, 3, 3);
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