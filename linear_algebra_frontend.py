"""
    The GUI portion of the Linear Algebra GUI.
    Take in user inputs for matrices to either solve via Gaussian reduction or invert (assuming they are square).
    Display the steps taken by the algorithm in the text display so that users can follow along and check their work.
"""

import ctypes
import tkinter as tk
import tkinter.scrolledtext as tkst
from tkinter import ttk
import tracemalloc
from typing import List, Union

import numpy as np

import ctypes_linear_algebra


# TODO: Shift the validation step to be when the "solve matrix" button is pressed instead.
class NumberOnlyEntry(tk.Entry):
    def __init__(self, master=None, **kwargs) -> None:
        self.var = tk.StringVar()
        tk.Entry.__init__(self, master, textvariable=self.var, **kwargs)
        self.old_value = ""
        self.var.trace("w", self.check)
        self.get, self.set = self.var.get, self.var.set

    def check(self, *args) -> None:
        """
            Check that the value entered is a number.
            Exceptions to this check are the negative sign '-' and no entry.
        """

        try:
            # We assume that negative numbers will have more to it
            if self.get() == "-":
                return
            elif self.get() == "":
                return
            else:
                new_value = float(self.get())
                self.old_value = new_value
        except ValueError:
            pass


class MatrixInput:
    def __init__(
        self,
        parent: Union[tk.Frame, ttk.LabelFrame],
        label: str = "Matrix",
        num_rows: int = 2,
        num_cols: int = 2,
    ) -> None:
        self.parent = parent
        self.num_rows = num_rows
        self.num_cols = num_cols
        self._frame = ttk.LabelFrame(master=self.parent, text=label, relief="raised")
        self.matrix_data = None
        self.entries: List[NumberOnlyEntry] = []
        self.metadata: ctypes_linear_algebra.MatrixMetadata = ctypes_linear_algebra.MatrixMetadata(
            num_rows=self.num_rows,
            num_cols=self.num_cols,
            matrix_rank=-1,
            is_consistent=-1,
            matrix_determinant=-1,
        )
        # self._frame.pack(fill=tk.X, expand=True)
        for row in range(self.num_rows):
            for col in range(self.num_cols):
                matrix_value_entry = NumberOnlyEntry(master=self._frame)
                matrix_value_entry.grid(row=row, column=col)
                self.entries.append(matrix_value_entry)
        self.update()
    # @profile
    def update_display(self) -> None:
        """
            Update the GUI display of entries to match the dimensions of the matrix.

            Given an MxN matrix, this function should update the GUI to ensure that
            there are (M*N) NumberOnlyEntry components arranged in an MxN grid.

            Parameters
            ----------
            None.

            Returns
            -------
            None.
        """

        # First, remove all entries currently displayed and clear the list of entries, resetting it back to its original state.
        for entry in self.entries:
            entry.grid_forget()
        self.entries.clear()
        # Fill in the values and grid them according to the new dimensions.
        for row in range(self.num_rows):
            for col in range(self.num_cols):
                matrix_value_entry = NumberOnlyEntry(master=self._frame)
                matrix_value_entry.grid(row=row, column=col)
                self.entries.append(matrix_value_entry)

    def add_row(self) -> None:
        """
            Add a row to the matrix.

            NOTE: Automatically calls update_display()

            Parameters
            ----------
            None.

            Returns
            -------
            None.

        """
        self.num_rows += 1
        self.update_display()

    def remove_row(self) -> None:
        """
            Remove a row from the matrix. Does nothing if there is only one row.

            NOTE: Automatically calls update_display()

            Parameters
            ----------
            None.

            Returns
            -------
            None.

        """

        if self.num_rows > 1:
            self.num_rows -= 1
            self.update_display()

    def add_col(self) -> None:
        """
            Add a column to the matrix.

            NOTE: Automatically calls update_display()

            Parameters
            ----------
            None.

            Returns
            -------
            None.

        """

        self.num_cols += 1
        self.update_display()

    def remove_col(self) -> None:
        """
            Remove a column from the matrix. Does nothing if there is only one column.

            NOTE: Automatically calls update_display()

            Parameters
            ----------
            None.

            Returns
            -------
            None.

        """

        if self.num_cols > 1:
            self.num_cols -= 1
            self.update_display()
    # @profile
    def update(self) -> None:
        """
            The update function for the widget. Should run every 100 milliseconds.

            For MatrixInput, the update function will update its metadata such that its dimensions reflect
            the current user-defined dimensions, the matrix values are updated to either NaN if no values were
            provided for the corresponding NumberOnlyEntry or some float value.

            Parameters
            ----------
            None.

            Returns
            -------
            None.
        """

        self.metadata.num_rows = self.num_rows
        self.metadata.num_cols = self.num_cols
        # matrix_values = np.array(
        #     [
        #         entry.old_value
        #         if entry.old_value != "" and entry.old_value != "-"
        #         else np.nan
        #         for entry in self.entries
        #     ]
        # ).reshape(self.num_rows, self.num_cols)
        # # Cast the matrix_data as a ctypes double pointer (i.e., double* in C)
        # self.matrix_data = matrix_values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        # Update every 100ms
        self.parent.after(100, self.update)


