#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <algorithm>
#include "../FootballLib/DataTypes.h"
#include "../FootballLib/Team.h"
#include "../FootballLib/DataLoader.h"
#include "../FootballLib/Match.h"

// NEW: Function to display Head-to-Head statistics
void displayH2HStats(const H2HStats& h2h, const std::string& homeTeam, const std::string& awayTeam) {
    if (h2h.totalMatches == 0) {
        std::cout << "\n--- Head-to-Head History ---" << std::endl;
        std::cout << "No previous matches found between these teams." << std::endl;
        return;
    }
    
    std::cout << "\n--- Head-to-Head History (Last " << h2h.totalMatches << " matches) ---" << std::endl;
    std::cout << std::fixed << std::setprecision(1);
    
    // Win percentages
    double homeWinPct = (static_cast<double>(h2h.homeTeamWins) / h2h.totalMatches) * 100.0;
    double drawPct = (static_cast<double>(h2h.draws) / h2h.totalMatches) * 100.0;
    double awayWinPct = (static_cast<double>(h2h.awayTeamWins) / h2h.totalMatches) * 100.0;
    
    std::cout << homeTeam << " Wins: " << h2h.homeTeamWins 
              << " (" << homeWinPct << "%)" << std::endl;
    std::cout << "Draws: " << h2h.draws 
              << " (" << drawPct << "%)" << std::endl;
    std::cout << awayTeam << " Wins: " << h2h.awayTeamWins 
              << " (" << awayWinPct << "%)" << std::endl;
    
    // Goal averages
    std::cout << "\nAverage Goals in H2H:" << std::endl;
    std::cout << "  " << homeTeam << ": " << h2h.avgHomeGoals << std::endl;
    std::cout << "  " << awayTeam << ": " << h2h.avgAwayGoals << std::endl;
    std::cout << "  Total per game: " << (h2h.avgHomeGoals + h2h.avgAwayGoals) << std::endl;
    
    // Market stats
    std::cout << "\nHistorical H2H Market Stats:" << std::endl;
    std::cout << "  Both Teams To Score: " << h2h.bttsPercentage << "%" << std::endl;
    std::cout << "  Over 2.5 Goals: " << h2h.over25Percentage << "%" << std::endl;
    
    // Recent matches
    std::cout << "\nRecent H2H Results (most recent first):" << std::endl;
    for (size_t i = 0; i < h2h.recentH2H.size() && i < 5; i++) {
        const auto& match = h2h.recentH2H[i];
        std::cout << "  " << match.dateStr << ": " 
                  << match.homeTeamName << " " << match.homeGoals 
                  << "-" << match.awayGoals << " " 
                  << match.awayTeamName << std::endl;
    }
}

// Updated predictMatch function
void predictMatch(Match& match, const Team& home, const Team& away,
                  double avgHomeGoals, double avgAwayGoals,
                  double avgHomeCorners, double avgAwayCorners) {
    std::cout << "\nSimulating 10,000 matches..." << std::endl;
    match.runFullSimulation(home, away, avgHomeGoals, avgAwayGoals, avgHomeCorners, avgAwayCorners);
    
    std::cout << "\n--- PREDICTION: " << home.name << " vs. " << away.name << " ---" << std::endl;
    std::cout << std::fixed << std::setprecision(1);
    
    // --- Win/Draw/Loss ---
    std::cout << home.name << " Win: " << match.getHomeWinPercent() << "%" << std::endl;
    std::cout << "Draw: " << match.getDrawPercent() << "%" << std::endl;
    std::cout << away.name << " Win: " << match.getAwayWinPercent() << "%" << std::endl;
    
    // --- Goal Totals ---
    std::cout << "\n--- Goal Totals (Over/Under) ---" << std::endl;
    std::cout << "  Over 0.5: " << match.getOver05Percent() << "%" << std::endl;
    std::cout << "  Over 1.5: " << match.getOver15Percent() << "%" << std::endl;
    std::cout << "  Over 2.5: " << match.getOver25Percent() << "%" << std::endl;
    
    // --- Both Teams To Score ---
    std::cout << "\n--- Both Teams To Score ---" << std::endl;
    std::cout << "  Yes (BTTS): " << match.getBttsYesPercent() << "%" << std::endl;
    std::cout << "  No: " << match.getBttsNoPercent() << "%" << std::endl;
    
    // --- Corner Totals ---
    std::cout << "\n--- Corner Totals (Over/Under) ---" << std::endl;
    std::cout << "  Average Total Corners: " << match.getAverageTotalCorners() << std::endl;
    std::cout << "  Over 2.5 Corners: " << match.getCornerPercent(2.5, true) << "%" << std::endl;
    std::cout << "  Over 4.5 Corners: " << match.getCornerPercent(4.5, true) << "%" << std::endl;
    std::cout << "  Over 6.5 Corners: " << match.getCornerPercent(6.5, true) << "%" << std::endl;
    std::cout << "  Over 8.5 Corners: " << match.getCornerPercent(8.5, true) << "%" << std::endl;
    std::cout << "  Over 10.5 Corners: " << match.getCornerPercent(10.5, true) << "%" << std::endl;
    
    // --- Most Likely Scores ---
    std::cout << "\n--- Most Likely Scores ---" << std::endl;
    auto topScores = match.getMostLikelyScores();
    for (const auto& score : topScores) {
        std::cout << "  " << score.first << ": " << score.second << "%" << std::endl;
    }
}

