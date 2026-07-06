#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────
//  Text Utilities
// ─────────────────────────────────────────────
std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string stripPunct(const std::string& s) {
    std::string result;
    for (char c : s)
        if (std::isalnum(c) || c == '\'') result += c;
    return result;
}

// Common English stopwords
const std::unordered_set<std::string> STOPWORDS = {
    "a","an","the","is","it","in","on","at","to","for","of","and",
    "or","but","not","with","this","that","are","was","were","be",
    "has","have","had","do","does","did","will","would","could","should",
    "i","you","he","she","we","they","my","your","his","her","its"
};

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        word = stripPunct(toLower(word));
        if (!word.empty() && STOPWORDS.find(word) == STOPWORDS.end())
            tokens.push_back(word);
    }
    return tokens;
}

// ─────────────────────────────────────────────
//  Inverted Index
// ─────────────────────────────────────────────
struct Posting {
    int docId;
    std::vector<int> positions; // for phrase search
};

class InvertedIndex {
public:
    // term -> list of postings
    std::unordered_map<std::string, std::vector<Posting>> index;
    // docId -> {filename, raw token count}
    std::map<int, std::pair<std::string, int>> docMeta;
    int nextDocId = 0;

    void addDocument(const std::string& filename, const std::string& content) {
        int docId = nextDocId++;
        std::vector<std::string> tokens = tokenize(content);
        docMeta[docId] = {filename, (int)tokens.size()};

        std::unordered_map<std::string, std::vector<int>> termPositions;
        for (int i = 0; i < (int)tokens.size(); i++)
            termPositions[tokens[i]].push_back(i);

        for (auto& [term, positions] : termPositions) {
            index[term].push_back({docId, positions});
        }
    }

    // Term frequency of term t in document d
    double tf(const std::string& term, int docId) const {
        auto it = index.find(term);
        if (it == index.end()) return 0.0;
        for (const auto& p : it->second) {
            if (p.docId == docId) {
                int docLen = docMeta.at(docId).second;
                return docLen > 0 ? (double)p.positions.size() / docLen : 0.0;
            }
        }
        return 0.0;
    }

    // Inverse document frequency
    double idf(const std::string& term) const {
        auto it = index.find(term);
        if (it == index.end()) return 0.0;
        int df = (int)it->second.size();
        int N  = (int)docMeta.size();
        return std::log((double)(N + 1) / (df + 1)) + 1.0; // smoothed
    }
};

// ─────────────────────────────────────────────
//  Query Parser  (AND / OR / phrase)
// ─────────────────────────────────────────────
enum class QueryType { AND_QUERY, OR_QUERY, PHRASE_QUERY };

struct Query {
    QueryType type;
    std::vector<std::string> terms;
};

Query parseQuery(const std::string& raw) {
    Query q;
    if (raw.front() == '"' && raw.back() == '"') {
        q.type = QueryType::PHRASE_QUERY;
        std::string inner = raw.substr(1, raw.size() - 2);
        q.terms = tokenize(inner);
        return q;
    }
    if (raw.find(" AND ") != std::string::npos) {
        q.type = QueryType::AND_QUERY;
        std::istringstream iss(raw);
        std::string tok;
        while (iss >> tok) {
            if (tok != "AND") q.terms.push_back(toLower(tok));
        }
        return q;
    }
    q.type = QueryType::OR_QUERY;
    std::istringstream iss(raw);
    std::string tok;
    while (iss >> tok) {
        if (tok != "OR") q.terms.push_back(toLower(tok));
    }
    return q;
}

// ─────────────────────────────────────────────
//  Search Engine
// ─────────────────────────────────────────────
struct SearchResult {
    int docId;
    std::string filename;
    double score;
};

class SearchEngine {
    InvertedIndex idx;

    std::unordered_set<int> getCandidates(const Query& q) const {
        std::unordered_set<int> result;
        bool first = true;
        for (const auto& term : q.terms) {
            auto it = idx.index.find(term);
            if (it == idx.index.end()) {
                if (q.type == QueryType::AND_QUERY) return {}; // AND: must have all
                continue;
            }
            std::unordered_set<int> termDocs;
            for (const auto& p : it->second) termDocs.insert(p.docId);

            if (q.type == QueryType::AND_QUERY) {
                if (first) { result = termDocs; first = false; }
                else {
                    std::unordered_set<int> inter;
                    for (int id : result) if (termDocs.count(id)) inter.insert(id);
                    result = inter;
                }
            } else {
                for (int id : termDocs) result.insert(id);
            }
        }
        return result;
    }

