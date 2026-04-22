#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include<utility>

using namespace std;

class CampusCompass {
private:
    struct Edge {
        int to;
        int weight;
    };

    struct ClassInfo {
        int locationID;
        int startMinutes;
        int endMinutes;
    };

    struct Student {
        string name;
        string ufid;
        int residenceLoKid;
        std::set<string> classCodes;
    };

    // Think about what member variables you need to initialize
    unordered_map<int, vector<Edge>> graph;
    unordered_map<string, bool> EdgeOpen;
    unordered_map<string, ClassInfo> classes;
    unordered_map<string, Student> students;
    // perhaps some graph representation?

    bool IsValidUFID(const string& ufid) const;
    bool IsValidName(const string& name) const;
    bool IsValidClassCode(const string& code) const;
    bool ClassExists(const string& code) const;

    int ParseTimeToMinutes(const string& timeStr) const;
    vector<string> SplitCSVLine(const string& line) const;
    vector<string> TokenizeBySpaces(const string& text) const;


public:
    // Think about what helper functions you will need in the algorithm
    CampusCompass(); // constructor
    bool ParseCSV(const string &edges_filepath, const string &classes_filepath);
    bool ParseCommand(const string &command);

    bool InsertStudent(const string& name, const string& ufid, int residence, const vector<string>& classCodes);
    bool RemoveStudent(const string& ufid);
    bool DropClass(const string& ufid, const string& classCode);
    bool ReplaceClass(const string& ufid, const string& oldClass, const string& newClass);
    int RemoveClassForAll(const string& classCode);
    bool ToggleEdgeClosures(const vector<pair<int,int>>& edges);
    string CheckEdgeStatus(int a, int b) const;
    bool IsConnected(int a, int b) const;
    string GetShortestEdgesReport(const string& ufid) const;
    string GetStudentZoneReport(const string& ufid) const;
    string VerifyScheduleReport(const string& ufid) const;


};

