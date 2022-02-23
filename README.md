# qr-filetransfer-cpp

Transfer files over Wi-Fi from your computer to a mobile device by scanning a QR code.

This repository is inspired by [Claudio d'Angelis's qr-filetransfer](https://github.com/claudiodangelis/qr-filetransfer).

## State of the project:

This is absolutely a work in progress.

### Completed features
-   Serve a single file and close ( desktop to mobile)
-   Serve a single file and keep the server open ( desktop to mobile)
-   Upload a file ( from mobile to desktop)
-   Verbose option
-   Specify a custom port   
-   Interface names

### TODO 
-   Store/load settings


## Install

The project is buildable with CMake from both linux and windows.
The project depends on:

- cpp-httplib (web server) https://github.com/yhirose/cpp-httplib
- libqrencode (qr code generation) https://github.com/fukuchi/libqrencode
- fmt (string formatting) https://github.com/fmtlib/fmt
- taywee::args (command line arguments parsing) https://github.com/Taywee/args

You can install the dependencies in different ways, the suggested way is using Conan, but vcpkg should work as well.

For conan just take a look at `build.sh` for posix systems or `build.bat` for windows.

### But I'm lazy! give me the binary!
    TODO

## Usage

For transfering a single file from the desktop:

    qr-filetransfer /path/to/file.txt

    TODO
## How does it work?

`qr-filetransfer_cpp` binds a web server to the address you choose on a random port and creates a handler for it. The default handler serves the content and exits the program when the transfer is complete.

The tool prints a QR code that encodes the text:

    http://{address}:{port}/{random_path}

Most QR apps can detect URLs in decoded text and act accordingly (i.e. open the decoded URL with the default browser), so when the QR code is scanned the content will begin downloading by the mobile browser.

## Why?

From time to time i try to build someting in C++ from scratch for experimenting with the newest features of the language, newer libraries, newer build systems.
I usually end up thinking that it is till a mess, but this time... ok, it's still a mess but it was fun, and cpp-httplib is cool.

## License

MIT. See [LICENSE](LICENSE).