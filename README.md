antpatt
=======

Antenna pattern plotting and analysis software.

![Screenshot](/antpatt.png?raw=true)

Copyright (C) 2017-2024  Konrad Kosmatka

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

For Windows builds (binaries), see the [Releases](https://github.com/kkonradpl/antpatt/releases) page. To enable dark theme on Windows, run `antpatt.exe` with `-d` command line parameter. 

# Installing

After a successful build, just use:
```sh
$ sudo make install
```
in the `build` directory. This will install both the executable file `antpatt` and icons.

# Supported data formats

- Radiomobile – `ANT`
- MMANA-GAL – `CSV`
- Planet – `MSI`
- XDR-GTK (legacy) – `XDRP`

The whole project can be saved as `.antp.gz` file (compressed `.antp`) which is simply a JSON file with all settings included and data samples embedded. See `examples` directory.

# Data from MMANA-GAL

MMANA-GAL can export CSV files that antpatt accepts. Use the following settings in MMANA-GAL: File → Table of Angle/Gain (*.csv) dialog to export the CSV:

![Screenshot](/examples/mmanagal-export.png?raw=true)

# Interactive console mode

Interactive console mode (`-i` command line option) can be used for data streaming from another application for real-time antenna radiation pattern plotting. Commands consist of a single word and are case insensitive:

- `START` – create new measurement
- `STOP` – close current measurement
- `PUSH <float>` – add signal level sample
- `NAME <string>` – set plot name
- `FREQ <int 0 … 99 999 999>` – set plot frequency [kHz]
- `COLOR <string>` – set plot color (#XXXXXX)
- `AVG <int 0 … 10>` – set plot moving-average
- `FILL <int 0 … 1>` – set plot fill
- `REV <int 0 … 1>` – set plot reverse mode

Antpatt will send the following responses:

- `READY` – after application startup
- `BYE` – before application exit
- `OK` – after a successful command
- `ERROR` – after an incorrect command