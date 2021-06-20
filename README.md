<h1 align="center">
  FITS file viewer POC
</h1>

Contains a proof of concept FITS file viewer implemented using the cftisio library and QT.

## Dependencies

You will need QT5 and the cfitsio library. On debian based linux systems:

`sudo apt install qt5-default` for QT

`sudo apt install libcfitsio-dev` for cfitsio

You will also need some FITS files to test with.

## Building

Create a directory called "build" (or whatever you like, really, but build is already git ignored) in the root directory of the project.

Change to the build directory and execute `qmake ..`

You will now have a Makefile. Run `make`

You should now have an executable, unless you are missing a dependency somewhere. To run:

`./qtfits-poc <path-to-fits-file>`
