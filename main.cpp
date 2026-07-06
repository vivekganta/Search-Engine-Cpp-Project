#include "engine.h"
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[]) {
    SearchEngine engine;

    // Built-in sample documents
    engine.addDocument(0,  "Binary Search",       "Binary search is an efficient algorithm for finding an item in a sorted list by repeatedly halving the search space.");
    engine.addDocument(1,  "Hash Tables",          "A hash table uses a hash function to map keys to buckets enabling average O(1) lookup insert and delete operations.");
    engine.addDocument(2,  "Dynamic Programming",  "Dynamic programming solves problems by breaking them into overlapping subproblems and storing results to avoid redundant computation.");
    engine.addDocument(3,  "Graph BFS DFS",        "Breadth first search and depth first search are fundamental graph traversal algorithms used for shortest paths and connectivity.");
    engine.addDocument(4,  "Sorting Algorithms",   "Merge sort quick sort and heap sort are comparison based sorting algorithms with O(n log n) average time complexity.");
    engine.addDocument(5,  "Trees and BST",        "Binary search trees support O(log n) search insert delete when balanced. AVL trees and red black trees maintain balance automatically.");
    engine.addDocument(6,  "Heaps and Priority",   "A heap is a complete binary tree satisfying the heap property. Priority queues are implemented using heaps for O(log n) operations.");
    engine.addDocument(7,  "Linked Lists",         "Linked lists store elements in nodes connected by pointers enabling O(1) insert and delete but O(n) random access.");
    engine.addDocument(8,  "Trie Data Structure",  "A trie is a tree where each node represents a character prefix. Used for fast string search autocomplete and spell checking.");
    engine.addDocument(9,  "Segment Trees",        "Segment trees support range queries and point updates in O(log n). Useful for range sum minimum and maximum queries.");
    engine.addDocument(10, "Greedy Algorithms",    "Greedy algorithms make locally optimal choices at each step. Used in Dijkstra Huffman coding and interval scheduling problems.");
    engine.addDocument(11, "Backtracking",         "Backtracking explores all possibilities by building candidates incrementally and abandoning branches that cannot lead to valid solutions.");

    // Optionally load from file
    if (argc > 1) {
        if (!engine.loadFromFile(argv[1])) {
            std::cerr << "Warning: could not open file " << argv[1] << std::endl;
        }
    }

    std::cout << "\n╔══════════════════════════════════════╗\n";
    std::cout <<   "║         Mini Search Engine           ║\n";
    std::cout <<   "╚══════════════════════════════════════╝\n";
    std::cout << "  Indexed " << engine.docCount() << " documents, "
              << engine.termCount() << " unique terms\n\n";

    std::string query;
    while (true) {
        std::cout << "Search> ";
        if (!std::getline(std::cin, query) || query == "quit" || query == "exit") break;
        if (query.empty()) continue;

        auto results = engine.search(query, 5);

        if (results.empty()) {
            std::cout << "  No results found.\n\n";
            continue;
        }

        std::cout << "\n  Top " << results.size() << " results:\n";
        std::cout << "  " << std::string(50, '─') << "\n";
        for (int i = 0; i < (int)results.size(); i++) {
            std::cout << "  [" << (i+1) << "] " << results[i].title
                      << "  (score: " << std::fixed << std::setprecision(4) << results[i].score << ")\n"
                      << "      " << results[i].snippet << "...\n\n";
        }
    }
    return 0;
}
