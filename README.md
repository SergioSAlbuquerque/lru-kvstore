# lru-kvstore

Key-value store em memória escrito em **C++17**, com capacidade limitada e política de remoção **LRU (Least Recently Used)**, persistência em disco e uma interface de linha de comando no estilo do Redis.

```
$ ./build/kvcli 2
> SET user Sergio
OK
> SET lang C++
OK
> GET user
Sergio
> SET curso Engenharia     # cache cheia -> remove "lang" (menos usada)
OK
> GET lang
(nil)
> STATS
size=2 hits=1 misses=1 evictions=1
> SAVE dados.db
OK
```

## Como funciona

A cache combina duas estruturas da STL:

| Estrutura | Papel |
|---|---|
| `std::list<pair<Key, Value>>` | guarda a **ordem de uso**: frente = mais recente, fundo = próximo a ser removido |
| `std::unordered_map<Key, list::iterator>` | acha o nó de qualquer chave em **O(1)** |

Quando uma chave é lida ou escrita, o nó vai para a frente com `list::splice` (só religa ponteiros, sem copiar dados). Quando a cache enche, sai o elemento do fundo. Resultado: `get`, `put` e `erase` em **O(1)** médio.

## Conceitos de C++ usados

- **Templates**: `LruCache<Key, Value>` funciona com qualquer tipo de chave hasheável
- **`std::optional`** para representar "chave não encontrada" sem ponteiro nulo nem valor mágico
- **Move semantics** (`std::move`) para evitar cópias de valores grandes
- **RAII**: arquivos (`std::ofstream`/`ifstream`) fecham sozinhos ao sair do escopo
- **Exceções** para erros de I/O e argumentos inválidos
- **`std::filesystem`**, structured bindings, lambdas genéricas
- Formato de arquivo com prefixo de tamanho, então chaves e valores podem ter espaços e quebras de linha

## Compilar e rodar

Requer CMake ≥ 3.16 e um compilador com C++17 (GCC, Clang ou MSVC).

```bash
cmake -S . -B build
cmake --build build
./build/kvcli 100          # capacidade = 100 chaves
ctest --test-dir build     # roda os testes
```

## Estrutura

```
include/lru_cache.hpp   cache LRU genérica (header-only)
include/kv_store.hpp    store + persistência
src/kv_store.cpp
src/main.cpp            REPL
tests/test_main.cpp     testes unitários
```

## Possíveis melhorias

- Expiração por tempo (TTL) nas chaves
- Versão thread-safe com `std::shared_mutex`
- Servidor TCP para aceitar comandos pela rede
