# row_reduction_python_gui
A GUI using Python Tkinter, ctypes and C to perform the Gauss-Jordan row reduction algorithm. Includes a step-by-step process so that users can double-check their work more carefully.


**NOTE: Code does not appear to work in Python 3.10!**
---

# Purpose
Create an offline tool that utilises custom DLL/SO files in C to combine the GUI capabilities of Python and the fast computation speed of C.

# Motivation
Linear algebra is a fundamental subject if one wishes to deepen their understanding of topics such as deep learning. Per the adage, 'Practice Makes Perfect', this tool was created to help me practice some of the basics of linear algebra by allowing me to verify my work.

In addition, this project is meant to serve as an approchable, yet non-trivial, application of the ctypes library. While other libraries, such as cython, are present for making use of C code within Python, these libraries did not feel as conceptually straightforward as ctypes. However, ctypes also suffers from a lack of recent examples and current documentation. As such, this project will hopefully ameliorate both of these pain points for those interested in the library.

