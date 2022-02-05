#include <vector>
#include <string>
#include <iostream>
int LevenshteinDistance(const std::string& s1, const std::string& s2) {
    // initialize 0 matrix
    std::vector<std::vector<int>> distance = std::vector<std::vector<int>>(s1.size() + 1, std::vector<int>(s2.size() + 1, 0));
    const int iSize = s1.size() + 1;
    const int s1Size = s1.size();
    const int jSize = s2.size() + 1;
    const int s2Size = s2.size();
    // initialize first row and column
    for (int i = 0; i < iSize; ++i) {
        distance[i][0] = i;
    }
    for (int j = 0; j < jSize; ++j) {
        distance[0][j] = j;
    }
    // calculate the distance
    for (int j = 0; j < s2Size;++j) {
        for (int i = 0; i < s1Size;++i) {
            int cost;
            if (s1[i] == s2[j]) {
                cost = 0;
            } else {
                cost = 1;
            }
            distance[i + 1][j + 1] = std::min(
                std::min(distance[i][j + 1] + 1, distance[i + 1][j] + 1),
                distance[i][j] + cost
            );
        }
    }
    return distance[iSize - 1][jSize - 1];
}
int main() {
    int dis = LevenshteinDistance("kitten", "sitting");
    std::cout << dis << std::endl;
    dis = LevenshteinDistance("sitting", "kitten");
    std::cout << dis << std::endl;
}
