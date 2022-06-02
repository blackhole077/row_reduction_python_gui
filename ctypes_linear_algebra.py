import ctypes
import os
from sys import platform as sys_platform
from typing import *
DLL_FILE_NAME_STUB = "row_reduction"


class MatrixMetadata(ctypes.Structure):
    """
    A ctypes structure that holds metadata information about a given matrix (and its augmented form).

    Fields/Attributes
    -----------------
    """

    _fields_ = [
        ("num_rows", ctypes.c_int),
        ("num_cols", ctypes.c_int),
        ("augmented_matrix_rank", ctypes.c_int),
        ("matrix_rank", ctypes.c_int),
        ("is_consistent", ctypes.c_int),
        ("matrix_determinant", ctypes.c_double),
    ]


class String(ctypes.Structure):
    """ """

    _fields_ = [
        ("length", ctypes.c_int64),
        ("capacity", ctypes.c_int64),
        ("attempted_to_write_more_than_capacity", ctypes.c_int),
        ("buffer", ctypes.c_char_p),
    ]

def make_string(length: int, capacity: int, buffer: Union[str,bytes]):
    if isinstance(buffer, str):
        return ctypes.POINTER(String(length, capacity, 0, bytes(buffer, "utf-8")))
    else:
        return ctypes.POINTER(String(length, capacity, 0, buffer))

def get_dict(struct: ctypes.Structure) -> dict:
    """
        Convert a ctypes Structure into a Python dictionary.

        Attempts to convert all fields from ctypes-specific objects
        to Python types. This function does work recursively, so nested
        structures should not be an issue.

        NOTE: This function does NOT convert pointers into arrays.

    Args:
        struct (ctypes.Structure):
            The struct you wish to convert into a dictionary

    Returns:
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
        # If it is has fields then it is probably a struct
        elif hasattr(value, "_fields_"):
            value = get_dict(value)
        result[field] = value
    return result


def string_read_line(buffer: bytes, start_index: int = 0):
    byte_array_to_display: bytearray = bytearray()
    for index, byte in enumerate(buffer[start_index:]):
        if byte is not None:
            if byte != ord("\r") and byte != ord("\n"):
                byte_array_to_display.append(byte)
            # If a newline is reached then a complete chunk of data is ready to process
            if byte == ord("\n"):
                print(f"Record to return (hit newline): {byte_array_to_display}")
                return byte_array_to_display, index+1
        else:
            print(f"Record to return (none byte): {byte_array_to_display}")
            return byte_array_to_display, index
    # If somehow the buffer is either empty or reaching the end of it doesn't trigger the other return statements
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
        file_to_load = os.path.join(current_directory, f"{DLL_FILE_NAME_STUB}.dll")
    else:
        file_to_load = os.path.join(current_directory, f"{DLL_FILE_NAME_STUB}.so")
    print(f"Platform: {current_system}\tLoading File: {file_to_load}")
    if os.path.exists(file_to_load):
        return ctypes.cdll.LoadLibrary(file_to_load)
    else:
        raise ValueError(
            f"{file_to_load} cannot be found in current directory {current_directory}. Please make sure this file is in the same location as ctypes_gps.py"
        )


###############################################################################
#                                                                             #
#                                                                             #
#                  CTYPES_LINEAR_ALGEBRA CODE THAT RUNS ON STARTUP            #
#                                                                             #
#                                                                             #
###############################################################################

linear_algebra_dll = find_library_file()
# EXPORT void python_perform_gauss_jordan_reduction(double *matrix_to_reduce, int num_rows, int num_cols, struct MatrixMetadata *metadata)
perform_gauss_jordan_reduction = (
    linear_algebra_dll.python_perform_gauss_jordan_reduction
)
perform_gauss_jordan_reduction.argtypes = (
    ctypes.POINTER(ctypes.c_double),  # matrix_to_reduce
    ctypes.c_int,  # num_rows
    ctypes.c_int,  # num_cols,
    ctypes.POINTER(String),  # message_buffer
    ctypes.POINTER(MatrixMetadata),  # metadata,
)
perform_gauss_jordan_reduction.restype = None
