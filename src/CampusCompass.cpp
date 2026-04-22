#include "CampusCompass.h"

#include <cctype>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

CampusCompass::CampusCompass() {
    // initialize your object
}
int CampusCompass::ParseTimeToMinutes(const string& timeStr) const {
    if (timeStr.length() != 5 || timeStr[2] != ':') {
        return -1;
    }

    string hourPart = timeStr.substr(0, 2);
    string minutePart = timeStr.substr(3, 2);

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
        parts.push_back(current);
    }

    return parts;
}

bool CampusCompass::ParseCSV(const string &edges_filepath, const string &classes_filepath) {
    ifstream classesFile(classes_filepath);

    if (!classesFile.is_open()) {
        return false;
    }

    classes.clear();
    students.clear();

    string line;

    getline(classesFile, line);

    while (getline(classesFile, line)) {
        if (line.empty()) {
            continue;
        }

        vector<string> parts = SplitCSVLine(line);

        if (parts.size() < 4) {
            continue;
        }

        string classCode = parts[0];

        if (!IsValidClassCode(classCode)) {
            continue;
        }

        int locationID = stoi(parts[1]);
        int startMinutes = ParseTimeToMinutes(parts[2]);
        int endMinutes = ParseTimeToMinutes(parts[3]);

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
        return false;
    }

    if (command.rfind("insert ", 0) == 0) {
        size_t firstQuote = command.find('"');
        size_t secondQuote = command.find('"', firstQuote + 1);

        if (firstQuote == string::npos || secondQuote == string::npos || secondQuote <= firstQuote) {
            return false;
        }

        string name = command.substr(firstQuote + 1, secondQuote - firstQuote - 1);

        string rest = command.substr(secondQuote + 1);
        vector<string> tokens = TokenizeBySpaces(rest);

        if (tokens.size() < 4) {
            return false;
        }

        string ufid = tokens[0];

        int residenceLocationID;
        int numClasses;

        try {
            residenceLocationID = stoi(tokens[1]);
            numClasses = stoi(tokens[2]);
        } catch (...) {
            return false;
        }

        if (numClasses < 1 || numClasses > 6) {
            return false;
        }

        if (static_cast<int>(tokens.size()) != 3 + numClasses) {
            return false;
        }

        vector<string> classCodes;
        for (int i = 0; i < numClasses; i++) {
            classCodes.push_back(tokens[3 + i]);
        }

        return InsertStudent(name, ufid, residenceLocationID, classCodes);
    }

    vector<string> tokens = TokenizeBySpaces(command);

    if (tokens.size() == 2 && tokens[0] == "remove") {
        return RemoveStudent(tokens[1]);
    }

    if (tokens.size() == 3 && tokens[0] == "dropClass") {
        return DropClass(tokens[1], tokens[2]);
    }

    if (tokens.size() == 4 && tokens[0] == "replaceClass") {
        return ReplaceClass(tokens[1], tokens[2], tokens[3]);
    }

    if (tokens.size() == 2 && tokens[0] == "removeClass") {
        return RemoveClassForAll(tokens[1]) != -1;
    }

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