#include "CampusCompass.h"

#include <cctype>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <limits>
#include <functional>
#include <algorithm>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}
int CampusCompass::ParseTimeToMinutes(const string& timeStr) const {
    size_t colonPos = timeStr.find(':');

    if (colonPos == string::npos) {
        return -1;
    }

    string hourPart = timeStr.substr(0, colonPos);
    string minutePart = timeStr.substr(colonPos + 1);

    if (hourPart.empty() || minutePart.length() != 2) {
        return -1;
    }

    for (char c : hourPart) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return -1;
        }
    }

    for (char c : minutePart) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return -1;
        }
    }

    int hours = stoi(hourPart);
    int minutes = stoi(minutePart);

    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        return -1;
    }

    return hours * 60 + minutes;
}

vector<string> CampusCompass::SplitCSVLine(const string& line) const {
    vector<string> parts;
    string current;
    stringstream ss(line);

    while (getline(ss, current, ',')) {
        parts.push_back(Trim(current));
    }

    return parts;
}

string CampusCompass::Trim(const string& text) const {
    size_t start = 0;
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) {
        start++;
    }

    size_t end = text.size();
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) {
        end--;
    }

    return text.substr(start, end - start);
}

bool CampusCompass::ParseCSV(const string& edges_filepath, const string& classes_filepath) {
    ifstream edgesFile(edges_filepath);
    ifstream classesFile(classes_filepath);

    if (!edgesFile.is_open() || !classesFile.is_open()) {
        return false;
    }

    classes.clear();
    students.clear();
    EdgeOpen.clear();
    adjList.clear();

    string line;

    getline(edgesFile, line);

    while (getline(edgesFile, line)) {
        if (line.empty()) {
            continue;
        }

        vector<string> parts = SplitCSVLine(line);

        if (parts.size() < 5) {
            continue;
        }

        int location1 = stoi(parts[0]);
        int location2 = stoi(parts[1]);
        int weight = stoi(parts[4]);

        EdgeOpen[MakeEdgeKey(location1, location2)] = true;
        adjList[location1].push_back({location2, weight});
        adjList[location2].push_back({location1, weight});
    }

    getline(classesFile, line);

    while (getline(classesFile, line)) {
        if (line.empty()) {
            continue;
        }

        vector<string> parts = SplitCSVLine(line);

        if (parts.size() < 4) {
            continue;
        }

        string classCode = Trim(parts[0]);

        if (!IsValidClassCode(classCode)) {
            continue;
        }

        int locationID = stoi(Trim(parts[1]));
        int startMinutes = ParseTimeToMinutes(Trim(parts[2]));
        int endMinutes = ParseTimeToMinutes(Trim(parts[3]));

        if (startMinutes == -1 || endMinutes == -1) {
            continue;
        }

        ClassInfo info;
        info.locationID = locationID;
        info.startMinutes = startMinutes;
        info.endMinutes = endMinutes;

        classes[classCode] = info;
    }
    return true;
}

