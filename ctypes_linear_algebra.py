"""
    Python code that serves as the middle-man between the GUI and the C code.
    Handles the data marshalling and Python definition of functions to ensure
    functions match their C counterpart.
"""

import ctypes
import os
from sys import platform as sys_platform
from typing import *
from memory_profiler import profile

###############################################################################
#                                                                             #
#                                                                             #
#              CTYPES_LINEAR_ALGEBRA CLASSES AND CLASS FUNCTIONS              #
#                                                                             #
#                                                                             #
###############################################################################

class MatrixMetadata(ctypes.Structure):
    """
        A ctypes structure that holds metadata information about a given matrix (and its augmented form).

        Fields/Attributes
        -----------------
        num_rows: int
            The number of rows in the matrix.
        num_cols: int
            The number of columns in the matrix.
        is_consistent: int
            A 0/1 integer flag that indicates whether the matrix is consistent. In essence, it determines whether a System of Linear Equations is solvable.
            When initializing, the default value should be -1 to indicate an 'Unknown' value.
        matrix_determinant: double
            A value that represents the determinant of the matrix. Has many uses, including determining whether a matrix is invertible.
            When initializing, the default value should be -1 to indicate an 'Unknown' value (although this could be a valid determinant as well).

        How To Initialize
        -----------------
        Initialization of a ctypes.Structure object is actually exactly the same as any custom Python class that has an __init__ function.
        The reason it might be unintuitive is that, unlike most Python classes, you do not have to write an __init__ function for whatever reason.
        
        Example of initializing a MatrixMetadata structure and accessing its attributes:
            >>> matrix_metadata = MatrixMetadata(num_rows=3, num_cols=3, is_consistent=-1, matrix_determinant=-1)
            >>> print(f"Number of Rows in Matrix: {matrix_metadata.num_rows}")
            >>> print(f"Number of Columns in Matrix: {matrix_metadata.num_cols}")


    """

    _fields_ = [
        ("num_rows", ctypes.c_int),
        ("num_cols", ctypes.c_int),
        ("matrix_rank", ctypes.c_int),
        ("is_consistent", ctypes.c_int),
        ("matrix_determinant", ctypes.c_double),
    ]


class String(ctypes.Structure):
    """
        A ctypes structure that functions similarly to the str class in Python.

        Fields/Attributes
        -----------------
        length: int64
            An integer representing the current length of the String structure, in bytes. Bear in mind that one char in C is one byte.
        capacity: int64
            An integer representing the total length of the String structure, in bytes. Used to avoid buffer overflow errors.
        attempted_to_write_more_than_capacity: int
            A 0/1 integer flag that indicates whether the most recent write operation that was performed attempted to write more characters
            to the string than its capacity. Essentially a simpler way to check if a buffer overflow may have occurred.
        buffer: void*
            A void pointer to some memory that can be written to. SHould house the bytes of string data and have at least capacity bytes of
            memory allocated to it (if this structure is created on the C side.) The reason it is void* instead of char* is to avoid type mismatches, though
            it may be possible to use a char* instead.

        How To Initialize
        -----------------
        Initialization of a ctypes.Structure object is actually exactly the same as any custom Python class that has an __init__ function.
        The reason it might be unintuitive is that, unlike most Python classes, you do not have to write an __init__ function for whatever reason.
        
        Example of initializing a String structure and accessing its attributes:
            >>> buffer_string = String(0, 4096, 0, b"")
            >>> print(f"Length of String: {buffer_string.length}")
            >>> print(f"Capacity of String: {buffer_string.capacity}")
            >>> print(f"String Contents: {str(buffer_string.buffer, 'utf-8')}")
    """

    _fields_ = [
        ("length", ctypes.c_int64),
        ("capacity", ctypes.c_int64),
        ("attempted_to_write_more_than_capacity", ctypes.c_int),
        ("buffer", ctypes.c_char_p),
    ]


def get_dict(struct: ctypes.Structure) -> dict:
    """
        Convert a ctypes Structure into a Python dictionary.

        Attempts to convert all fields from ctypes-specific objects
        to Python types. This function does work recursively, so nested
        structures should not be an issue.

        NOTE: This function does NOT convert pointers into arrays.

        Parameters
        ----------
        struct (ctypes.Structure):
            The struct you wish to convert into a dictionary.

        Returns
        -------
        result:
            The resulting dictionary. Keys are taken from the field names,
            and values are converted into Python objects.
    """

    result = {}
    for field, _ in struct._fields_:
        value = getattr(struct, field)
        # If it's not a primitive type and it evaluates to false then we assume it's a null pointer/value
        if (type(value) not in [int, float, bool, bytes]) and not bool(value):
            value = None
        # If it's an array, pass for now. We might be able to convert it into a Numpy array later
        elif hasattr(value, "_type_"):
            pass
        # If it is has fields then it is probably a ctypes Structure, so we recursively call get_dict.
        elif hasattr(value, "_fields_"):
            value = get_dict(value)
        result[field] = value
    return result


