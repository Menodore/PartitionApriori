#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <sstream>
#include <algorithm>

using namespace std;

typedef set<int> Itemset;

// Function to generate candidate itemsets of size k from frequent itemsets of size k-1
vector<Itemset> generateCandidates(const vector<Itemset>& frequentItemsets, int k) {
    vector<Itemset> candidates;
    for (size_t i = 0; i < frequentItemsets.size(); ++i) {
        for (size_t j = i + 1; j < frequentItemsets.size(); ++j) {
            Itemset candidate(frequentItemsets[i]);
            candidate.insert(frequentItemsets[j].begin(), frequentItemsets[j].end());

            if (candidate.size() == k) {
                candidates.push_back(candidate);
            }
        }
    }
    return candidates;
}

// Function to count support for itemsets
map<Itemset, int> countSupport(const vector<Itemset>& transactions, const vector<Itemset>& candidates) {
    map<Itemset, int> itemsetCounts;
    for (const auto& transaction : transactions) {
        for (const auto& candidate : candidates) {
            if (includes(transaction.begin(), transaction.end(), candidate.begin(), candidate.end())) {
                itemsetCounts[candidate]++;
            }
        }
    }
    return itemsetCounts;
}

// Function to filter candidates by minimum support
vector<Itemset> filterBySupport(const map<Itemset, int>& candidateCounts, int minSupport) {
    vector<Itemset> frequentItemsets;
    for (const auto& candidateCount : candidateCounts) {
        if (candidateCount.second >= minSupport) {
            frequentItemsets.push_back(candidateCount.first);
        }
    }
    return frequentItemsets;
}

// Function to find frequent itemsets in a partition
vector<Itemset> findFrequentItemsets(const vector<Itemset>& transactions, int minSupport, const string& outFileName) {
    // Step 1: Generate frequent 1-itemsets
    fstream outFile(outFileName);
    map<int, int> itemCounts;
    for (const auto& transaction : transactions) {
        for (int item : transaction) {
            itemCounts[item]++;
        }
    }

    vector<Itemset> frequentItemsets;
    for (const auto& itemCount : itemCounts) {
        if (itemCount.second >= minSupport) {
            frequentItemsets.push_back({ itemCount.first });
        }
    }

    int k = 2;
    while (!frequentItemsets.empty()) {
        // Step 2: Generate candidate k-itemsets using the previous frequent itemsets
        vector<Itemset> candidates = generateCandidates(frequentItemsets, k);

        // Step 3: Count the support for candidates and filter by minimum support
        map<Itemset, int> candidateCounts = countSupport(transactions, candidates);
        frequentItemsets = filterBySupport(candidateCounts, minSupport);

        // Output the frequent itemsets of size k
        if (!frequentItemsets.empty()) {
            outFile << "Frequent " << k << "-itemsets in partition: " << endl;
            for (const auto& itemset : frequentItemsets) {
                for (int item : itemset) {
                    outFile << item << " ";
                }
                outFile << endl;
            }
        }

        k++;
    }

    return frequentItemsets;
}

// Partition-based Apriori algorithm
void partitionApriori(const vector<Itemset>& transactions, int globalMinSupport, int partitionSize, const string& outFileName) {
    vector<Itemset> globalCandidates;
    ofstream outFile(outFileName);
    vector<vector<Itemset>> partitions;

    // Step 1: Partition the dataset
    for (size_t i = 0; i < transactions.size(); i += partitionSize) {
        vector<Itemset> partition(transactions.begin() + i, transactions.begin() + min(i + partitionSize, transactions.size()));
        partitions.push_back(partition);
    }

    // Step 2: Find local frequent itemsets in each partition
    for (const auto& partition : partitions) {
        int localMinSupport = (globalMinSupport * partition.size()) / transactions.size();
        if (localMinSupport == 0) localMinSupport = 1; // Ensure minSupport is at least 1
        vector<Itemset> localFrequentItemsets = findFrequentItemsets(partition, localMinSupport, "out_partition.txt");
        globalCandidates.insert(globalCandidates.end(), localFrequentItemsets.begin(), localFrequentItemsets.end());
    }

    // Step 3: Remove duplicate candidates
    sort(globalCandidates.begin(), globalCandidates.end());
    globalCandidates.erase(unique(globalCandidates.begin(), globalCandidates.end()), globalCandidates.end());

    // Step 4: Count global support for candidate itemsets
    map<Itemset, int> globalCounts = countSupport(transactions, globalCandidates);

    // Step 5: Filter globally frequent itemsets
    vector<Itemset> globalFrequentItemsets = filterBySupport(globalCounts, globalMinSupport);

    // Step 6: Output globally frequent itemsets
    outFile << "Global Frequent Itemsets: " << endl;
    for (const auto& itemset : globalFrequentItemsets) {
        for (int item : itemset) {
            outFile << item << " ";
        }
        outFile << " (Support: " << globalCounts[itemset] << ")" << endl;
    }
}


vector<Itemset> readTransactions(const string& filename) {
    ifstream file(filename);
    vector<Itemset> transactions;
    string line;

    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            Itemset transaction;
            int item;
            while (ss >> item) {
                transaction.insert(item);
            }
            transactions.push_back(transaction);
        }
        file.close();
    }
    else {
        cerr << "Unable to open file" << endl;
    }

    return transactions;
}

int main() {
    string filename = "td.txt";
    string outFileName = "out_partition.txt";
    int globalMinSupport = 40;  
    int partitionSize = 9;   

    vector<Itemset> transactions = readTransactions(filename);

    partitionApriori(transactions, globalMinSupport, partitionSize, outFileName);

    return 0;
}