vector<string> CampusCompass::TokenizeBySpaces(const string& text) const {
    vector<string> tokens;
    string token;
    stringstream ss(text);

    while (ss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

bool CampusCompass::ParseCommand(const string& command) {
    if (command.empty()) {
        cout << "unsuccessful" << '\n';
        return false;
    }

    if (command.rfind("insert ", 0) == 0) {
        size_t firstQuote = command.find('"');
        size_t secondQuote = command.find('"', firstQuote + 1);

        if (firstQuote == string::npos || secondQuote == string::npos || secondQuote <= firstQuote) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        string name = command.substr(firstQuote + 1, secondQuote - firstQuote - 1);

        string rest = command.substr(secondQuote + 1);
        vector<string> tokens = TokenizeBySpaces(rest);

        if (tokens.size() < 4) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        string ufid = tokens[0];
        int residenceLocationID;
        int numClasses;

        try {
            residenceLocationID = stoi(tokens[1]);
            numClasses = stoi(tokens[2]);
        } catch (...) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        if (numClasses < 1 || numClasses > 6) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        if (static_cast<int>(tokens.size()) != 3 + numClasses) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        vector<string> classCodes;
        for (int i = 0; i < numClasses; i++) {
            classCodes.push_back(tokens[3 + i]);
        }

        bool ok = InsertStudent(name, ufid, residenceLocationID, classCodes);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    vector<string> tokens = TokenizeBySpaces(command);

    if (tokens.size() == 2 && tokens[0] == "remove") {
        bool ok = RemoveStudent(tokens[1]);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    if (tokens.size() == 3 && tokens[0] == "dropClass") {
        bool ok = DropClass(tokens[1], tokens[2]);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    if (tokens.size() == 4 && tokens[0] == "replaceClass") {
        bool ok = ReplaceClass(tokens[1], tokens[2], tokens[3]);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    if (tokens.size() == 2 && tokens[0] == "removeClass") {
        int result = RemoveClassForAll(tokens[1]);
        if (result == -1) {
            cout << "unsuccessful" << '\n';
            return false;
        }
        cout << result << '\n';
        return true;
    }

    if (tokens.size() >= 4 && tokens[0] == "toggleEdgesClosure") {
        int pairCount;

        try {
            pairCount = stoi(tokens[1]);
        } catch (...) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        if (pairCount <= 0 || static_cast<int>(tokens.size()) != 2 + 2 * pairCount) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        vector<pair<int, int>> edges;
        for (int i = 0; i < pairCount; i++) {
            int a, b;
            try {
                a = stoi(tokens[2 + 2 * i]);
                b = stoi(tokens[3 + 2 * i]);
            } catch (...) {
                cout << "unsuccessful" << '\n';
                return false;
            }
            edges.push_back({a, b});
        }

        bool ok = ToggleEdgeClosures(edges);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    if (tokens.size() == 3 && tokens[0] == "checkEdgeStatus") {
        int a, b;
        try {
            a = stoi(tokens[1]);
            b = stoi(tokens[2]);
        } catch (...) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        cout << CheckEdgeStatus(a, b) << '\n';
        return true;
    }

    if (tokens.size() == 3 && tokens[0] == "isConnected") {
        int a, b;
        try {
            a = stoi(tokens[1]);
            b = stoi(tokens[2]);
        } catch (...) {
            cout << "unsuccessful" << '\n';
            return false;
        }

        bool ok = IsConnected(a, b);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    }

    if (tokens.size() == 2 && tokens[0] == "printShortestEdges") {
        string report = GetShortestEdgesReport(tokens[1]);
        cout << report << '\n';
        return report != "unsuccessful";
    }

    if (tokens.size() == 2 && tokens[0] == "printStudentZone") {
        string report = GetStudentZoneReport(tokens[1]);
        cout << report << '\n';
        return report != "unsuccessful";
    }

    if (tokens.size() == 2 && tokens[0] == "verifySchedule") {
        string report = VerifyScheduleReport(tokens[1]);
        cout << report << '\n';
        return report != "unsuccessful";
    }

    cout << "unsuccessful" << '\n';
    return false;
}


bool CampusCompass::IsValidUFID(const string& ufid) const {
    if (ufid.length() != 8) {
        return false;
    }

    for (char c : ufid) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}
bool CampusCompass::IsValidName(const string& name) const {
    if (name.empty()) {
        return false;
    }

    for (char c : name) {
        if (!(isalpha(static_cast<unsigned char>(c)) || c == ' ')) {
            return false;
        }
    }

    return true;
}

bool CampusCompass::IsValidClassCode(const string& code) const {
    if (code.length() != 7) {
        return false;
    }

    for (int i = 0; i < 3; i++) {
        if (!isupper(static_cast<unsigned char>(code[i]))) {
            return false;
        }
    }

    for (int i = 3; i < 7; i++) {
        if (!isdigit(static_cast<unsigned char>(code[i]))) {
            return false;
        }
    }

    return true;
}

bool CampusCompass::ClassExists(const string& code) const {
    return classes.find(code) != classes.end();
}

bool CampusCompass::InsertStudent(const string& name, const string& ufid, int residence, const vector<string>& classCodes) {
    if (!IsValidName(name)) {
        return false;
    }

    if (!IsValidUFID(ufid)) {
        return false;
    }

    if (students.find(ufid) != students.end()) {
        return false;
    }

    if (classCodes.size() < 1 || classCodes.size() > 6) {
        return false;
    }

    set<string> uniqueClasses;

    for (const string& code : classCodes) {
        if (!IsValidClassCode(code)) {
            return false;
        }

        if (!ClassExists(code)) {
            return false;
        }

        uniqueClasses.insert(code);
    }

    if (uniqueClasses.size() != classCodes.size()) {
        return false;
    }

    Student s;
    s.name = name;
    s.ufid = ufid;
    s.residenceLoKid = residence;
    s.classCodes = uniqueClasses;

    students[ufid] = s;
    return true;
}

bool CampusCompass::RemoveStudent(const string& ufid) {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return false;
    }

    students.erase(it);
    return true;
}

bool CampusCompass::DropClass(const string& ufid, const string& classCode) {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return false;
    }

    Student& student = it->second;

    auto classIt = student.classCodes.find(classCode);
    if (classIt == student.classCodes.end()) {
        return false;
    }

    student.classCodes.erase(classIt);

    if (student.classCodes.empty()) {
        students.erase(it);
    }

    return true;
}

bool CampusCompass::ReplaceClass(const string& ufid, const string& oldClassCode, const string& newClassCode) {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return false;
    }

    Student& student = it->second;

    if (student.classCodes.find(oldClassCode) == student.classCodes.end()) {
        return false;
    }

    if (!ClassExists(newClassCode)) {
        return false;
    }

    if (student.classCodes.find(newClassCode) != student.classCodes.end()) {
        return false;
    }

    student.classCodes.erase(oldClassCode);
    student.classCodes.insert(newClassCode);

    return true;
}

int CampusCompass::RemoveClassForAll(const string& classCode) {
    if (!IsValidClassCode(classCode)) {
        return -1;
    }

    if (!ClassExists(classCode)) {
        return -1;
    }

    int affectedCount = 0;
    vector<string> studentsToRemove;

    for (auto& pair : students) {
        Student& student = pair.second;

        auto classIt = student.classCodes.find(classCode);
        if (classIt != student.classCodes.end()) {
            student.classCodes.erase(classIt);
            affectedCount++;

            if (student.classCodes.empty()) {
                studentsToRemove.push_back(student.ufid);
            }
        }
    }

    if (affectedCount == 0) {
        return -1;
    }

    for (const string& ufid : studentsToRemove) {
        students.erase(ufid);
    }

    return affectedCount;
}

string CampusCompass::MakeEdgeKey(int a, int b) const {
    if (a > b) {
        swap(a, b);
    }

    return to_string(a) + "|" + to_string(b);
}

bool CampusCompass::IsEdgePresent(int a, int b) const {
    return EdgeOpen.find(MakeEdgeKey(a, b)) != EdgeOpen.end();
}

bool CampusCompass::IsEdgeOpenInternal(int a, int b) const {
    auto it = EdgeOpen.find(MakeEdgeKey(a, b));

    if (it == EdgeOpen.end()) {
        return false;
    }

    return it->second;
}

bool CampusCompass::ToggleEdgeClosures(const vector<pair<int, int>>& edges) {
    for (const auto& edge : edges) {
        int a = edge.first;
        int b = edge.second;

        string key = MakeEdgeKey(a, b);

        if (EdgeOpen.find(key) == EdgeOpen.end()) {
            return false;
        }

        EdgeOpen[key] = !EdgeOpen[key];
    }

    return true;
}

string CampusCompass::CheckEdgeStatus(int a, int b) const {
    if (!IsEdgePresent(a, b)) {
        return "DNE";
    }

    if (IsEdgeOpenInternal(a, b)) {
        return "open";
    }

    return "closed";
}

bool CampusCompass::BFSConnectedInternal(int start, int end) const {
    if (adjList.find(start) == adjList.end() ||
        adjList.find(end) == adjList.end()) {
        return false;
        }

    if (start == end) {
        return true;
    }

    unordered_set<int> visited;
    queue<int> q;

    visited.insert(start);
    q.push(start);

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        for (const Edge& edge : adjList.at(current)) {
            int neighbor = edge.to;

            if (!IsEdgeOpenInternal(current, neighbor)) {
                continue;
            }

            if (visited.find(neighbor) != visited.end()) {
                continue;
            }

            if (neighbor == end) {
                return true;
            }

            visited.insert(neighbor);
            q.push(neighbor);
        }
    }

    return false;
}

bool CampusCompass::IsConnected(int a, int b) const {
    return BFSConnectedInternal(a, b);
}

unordered_map<int, int> CampusCompass::DijkstraDistancesInternal(int start) const {
    const int INF = numeric_limits<int>::max();

    unordered_map<int, int> dist;

    for (const auto& pair : adjList) {
        dist[pair.first] = INF;
    }

    if (adjList.find(start) == adjList.end()) {
        return dist;
    }

    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        int currentDist = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        if (currentDist > dist[currentNode]) {
            continue;
        }

        for (const Edge& edge : adjList.at(currentNode)) {
            int neighbor = edge.to;
            int weight = edge.weight;

            if (!IsEdgeOpenInternal(currentNode, neighbor)) {
                continue;
            }

            int newDist = currentDist + weight;

            if (newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                pq.push({newDist, neighbor});
            }
        }
    }

    return dist;
}

string CampusCompass::GetShortestEdgesReport(const string& ufid) const {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return "unsuccessful";
    }

    const Student& student = it->second;
    unordered_map<int, int> dist = DijkstraDistancesInternal(student.residenceLoKid);

    const int INF = numeric_limits<int>::max();

    ostringstream out;
    out << "Time For Shortest Edges: " << student.name;

    for (const string& classCode : student.classCodes) {
        int classLocation = classes.at(classCode).locationID;
        int time = -1;

        auto distIt = dist.find(classLocation);
        if (distIt != dist.end() && distIt->second != INF) {
            time = distIt->second;
        }

        out << '\n' << classCode << ": " << time;
    }

    return out.str();
}

unordered_map<int, int> CampusCompass::DijkstraWithParentInternal(
    int start,
    unordered_map<int, int>& parent
) const {
    const int INF = numeric_limits<int>::max();

    unordered_map<int, int> dist;
    parent.clear();

    for (const auto& pair : adjList) {
        dist[pair.first] = INF;
    }

    if (adjList.find(start) == adjList.end()) {
        return dist;
    }

    using P = pair<int, int>;
    priority_queue<P, vector<P>, greater<P>> pq;

    dist[start] = 0;
    parent[start] = -1;
    pq.push({0, start});

    while (!pq.empty()) {
        int currentDist = pq.top().first;
        int currentNode = pq.top().second;
        pq.pop();

        if (currentDist > dist[currentNode]) {
            continue;
        }

        for (const Edge& edge : adjList.at(currentNode)) {
            int neighbor = edge.to;
            int weight = edge.weight;

            if (!IsEdgeOpenInternal(currentNode, neighbor)) {
                continue;
            }

            int newDist = currentDist + weight;

            if (newDist < dist[neighbor]) {
                dist[neighbor] = newDist;
                parent[neighbor] = currentNode;
                pq.push({newDist, neighbor});
            }
        }
    }

    return dist;
}

int CampusCompass::ComputeStudentZoneCostInternal(const Student& student) const {
    unordered_map<int, int> parent;
    unordered_map<int, int> dist = DijkstraWithParentInternal(student.residenceLoKid, parent);

    const int INF = numeric_limits<int>::max();

    unordered_set<int> zoneNodes;
    zoneNodes.insert(student.residenceLoKid);

    bool hasReachableClass = false;

    for (const string& classCode : student.classCodes) {
        int target = classes.at(classCode).locationID;

        if (dist.find(target) == dist.end() || dist[target] == INF) {
            continue;
        }

        hasReachableClass = true;

        int current = target;
        while (current != -1) {
            zoneNodes.insert(current);

            auto it = parent.find(current);
            if (it == parent.end()) {
                break;
            }

            current = it->second;
        }
    }

    if (!hasReachableClass) {
        return -1;
    }

    unordered_map<int, vector<pair<int, int>>> inducedGraph;

    for (int node : zoneNodes) {
        for (const Edge& edge : adjList.at(node)) {
            int neighbor = edge.to;
            int weight = edge.weight;

            if (!IsEdgeOpenInternal(node, neighbor)) {
                continue;
            }

            if (zoneNodes.find(neighbor) == zoneNodes.end()) {
                continue;
            }

            inducedGraph[node].push_back({neighbor, weight});
        }
    }

    // Prim's MST
    unordered_set<int> visited;
    using P = pair<int, int>; // (weight, node)
    priority_queue<P, vector<P>, greater<P>> pq;

    int start = *zoneNodes.begin();
    pq.push({0, start});

    int totalCost = 0;

    while (!pq.empty()) {
        int weight = pq.top().first;
        int node = pq.top().second;
        pq.pop();

        if (visited.find(node) != visited.end()) {
            continue;
        }

        visited.insert(node);
        totalCost += weight;

        for (const auto& neighborPair : inducedGraph[node]) {
            int neighbor = neighborPair.first;
            int edgeWeight = neighborPair.second;

            if (visited.find(neighbor) == visited.end()) {
                pq.push({edgeWeight, neighbor});
            }
        }
    }

    if (visited.size() != zoneNodes.size()) {
        return -1;
    }

    return totalCost;
}

string CampusCompass::GetStudentZoneReport(const string& ufid) const {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return "unsuccessful";
    }

    int cost = ComputeStudentZoneCostInternal(it->second);

    if (cost == -1) {
        return "unsuccessful";
    }

    return "Student Zone Cost For " + it->second.name + ": " + to_string(cost);
}

