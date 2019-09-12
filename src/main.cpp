#include <restinio/all.hpp>
#include <fmt/printf.h>
#include <args.hxx>
#include <asio.hpp>
#include <iostream>
#include <stdexcept>
#include <qrencode.h>
#include "util.h"
#include "server.h"

//! List all the available ip addresses, prompt the user to select one via stdin, return the selected address
std::string ChooseInterface() {
    auto interfaces = Util::ListInterfaces();
    std::sort(interfaces.begin(), interfaces.end());
    unsigned int n = 0;
    std::cout << "Choose a network address by its number \n";
    for (const auto i : interfaces) {
        fmt::printf("%d -> %s \n", n, i.to_string());
        n++;
    }
    std::cout << "\nInput number: ";

    std::cin >> n;
    if (n >= interfaces.size()) {
        std::cerr << "invalid index" << std::endl;
        throw std::invalid_argument("invalid index");
        return std::string{};
    }

    return interfaces[n].address;
}

//! Generate a qrcode from a string and print it to stdout
void printQr(const std::string &url) {
    QRcode *qrcode = QRcode_encodeString(url.c_str(), 1, QR_ECLEVEL_M, QR_MODE_8, 1);
    unsigned char *data = qrcode->data;
    const size_t size = qrcode->width;
    const size_t line_size = size * 2 + 4;
    std::string line_buffer(line_size, (char)219);

    std::cout << "\n " << line_buffer << '\n';
    for (int r = 0; r < size; r++) {
        int cc = 2;
        for (int c = 0; c < size; c++) {
            const char x = (*data & 1) ? ' ' : 219;
            line_buffer[cc++] = x;
            line_buffer[cc++] = x;
            data++;
		}
        std::cout << ' ' << line_buffer << '\n';
    }
    line_buffer.assign(line_size, (char)219);
    std::cout << ' ' << line_buffer << std::endl;
    QRcode_free(qrcode);
}

int main(int argc, char **argv) {
    const unsigned short port = 8080;
    std::string file_path = "D:\\tmp\\dummy.jpg";
    //parse the commandline options
    args::ArgumentParser parser("qr-filentransfer_cpp", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Group group(parser);
    args::Flag keep(group, "keep-alive", "keeps server alive, won't shut it down after transfer", {'k', "keep-alive"});
    args::Positional<std::string> filename(parser, "filename", "file to serve");
    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Completion &e) {
        std::cout << e.what();
        return 0;
    } catch (const args::Help &) {
        std::cout << parser;
        return 0;
    } catch (const args::ParseError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    if (keep) {
        std::cout << "bar" << std::endl;
    }
    if (filename) {
        std::cout << filename.Get() << " will be served" << std::endl;
        file_path = filename.Get();
    }
    //bind the server
    auto addr = ChooseInterface();
    fmt::printf("Binding to -> %s\n", addr);
    std::string rand_path = Util::RandomizePath(file_path);
    std::string path = fmt::sprintf("http://%s:%d/%s", addr, port, rand_path);
    fmt::printf("file in -> %s\n", path);
    printQr(path);
    Server s{addr, port, file_path, rand_path, keep};
    printf("Server is ready\n");
    s.Wait();
    //make the qr

    return 0;
}
