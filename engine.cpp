#include "engine.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

// ─── Text Utilities ───────────────────────────────────────────────────────────

static std::string toLower(std::string s) {
    for (char& c : s) c = tolower(c);
    return s;
}

static std::string stripPunct(const std::string& word) {
    std::string result;
    for (char c : word)
        if (isalnum(c)) result += c;
    return result;
}

static std::vector<std::string> tokenize(const std::string& text) {
    std::istringstream iss(text);
    std::vector<std::string> tokens;
    std::string word;
    while (iss >> word) {
        word = stripPunct(toLower(word));
        if (!word.empty()) tokens.push_back(word);
    }
    return tokens;
}

// ─── SearchEngine ─────────────────────────────────────────────────────────────

void SearchEngine::addDocument(int docId, const std::string& title, const std::string& content) {
    documents[docId] = {docId, title, content};
    auto tokens = tokenize(content + " " + title);

    std::unordered_map<std::string, int> freq;
    for (const auto& t : tokens) freq[t]++;

    for (const auto& [term, count] : freq) {
        invertedIndex[term][docId] = count;
        termDocFreq[term]++;
    }
    docTermCount[docId] = tokens.size();
}

bool SearchEngine::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string line;
    int docId = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        // Format: TITLE | CONTENT
        auto sep = line.find('|');
        if (sep == std::string::npos) {
            addDocument(docId++, "Doc " + std::to_string(docId), line);
        } else {
            addDocument(docId++, line.substr(0, sep - 1), line.substr(sep + 2));
        }
    }
    return true;
}

double SearchEngine::tfidf(const std::string& term, int docId) const {
    auto idxIt = invertedIndex.find(term);
    if (idxIt == invertedIndex.end()) return 0.0;

    auto docIt = idxIt->second.find(docId);
    if (docIt == idxIt->second.end()) return 0.0;

    int termCount    = docTermCount.at(docId);
    double tf        = (double)docIt->second / termCount;

    int N            = documents.size();
    int df           = termDocFreq.at(term);
    double idf       = std::log((double)(N + 1) / (df + 1)) + 1.0;

    return tf * idf;
}

std::vector<SearchResult> SearchEngine::search(const std::string& query, int topK) const {
    auto queryTerms = tokenize(query);
    std::unordered_map<int, double> scores;

    for (const auto& term : queryTerms) {
        auto it = invertedIndex.find(term);
        if (it == invertedIndex.end()) continue;
        for (const auto& [docId, _] : it->second)
            scores[docId] += tfidf(term, docId);
    }

    std::vector<SearchResult> results;
    for (const auto& [docId, score] : scores) {
        std::string snippet = documents.at(docId).content.substr(0, 120);
        results.push_back({docId, documents.at(docId).title, snippet, score});
    }

    std::sort(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) { return a.score > b.score; });

    if ((int)results.size() > topK) results.resize(topK);
    return results;
}

int SearchEngine::docCount() const { return documents.size(); }
int SearchEngine::termCount() const { return invertedIndex.size(); }
