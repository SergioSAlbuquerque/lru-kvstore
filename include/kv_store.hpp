#pragma once

#include "lru_cache.hpp"

#include <filesystem>
#include <optional>
#include <string>

// Key-value store em memória com capacidade limitada (política LRU)
// e persistência simples em arquivo texto.
class KvStore {
public:
    explicit KvStore(std::size_t capacity);

    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);

    std::size_t size() const noexcept { return cache_.size(); }
    std::size_t hits() const noexcept { return cache_.hits(); }
    std::size_t misses() const noexcept { return cache_.misses(); }
    std::size_t evictions() const noexcept { return evictions_; }

    // Formato do arquivo: uma linha por par -> <tam_chave> <tam_valor> <chave><valor>
    // Usar tamanhos evita problemas com espaços ou quebras de linha nos dados.
    void save(const std::filesystem::path& file) const;
    void load(const std::filesystem::path& file);

private:
    LruCache<std::string, std::string> cache_;
    std::size_t evictions_ = 0;
};
