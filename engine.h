#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct Document {
    int id;
    std::string title;
    std::string content;
};

struct SearchResult {
    int docId;
    std::string title;
    std::string snippet;
    double score;
};

class SearchEngine {
    std::unordered_map<int, Document>                          documents;
    std::unordered_map<std::string, std::unordered_map<int,int>> invertedIndex;  // term -> {docId -> freq}
    std::unordered_map<std::string, int>                       termDocFreq;      // term -> #docs containing it
    std::unordered_map<int, int>                               docTermCount;     // docId -> total tokens

    double tfidf(const std::string& term, int docId) const;

public:
    void addDocument(int docId, const std::string& title, const std::string& content);
    bool loadFromFile(const std::string& filepath);
    std::vector<SearchResult> search(const std::string& query, int topK = 5) const;
    int docCount()  const;
    int termCount() const;
};
