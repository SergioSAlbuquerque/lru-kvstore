#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>

// Cache LRU (Least Recently Used) genérica.
//
// Ideia: uma lista duplamente encadeada guarda a ordem de uso
// (frente = mais recente, fundo = menos recente) e um hash map
// aponta cada chave para o seu nó na lista.
// Assim get/put/erase são todos O(1) em média.
template <typename Key, typename Value>
class LruCache {
public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("capacity must be > 0");
        }
    }

    // Retorna o valor (se existir) e marca a chave como usada agora.
    std::optional<Value> get(const Key& key) {
        auto it = index_.find(key);
        if (it == index_.end()) {
            ++misses_;
            return std::nullopt;
        }
        ++hits_;
        // splice move o nó para a frente sem copiar nem realocar.
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    // Insere ou atualiza. Se a cache estiver cheia, remove o menos usado.
    // Retorna a chave despejada (eviction), se houve.
    std::optional<Key> put(const Key& key, Value value) {
        auto it = index_.find(key);
        if (it != index_.end()) {
            it->second->second = std::move(value);
            items_.splice(items_.begin(), items_, it->second);
            return std::nullopt;
        }

        std::optional<Key> evicted;
        if (items_.size() == capacity_) {
            auto& lru = items_.back();
            evicted = lru.first;
            index_.erase(lru.first);
            items_.pop_back();
        }

        items_.emplace_front(key, std::move(value));
        index_[key] = items_.begin();
        return evicted;
    }

    bool erase(const Key& key) {
        auto it = index_.find(key);
        if (it == index_.end()) return false;
        items_.erase(it->second);
        index_.erase(it);
        return true;
    }

    bool contains(const Key& key) const { return index_.count(key) > 0; }
    std::size_t size() const noexcept { return items_.size(); }
    std::size_t capacity() const noexcept { return capacity_; }
    std::size_t hits() const noexcept { return hits_; }
    std::size_t misses() const noexcept { return misses_; }

    // Percorre do mais recente para o menos recente.
    template <typename Fn>
    void for_each(Fn&& fn) const {
        for (const auto& [k, v] : items_) fn(k, v);
    }

private:
    using Node = std::pair<Key, Value>;
    using ListIt = typename std::list<Node>::iterator;

    std::size_t capacity_;
    std::list<Node> items_;
    std::unordered_map<Key, ListIt> index_;
    std::size_t hits_ = 0;
    std::size_t misses_ = 0;
};
