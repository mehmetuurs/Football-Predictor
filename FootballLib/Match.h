#ifndef MATCH_H
#define MATCH_H

#include <random>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include "Team.h" // Needs Team definition

class Match {
public:
    void runFullSimulation(const Team& home, const Team& away,
                           double avgHomeGoals, double avgAwayGoals,
                           double avgHomeCorners, double avgAwayCorners)
    {
        // Reset stats
        homeWins = 0; draws = 0; awayWins = 0;
        over05 = 0; over15 = 0; over25 = 0;
        totalSimulatedCorners = 0;
        cornerCounts.clear();
        scoreCounts.clear();
        bttsCount = 0; // NEW: Reset BTTS counter


        for (int i = 0; i < simulationsToRun; ++i) {
            simulateSingleGame(home, away, avgHomeGoals, avgAwayGoals, avgHomeCorners, avgAwayCorners);

            // Tally Win/Draw/Loss
            if (homeGoals > awayGoals) homeWins++;
            else if (homeGoals < awayGoals) awayWins++;
            else draws++;

            // Tally Goal Over/Under
            int totalGoals = homeGoals + awayGoals;
            if (totalGoals > 0) over05++;
            if (totalGoals > 1) over15++;
            if (totalGoals > 2) over25++;

            // NEW: Tally BTTS
            if (homeGoals > 0 && awayGoals > 0) {
                bttsCount++;
            }

            // Tally Corner stats
            int totalCorners = homeCorners + awayCorners;
            totalSimulatedCorners += totalCorners;
            cornerCounts[totalCorners]++;

            scoreCounts[{homeGoals, awayGoals}]++;
        }
    }

    // --- Goal Getters ---
    double getHomeWinPercent() const { return (simulationsToRun > 0) ? (double)homeWins / simulationsToRun * 100.0 : 0.0;}
    double getDrawPercent() const { return (simulationsToRun > 0) ? (double)draws / simulationsToRun * 100.0 : 0.0;}
    double getAwayWinPercent() const { return (simulationsToRun > 0) ? (double)awayWins / simulationsToRun * 100.0 : 0.0;}
    double getOver05Percent() const { return (simulationsToRun > 0) ? (double)over05 / simulationsToRun * 100.0 : 0.0; }
    double getOver15Percent() const { return (simulationsToRun > 0) ? (double)over15 / simulationsToRun * 100.0 : 0.0; }
    double getOver25Percent() const { return (simulationsToRun > 0) ? (double)over25 / simulationsToRun * 100.0 : 0.0; }

    // --- NEW: BTTS Getters ---
    double getBttsYesPercent() const { return (simulationsToRun > 0) ? (double)bttsCount / simulationsToRun * 100.0 : 0.0; }
    double getBttsNoPercent() const { return 100.0 - getBttsYesPercent(); }


    // --- Corner Getters ---
    double getAverageTotalCorners() const { return (simulationsToRun > 0) ? static_cast<double>(totalSimulatedCorners) / simulationsToRun : 0.0; }
    double getCornerPercent(double line, bool over) const {
        int count = 0;
        for(const auto& pair : cornerCounts) {
            if (over && pair.first > line) { count += pair.second; }
            else if (!over && pair.first < line) { count += pair.second; }
        }
        return (simulationsToRun > 0) ? static_cast<double>(count) / simulationsToRun * 100.0 : 0.0;
    }


    // --- Most Likely Scores Getter ---
    std::vector<std::pair<std::string, double>> getMostLikelyScores(int topN = 5) {
        std::vector<std::pair<int, std::pair<int, int>>> sortedScores;
        for (auto const& [score, count] : scoreCounts) { sortedScores.push_back({count, score}); }
        std::sort(sortedScores.rbegin(), sortedScores.rend());
        std::vector<std::pair<std::string, double>> topList;
        for (int i = 0; i < topN && i < sortedScores.size(); ++i) {
            std::string scoreStr = std::to_string(sortedScores[i].second.first) + " - " + std::to_string(sortedScores[i].second.second);
            double percent = (simulationsToRun > 0) ? (double)sortedScores[i].first / simulationsToRun * 100.0 : 0.0;
            topList.push_back({scoreStr, percent});
        }
        return topList;
    }

private:
    // Goal related
    int homeGoals = 0, awayGoals = 0;
    int homeWins = 0, draws = 0, awayWins = 0;
    int over05 = 0, over15 = 0, over25 = 0;
    std::map<std::pair<int, int>, int> scoreCounts;
    int bttsCount = 0; // NEW: BTTS counter

    // Corner related
    int homeCorners = 0, awayCorners = 0;
    long long totalSimulatedCorners = 0;
    std::map<int, int> cornerCounts;

    int simulationsToRun = 10000;


    void simulateSingleGame(const Team& home, const Team& away,
                            double avgHomeGoals, double avgAwayGoals,
                            double avgHomeCorners, double avgAwayCorners)
    {
        double lambdaHomeG = home.homeAttackStrength * away.awayDefenseStrength * avgHomeGoals;
        double lambdaAwayG = away.awayAttackStrength * home.homeDefenseStrength * avgAwayGoals;
        if (lambdaHomeG < 0.01) lambdaHomeG = 0.01;
        if (lambdaAwayG < 0.01) lambdaAwayG = 0.01;

        double lambdaHomeC = home.homeCornerAttackStrength * away.awayCornerDefenseStrength * avgHomeCorners;
        double lambdaAwayC = away.awayCornerAttackStrength * home.homeCornerDefenseStrength * avgAwayCorners;
        if (lambdaHomeC < 0.01) lambdaHomeC = 0.01;
        if (lambdaAwayC < 0.01) lambdaAwayC = 0.01;

        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::poisson_distribution<> distHomeG(lambdaHomeG);
        std::poisson_distribution<> distAwayG(lambdaAwayG);
        std::poisson_distribution<> distHomeC(lambdaHomeC);
        std::poisson_distribution<> distAwayC(lambdaAwayC);

        homeGoals = distHomeG(gen);
        awayGoals = distAwayG(gen);
        homeCorners = distHomeC(gen);
        awayCorners = distAwayC(gen);
    }
};

#endif // MATCH_H