// --- main function ---
int main() {
    DataLoader loader;
    
    // --- UPDATED: Define all data files with your names ---
    std::vector<std::string> dataFiles = {
        "FootballApp/live_data.csv",  // Current season
        "FootballApp/T1.csv",          // Historical season 1
        "FootballApp/T1-2.csv"         // Historical season 2
    };
    
    // --- CHANGED: Call new function ---
    if (!loader.loadMultipleFiles(dataFiles)) {
        std::cerr << "Error loading data files. Exiting." << std::endl;
        return 1;
    }
    
    // --- Fixture loading remains the same ---
    if (!loader.loadFixtures("FootballApp/fixtures.csv")) {
        std::cerr << "Warning: Could not load fixtures.csv. Continuing without fixture list." << std::endl;
    }
    
    std::map<std::string, Team> overallTeams = loader.getTeams();
    std::vector<Fixture> allFixtures = loader.getUpcomingFixtures();
    
    Match match;
    std::string choice;
    
    while(true) {
        std::cout << "\n------------------------------------" << std::endl;
        std::cout << "--- Turkish Super Lig Predictor ---" << std::endl;
        std::cout << "1. Predict Match(es) by Date (using Form)" << std::endl;
        std::cout << "2. Predict All Fixtures from fixtures.csv (using Form)" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Enter choice: ";
        std::getline(std::cin, choice);
        
        if (!choice.empty() && choice[0] == '1') {
            std::string dateStr;
            std::cout << "Enter Match Date (dd/mm/yyyy): ";
            std::getline(std::cin, dateStr);
            
            std::vector<Fixture> matchesOnDate;
            for(const auto& fix : allFixtures) {
                if (fix.dateStr == dateStr) { 
                    matchesOnDate.push_back(fix); 
                }
            }
            
            if (matchesOnDate.empty()) {
                std::cout << "No matches found for " << dateStr << " in fixtures.csv." << std::endl;
                std::cout << "Note: You can still get predictions by adding fixtures to fixtures.csv." << std::endl;
                continue;
            }
            
            std::cout << "\nFound " << matchesOnDate.size() << " match(es) on " << dateStr << ". Predicting using form:" << std::endl;
            
            std::map<std::string, Team> formTeams = loader.calculateFormStrengths(dateStr);
            if (formTeams.empty()) { 
                std::cout << "Error calculating form strengths for date " << dateStr << ". Invalid date?" << std::endl; 
                continue; 
            }
            
            for (const auto& fixture : matchesOnDate) {
                if (formTeams.count(fixture.homeTeamName) && formTeams.count(fixture.awayTeamName)) {
                    std::cout << "\n========================================" << std::endl;
                    std::cout << "=== Predicting: " << fixture.homeTeamName << " vs " << fixture.awayTeamName << " ===" << std::endl;
                    std::cout << "========================================" << std::endl;
                    
                    // NEW: Get and display Head-to-Head stats
                    H2HStats h2h = loader.getHeadToHeadStats(
                        fixture.homeTeamName, 
                        fixture.awayTeamName, 
                        dateStr,  // Only use matches before this date
                        10        // Last 10 H2H matches
                    );
                    displayH2HStats(h2h, fixture.homeTeamName, fixture.awayTeamName);
                    
                    // Then show the prediction
                    predictMatch(match, formTeams[fixture.homeTeamName], formTeams[fixture.awayTeamName],
                                loader.getLeagueAvgHomeGoals(), loader.getLeagueAvgAwayGoals(),
                                loader.getLeagueAvgHomeCorners(), loader.getLeagueAvgAwayCorners());
                } else {
                    std::cout << "\nSkipping: " << fixture.homeTeamName << " vs " << fixture.awayTeamName 
                              << " - team not found after form calculation." << std::endl;
                }
                std::cout << "------------------------------------" << std::endl;
            }
        }
        else if (!choice.empty() && choice[0] == '2') {
            if (allFixtures.empty()) { 
                std::cout << "No upcoming fixtures found in fixtures.csv." << std::endl; 
                continue; 
            }
            
            std::cout << "\n--- Predicting All Fixtures from fixtures.csv (using Form) ---" << std::endl;
            
            for (const auto& fixture : allFixtures) {
                std::map<std::string, Team> formTeams = loader.calculateFormStrengths(fixture.dateStr);
                
                if (formTeams.empty() || !formTeams.count(fixture.homeTeamName) || !formTeams.count(fixture.awayTeamName)) {
                    std::cout << "Skipping: " << fixture.homeTeamName << " vs " << fixture.awayTeamName 
                              << " - team not found or no form data." << std::endl;
                } else {
                    std::cout << "\n========================================" << std::endl;
                    std::cout << "=== " << fixture.dateStr << ": " << fixture.homeTeamName << " vs " << fixture.awayTeamName << " ===" << std::endl;
                    std::cout << "========================================" << std::endl;
                    
                    // NEW: Get and display Head-to-Head stats
                    H2HStats h2h = loader.getHeadToHeadStats(
                        fixture.homeTeamName, 
                        fixture.awayTeamName, 
                        fixture.dateStr,
                        10
                    );
                    displayH2HStats(h2h, fixture.homeTeamName, fixture.awayTeamName);
                    
                    std::cout << "\n--- Using Form Strengths (Last 5 Games before " << fixture.dateStr << ") ---" << std::endl;
                    predictMatch(match, formTeams[fixture.homeTeamName], formTeams[fixture.awayTeamName],
                                loader.getLeagueAvgHomeGoals(), loader.getLeagueAvgAwayGoals(),
                                loader.getLeagueAvgHomeCorners(), loader.getLeagueAvgAwayCorners());
                }
                std::cout << "------------------------------------" << std::endl;
            }
        }
        else if (!choice.empty() && choice[0] == '3') {
            break;
        }
        else {
            if (!choice.empty()) { 
                std::cout << "Invalid choice." << std::endl; 
            }
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}