#ifndef TEAM_H
#define TEAM_H

#include <string>
#include <vector>
#include "DataTypes.h" // Include the header for MatchResult

class Team {
public:
    std::string name;

    // Relative GOAL strengths
    double homeAttackStrength = 1.0;
    double homeDefenseStrength = 1.0;
    double awayAttackStrength = 1.0;
    double awayDefenseStrength = 1.0;

    // NEW: Relative CORNER strengths
    double homeCornerAttackStrength = 1.0; // How many corners team wins at home
    double homeCornerDefenseStrength = 1.0; // How many corners team concedes at home
    double awayCornerAttackStrength = 1.0; // How many corners team wins away
    double awayCornerDefenseStrength = 1.0; // How many corners team concedes away

    // Match history
    std::vector<MatchResult> matchHistory;

    // Constructors
    Team() : name("") {}
    Team(const std::string& teamName) : name(teamName) {}
};

#endif // TEAM_H