string CampusCompass::VerifyScheduleReport(const string& ufid) const {
    auto it = students.find(ufid);

    if (it == students.end()) {
        return "unsuccessful";
    }

    const Student& student = it->second;

    if (student.classCodes.size() <= 1) {
        return "unsuccessful";
    }

    vector<string> orderedClasses(student.classCodes.begin(), student.classCodes.end());

    sort(orderedClasses.begin(), orderedClasses.end(),
         [&](const string& a, const string& b) {
             if (classes.at(a).startMinutes != classes.at(b).startMinutes) {
                 return classes.at(a).startMinutes < classes.at(b).startMinutes;
             }
             return a < b;
         });

    ostringstream out;
    out << "Schedule Check for " << student.name << ":";

    for (size_t i = 0; i + 1 < orderedClasses.size(); i++) {
        const string& firstClass = orderedClasses[i];
        const string& secondClass = orderedClasses[i + 1];

        int firstLocation = classes.at(firstClass).locationID;
        int secondLocation = classes.at(secondClass).locationID;

        int availableGap = classes.at(secondClass).startMinutes - classes.at(firstClass).endMinutes;

        unordered_map<int, int> dist = DijkstraDistancesInternal(firstLocation);
        const int INF = numeric_limits<int>::max();

        bool possible = false;

        auto distIt = dist.find(secondLocation);
        if (distIt != dist.end() && distIt->second != INF) {
            if (availableGap >= distIt->second) {
                possible = true;
            }
        }

        out << '\n'
            << firstClass << " - " << secondClass << ": "
            << (possible ? "successful" : "unsuccessful");
    }

    return out.str();
}