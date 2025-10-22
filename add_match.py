import csv
from datetime import datetime

csv_file = 'FootballApp/live_data.csv'

date = input("Match date (DD/MM/YYYY): ")
time = input("Time (HH:MM): ")
home = input("Home team: ")
away = input("Away team: ")
home_goals = input("Home goals: ")
away_goals = input("Away goals: ")

row = ['TR1', date, time, home, away, home_goals, away_goals,
       '', '', '', '', '', '', '', '', '', 0, 0]

with open(csv_file, 'a', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(row)

print(f"✅ Added: {home} {home_goals}-{away_goals} {away} ({date})")
