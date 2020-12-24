# CloudCompare Python Plugin

Early step attempt at allowing to use Python to automate some stuff in CloudCompare.

# Dependencies

 - Python
 - pybind11

# Building & Installing

## Windows

1) Clone this project in CloudCompare/plugins/private


2) Create a Virtual env or a conda env

    ```shell script
    # Python's venv
    python -m venv --system-site-packages .\pyccvenv 
   
    # Conda environment
    conda create -n pyccenv
    ```

3) Activate the environment

    ```shell script
    # Python's venv (PowerShell)
   cd .\pyccenv
   .\Scripts\Activate.ps1
    
    # Conda environment
    conda create -n pyccenv
    ```
   
4) Install dependencies

    ```shell script
    # Python's venv (PowerShell)
   pip install pybind11
    
    # Conda environment
   conda install pybind11
    ```
   
5) Start cmake or your IDE from within the environment

    The option to build the plugin is 
    ```shell script
    -DPLUGIN_PYTHON=ON
    ```
    In the cmake Invocation you may also have to use:
    ```shell script
    -DPYTHON_INCLUDE_DIR=$(python-c"from distutils.sysconfig import get_python_inc; print(get_python_inc())")
    -DPYTHON_LIBRARY=$(python-c"import distutils.sysconfig as sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
    ```
5) Build & Install

    During the installation step, the virtual environment will be copied to the installation folder
    so launching CC from the install folder should just work.
    
    Use `-DPLUGIN_PYTHON_COPY_ENV=OFF` after the plugin has been installed once to gain time.

## Linux

1) Install additional dependencies (package name for debian/ubuntu)

    `libpython3-dev qtwebengine5-dev`
    if you are using Ubuntu 20.04 or later you can install pybind via the  `pybind11-dev` package
    on older version you have to compile and install pybind11 yourself as the version in the repo
    are too old

2) Run cmake with `-DPLUGIN_PYTHON=ON`
3) Build
4) Install

# Running

The plugin exposes a Python REPL and a mini code editor to run Python Code.

The Api available from Python is still limited (especially for creating / deleting entities) and
tools available from the gui are not (yet?) callable from python scripts. 


# Running tests

Tests works by running CloudCompare in command line mode.
To run them, you need pytest, and you need to provide to pytest the path the where CloudCompare is installed

```shell
# in CloudCompare/plugins/private/CloudCompare-PythonPlugin:

pytest --cloudcompare_exe "C:\CMakeBuilds\CloudCompare\Release\install\CloudCompare\CloudCompare.exe" tests
```

# Building the documentation

Building the documentation also works by running CloudCompare in command line mode.
To build the documentation:

``` powershell
cd docs
.\make-docs.ps1 "C:\CMakeBuilds\CloudCompare\Release\install\CloudCompare\CloudCompare.exe"
```

Which by default will build the docs in HTML in docs/_build. Open docs/_build/index.html to view it.
(The output dir can be changed using the `-OutputDir` option)


## notes
editor icon base on https://www.flaticon.com/free-icon/blank-page_18530

python icon from https://python.org