#@profile
def string_read_line(
    buffer: Union[bytearray, bytes], start_index: int = 0
) -> Tuple[bytearray, int]:
    """
        Read a line from a buffer.

        Read characters from a buffer of bytes, starting either from the start_index value provided
        until either a newline is hit or None is hit. Once this happens, it returns the characters that
        have been appended so far, along with the index of where the next call to the function should start.

        NOTE: This function does not append return carriage ('\\r') tokens, effectively stripping them out.

        Parameters
        ----------
        buffer: bytes
            A buffer of characters to be read. The implicit assumption of the buffer is that the characters
            fall within the UTF-8 encoding (ASCII probably works too).
        start_index: int, default 0
            The index to start from when enumerating. It is used to create a slice copy of the buffer.

        Returns
        -------
        ret: Tuple[bytearray, int]
            A tuple that contains the following pieces:
            byte_array_to_display: bytearray
                A mutable form of the characters that were read from the buffer.
            index: int
                The index of the last character present in byte_array_to_display.
    """
    byte_array_to_display: bytearray = bytearray()
    for index, byte in enumerate(buffer[start_index:]):
        if byte is not None:
            if byte != ord("\r") and byte != ord("\n"):
                byte_array_to_display.append(byte)
            # If a newline is reached then a complete chunk of data is ready to process
            if byte == ord("\n"):
                # print(f"Record to return (hit newline): {byte_array_to_display}")
                return byte_array_to_display, index + 1
        else:
            # print(f"Record to return (none byte): {byte_array_to_display}")
            if isinstance(buffer, bytearray):
                buffer.clear()
            return byte_array_to_display, index
    # If somehow the buffer is either empty or reaching the end of it doesn't trigger the other return statements
    if start_index >= len(buffer) and isinstance(buffer, bytearray):
        buffer.clear()
    return bytearray("", "utf-8"), 0


def find_library_file() -> ctypes.CDLL:
    """A small helper function to find and load the shared object file for elevation parsing.

    Raises:
        ValueError: If the shared object file is not in the same directory as this file, then
        an error is raised to inform the developer that they need to place it there manually.
        This is because I cannot be certain of what any new data directory structure looks like.

    Returns:
        ctypes.CDLL: The Dynamic Loaded Library (DLL) or Shared Object (SO) file.
    """
    current_directory = os.getcwd()
    current_system = sys_platform
    file_to_load: str = ""
    if current_system == "win32":
        file_to_load = os.path.join(current_directory, "row_reduction.dll")
    else:
        file_to_load = os.path.join(current_directory, "row_reduction.so")
    print(f"Platform: {current_system}\tLoading File: {file_to_load}")
    if os.path.exists(file_to_load):
        return ctypes.cdll.LoadLibrary(file_to_load)
    else:
        raise ValueError(
            f"{file_to_load} cannot be found in current directory {current_directory}. Please make sure this file is in the same location as {__name__}.py"
        )

###############################################################################
#                                                                             #
#                                                                             #
#               CTYPES_LINEAR_ALGEBRA CODE THAT RUNS ON STARTUP               #
#                                                                             #
#                                                                             #
###############################################################################

linear_algebra_dll = find_library_file()
perform_gauss_jordan_reduction = (
    linear_algebra_dll.python_perform_gauss_jordan_reduction
)
# While I am not certain that the order of the arguments matters, I do it anyway to potentially avoid any bugs.
perform_gauss_jordan_reduction.argtypes = (
    ctypes.POINTER(ctypes.c_double),  # matrix_to_reduce
    ctypes.POINTER(ctypes.c_double),  # matrix_augment
    ctypes.POINTER(String),  # String *message_buffer
    ctypes.POINTER(MatrixMetadata),  # MatrixMetadata *metadata
    ctypes.POINTER(MatrixMetadata),  # MatrixMetadata *augment_metadata
)
# The restype is None because the function on the C side of the code is void
perform_gauss_jordan_reduction.restype = None

perform_square_matrix_inversion = (
    linear_algebra_dll.python_perform_square_matrix_inversion_gaussian_reduction
)
# While I am not certain that the order of the arguments matters, I do it anyway to potentially avoid any bugs.
perform_square_matrix_inversion.argtypes = (
    ctypes.POINTER(ctypes.c_double),  # *matrix_to_invert
    ctypes.POINTER(MatrixMetadata),  # MatrixMetadata *matrix_to_invert_metadata
    ctypes.POINTER(String),  # String *message_buffer
)
# The restype is None because the function on the C side of the code is void
perform_square_matrix_inversion.restype = None
