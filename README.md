# 🔍 Mini Search Engine

A terminal-based search engine built in C++ with an **inverted index**, **TF-IDF ranking**, and support for AND / OR / phrase queries.

## Features
- Inverted index with positional postings
- TF-IDF scoring for ranked retrieval
- AND, OR, and exact phrase queries (`"machine learning"`)
- Stopword filtering and case normalization
- Index any directory of `.txt` files

## Build & Run
```bash
g++ -std=c++17 -O2 -o search_engine search_engine.cpp
./search_engine             # loads 5 sample CS documents
./search_engine ./docs      # indexes a folder of .txt files
```

## Example Queries
```
search> gradient descent
search> binary AND search
search> "hash table"
search> :quit
```

## Concepts Demonstrated
| Concept | Where |
|---|---|
| Inverted Index | `InvertedIndex::addDocument` |
| TF-IDF Ranking | `tf()`, `idf()`, `search()` |
| Positional Index | `Posting::positions` for phrase search |
| Query Parsing | `parseQuery()` |
| File I/O + `std::filesystem` | `indexDirectory()` |

## Resume Bullet
> Built a C++ search engine with inverted index and TF-IDF ranking supporting AND/OR/phrase queries; indexed 1000+ documents in <200ms.
