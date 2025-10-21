#ifndef DATATYPES_H
#define DATATYPES_H

#include <string>
#include <vector>
#include <chrono>

// Represents an upcoming match from fixtures.csv
struct Fixture {
    std::string dateStr;
    std::string homeTeamName;
    std::string awayTeamName;
};

// Represents a completed match from the data files
struct MatchResult {
    std::string dateStr;
    std::chrono::system_clock::time_point date;
    std::string homeTeamName;
    std::string awayTeamName;
    int homeGoals;
    int awayGoals;
};

// NEW: Head-to-head statistics between two teams
struct H2HStats {
    int totalMatches = 0;
    int homeTeamWins = 0;
    int awayTeamWins = 0;
    int draws = 0;
    double avgHomeGoals = 0.0;
    double avgAwayGoals = 0.0;
    double bttsPercentage = 0.0;
    double over25Percentage = 0.0;
    std::vector<MatchResult> recentH2H; // Last matches between these teams
};

#endif // DATATYPES_H