// Testes sem dependências externas: cada CHECK que falha é contado
// e o programa retorna código != 0 (o CTest entende como falha).
#include "kv_store.hpp"
#include "lru_cache.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

static int failures = 0;
#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << __FILE__ << ':' << __LINE__ << " FALHOU: " #cond "\n"; \
            ++failures;                                                    \
        }                                                                  \
    } while (0)

static void test_basic_get_put() {
    LruCache<int, std::string> c(2);
    c.put(1, "um");
    CHECK(c.get(1) == "um");
    CHECK(!c.get(2).has_value());
    CHECK(c.hits() == 1 && c.misses() == 1);
}

static void test_eviction_order() {
    LruCache<int, int> c(2);
    c.put(1, 10);
    c.put(2, 20);
    c.get(1);                 // 1 vira o mais recente
    auto ev = c.put(3, 30);   // deve despejar o 2
    CHECK(ev == 2);
    CHECK(c.contains(1) && c.contains(3) && !c.contains(2));
}

static void test_update_does_not_evict() {
    LruCache<int, int> c(2);
    c.put(1, 10);
    c.put(2, 20);
    CHECK(!c.put(1, 11).has_value());
    CHECK(c.size() == 2);
    CHECK(c.get(1) == 11);
}

static void test_erase() {
    LruCache<std::string, int> c(3);
    c.put("a", 1);
    CHECK(c.erase("a"));
    CHECK(!c.erase("a"));
    CHECK(c.size() == 0);
}

static void test_zero_capacity_throws() {
    bool threw = false;
    try { LruCache<int, int> c(0); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
}

static void test_store_save_load_roundtrip() {
    auto file = std::filesystem::temp_directory_path() / "kv_test.db";
    {
        KvStore s(10);
        s.set("nome", "Sergio");
        s.set("frase com espaco", "valor\ncom quebra de linha");
        s.save(file);
    }
    KvStore t(10);
    t.load(file);
    CHECK(t.get("nome") == "Sergio");
    CHECK(t.get("frase com espaco") == "valor\ncom quebra de linha");
    std::filesystem::remove(file);
}

static void test_store_counts_evictions() {
    KvStore s(1);
    s.set("a", "1");
    s.set("b", "2");
    CHECK(s.evictions() == 1);
    CHECK(!s.get("a").has_value());
}

int main() {
    test_basic_get_put();
    test_eviction_order();
    test_update_does_not_evict();
    test_erase();
    test_zero_capacity_throws();
    test_store_save_load_roundtrip();
    test_store_counts_evictions();

    if (failures == 0) std::cout << "Todos os testes passaram.\n";
    return failures == 0 ? 0 : 1;
}
