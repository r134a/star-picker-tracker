# Documentation
Documentation is provided as in-code Doxygen style comment.
To generate the documentation, change to the root folder
of this project (where this platformio.ini file is) and
issue the doxygen command. The output will be directed to
the html folder. If doxygen was installed using all default
options and without a path to it set, the proper command
from PowerShell is (mark the back-tick)
```
C:\Program` Files\doxygen\bin\doxygen
```

# Building
Building and uploading should be done with PlatformIO. By far the
easiest method is using the PlatformIO extension for VSCode. Use
the upload icon (right pointing arrow) in VSCode's status line.

An untested method is using the command line tool
```
cd [root of the project]
~\.platformio\penv\Scripts\platformio.exe run --target upload
```