# BUG: There is a probable memory leak that occurs due to how ctypes handles char*/string objects and I'm still trying to figure out how to fix it.
class TextLogWidget:
    """
        This class handles displaying any text that comes through its queue-like structure.

        Attributes
        ----------
        master: Union[tk.Frame, ttk.Frame, ttk.PanedWindow]
            The Tkinter frame that will be the parent of this widget.
    """

    # TODO: Change the structure so that the text_log is a memoryview or something of the String structure?
    def __init__(self, master: Union[tk.Frame, ttk.Frame, ttk.Panedwindow]) -> None:
        """
            Initialize the TextLog widget.

            Parameters
            ----------
            master: Union[tk.Frame, ttk.Frame, ttk.Panedwindow]
                The Tkinter frame that will be the parent of this widget.

            Returns
            -------
            None.
        """

        self.master = master
        self._frame = ttk.LabelFrame(master=self.master, text="Text Log", relief="flat")
        self._frame.grid(row=0, column=0)
        self.text_display = tkst.ScrolledText(self._frame, width=170, height=15)
        self.text_display.pack(fill=tk.BOTH)
        # disable typing in the entry (might need to use menu to copy text...)
        self.buffer_index = 0
        self.text_display.bind("<Key>", lambda e: "break")
        self.text_log = ctypes_linear_algebra.String(0, 4096, 0, b"")
        self.update()

    def display(self, text: Union[str, bytearray]) -> None:
        """
            Display the text.

            Given some text, insert it into the end of the displayed text, along with a newline.

            NOTE: If a bytearray is provided, then the function will attempt to decode it as a UTF-8 string.

            Parameters
            ----------
            text: Union[str, bytearray]
                The text to display.

            Returns
            -------
            None.
        """

        if isinstance(text, bytearray):
            text = text.decode("utf-8")
        # If there is valid text to display, then insert it into the text display. Otherwise we ignore it.
        if text:
            self.text_display.insert(tk.END, text)
            self.text_display.insert(tk.END, "\n")
            self.text_display.see("end")
    # @profile
    def update(self) -> None:
        """
        Check the queue and display any new data.

        Parameters
        ----------
        None.

        Returns
        -------
        None.
        """

        buffer: bytearray = bytearray(memoryview(getattr(self.text_log, "buffer")))
        # TODO: Add something that clears the buffer once there is no more stuff left to read. This means that we cannot use the bytes data structure, as it is considered immutable.
        record, new_index = ctypes_linear_algebra.string_read_line(
            buffer, self.buffer_index
        )
        ### Increment the buffer index to where it needs to start on the next pass ###
        self.buffer_index += new_index
        ### Display the record ###
        self.display(record)
        self.master.after(40, self.update)


###############################################################################
#                                                                             #
#                                                                             #
#                   CTYPES_LINEAR_ALGEBRA 'INTERFACE LOGIC'                   #
#                                                                             #
#                                                                             #
###############################################################################


# @profile
def perform_matrix_row_reduction(
    matrix_input: MatrixInput,
    matrix_augment: MatrixInput,
    text_display_widget: TextLogWidget,
):
    for entry in matrix_input.entries:
        if not entry:
            text_display_widget.display(
                "Error: Not all entries in the input matrix are filled in."
            )
            return
    for entry in matrix_augment.entries:
        if not entry:
            text_display_widget.display(
                "Error: Not all entries in the matrix augment are filled in."
            )
            return
    if matrix_input.metadata.num_rows != matrix_augment.metadata.num_rows:
        text_display_widget.display(
            "Error: The input matrix and its augment do not share the same number of rows."
        )
        return

    ctypes_linear_algebra.perform_gauss_jordan_reduction(
        matrix_input.matrix_data,
        matrix_augment.matrix_data,
        ctypes.byref(text_display_widget.text_log),
        ctypes.byref(matrix_input.metadata),
        ctypes.byref(matrix_augment.metadata),
    )


