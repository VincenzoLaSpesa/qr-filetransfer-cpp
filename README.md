# qr-filetransfer-cpp

Transfer files over Wi-Fi from your computer to a mobile device by scanning a QR code.

This repository is inspired by [Claudio d'Angelis's qr-filetransfer](https://github.com/claudiodangelis/qr-filetransfer), but also adds a simple gui and it can server entire folders without zipping them to a single file.

## State of the project:

This is absolutely a work in progress.

### Completed features
-   Serve a single file and close ( desktop to mobile)
-   Serve a single file and keep the server open ( desktop to mobile)
-   Upload a file ( from mobile to desktop)
-   Verbose option
-   Specify a custom port   
-   Interface names ( on windows)

### TODO ( in order of importance)
-   Serve an entire folder (recursively)
-   Interface names ( on posix)
-   Better handling for "post" requests
-   Full static linking
-   Create a GUI with (probably) FLTK
-   Store/load settings


## Install

The project is buildable with CMake from both linux and windows.
The project depends on:

- restinio (web server)
- libqrencode (qr code generation)
- asio (networking abstraction layer)
- fmt (string interpolation)
- taywee::args (command line arguments parsing)

You can install the dependencies in different ways, the suggested one is vcpkg.
With vcpkg it's just:

    ./vcpkg install args asio fmt restinio --triplet x64-windows

Remember to specify the correct system with the correct architecture. CMake is very picky even for headers only libraries.

Once you have cmake and the dependencies installed you can generate your solution with:

    cmake .. -DCMAKE_PREFIX_PATH=%ABS_PATH% -G "Visual Studio 15 2017" -Ax64 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=path_to_vcpkg_toolchain
    
    cmake --build . --config Debug 

For visual studio or:

    TODO

For gnu make.

(With a recent version of Visual Studio you can even try to import the CMake and let visual studio figure out how to make its solution. I've never tried)

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

## License

MIT. See [LICENSE](LICENSE).