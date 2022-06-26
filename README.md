# row_reduction_python_gui
A GUI using Python `Tkinter`, `ctypes` and C to perform the Gauss-Jordan row reduction algorithm. Includes a step-by-step process so that users can double-check their work more carefully.


**NOTE: Code does not appear to work in Python 3.10!**
---

# Purpose
Create an offline tool that utilizes custom DLL/SO files in C to combine the GUI capabilities of Python and the fast computation speed of C.

# Motivation
Linear algebra is a fundamental subject if one wishes to deepen their understanding of topics such as deep learning. Per the adage, 'Practice Makes Perfect', this tool was created to help me practice some of the basics of linear algebra by allowing me to verify my work.

In addition, this project is meant to serve as an approachable, yet non-trivial, application of the `ctypes` library. While other libraries are present for making use of C code within Python, these libraries did not feel as conceptually straightforward as `ctypes`. However, `ctypes` also suffers from a lack of recent examples and current documentation. As such, this project will hopefully ameliorate both of these pain points for those interested in the library.

## Why ctypes?

There are many different libraries that exist for joining C and Python together. As mentioned earlier, `ctypes` doesn't even have a very robust set of examples to accompany the documentation. So why use `ctypes`?

The first reason is simple: This is the first one I figured out. Of course, I tried to understand and utilize libraries like `Cython` and `CFFI`, but they simply did not click with me as quickly as `ctypes` did. Second, I realized that presenting a use case in the form of this project may be helpful for those trying to understand the library better, especially if other users don't need the same amount of features present in other, more sophisticated, libraries like `Cython`. Finally, the `ctypes` library is built into Python itself, meaning there's no additional setup required to get it running. For something as simple as this GUI, adding additional steps feels far more cumbersome for little gain.


# Required Libraries
Below are a list of libraries needed to run the Linear Algebra GUI. Libraries that come with any installation of Python will be marked with an asterisk(*):
 - Numpy
 - Tkinter(*)
 - ctypes(*)
 - sys(*)
 - os(*)