#ifndef DATALOADER_H
#define DATALOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <limits>
#include "DataTypes.h"
#include "Team.h"

// Struct for temporary raw stats
struct TeamData {
    int homeMatches = 0, awayMatches = 0;
    int homeGoalsScored = 0, awayGoalsScored = 0;
    int homeGoalsConceded = 0, awayGoalsConceded = 0;
    int homeCornersFor = 0, awayCornersFor = 0;
    int homeCornersAgainst = 0, awayCornersAgainst = 0;
};

class DataLoader {
private:
    // NEW: Four distinct league averages
    double leagueAvgHomeGoalsScored = 0.0;
    double leagueAvgAwayGoalsScored = 0.0;
    double leagueAvgHomeGoalsConceded = 0.0; // Avg goals conceded by home teams
    double leagueAvgAwayGoalsConceded = 0.0; // Avg goals conceded by away teams
    double leagueAvgHomeCorners = 0.0;
    double leagueAvgAwayCorners = 0.0;

    std::vector<Fixture> upcomingFixtures;
    std::map<std::string, Team> loadedTeams;

public:
    // --- NEW: Load data from multiple files ---
    bool loadMultipleFiles(const std::vector<std::string>& filePaths) {
        loadedTeams.clear();
        std::vector<MatchResult> allResults;
        std::map<std::string, TeamData> rawData;
        int totalHomeGoals = 0, totalAwayGoals = 0, totalMatches = 0;
        int totalHomeCorners = 0, totalAwayCorners = 0;

        // Loop through each results file
        for (const std::string& filePath : filePaths) {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                std::cerr << "Warning: Could not open stats file: " << filePath << std::endl;
                continue; // Skip this file if it can't be opened
            }

            std::cout << "Processing file: " << filePath << "..." << std::endl;
            std::string line;
            std::getline(file, line); // Skip header

            while (std::getline(file, line)) {
                if (line.empty() || line.find(',') == std::string::npos) continue;

                std::stringstream ss(line);
                std::string cell;
                std::vector<std::string> row;
                while (std::getline(ss, cell, ',')) { row.push_back(cell); }

                // Need at least 19 columns for basic data + corners (HC, AC)
                if (row.size() < 19) continue;

                try {
                    std::string dateStr = row[1];
                    std::string homeTeamName = row[3];
                    std::string awayTeamName = row[4];
                    if (homeTeamName.empty() || awayTeamName.empty()) continue;

                    auto matchDate = parseDate(dateStr);
                    if (matchDate == std::chrono::system_clock::from_time_t(0)) continue;

                    // Ensure team objects exist
                    if (loadedTeams.find(homeTeamName) == loadedTeams.end()) { loadedTeams[homeTeamName] = Team(homeTeamName); }
                    if (loadedTeams.find(awayTeamName) == loadedTeams.end()) { loadedTeams[awayTeamName] = Team(awayTeamName); }

                    // Check score columns (only process completed games for stats)
                    if (!row[5].empty() && !row[6].empty() && isdigit(row[5][0]) && isdigit(row[6][0])) {
                        int homeGoals = std::stoi(row[5]); 
                        int awayGoals = std::stoi(row[6]);
                        int homeCorners = 0, awayCorners = 0;
                        if (!row[17].empty()) try { homeCorners = std::stoi(row[17]); } catch (...) {}
                        if (!row[18].empty()) try { awayCorners = std::stoi(row[18]); } catch (...) {}

                        MatchResult result = {dateStr, matchDate, homeTeamName, awayTeamName, homeGoals, awayGoals};
                        allResults.push_back(result);

                        // Accumulate raw data
                        rawData[homeTeamName].homeMatches++;
                        rawData[homeTeamName].homeGoalsScored += homeGoals;
                        rawData[homeTeamName].homeGoalsConceded += awayGoals;
                        rawData[homeTeamName].homeCornersFor += homeCorners;
                        rawData[homeTeamName].homeCornersAgainst += awayCorners;

                        rawData[awayTeamName].awayMatches++;
                        rawData[awayTeamName].awayGoalsScored += awayGoals;
                        rawData[awayTeamName].awayGoalsConceded += homeGoals;
                        rawData[awayTeamName].awayCornersFor += awayCorners;
                        rawData[awayTeamName].awayCornersAgainst += homeCorners;

                        totalHomeGoals += homeGoals;
                        totalAwayGoals += awayGoals;
                        totalHomeCorners += homeCorners;
                        totalAwayCorners += awayCorners;
                        totalMatches++;
                    }
                } catch (const std::exception& e) { /* Skip bad lines */ }
            }
            file.close();
        } // End loop through files

