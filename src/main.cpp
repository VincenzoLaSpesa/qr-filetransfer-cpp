#include <restinio/all.hpp>
#include <fmt/printf.h>
#include <args.hxx>
#include <asio.hpp>
#include <iostream>
#include <stdexcept>
#include <qrencode.h>
#include "Util.h"
#include "Server.h"

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
    std::string file_path;
    //parse the commandline options
    args::ArgumentParser parser("qr-filentransfer-cpp", "");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> served_path(parser, "servedpath", "The path the server will listen to. A random one will be generated if omitted", {'s'});
    args::Group group(parser);
    args::Flag keep(group, "keep-alive", "keeps server alive, won't shut it down after transfer", {'k', "keep-alive"});
    args::Flag receive(group, "receive", "allows the client to send files", {'r', "receive"});
    args::Positional<std::string> filename(parser, "filename", "file to serve");
    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Completion &e) {
        std::cout << e.what();
        return -1;
    } catch (const args::Help &) {
        std::cout << parser;
        return -2;
    } catch (const args::ParseError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return -3;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    if (filename) {
        std::cout << filename.Get() << " will be served" << std::endl;
        file_path = filename.Get();
    } else {
        std::cerr << "A filename is needed \n";
    }
    auto addr = ChooseInterface();
    fmt::printf("Binding to -> %s\n", addr);
    std::string rand_path;
    if (served_path)
        rand_path = args::get(served_path);
    else
        rand_path = Util::RandomizePath(file_path);

    std::string path = fmt::sprintf("http://%s:%d/%s", addr, port, rand_path);
    fmt::printf("The served url is -> %s\n", path);
    printQr(path);
    Server s{addr, port, file_path, rand_path, keep, receive};
    printf("Server is ready\n");
    s.Wait();
    return 0;
}
