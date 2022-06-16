import ctypes
import tkinter as tk
import tkinter.scrolledtext as tkst
from tkinter import ttk
from typing import List, Union

import numpy as np

import ctypes_linear_algebra

# matrix_data = np.zeros(shape=(num_rows.value, num_cols.value)).ctypes.data_as(ctypes.POINTER(ctypes.c_int16))

# TODO: This class is actually rather annoying as a user to deal with. Constantly updating numbers and having unexpected behavior is not worth it.
# TODO: Shift the validation step to be when the "solve matrix" button is pressed instead.
class NumberOnlyEntry(tk.Entry):
    def __init__(self, master=None, **kwargs):
        self.var = tk.StringVar()
        tk.Entry.__init__(self, master, textvariable=self.var, **kwargs)
        self.old_value = ""
        self.var.trace("w", self.check)
        self.get, self.set = self.var.get, self.var.set

    def check(self, *args):
        try:
            # We assume that negative numbers will have more to it
            if self.get() == "-":
                return
            new_value = float(self.get())
            self.old_value = new_value
        except ValueError:
            self.set(self.old_value)


# TODO: Add the ability to change the number of rows and columns
class MatrixInput:
    def __init__(
        self,
        parent: Union[tk.Frame, ttk.LabelFrame],
        label: str = "Matrix",
        num_rows: int = 3,
        num_cols: int = 3,
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
            augmented_matrix_rank=-1,
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

    def update_display(self):
        for entry in self.entries:
            entry.grid_forget()
        self.entries.clear()
        for row in range(self.num_rows):
            for col in range(self.num_cols):
                matrix_value_entry = NumberOnlyEntry(master=self._frame)
                matrix_value_entry.grid(row=row, column=col)
                self.entries.append(matrix_value_entry)

    def add_row(self):
        self.num_rows += 1
        self.update_display()

    def remove_row(self):
        if self.num_rows > 1:
            self.num_rows -= 1
            self.update_display()

    def add_col(self):
        self.num_cols += 1
        self.update_display()

    def remove_col(self):
        if self.num_cols > 1:
            self.num_cols -= 1
            self.update_display()

    def update(self) -> None:
        self.metadata.num_rows = self.num_rows
        self.metadata.num_cols = self.num_cols
        matrix_values = np.array(
            [
                entry.old_value
                if entry.old_value != "" and entry.old_value != "-"
                else np.nan
                for entry in self.entries
            ]
        ).reshape(self.num_rows, self.num_cols)
        self.matrix_data = matrix_values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        self.parent.after(100, self.update)


class TextLogWidget:
    """
    This class handles displaying any text that comes through its queue-like structure.

    Attributes
    ----------
    master: Union[tk.Frame, ttk.Frame, ttk.PanedWindow]
        The Tkinter frame that will be the parent of this widget.
    """

    def __init__(self, master: Union[tk.Frame, ttk.Frame, ttk.Panedwindow]) -> None:
        """
        Initialize the DropTextLog widget.

        Parameters
        ----------
        master: Union[tk.Frame, ttk.Frame, ttk.Panedwindow]
            The Tkinter frame that will be the parent of this widget.

        Returns
        -------
        None.
        """

        self.master = master
        # self._config = config
        self._frame = ttk.LabelFrame(
            master=self.master, text="Drop Analytics", relief="flat"
        )
        # self._frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        self._frame.grid(row=0, column=0)
        self.text_display = tkst.ScrolledText(
            self._frame, width=170, height=15
        )  # , font=self._config.application_font
        self.text_display.pack(fill=tk.BOTH)
        # disable typing in the entry (might need to use menu to copy text...)
        self.buffer_index = 0
        self.text_display.bind("<Key>", lambda e: "break")
        self.text_log = ctypes_linear_algebra.String(0, 4096, 0, b"")
        self.update()

    # BUG: This only displays the first instruction. Chances are we need to configure the start index more carefully.
    def display(self, text: Union[str, bytearray]) -> None:
        """
        Display the text.

        Given some text, insert it into the display widget.

        NOTE: The behavior of this function is the same with
        both regular strings and encoded strings.

        Parameters
        ----------
        text: Union[str, bytearray]
            The text to display.

        Returns
        -------
        None.
        """

        self.text_display.configure(state="normal")
        if isinstance(text, bytearray):
            text = text.decode("utf-8")
        if text:
            self.text_display.insert(tk.END, text)
            self.text_display.insert(tk.END, "\n")

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

        buffer: bytes = getattr(self.text_log, "buffer")
        # TODO: Add something that clears the buffer once there is no more stuff left to read.
        record, new_index = ctypes_linear_algebra.string_read_line(
            buffer, self.buffer_index
        )
        ### Increment the buffer index to where it needs to start on the next pass ###
        self.buffer_index += new_index
        ### Display the record ###
        self.display(record)
        self.master.after(100, self.update)


# TODO: Add another (read and copy only?) MatrixInput that just shows the completed row reduction (assuming one exists).
### CONSTRUCT THE GUI ###
main_window = tk.Tk()
main_window.wm_title("Linear Algebra Calculator GUI")
matrix_frame = ttk.Frame(master=main_window)
matrix_input = MatrixInput(matrix_frame, "Input Matrix", 3, 3)
matrix_augment = MatrixInput(matrix_frame, "Matrix Augmentation", 3, 1)
matrix_input._frame.pack()
matrix_augment._frame.pack()
matrix_frame.grid(row=0, column=0)
matrix_output = TextLogWidget(main_window)
matrix_output._frame.grid(row=1, column=0)
button_panel = ttk.Labelframe(main_window, text="Matrix Buttons")
button_panel.grid(row=4, column=0)
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
    command=lambda: ctypes_linear_algebra.perform_gauss_jordan_reduction(
        matrix_input.matrix_data,
        matrix_augment.matrix_data,
        ctypes.byref(matrix_output.text_log),
        ctypes.byref(matrix_input.metadata),
        ctypes.byref(matrix_augment.metadata),
    ),
)
perform_reduction_button.grid(row=3, column=0)
add_row_button.pack()
remove_row_button.pack()
add_col_button.pack()
remove_col_button.pack()
s = ttk.Style(main_window)
s.theme_use("default")

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
main_window.bind("<Control-w>", exit)
# Add the menubar (otherwise it won't show up)
main_window.config(menu=menubar)
main_window.mainloop()
