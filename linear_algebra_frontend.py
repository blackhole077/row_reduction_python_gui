import ctypes
import tkinter as tk
import tkinter.scrolledtext as tkst
from queue import Empty, Queue
from tkinter import ttk
from typing import List, Union

import numpy as np

import ctypes_linear_algebra

# matrix_data = np.zeros(shape=(num_rows.value, num_cols.value)).ctypes.data_as(ctypes.POINTER(ctypes.c_int16))


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
        # self._frame.pack(fill=tk.X, expand=True)
        for row in range(self.num_rows):
            for col in range(self.num_cols):
                matrix_value_entry = NumberOnlyEntry(master=self._frame)
                matrix_value_entry.grid(row=row, column=col)
                self.entries.append(matrix_value_entry)
        self.update()
    
    def update(self) -> None:
        matrix_values = np.array(
            [
                entry.old_value if entry.old_value != "" and entry.old_value != "-" else np.nan
                for entry in self.entries
            ]
        ).reshape(self.num_rows, self.num_cols)
        self.matrix_data = matrix_values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        self.parent.after(100, self.update)

class DropTextLogWidget:
    """
    This class handles displaying the compiled data that
    normally comes from the DataCompiler.

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
        self.text_display.pack(fill=tk.BOTH, expand=True)
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

        # print(ctypes_linear_algebra.get_dict(self.text_log))
        buffer: bytes = getattr(self.text_log, "buffer")
        record, self.buffer_index = ctypes_linear_algebra.string_read_line(
            buffer, self.buffer_index
        )
        # The octothorpe is used to denote data from the DataCompiler
        self.display(record)
        self.master.after(1000, self.update)


### CONSTRUCT THE GUI ###
main_window = tk.Tk()
main_window.wm_title("Linear Algebra Calculator GUI")
matrix_input = MatrixInput(main_window, "Input Matrix (Augmented)", 3, 4)
matrix_input._frame.grid(row=0, column=0)
matrix_output = DropTextLogWidget(main_window)
matrix_output._frame.grid(row=1, column=0)
matrix_metadata = ctypes_linear_algebra.MatrixMetadata()
perform_reduction_button = ttk.Button(main_window, command=lambda: ctypes_linear_algebra.perform_gauss_jordan_reduction(matrix_input.matrix_data, matrix_input.num_rows, matrix_input.num_cols, ctypes.byref(matrix_output.text_log), ctypes.byref(matrix_metadata)))
perform_reduction_button.grid(row=3, column=0)
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