    bool phraseMatch(const std::vector<std::string>& terms, int docId) const {
        if (terms.empty()) return false;
        auto it = idx.index.find(terms[0]);
        if (it == idx.index.end()) return false;

        std::vector<int> startPositions;
        for (const auto& p : it->second)
            if (p.docId == docId) { startPositions = p.positions; break; }

        for (int startPos : startPositions) {
            bool match = true;
            for (int i = 1; i < (int)terms.size(); i++) {
                auto it2 = idx.index.find(terms[i]);
                if (it2 == idx.index.end()) { match = false; break; }
                bool found = false;
                for (const auto& p : it2->second) {
                    if (p.docId != docId) continue;
                    for (int pos : p.positions) {
                        if (pos == startPos + i) { found = true; break; }
                    }
                    if (found) break;
                }
                if (!found) { match = false; break; }
            }
            if (match) return true;
        }
        return false;
    }

public:
    void indexDirectory(const std::string& dirPath) {
        int count = 0;
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (entry.path().extension() == ".txt") {
                std::ifstream f(entry.path());
                if (!f) continue;
                std::string content((std::istreambuf_iterator<char>(f)),
                                     std::istreambuf_iterator<char>());
                idx.addDocument(entry.path().filename().string(), content);
                count++;
            }
        }
        std::cout << "✓ Indexed " << count << " documents.\n";
    }

    void indexText(const std::string& name, const std::string& text) {
        idx.addDocument(name, text);
    }

    std::vector<SearchResult> search(const std::string& rawQuery, int topK = 5) {
        Query q = parseQuery(rawQuery);
        std::unordered_set<int> candidates = getCandidates(q);

        if (q.type == QueryType::PHRASE_QUERY) {
            std::unordered_set<int> phraseMatches;
            for (int docId : candidates)
                if (phraseMatch(q.terms, docId)) phraseMatches.insert(docId);
            candidates = phraseMatches;
        }

        // Score using TF-IDF
        std::vector<SearchResult> results;
        for (int docId : candidates) {
            double score = 0.0;
            for (const auto& term : q.terms)
                score += idx.tf(term, docId) * idx.idf(term);
            results.push_back({docId, idx.docMeta[docId].first, score});
        }

        std::sort(results.begin(), results.end(),
                  [](const SearchResult& a, const SearchResult& b) {
                      return a.score > b.score;
                  });
        if ((int)results.size() > topK) results.resize(topK);
        return results;
    }

    void printStats() const {
        std::cout << "Index stats: "
                  << idx.docMeta.size() << " docs, "
                  << idx.index.size() << " unique terms\n";
    }
};

// ─────────────────────────────────────────────
//  Main / REPL
// ─────────────────────────────────────────────
void seedSampleDocs(SearchEngine& engine) {
    engine.indexText("algorithms.txt",
        "Binary search is an efficient algorithm for finding an element in a sorted array. "
        "It works by repeatedly dividing the search interval in half. "
        "The time complexity of binary search is O(log n).");

    engine.indexText("data_structures.txt",
        "A hash table is a data structure that implements an associative array. "
        "It uses a hash function to compute an index into an array of buckets. "
        "Hash tables offer O(1) average time complexity for search insert delete.");

    engine.indexText("operating_systems.txt",
        "A process is an instance of a program in execution. "
        "The operating system manages processes using a process control block. "
        "Context switching allows the CPU to switch between multiple processes.");

    engine.indexText("machine_learning.txt",
        "Gradient descent is an optimization algorithm used in machine learning. "
        "It iteratively adjusts parameters to minimize a loss function. "
        "Deep learning models use backpropagation with gradient descent.");

    engine.indexText("networking.txt",
        "TCP provides reliable ordered delivery of a stream of bytes. "
        "The three-way handshake establishes a TCP connection between client and server. "
        "UDP is connectionless and faster but does not guarantee delivery.");
}

int main(int argc, char* argv[]) {
    SearchEngine engine;

    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║       Mini Search Engine v1.0        ║\n";
    std::cout << "╚══════════════════════════════════════╝\n\n";

    if (argc > 1) {
        engine.indexDirectory(argv[1]);
    } else {
        std::cout << "No directory provided. Loading sample documents...\n";
        seedSampleDocs(engine);
    }
    engine.printStats();

    std::cout << "\nQuery syntax:\n";
    std::cout << "  word1 word2       → OR search\n";
    std::cout << "  word1 AND word2   → AND search\n";
    std::cout << "  \"phrase here\"     → Phrase search\n";
    std::cout << "  :quit             → Exit\n\n";

    std::string query;
    while (true) {
        std::cout << "search> ";
        std::getline(std::cin, query);
        if (query == ":quit" || query == ":q") break;
        if (query.empty()) continue;

        auto results = engine.search(query);
        if (results.empty()) {
            std::cout << "  No results found.\n\n";
            continue;
        }
        std::cout << "  Top " << results.size() << " result(s):\n";
        for (int i = 0; i < (int)results.size(); i++) {
            std::cout << "  " << (i+1) << ". " << std::left << std::setw(30)
                      << results[i].filename
                      << "  score: " << std::fixed << std::setprecision(4)
                      << results[i].score << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}