# @profile
def perform_square_matrix_inversion(
    matrix_input: MatrixInput, text_display_widget: TextLogWidget
):
    if matrix_input.metadata.num_rows != matrix_input.metadata.num_cols:
        text_display_widget.display(
            "Error: The input matrix is not square, and thus cannot be inverted."
        )
    for entry in matrix_input.entries:
        if not entry:
            text_display_widget.display(
                "Error: Not all entries in the input matrix are filled in."
            )
            return

    matrix_values = np.array(
        [
            entry.old_value
            if entry.old_value != "" and entry.old_value != "-"
            else np.nan
            for entry in matrix_input.entries
        ]
    ).reshape(matrix_input.num_rows, matrix_input.num_cols)
    # Cast the matrix_data as a ctypes double pointer (i.e., double* in C)
    matrix_values_ptr = matrix_values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
    ctypes_linear_algebra.perform_square_matrix_inversion(
        matrix_values_ptr,
        ctypes.byref(matrix_input.metadata),
        ctypes.byref(text_display_widget.text_log), # Replace this with None such that the CDLL will default to STDOUT
    )
    del matrix_values_ptr
    del matrix_values

def trace_malloc_and_exit(*args):
    snapshot = tracemalloc.take_snapshot()
    top_stats = snapshot.statistics('lineno')

    print("[ Top 10 ]")
    for stat in top_stats[:10]:
        print(stat)
    exit()

### CONSTRUCT THE GUI ###
tracemalloc.start()
main_window = tk.Tk()
main_window.wm_title("Linear Algebra Calculator GUI")
matrix_frame = ttk.Frame(master=main_window)
matrix_input = MatrixInput(matrix_frame, "Input Matrix")
matrix_augment = MatrixInput(matrix_frame, "Matrix Augmentation", 3, 1)
matrix_input._frame.pack()
matrix_augment._frame.pack()
matrix_frame.grid(row=0, column=0)
matrix_output = TextLogWidget(main_window)
matrix_output._frame.grid(row=1, column=0)
button_panel = ttk.Labelframe(main_window, text="Matrix Buttons")
add_row_button = ttk.Button(
    button_panel, text="Add Row", command=lambda: matrix_input.add_row(),
)
remove_row_button = ttk.Button(
    button_panel, text="Remove Row", command=lambda: matrix_input.remove_row(),
)
add_col_button = ttk.Button(
    button_panel, text="Add Column", command=lambda: matrix_input.add_col(),
)

remove_col_button = ttk.Button(
    button_panel, text="Remove Column", command=lambda: matrix_input.remove_col(),
)
perform_reduction_button = ttk.Button(
    main_window,
    text="Solve Matrix",
    command=lambda: perform_matrix_row_reduction(
        matrix_input, matrix_augment, matrix_output
    ),
)
perform_reduction_button.grid(row=3, column=0)
perform_inversion_button = ttk.Button(
    main_window,
    text="Invert Matrix",
    command=lambda: perform_square_matrix_inversion(matrix_input, matrix_output),
)
perform_inversion_button.grid(row=4, column=0)
button_panel.grid(row=5, column=0)
add_row_button.pack()
remove_row_button.pack()
add_col_button.pack()
remove_col_button.pack()
gui_style = ttk.Style(main_window)
gui_style.theme_use("default")

### MENU BAR ###
menubar = tk.Menu(main_window)
file_menu = tk.Menu(menubar, tearoff=0)
# file_menu.add_command(label="Close", command=main_window.quit)
# file_menu.add_separator()
file_menu.add_command(label="Exit", command=main_window.quit)
# Add the file_menu to the main menu bar(s)
menubar.add_cascade(label="File", menu=file_menu)
# Set final keybindings and settings before running the mainloop
main_window.geometry("1200x600")
main_window.bind("<Control-w>", trace_malloc_and_exit)
# Add the menubar (otherwise it won't show up)
main_window.config(menu=menubar)
main_window.mainloop()
