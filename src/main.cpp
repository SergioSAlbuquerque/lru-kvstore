// REPL (linha de comando interativa) para o KvStore.
#include "kv_store.hpp"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

namespace {

void print_help() {
    std::cout << "Comandos:\n"
                 "  SET <chave> <valor...>   insere/atualiza\n"
                 "  GET <chave>              busca\n"
                 "  DEL <chave>              remove\n"
                 "  STATS                    tamanho, hits, misses, evictions\n"
                 "  SAVE <arquivo>           salva em disco\n"
                 "  LOAD <arquivo>           carrega do disco\n"
                 "  HELP | EXIT\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::size_t capacity = 100;
    if (argc > 1) capacity = std::stoul(argv[1]);

    KvStore store(capacity);
    std::cout << "lru-kvstore (capacidade " << capacity << "). Digite HELP.\n";

    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd, key;
        iss >> cmd;
        for (auto& c : cmd) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        try {
            if (cmd.empty()) {
                continue;
            } else if (cmd == "SET" && iss >> key) {
                std::string value;
                std::getline(iss >> std::ws, value);
                store.set(key, value);
                std::cout << "OK\n";
            } else if (cmd == "GET" && iss >> key) {
                auto v = store.get(key);
                std::cout << (v ? *v : "(nil)") << '\n';
            } else if (cmd == "DEL" && iss >> key) {
                std::cout << (store.del(key) ? "1" : "0") << '\n';
            } else if (cmd == "STATS") {
                std::cout << "size=" << store.size() << " hits=" << store.hits()
                          << " misses=" << store.misses()
                          << " evictions=" << store.evictions() << '\n';
            } else if (cmd == "SAVE" && iss >> key) {
                store.save(key);
                std::cout << "OK\n";
            } else if (cmd == "LOAD" && iss >> key) {
                store.load(key);
                std::cout << "OK\n";
            } else if (cmd == "HELP") {
                print_help();
            } else if (cmd == "EXIT" || cmd == "QUIT") {
                break;
            } else {
                std::cout << "ERRO: comando invalido (HELP)\n";
            }
        } catch (const std::exception& e) {
            std::cout << "ERRO: " << e.what() << '\n';
        }
    }
    return 0;
}
