#include <iostream>
#include <string>

// Declared in server.cpp
int run_server(int port, const std::string& web_root);

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string web_root = "./web";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--web" && i + 1 < argc) {
            web_root = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: chronocarve [options]\n"
                      << "Options:\n"
                      << "  --port <number>     Port to listen on (default: 8080)\n"
                      << "  --web <dir>         Path to web assets directory (default: ./web)\n"
                      << "  --help, -h          Show this help message\n";
            return 0;
        }
    }

    std::cout << "Starting ChronoCarve Content-Aware Image Reframing Engine..." << std::endl;
    return run_server(port, web_root);
}
