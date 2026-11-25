#include "kv_store.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>

KvStore::KvStore(std::size_t capacity) : cache_(capacity) {}

void KvStore::set(const std::string& key, const std::string& value) {
    if (cache_.put(key, value)) ++evictions_;
}

std::optional<std::string> KvStore::get(const std::string& key) {
    return cache_.get(key);
}

bool KvStore::del(const std::string& key) { return cache_.erase(key); }

void KvStore::save(const std::filesystem::path& file) const {
    std::ofstream out(file, std::ios::binary);  // RAII: fecha sozinho no fim do escopo
    if (!out) throw std::runtime_error("could not open " + file.string());

    // Salva do menos recente para o mais recente, para que o load
    // reconstrua a mesma ordem de uso.
    std::vector<std::pair<std::string, std::string>> pairs;
    cache_.for_each([&](const auto& k, const auto& v) { pairs.emplace_back(k, v); });
    for (auto it = pairs.rbegin(); it != pairs.rend(); ++it) {
        out << it->first.size() << ' ' << it->second.size() << ' '
            << it->first << it->second << '\n';
    }
}

void KvStore::load(const std::filesystem::path& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) throw std::runtime_error("could not open " + file.string());

    std::size_t klen = 0, vlen = 0;
    while (in >> klen >> vlen) {
        in.get();  // consome o espaço separador
        std::string key(klen, '\0'), value(vlen, '\0');
        if (!in.read(key.data(), static_cast<std::streamsize>(klen)) ||
            !in.read(value.data(), static_cast<std::streamsize>(vlen))) {
            throw std::runtime_error("corrupted file: " + file.string());
        }
        in.get();  // consome o '\n'
        set(key, value);
    }
}
