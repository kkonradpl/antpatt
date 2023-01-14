antpatt
=======

Antenna pattern plotting and analysis software.

![Screenshot](/antpatt.png?raw=true)

Copyright (C) 2017-2023  Konrad Kosmatka

https://fmdx.pl/antpatt/

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# Build
In order to build antpatt you will need:
- CMake
- C compiler

You will also need several dependencies:
- GTK+ 3 & dependencies
- GSL
- JSON-C
- zlib

Once you have all the necessary dependencies, you can use scripts available in the `build` directory.

# Installing
After a successful build, just use:
```sh
$ sudo make install
```
in the `build` directory. This will install both the executable file `antpatt` and icons.