        if (totalMatches == 0) {
            std::cerr << "Error: No completed matches found in any data file." << std::endl;
            return false;
        }

        std::cout << "Total historical matches processed: " << totalMatches << std::endl;

        // --- Calculate NEW League Averages ---
        leagueAvgHomeGoalsScored = static_cast<double>(totalHomeGoals) / totalMatches;
        leagueAvgAwayGoalsScored = static_cast<double>(totalAwayGoals) / totalMatches;

        // Average goals conceded by home team = average scored by away team
        leagueAvgHomeGoalsConceded = leagueAvgAwayGoalsScored;
        // Average goals conceded by away team = average scored by home team
        leagueAvgAwayGoalsConceded = leagueAvgHomeGoalsScored;

        leagueAvgHomeCorners = static_cast<double>(totalHomeCorners) / totalMatches;
        leagueAvgAwayCorners = static_cast<double>(totalAwayCorners) / totalMatches;

        // Add history and calculate overall strengths using NEW averages
        std::sort(allResults.begin(), allResults.end(), [](const MatchResult& a, const MatchResult& b) { return a.date < b.date; });

        for (auto& pair : loadedTeams) {
            std::string teamName = pair.first;
            Team& team = pair.second;

            // Add all historical matches for this team
            for(const auto& result : allResults) {
                if (result.homeTeamName == teamName || result.awayTeamName == teamName) {
                    team.matchHistory.push_back(result);
                }
            }

            // Calculate overall (all-time) strengths
            const TeamData& data = rawData[teamName];
            if (data.homeMatches > 0) {
                team.homeAttackStrength = (static_cast<double>(data.homeGoalsScored) / data.homeMatches) / leagueAvgHomeGoalsScored;
                team.homeDefenseStrength = (static_cast<double>(data.homeGoalsConceded) / data.homeMatches) / leagueAvgHomeGoalsConceded;
                team.homeCornerAttackStrength = (static_cast<double>(data.homeCornersFor) / data.homeMatches) / leagueAvgHomeCorners;
                team.homeCornerDefenseStrength = (static_cast<double>(data.homeCornersAgainst) / data.homeMatches) / leagueAvgAwayCorners; // Defend against away corners
            }
            if (data.awayMatches > 0) {
                team.awayAttackStrength = (static_cast<double>(data.awayGoalsScored) / data.awayMatches) / leagueAvgAwayGoalsScored;
                team.awayDefenseStrength = (static_cast<double>(data.awayGoalsConceded) / data.awayMatches) / leagueAvgAwayGoalsConceded;
                team.awayCornerAttackStrength = (static_cast<double>(data.awayCornersFor) / data.awayMatches) / leagueAvgAwayCorners;
                team.awayCornerDefenseStrength = (static_cast<double>(data.awayCornersAgainst) / data.awayMatches) / leagueAvgHomeCorners; // Defend against home corners
            }
        }
        return true;
    }

    // --- THIS FUNCTION IS NO LONGER USED, but we leave it to avoid errors ---
    // --- We will change the call in main.cpp ---
    bool loadTeamStats(const std::string& filePath) {
        std::cerr << "Warning: Obsolete function loadTeamStats called." << std::endl;
        // Just call the new function with the single file
        return loadMultipleFiles({filePath});
    }

    // --- Fixture Loading --- FIXED: NO HEADER SKIP
    bool loadFixtures(const std::string& filePath) {
        upcomingFixtures.clear();
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open fixtures file: " << filePath << std::endl;
            return false;
        }
        
        std::string line;
        // FIXED: Removed header skip since fixtures.csv has no header
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> row;
            while (std::getline(ss, cell, ',')) { row.push_back(cell); }
            if (row.size() < 3) continue;
            upcomingFixtures.push_back(Fixture{row[0], row[1], row[2]});
        }
        file.close();
        return !upcomingFixtures.empty();
    }

    // --- Form Calculation ---
    std::map<std::string, Team> calculateFormStrengths(const std::string& fixtureDateStr, int formMatches = 5) {
        auto fixtureDate = parseDate(fixtureDateStr);
        if (fixtureDate == std::chrono::system_clock::from_time_t(0)) {
            std::cerr << "Error: Invalid fixture date for form calculation: " << fixtureDateStr << std::endl;
            return {};
        }

        std::map<std::string, Team> formTeams;
        std::map<std::string, TeamData> rawData;

        // Use a copy of all loaded teams (which have their full history)
        for (const auto& pair : loadedTeams) {
            const std::string& teamName = pair.first;
            const Team& overallTeam = pair.second;
            formTeams[teamName] = Team(teamName); // Create new team for form stats

            int homeMatches = 0, awayMatches = 0;

            // Iterate history backwards to find last 'formMatches'
            for (auto it = overallTeam.matchHistory.rbegin(); it != overallTeam.matchHistory.rend(); ++it) {
                const MatchResult& result = *it;

                // Only use matches *before* the fixture date
                if (result.date >= fixtureDate) continue;

                if (result.homeTeamName == teamName && homeMatches < formMatches) {
                    rawData[teamName].homeMatches++;
                    rawData[teamName].homeGoalsScored += result.homeGoals;
                    rawData[teamName].homeGoalsConceded += result.awayGoals;
                    homeMatches++;
                } else if (result.awayTeamName == teamName && awayMatches < formMatches) {
                    rawData[teamName].awayMatches++;
                    rawData[teamName].awayGoalsScored += result.awayGoals;
                    rawData[teamName].awayGoalsConceded += result.homeGoals;
                    awayMatches++;
                }
                if (homeMatches >= formMatches && awayMatches >= formMatches) break;
            }
        }

        // Calculate form strengths based on raw data
        for (auto& pair : formTeams) {
            std::string teamName = pair.first;
            Team& team = pair.second;
            const TeamData& data = rawData[teamName];

            // Use league averages calculated from *all* historical data
            if (data.homeMatches > 0) {
                team.homeAttackStrength = (static_cast<double>(data.homeGoalsScored) / data.homeMatches) / leagueAvgHomeGoalsScored;
                team.homeDefenseStrength = (static_cast<double>(data.homeGoalsConceded) / data.homeMatches) / leagueAvgHomeGoalsConceded;
            } else { // No recent home games, use overall strength as fallback
                team.homeAttackStrength = loadedTeams[teamName].homeAttackStrength;
                team.homeDefenseStrength = loadedTeams[teamName].homeDefenseStrength;
            }
            if (data.awayMatches > 0) {
                team.awayAttackStrength = (static_cast<double>(data.awayGoalsScored) / data.awayMatches) / leagueAvgAwayGoalsScored;
                team.awayDefenseStrength = (static_cast<double>(data.awayGoalsConceded) / data.awayMatches) / leagueAvgAwayGoalsConceded;
            } else { // No recent away games, use overall strength as fallback
                team.awayAttackStrength = loadedTeams[teamName].awayAttackStrength;
                team.awayDefenseStrength = loadedTeams[teamName].awayDefenseStrength;
            }

            // NOTE: Form for corners is not calculated, we'll use overall corner strength
            team.homeCornerAttackStrength = loadedTeams[teamName].homeCornerAttackStrength;
            team.homeCornerDefenseStrength = loadedTeams[teamName].homeCornerDefenseStrength;
            team.awayCornerAttackStrength = loadedTeams[teamName].awayCornerAttackStrength;
            team.awayCornerDefenseStrength = loadedTeams[teamName].awayCornerDefenseStrength;
        }
        return formTeams;
    }

    // --- NEW: Head-to-Head Analysis ---
    H2HStats getHeadToHeadStats(const std::string& homeTeam, 
                                const std::string& awayTeam, 
                                const std::string& beforeDate = "",
                                int maxMatches = 10) {
        H2HStats stats;
        
        auto cutoffDate = std::chrono::system_clock::time_point::max();
        if (!beforeDate.empty()) {
            cutoffDate = parseDate(beforeDate);
        }
        
        // Check if teams exist
        if (loadedTeams.find(homeTeam) == loadedTeams.end() || 
            loadedTeams.find(awayTeam) == loadedTeams.end()) {
            return stats;
        }
        
        const Team& team = loadedTeams[homeTeam];
        int totalGoalsHome = 0, totalGoalsAway = 0;
        int bttsCount = 0, over25Count = 0;
        
        // Search through all match history
        for (auto it = team.matchHistory.rbegin(); 
             it != team.matchHistory.rend() && stats.totalMatches < maxMatches; 
             ++it) {
            const MatchResult& match = *it;
            
            // Only matches before cutoff date
            if (match.date >= cutoffDate) continue;
            
            // Check if this match involves both teams
            bool isH2H = false;
            bool homeTeamWasHome = false;
            
            if (match.homeTeamName == homeTeam && match.awayTeamName == awayTeam) {
                isH2H = true;
                homeTeamWasHome = true;
            } else if (match.homeTeamName == awayTeam && match.awayTeamName == homeTeam) {
                isH2H = true;
                homeTeamWasHome = false;
            }
            
            if (!isH2H) continue;
            
            // Add to recent H2H list
            stats.recentH2H.push_back(match);
            stats.totalMatches++;
            
            // Calculate statistics based on perspective of current fixture
            int goalsForHomeTeam, goalsForAwayTeam;
            if (homeTeamWasHome) {
                goalsForHomeTeam = match.homeGoals;
                goalsForAwayTeam = match.awayGoals;
            } else {
                goalsForHomeTeam = match.awayGoals;
                goalsForAwayTeam = match.homeGoals;
            }
            
            totalGoalsHome += goalsForHomeTeam;
            totalGoalsAway += goalsForAwayTeam;
            
            // Determine result
            if (goalsForHomeTeam > goalsForAwayTeam) {
                stats.homeTeamWins++;
            } else if (goalsForAwayTeam > goalsForHomeTeam) {
                stats.awayTeamWins++;
            } else {
                stats.draws++;
            }
            
            // BTTS and Over 2.5
            if (goalsForHomeTeam > 0 && goalsForAwayTeam > 0) bttsCount++;
            if ((goalsForHomeTeam + goalsForAwayTeam) > 2) over25Count++;
        }
        
        // Calculate averages and percentages
        if (stats.totalMatches > 0) {
            stats.avgHomeGoals = static_cast<double>(totalGoalsHome) / stats.totalMatches;
            stats.avgAwayGoals = static_cast<double>(totalGoalsAway) / stats.totalMatches;
            stats.bttsPercentage = (static_cast<double>(bttsCount) / stats.totalMatches) * 100.0;
            stats.over25Percentage = (static_cast<double>(over25Count) / stats.totalMatches) * 100.0;
        }
        
        return stats;
    }

    // --- Getters ---
    std::map<std::string, Team> getTeams() const { return loadedTeams; }
    std::vector<Fixture> getUpcomingFixtures() const { return upcomingFixtures; }

    // NEW: Pass the correct averages
    double getLeagueAvgHomeGoals() const { return leagueAvgHomeGoalsScored; }
    double getLeagueAvgAwayGoals() const { return leagueAvgAwayGoalsScored; }
    double getLeagueAvgHomeCorners() const { return leagueAvgHomeCorners; }
    double getLeagueAvgAwayCorners() const { return leagueAvgAwayCorners; }

private:
    std::chrono::system_clock::time_point parseDate(const std::string& dateStr) {
        std::tm tm = {};
        std::stringstream ss(dateStr);
        if (dateStr.find('/') != std::string::npos) {
            ss >> std::get_time(&tm, "%d/%m/%Y");
        } else if (dateStr.find('-') != std::string::npos) {
            ss >> std::get_time(&tm, "%Y-%m-%d");
        } else {
            return std::chrono::system_clock::from_time_t(0); // Invalid format
        }
        if (ss.fail()) {
            return std::chrono::system_clock::from_time_t(0); // Parsing failed
        }
        tm.tm_isdst = -1; // Not known
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    }

    bool isdigit(char c) { return c >= '0' && c <= '9'; }
};

#endif // DATALOADER_H