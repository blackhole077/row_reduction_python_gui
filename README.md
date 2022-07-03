# Linear Algebra Solver Using ctypes and Tkinter
A GUI using Python `Tkinter`, `ctypes` and C to perform the Gauss-Jordan row reduction algorithm. Includes a step-by-step process so that users can double-check their work more carefully.

**NOTE: Code does not appear to work in Python 3.10!**

---

# Purpose
Create an offline tool that utilizes custom DLL/SO files in C to combine the GUI capabilities of Python and the fast computation speed of C. Should also provide a non-trivial example of using the `ctypes` library to integrate C functionality into Python.

# Motivation
Linear algebra is a fundamental subject if one wishes to deepen their understanding of topics such as deep learning. Per the adage, 'Practice Makes Perfect', this tool was created to help me practice some of the basics of linear algebra by allowing me to verify my work.

In addition, this project is meant to serve as an approachable, yet non-trivial, application of the `ctypes` library. While other libraries are present for making use of C code within Python, these libraries did not feel as conceptually straightforward as `ctypes`. However, `ctypes` also suffers from a lack of recent examples and current documentation. As such, this project will hopefully ameliorate both of these pain points for those interested in the library.

## Why ctypes?

There are many different libraries that exist for joining C and Python together. As mentioned earlier, `ctypes` doesn't even have a very robust set of examples to accompany the documentation. So why use `ctypes`?

The first reason is simple: This is the first one I figured out. Of course, I tried to understand and utilize libraries like `Cython` and `CFFI`, but they simply did not click with me as quickly as `ctypes` did. 

Second, I realized that presenting a use case in the form of this project may be helpful for those trying to understand the library better, especially if other users don't need the same amount of features present in other, more sophisticated, libraries like `Cython`. Finally, the `ctypes` library is built into Python itself, meaning there's no additional setup required to get it running. For something as simple as this GUI, adding additional steps feels far more cumbersome for little gain.

However, it should be noted that **`ctypes` is not without issue, and some of these issues are discussed below**.

---

# Development setup
Below are instructions and some discussion of setting up the repository for additional development or experimentation.

## Required Programs
Below are a list of auxiliary programs that are required or recommended for running this code, along with an explanation of why said programs are needed.
- Visual Studio Community (2019): In order to generate the DLL file, or compile the C code into an executable, you will need the `vcvarsall.bat` file, which is provided when installing Visual Studio.
- Anaconda (optional): A Python package and virtual environment manager. The `python39.dll` (if you are using Python 3.9.x, this is what the file will look like) file is important for creating DLL files.

## Required Libraries
Below are a list of libraries needed to run the Linear Algebra GUI. Libraries that come with any installation of Python will be marked with an asterisk(*):
 - Numpy
 - Tkinter(*)
 - ctypes(*)
 - sys(*)
 - os(*)

## Recommended Libraries
Below are a list of libraries that will assist in developing the Linear Algebra GUI further:
- memory_profiler (or Valgrind if you are using Linux)

# Known Issues
## Memory Leakage
It appears that, using the `memory_profiler` library shows that there is some sort of memory leak. It is likely that this is **due to how `ctypes` handles converting char\***, meaning that in order to solve the issue, it might require re-designing the String.c file. This [link discussing the issues with ctypes](https://behaviour.space/posts/2021-03-28-ctypes-weird-and-inconvenient-typing.html) provides some more detail to support this hypothesis.