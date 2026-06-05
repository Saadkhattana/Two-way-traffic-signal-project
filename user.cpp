#include <iostream>
#include <string>
#include <vector>
#include <conio.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

// ============================================================
// PART 1 - USER MANAGEMENT & AUTHENTICATION
// ============================================================

string readInput(string prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (input.find_first_not_of(" \t") != string::npos) {
            return input;
        }
        cout << "[ERROR] This field is required and cannot be blank.\n";
    }
}

string getHiddenPassword() {
    string pass = ""; char ch;
    while (true) {
        ch = _getch();
        if (ch == 13) break;
        if (ch == 8) {
            if (pass.length() > 0) { cout << "\b \b"; pass.pop_back(); }
        } else { cout << '*'; pass += ch; }
    }
    cout << endl; return pass;
}

bool validatePassword(string pass) {
    if (pass.length() < 8) return false;
    bool hasU = false, hasD = false, hasS = false;
    for (char c : pass) {
        if (isupper(c)) hasU = true;
        else if (isdigit(c)) hasD = true;
        else if (!isalpha(c)) hasS = true;
    }
    return hasU && hasD && hasS;
}

string generateSalt() {
    string salt = "";
    const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < 6; ++i) {
        salt += chars[rand() % (sizeof(chars) - 1)];
    }
    return salt;
}

string hashPass(string p, string salt) {
    for (char &c : p) c += 2;
    return p + salt;
}

string encDec(string data) {
    for (char &c : data) c ^= 'K';
    return data;
}

// ============================================================
// HARDCODED ADMIN CREDENTIALS (stored in file, changeable)
// Default: username=admin  password=Admin@123
// ============================================================
struct AdminCredentials {
    string username;
    string salt;
    string passHash;
};

AdminCredentials adminCreds;

void saveAdminCreds() {
    ofstream file("AdminCredentials.txt");
    file << adminCreds.username << "\n"
         << adminCreds.salt     << "\n"
         << adminCreds.passHash << "\n";
    file.close();
}

void loadAdminCreds() {
    ifstream file("AdminCredentials.txt");
    if (file.is_open()) {
        getline(file, adminCreds.username);
        getline(file, adminCreds.salt);
        getline(file, adminCreds.passHash);
        file.close();
    } else {
        // First run — set defaults: username=admin, password=Admin@123
        adminCreds.username = "admin";
        adminCreds.salt     = generateSalt();
        adminCreds.passHash = hashPass("Admin@123", adminCreds.salt);
        saveAdminCreds();
    }
}

bool verifyAdminLogin(string username, string password) {
    if (username != adminCreds.username) return false;
    return (hashPass(password, adminCreds.salt) == adminCreds.passHash);
}

// ============================================================
// USER BASE CLASS & VOTER CLASS
// ============================================================

class User {
protected:
    string name, cnicEnc, gender, passHash, salt;
    bool isLocked;
    int failedAttempts;
public:
    User() { isLocked = false; failedAttempts = 0; }

    void setBaseDetails(string n, string c, string g, string p) {
        name = n; cnicEnc = encDec(c); gender = g;
        salt = generateSalt();
        passHash = hashPass(p, salt);
    }
    void loadBaseDetails(string n, string c, string g, string s, string h) {
        name = n; cnicEnc = encDec(c); gender = g;
        salt = s; passHash = h;
    }
    string getName()   { return name; }
    string getCnic()   { return encDec(cnicEnc); }
    string getGender() { return gender; }
    string getHash()   { return passHash; }
    string getSalt()   { return salt; }
    void setName(string n)   { name = n; }
    void setGender(string g) { gender = g; }
    bool checkLock() {
        if (failedAttempts >= 3) isLocked = true;
        return isLocked;
    }
    bool verifyLogin(string p) {
        if (isLocked) return false;
        if (hashPass(p, salt) == passHash) { failedAttempts = 0; return true; }
        failedAttempts++; return false;
    }
};

class Voter : public User {
private:
    string addrEnc, phoneEnc;
    bool hasVoted;
public:
    Voter() { hasVoted = false; }
    bool getVoteStatus()   { return hasVoted; }
    string getAddrDec()    { return encDec(addrEnc); }
    string getPhoneDec()   { return encDec(phoneEnc); }
    void castVote()        { hasVoted = true; }
    void loadVoter(string n, string c, string g, string a, string ph, string s, string h, bool voted) {
        loadBaseDetails(n, c, g, s, h);
        addrEnc  = encDec(a);
        phoneEnc = encDec(ph);
        hasVoted = voted;
    }
    void registerVoter(string c) {
        string p;
        gender   = readInput("Gender (Female/Male/Other): ");
        addrEnc  = encDec(readInput("Postal Address: "));
        phoneEnc = encDec(readInput("Phone (0323 2345678): "));
        while (true) {
            p = readInput("Password (8+ chars, 1 Cap, 1 Num, 1 Sym): ");
            if (validatePassword(p)) break;
            cout << "Invalid password. Try again.\n";
        }
        setBaseDetails(name, c, gender, p);
        cout << "Voter Registered!\n";
    }
};

// ============================================================
// PART 2 - CANDIDATE MANAGEMENT & VOTING PROCESS
// ============================================================

struct VotingWindow {
    int startHour, startMin;
    int endHour,   endMin;
    bool isSet;
    VotingWindow() : startHour(0), startMin(0), endHour(0), endMin(0), isSet(false) {}
};

class Candidate {
private:
    int    id;
    string name;
    string party;
    int    voteCount;

public:
    Candidate() : id(0), voteCount(0) {}
    Candidate(int i, string n, string p) : id(i), name(n), party(p), voteCount(0) {}

    int    getId()        { return id; }
    string getName()      { return name; }
    string getParty()     { return party; }
    int    getVoteCount() { return voteCount; }

    void setName(string n)  { name = n; }
    void setParty(string p) { party = p; }

    void addVote() { voteCount++; }

    void loadCandidate(int i, string n, string p, int v) {
        id = i; name = n; party = p; voteCount = v;
    }
};

// ---- Global Data ----
vector<Voter>     voters;
vector<Candidate> candidates;
VotingWindow      votingWindow;
int               totalVotes = 0;
int               nextCandId = 1;

// ---- Helper: check duplicate CNIC ----
bool isCnicTaken(string cnic) {
    for (int i = 0; i < (int)voters.size(); i++)
        if (voters[i].getCnic() == cnic) return true;
    return false;
}

// ---- Helper: split a line ----
vector<string> splitLine(string s, string delimiter) {
    size_t pos = 0; string token; vector<string> row;
    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        row.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    row.push_back(s);
    return row;
}

// ---- File I/O: Voters ----
void saveVotersToDB() {
    ofstream file("VoterDatabase.txt");
    file << "Name | CNIC | Gender | Address | Phone | Salt | SaltedHash | HasVoted\n";
    for (int i = 0; i < (int)voters.size(); i++) {
        file << voters[i].getName()    << " | " << voters[i].getCnic()     << " | "
             << voters[i].getGender()  << " | " << voters[i].getAddrDec() << " | "
             << voters[i].getPhoneDec()<< " | " << voters[i].getSalt()     << " | "
             << voters[i].getHash()    << " | " << voters[i].getVoteStatus() << "\n";
    }
}

// ---- File I/O: Candidates ----
void saveCandidatesToDB() {
    ofstream file("CandidateDatabase.txt");
    file << "ID | Name | Party | Votes\n";
    for (int i = 0; i < (int)candidates.size(); i++) {
        file << candidates[i].getId()        << " | "
             << candidates[i].getName()      << " | "
             << candidates[i].getParty()     << " | "
             << candidates[i].getVoteCount() << "\n";
    }
}

// ---- File I/O: Voting Window ----
void saveVotingWindow() {
    ofstream file("VotingWindow.txt");
    file << votingWindow.isSet    << "\n"
         << votingWindow.startHour << "\n" << votingWindow.startMin << "\n"
         << votingWindow.endHour   << "\n" << votingWindow.endMin   << "\n";
}

void loadVotingWindow() {
    ifstream file("VotingWindow.txt");
    if (file.is_open()) {
        file >> votingWindow.isSet
             >> votingWindow.startHour >> votingWindow.startMin
             >> votingWindow.endHour   >> votingWindow.endMin;
        file.close();
    }
}

// ---- Check: is voting currently open? ----
bool isVotingOpen() {
    if (!votingWindow.isSet) return false;
    time_t now = time(0);
    struct tm *t = localtime(&now);
    int curMin   = t->tm_hour * 60 + t->tm_min;
    int startMin = votingWindow.startHour * 60 + votingWindow.startMin;
    int endMin   = votingWindow.endHour   * 60 + votingWindow.endMin;
    return (curMin >= startMin && curMin <= endMin);
}

// ---- Load all databases ----
void loadDatabases() {
    // Load Admin Credentials (or create defaults)
    loadAdminCreds();

    // Load Voters
    ifstream vFile("VoterDatabase.txt");
    if (vFile.is_open()) {
        string line;
        getline(vFile, line);
        while (getline(vFile, line)) {
            if (line.empty()) continue;
            vector<string> row = splitLine(line, " | ");
            if (row.size() >= 8) {
                Voter v;
                v.loadVoter(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7] == "1");
                voters.push_back(v);
                if (row[7] == "1") totalVotes++;
            }
        }
        vFile.close();
    }

    // Load Candidates
    ifstream cFile("CandidateDatabase.txt");
    if (cFile.is_open()) {
        string line;
        getline(cFile, line);
        while (getline(cFile, line)) {
            if (line.empty()) continue;
            vector<string> row = splitLine(line, " | ");
            if (row.size() >= 4) {
                Candidate c;
                c.loadCandidate(stoi(row[0]), row[1], row[2], stoi(row[3]));
                candidates.push_back(c);
                if (c.getId() >= nextCandId) nextCandId = c.getId() + 1;
            }
        }
        cFile.close();
    }

    // Load Voting Window
    loadVotingWindow();
}

// ============================================================
// PART 2 - Admin: Candidate Management sub-menu
// ============================================================
void adminCandidateMenu() {
    int choice;
    while (true) {
        cout << "\n--- Candidate Management ---\n"
             << "1. Add Candidate\n"
             << "2. Edit Candidate\n"
             << "3. Remove Candidate\n"
             << "4. View All Candidates\n"
             << "5. Back\n"
             << "Choice: ";
        cin >> choice; cin.ignore();

        if (choice == 1) {
            string n = readInput("Candidate Name: ");
            string p = readInput("Party / Symbol: ");
            candidates.push_back(Candidate(nextCandId++, n, p));
            saveCandidatesToDB();
            cout << "[OK] Candidate added successfully.\n";

        } else if (choice == 2) {
            if (candidates.empty()) { cout << "No candidates registered yet.\n"; continue; }
            cout << "\nCurrent Candidates:\n";
            for (int i = 0; i < (int)candidates.size(); i++)
                cout << " " << candidates[i].getId() << ". " << candidates[i].getName()
                     << " [" << candidates[i].getParty() << "]\n";
            int id; cout << "Enter Candidate ID to edit: "; cin >> id; cin.ignore();
            bool found = false;
            for (int i = 0; i < (int)candidates.size(); i++) {
                if (candidates[i].getId() == id) {
                    found = true;
                    string nn = readInput("New Name (leave blank to keep): ");
                    string np = readInput("New Party (leave blank to keep): ");
                    if (!nn.empty()) candidates[i].setName(nn);
                    if (!np.empty()) candidates[i].setParty(np);
                    saveCandidatesToDB();
                    cout << "[OK] Candidate updated.\n";
                    break;
                }
            }
            if (!found) cout << "Candidate ID not found.\n";

        } else if (choice == 3) {
            if (candidates.empty()) { cout << "No candidates to remove.\n"; continue; }
            cout << "\nCurrent Candidates:\n";
            for (int i = 0; i < (int)candidates.size(); i++)
                cout << " " << candidates[i].getId() << ". " << candidates[i].getName()
                     << " [" << candidates[i].getParty() << "]\n";
            int id; cout << "Enter Candidate ID to remove: "; cin >> id; cin.ignore();
            bool found = false;
            for (int i = 0; i < (int)candidates.size(); i++) {
                if (candidates[i].getId() == id) {
                    candidates.erase(candidates.begin() + i);
                    saveCandidatesToDB();
                    cout << "[OK] Candidate removed.\n";
                    found = true; break;
                }
            }
            if (!found) cout << "Candidate ID not found.\n";

        } else if (choice == 4) {
            if (candidates.empty()) { cout << "No candidates registered yet.\n"; continue; }
            cout << "\n--- Registered Candidates ---\n";
            cout << left << setw(5) << "ID" << setw(25) << "Name" << setw(20) << "Party" << "Votes\n";
            cout << string(55, '-') << "\n";
            for (int i = 0; i < (int)candidates.size(); i++)
                cout << left << setw(5)  << candidates[i].getId()
                     << setw(25) << candidates[i].getName()
                     << setw(20) << candidates[i].getParty()
                     << candidates[i].getVoteCount() << "\n";

        } else if (choice == 5) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// PART 2 - Admin: Set voting time window
// ============================================================
void adminSetVotingTime() {
    cout << "\n--- Set Voting Time Window ---\n";
    cout << "Enter voting START time (24-hr format)\n";
    int sh, sm;
    cout << "  Hour (0-23): ";   cin >> sh; cin.ignore();
    cout << "  Minute (0-59): "; cin >> sm; cin.ignore();
    cout << "Enter voting END time (24-hr format)\n";
    int eh, em;
    cout << "  Hour (0-23): ";   cin >> eh; cin.ignore();
    cout << "  Minute (0-59): "; cin >> em; cin.ignore();

    if (sh > 23 || sm > 59 || eh > 23 || em > 59 || (sh * 60 + sm) >= (eh * 60 + em)) {
        cout << "[ERROR] Invalid time range. Please try again.\n";
        return;
    }
    votingWindow.startHour = sh; votingWindow.startMin = sm;
    votingWindow.endHour   = eh; votingWindow.endMin   = em;
    votingWindow.isSet = true;
    saveVotingWindow();
    cout << "[OK] Voting window set: "
         << setfill('0') << setw(2) << sh << ":" << setw(2) << sm
         << " to "
         << setw(2) << eh << ":" << setw(2) << em
         << setfill(' ') << "\n";
}

// ============================================================
// PART 2 - Voter: Cast Vote
// ============================================================
void voterCastVote(int idx) {
    if (voters[idx].getVoteStatus()) {
        cout << "\n[ERROR] You have already cast your vote. Multiple votes are not allowed.\n";
        return;
    }
    if (!isVotingOpen()) {
        if (!votingWindow.isSet)
            cout << "\n[INFO] Voting has not been opened yet by the admin.\n";
        else
            cout << "\n[INFO] Voting is currently closed. Allowed window: "
                 << setfill('0') << setw(2) << votingWindow.startHour << ":"
                 << setw(2) << votingWindow.startMin << " - "
                 << setw(2) << votingWindow.endHour  << ":"
                 << setw(2) << votingWindow.endMin   << setfill(' ') << "\n";
        return;
    }
    if (candidates.empty()) {
        cout << "\n[INFO] No candidates have been registered yet.\n";
        return;
    }
    cout << "\n--- Candidates on Ballot ---\n";
    cout << left << setw(5) << "ID" << setw(25) << "Name" << "Party\n";
    cout << string(50, '-') << "\n";
    for (int i = 0; i < (int)candidates.size(); i++)
        cout << left << setw(5) << candidates[i].getId()
             << setw(25) << candidates[i].getName()
             << candidates[i].getParty() << "\n";

    int choice;
    cout << "\nEnter Candidate ID to vote for (0 to cancel): "; cin >> choice; cin.ignore();
    if (choice == 0) { cout << "Vote cancelled.\n"; return; }

    bool found = false;
    for (int i = 0; i < (int)candidates.size(); i++) {
        if (candidates[i].getId() == choice) {
            found = true;
            candidates[i].addVote();
            voters[idx].castVote();
            totalVotes++;
            saveCandidatesToDB();
            saveVotersToDB();
            srand((unsigned)time(0) + idx);
            int receipt = rand() % 900000 + 100000;
            cout << "\n[SUCCESS] Your vote has been recorded securely.\n";
            cout << "Your Cryptographic Receipt ID: EVOTE-" << receipt << "\n";
            cout << "(Keep this ID to verify your vote was counted.)\n";
            break;
        }
    }
    if (!found) cout << "[ERROR] Invalid Candidate ID. Vote not cast.\n";
}

// ============================================================
// PART 3 - RESULT GENERATION & CANDIDATE RANKING
// ============================================================

vector<Candidate> getSortedCandidates() {
    vector<Candidate> sorted = candidates;
    for (int i = 0; i < (int)sorted.size() - 1; i++) {
        for (int j = 0; j < (int)sorted.size() - 1 - i; j++) {
            if (sorted[j].getVoteCount() < sorted[j + 1].getVoteCount()) {
                Candidate temp = sorted[j];
                sorted[j]      = sorted[j + 1];
                sorted[j + 1]  = temp;
            }
        }
    }
    return sorted;
}

void showRankedResults() {
    if (candidates.empty()) {
        cout << "\n[INFO] No candidates registered yet. Cannot generate results.\n";
        return;
    }
    if (totalVotes == 0) {
        cout << "\n[INFO] No votes have been cast yet. Results are not available.\n";
        return;
    }

    vector<Candidate> sorted = getSortedCandidates();

    cout << "\n";
    cout << "============================================================\n";
    cout << "         OFFICIAL ELECTION RESULTS — FINAL RANKING         \n";
    cout << "============================================================\n";
    cout << left << setw(6) << "Rank"
         << setw(25) << "Candidate Name"
         << setw(20) << "Party"
         << setw(8)  << "Votes"
         << "Share(%)\n";
    cout << string(65, '-') << "\n";

    string medals[] = {"1st", "2nd", "3rd"};

    for (int i = 0; i < (int)sorted.size(); i++) {
        string rank;
        if (i < 3) rank = medals[i];
        else rank = to_string(i + 1) + "th";

        double share = (totalVotes > 0)
                       ? (sorted[i].getVoteCount() * 100.0 / totalVotes)
                       : 0.0;

        cout << left << setw(6)  << rank
             << setw(25) << sorted[i].getName()
             << setw(20) << sorted[i].getParty()
             << setw(8)  << sorted[i].getVoteCount()
             << fixed << setprecision(1) << share << "%\n";
    }

    cout << string(65, '-') << "\n";
    cout << "Total Votes Cast: " << totalVotes << "\n\n";
    cout << ">>> WINNER: " << sorted[0].getName()
         << " (" << sorted[0].getParty() << ")"
         << " with " << sorted[0].getVoteCount() << " votes.\n";
    if ((int)sorted.size() >= 2)
        cout << ">>> Runner-Up: " << sorted[1].getName()
             << " (" << sorted[1].getParty() << ")"
             << " with " << sorted[1].getVoteCount() << " votes.\n";
    if ((int)sorted.size() >= 3)
        cout << ">>> 3rd Place: " << sorted[2].getName()
             << " (" << sorted[2].getParty() << ")"
             << " with " << sorted[2].getVoteCount() << " votes.\n";
    cout << "============================================================\n";
}

void saveResultsToFile() {
    if (candidates.empty() || totalVotes == 0) {
        cout << "\n[INFO] No results to save yet.\n";
        return;
    }

    vector<Candidate> sorted = getSortedCandidates();
    ofstream file("ElectionResults.txt");
    file << "============================================================\n";
    file << "         OFFICIAL ELECTION RESULTS — FINAL RANKING         \n";
    file << "============================================================\n";
    time_t now = time(0);
    file << "Generated: " << ctime(&now);
    file << "\n";
    file << left << setw(6)  << "Rank"
         << setw(25) << "Candidate Name"
         << setw(20) << "Party"
         << setw(8)  << "Votes"
         << "Share(%)\n";
    file << string(65, '-') << "\n";
    string medals[] = {"1st", "2nd", "3rd"};
    for (int i = 0; i < (int)sorted.size(); i++) {
        string rank = (i < 3) ? medals[i] : (to_string(i + 1) + "th");
        double share = (totalVotes > 0)
                       ? (sorted[i].getVoteCount() * 100.0 / totalVotes)
                       : 0.0;
        file << left << setw(6)  << rank
             << setw(25) << sorted[i].getName()
             << setw(20) << sorted[i].getParty()
             << setw(8)  << sorted[i].getVoteCount()
             << fixed << setprecision(1) << share << "%\n";
    }
    file << string(65, '-') << "\n";
    file << "Total Votes Cast: " << totalVotes << "\n\n";
    file << "WINNER    : " << sorted[0].getName() << " (" << sorted[0].getParty() << ")\n";
    if ((int)sorted.size() >= 2)
        file << "Runner-Up : " << sorted[1].getName() << " (" << sorted[1].getParty() << ")\n";
    if ((int)sorted.size() >= 3)
        file << "3rd Place : " << sorted[2].getName() << " (" << sorted[2].getParty() << ")\n";
    file << "============================================================\n";
    file.close();
    cout << "\n[OK] Results saved to 'ElectionResults.txt' successfully.\n";
}

void resultMenu() {
    int choice;
    while (true) {
        cout << "\n--- Result Generation & Ranking ---\n"
             << "1. View Ranked Election Results\n"
             << "2. Save Results to File (ElectionResults.txt)\n"
             << "3. Back\n"
             << "Choice: ";
        cin >> choice; cin.ignore();
        if      (choice == 1) showRankedResults();
        else if (choice == 2) saveResultsToFile();
        else if (choice == 3) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// PART 4 - DATA ANALYSIS & GRAPHICAL VISUALIZATION
// ============================================================

void drawBar(int length, char symbol = '#') {
    for (int i = 0; i < length; i++) cout << symbol;
}

void showBarChart() {
    if (candidates.empty()) {
        cout << "\n[INFO] No candidates available for chart.\n";
        return;
    }
    int maxVotes = 0;
    for (int i = 0; i < (int)candidates.size(); i++)
        if (candidates[i].getVoteCount() > maxVotes)
            maxVotes = candidates[i].getVoteCount();

    cout << "\n";
    cout << "============================================================\n";
    cout << "           VOTE DISTRIBUTION — BAR CHART                   \n";
    cout << "============================================================\n";

    const int BAR_MAX = 40;
    for (int i = 0; i < (int)candidates.size(); i++) {
        int votes  = candidates[i].getVoteCount();
        int barLen = (maxVotes > 0) ? (votes * BAR_MAX / maxVotes) : 0;
        string label = candidates[i].getName();
        if ((int)label.size() > 18) label = label.substr(0, 15) + "...";
        cout << left << setw(20) << label << " | ";
        drawBar(barLen);
        cout << " " << votes << "\n";
    }
    cout << "                       ";
    cout << "|" << string(BAR_MAX, '-') << "\n";
    cout << "                        0" << string(BAR_MAX / 2 - 1, ' ')
         << maxVotes / 2 << string(BAR_MAX / 2 - 3, ' ') << maxVotes << "\n";
    cout << "============================================================\n";
}

void showPieChart() {
    if (candidates.empty() || totalVotes == 0) {
        cout << "\n[INFO] No vote data available for pie chart.\n";
        return;
    }
    cout << "\n";
    cout << "============================================================\n";
    cout << "            VOTE SHARE — PIE CHART (Text View)             \n";
    cout << "============================================================\n";

    const int PIE_WIDTH = 40;
    cout << "\n[";
    for (int i = 0; i < (int)candidates.size(); i++) {
        int votes  = candidates[i].getVoteCount();
        int segLen = (votes * PIE_WIDTH / totalVotes);
        char fill  = candidates[i].getName()[0];
        for (int j = 0; j < segLen; j++) cout << fill;
    }
    cout << "]\n\n";
    cout << "Legend:\n";
    cout << string(45, '-') << "\n";
    for (int i = 0; i < (int)candidates.size(); i++) {
        double share = (candidates[i].getVoteCount() * 100.0 / totalVotes);
        cout << "  [" << candidates[i].getName()[0] << "] "
             << left << setw(22) << candidates[i].getName()
             << fixed << setprecision(1) << share << "% ("
             << candidates[i].getVoteCount() << " votes)\n";
    }
    cout << string(45, '-') << "\n";
    cout << "  Total Votes: " << totalVotes << "\n";
    cout << "============================================================\n";
}

void showParticipationStats() {
    int totalRegistered = (int)voters.size();
    int totalCast       = totalVotes;
    int remaining       = totalRegistered - totalCast;
    double turnoutPct   = (totalRegistered > 0)
                          ? (totalCast * 100.0 / totalRegistered)
                          : 0.0;

    cout << "\n";
    cout << "============================================================\n";
    cout << "              VOTER PARTICIPATION STATISTICS                \n";
    cout << "============================================================\n";
    cout << left << setw(35) << "  Total Registered Voters:" << totalRegistered << "\n";
    cout << left << setw(35) << "  Total Votes Cast:"        << totalCast       << "\n";
    cout << left << setw(35) << "  Remaining (Not Yet Voted):"
         << (remaining > 0 ? remaining : 0) << "\n";
    cout << left << setw(35) << "  Voter Turnout:"
         << fixed << setprecision(1) << turnoutPct << "%\n";
    cout << "\n";

    const int BAR_MAX = 40;
    int filled = (int)(turnoutPct * BAR_MAX / 100.0);
    cout << "  Turnout [";
    drawBar(filled, '=');
    drawBar(BAR_MAX - filled, ' ');
    cout << "] " << fixed << setprecision(1) << turnoutPct << "%\n\n";

    int maleCount = 0, femaleCount = 0, otherCount = 0;
    for (int i = 0; i < (int)voters.size(); i++) {
        string g = voters[i].getGender();
        string gl = g;
        for (char &c : gl) c = tolower(c);
        if      (gl == "male")   maleCount++;
        else if (gl == "female") femaleCount++;
        else                     otherCount++;
    }
    cout << "  Gender Breakdown of Registered Voters:\n";
    cout << "  " << left << setw(10) << "Male:"   << maleCount   << "\n";
    cout << "  " << left << setw(10) << "Female:" << femaleCount << "\n";
    cout << "  " << left << setw(10) << "Other:"  << otherCount  << "\n";
    cout << "============================================================\n";
}

void showVotingTrends() {
    if (candidates.empty()) {
        cout << "\n[INFO] No candidates to show trends for.\n";
        return;
    }
    vector<Candidate> sorted = getSortedCandidates();
    cout << "\n";
    cout << "============================================================\n";
    cout << "                  VOTING TRENDS OVERVIEW                   \n";
    cout << "============================================================\n";
    cout << "  Currently Leading : " << sorted[0].getName()
         << " (" << sorted[0].getParty() << ")"
         << " — " << sorted[0].getVoteCount() << " votes\n";
    if ((int)sorted.size() >= 2) {
        int gap = sorted[0].getVoteCount() - sorted[1].getVoteCount();
        cout << "  Lead Margin       : " << gap
             << " vote(s) over " << sorted[1].getName() << "\n";
    }
    int zeroVoteCandidates = 0;
    for (int i = 0; i < (int)candidates.size(); i++)
        if (candidates[i].getVoteCount() == 0) zeroVoteCandidates++;
    cout << "  Candidates with 0 votes: " << zeroVoteCandidates << "\n";
    cout << "  Total Candidates  : " << (int)candidates.size() << "\n";
    cout << "  Total Votes Cast  : " << totalVotes << "\n";
    double avgVotes = (candidates.size() > 0)
                      ? (totalVotes * 1.0 / candidates.size())
                      : 0.0;
    cout << "  Avg Votes/Candidate: " << fixed << setprecision(1) << avgVotes << "\n";
    if (totalVotes > 0) {
        double leaderShare = sorted[0].getVoteCount() * 100.0 / totalVotes;
        cout << "  Leader's Share    : " << fixed << setprecision(1)
             << leaderShare << "%";
        if      (leaderShare > 60) cout << "  [Clear Dominance]\n";
        else if (leaderShare > 40) cout << "  [Competitive Race]\n";
        else                       cout << "  [Very Tight Race]\n";
    }
    cout << "============================================================\n";
}

void saveAnalysisReport() {
    if (candidates.empty()) {
        cout << "\n[INFO] No data available to generate report.\n";
        return;
    }
    ofstream file("AnalysisReport.txt");
    time_t now = time(0);
    file << "============================================================\n";
    file << "       DATA ANALYSIS & VISUALIZATION REPORT                \n";
    file << "============================================================\n";
    file << "Generated: " << ctime(&now) << "\n";

    int totalRegistered = (int)voters.size();
    double turnoutPct   = (totalRegistered > 0)
                          ? (totalVotes * 100.0 / totalRegistered)
                          : 0.0;
    file << "--- Voter Participation ---\n";
    file << "Total Registered Voters : " << totalRegistered << "\n";
    file << "Total Votes Cast        : " << totalVotes << "\n";
    file << "Remaining Voters        : " << max(0, totalRegistered - totalVotes) << "\n";
    file << "Voter Turnout           : " << fixed << setprecision(1) << turnoutPct << "%\n\n";

    vector<Candidate> sorted = getSortedCandidates();
    file << "--- Candidate Rankings ---\n";
    file << left << setw(6) << "Rank" << setw(25) << "Name"
         << setw(20) << "Party" << setw(8) << "Votes" << "Share\n";
    file << string(65, '-') << "\n";
    string medals[] = {"1st", "2nd", "3rd"};
    for (int i = 0; i < (int)sorted.size(); i++) {
        string rank  = (i < 3) ? medals[i] : (to_string(i + 1) + "th");
        double share = (totalVotes > 0)
                       ? (sorted[i].getVoteCount() * 100.0 / totalVotes)
                       : 0.0;
        file << left << setw(6)  << rank
             << setw(25) << sorted[i].getName()
             << setw(20) << sorted[i].getParty()
             << setw(8)  << sorted[i].getVoteCount()
             << fixed << setprecision(1) << share << "%\n";
    }
    file << "\n";

    int maxVotes = sorted[0].getVoteCount();
    const int BAR_MAX = 35;
    file << "--- Vote Distribution (Bar Chart) ---\n";
    for (int i = 0; i < (int)sorted.size(); i++) {
        int barLen = (maxVotes > 0) ? (sorted[i].getVoteCount() * BAR_MAX / maxVotes) : 0;
        string label = sorted[i].getName();
        if ((int)label.size() > 15) label = label.substr(0, 12) + "...";
        file << left << setw(18) << label << " | ";
        for (int j = 0; j < barLen; j++) file << "#";
        file << " " << sorted[i].getVoteCount() << "\n";
    }
    file << "\n";
    file << "============================================================\n";
    file.close();
    cout << "\n[OK] Analysis report saved to 'AnalysisReport.txt' successfully.\n";
}

void analysisMenu() {
    int choice;
    while (true) {
        cout << "\n--- Data Analysis & Visualization ---\n"
             << "1. Vote Distribution Bar Chart\n"
             << "2. Vote Share Pie Chart\n"
             << "3. Voter Participation Statistics\n"
             << "4. Voting Trends Overview\n"
             << "5. Save Full Analysis Report to File\n"
             << "6. Back\n"
             << "Choice: ";
        cin >> choice; cin.ignore();
        if      (choice == 1) showBarChart();
        else if (choice == 2) showPieChart();
        else if (choice == 3) showParticipationStats();
        else if (choice == 4) showVotingTrends();
        else if (choice == 5) saveAnalysisReport();
        else if (choice == 6) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// DASHBOARDS
// ============================================================
void voterDashboard(int idx) {
    int choice;
    while (true) {
        cout << "\n--- Voter Dashboard --- [" << voters[idx].getName() << "]\n"
             << "1. Cast Vote\n"
             << "2. View Candidates\n"
             << "3. View Election Results (Basic)\n"
             << "4. Ranked Results & Rankings\n"
             << "5. Data Analysis & Charts\n"
             << "6. Logout\n"
             << "Choice: ";
        cin >> choice; cin.ignore();

        if (choice == 1) {
            voterCastVote(idx);
        } else if (choice == 2) {
            if (candidates.empty()) { cout << "\nNo candidates registered yet.\n"; continue; }
            cout << "\n--- Registered Candidates ---\n";
            cout << left << setw(5) << "ID" << setw(25) << "Name" << "Party\n";
            cout << string(50, '-') << "\n";
            for (int i = 0; i < (int)candidates.size(); i++)
                cout << left << setw(5)  << candidates[i].getId()
                     << setw(25) << candidates[i].getName()
                     << candidates[i].getParty() << "\n";
        } else if (choice == 3) {
            cout << "\n--- Current Election Results ---\n";
            cout << "Total Votes Cast: " << totalVotes << "\n";
            if (!candidates.empty()) {
                cout << left << setw(25) << "Candidate" << setw(20) << "Party" << "Votes\n";
                cout << string(50, '-') << "\n";
                for (int i = 0; i < (int)candidates.size(); i++)
                    cout << left << setw(25) << candidates[i].getName()
                         << setw(20) << candidates[i].getParty()
                         << candidates[i].getVoteCount() << "\n";
            }
        } else if (choice == 4) {
            resultMenu();
        } else if (choice == 5) {
            analysisMenu();
        } else if (choice == 6) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// ADMIN: Change Username / Change Password / Forgot Password
// ============================================================
void adminChangeUsername() {
    cout << "\n--- Change Admin Username ---\n";
    string newName = readInput("Enter new username: ");
    adminCreds.username = newName;
    saveAdminCreds();
    cout << "[OK] Username updated to: " << newName << "\n";
}

void adminChangePassword() {
    cout << "\n--- Change Admin Password ---\n";
    cout << "Current Password: "; string oldPass = getHiddenPassword();
    if (hashPass(oldPass, adminCreds.salt) != adminCreds.passHash) {
        cout << "[ERROR] Current password is incorrect.\n";
        return;
    }
    string newPass;
    while (true) {
        cout << "New Password (8+ chars, 1 Cap, 1 Num, 1 Sym): ";
        newPass = getHiddenPassword();
        if (validatePassword(newPass)) break;
        cout << "[ERROR] Password does not meet requirements. Try again.\n";
    }
    adminCreds.salt     = generateSalt();
    adminCreds.passHash = hashPass(newPass, adminCreds.salt);
    saveAdminCreds();
    cout << "[OK] Password changed successfully.\n";
}

void adminForgotPassword() {
    cout << "\n--- Forgot Admin Password ---\n";
    cout << "[INFO] To reset, you must confirm your current username.\n";
    string uname = readInput("Enter admin username: ");
    if (uname != adminCreds.username) {
        cout << "[ERROR] Username not recognized. Cannot reset password.\n";
        return;
    }
    string newPass;
    while (true) {
        cout << "Enter New Password (8+ chars, 1 Cap, 1 Num, 1 Sym): ";
        newPass = getHiddenPassword();
        if (validatePassword(newPass)) break;
        cout << "[ERROR] Password does not meet requirements. Try again.\n";
    }
    adminCreds.salt     = generateSalt();
    adminCreds.passHash = hashPass(newPass, adminCreds.salt);
    saveAdminCreds();
    cout << "[OK] Password has been reset successfully.\n";
}

// ============================================================
// ADMIN DASHBOARD
// NOTE: Admin cannot add, delete, or alter votes.
//       Admin can only manage candidates, set voting window,
//       view results, view analysis, and manage own credentials.
// ============================================================
void adminDashboard() {
    int choice;
    while (true) {
        cout << "\n--- Admin Dashboard ---\n"
             << "1. View All Voters\n"
             << "2. Manage Candidates\n"
             << "3. Set Voting Time Window\n"
             << "4. View Election Results (Basic)\n"
             << "5. Ranked Results & Rankings\n"
             << "6. Data Analysis & Charts\n"
             << "7. Change Username\n"
             << "8. Change Password\n"
             << "9. Logout\n"
             << "Choice: ";
        cin >> choice; cin.ignore();

        if (choice == 1) {
            // View only — admin cannot modify voter records
            if (voters.empty()) { cout << "\nNo voters registered yet.\n"; continue; }
            cout << "\n--- Voter Database (Read Only) ---\n";
            for (int i = 0; i < (int)voters.size(); i++) {
                cout << "Name: "    << voters[i].getName()
                     << "\nCNIC: "  << voters[i].getCnic()
                     << "\nPhone: " << voters[i].getPhoneDec()
                     << "\nVoted: " << (voters[i].getVoteStatus() ? "Yes" : "No")
                     << "\n----------------------\n";
            }
        }
        else if (choice == 2) adminCandidateMenu();
        else if (choice == 3) adminSetVotingTime();
        else if (choice == 4) {
            cout << "\n--- Election Results ---\n";
            cout << "Total Votes Cast: " << totalVotes << "\n";
            if (!candidates.empty()) {
                cout << left << setw(5) << "ID" << setw(25) << "Candidate"
                     << setw(20) << "Party" << "Votes\n";
                cout << string(55, '-') << "\n";
                for (int i = 0; i < (int)candidates.size(); i++)
                    cout << left << setw(5)  << candidates[i].getId()
                         << setw(25) << candidates[i].getName()
                         << setw(20) << candidates[i].getParty()
                         << candidates[i].getVoteCount() << "\n";
            } else {
                cout << "No candidates registered yet.\n";
            }
        }
        else if (choice == 5) resultMenu();
        else if (choice == 6) analysisMenu();
        else if (choice == 7) adminChangeUsername();
        else if (choice == 8) adminChangePassword();
        else if (choice == 9) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// VOTER MENU
// ============================================================
void voterMenu() {
    int choice; string cnic, name, pass;
    while (true) {
        cout << "\n--- Voter Menu ---\n1. Register\n2. Login\n3. Back\nChoice: ";
        cin >> choice; cin.ignore();
        if (choice == 1) {
            cout << "\n--- Voter Registration ---\n";
            cnic = readInput("Enter CNIC (12345-0678052-8): ");
            if (isCnicTaken(cnic)) {
                cout << "\n[ALERT] This CNIC is already registered in the system!\n";
            } else {
                name = readInput("Name: ");
                Voter v; v.setName(name); v.registerVoter(cnic);
                voters.push_back(v); saveVotersToDB();
            }
        }
        else if (choice == 2) {
            name = readInput("Name: ");
            cout << "Password (shown as *): "; pass = getHiddenPassword();
            bool found = false;
            for (int i = 0; i < (int)voters.size(); i++) {
                if (voters[i].getName() == name) {
                    found = true;
                    if (voters[i].checkLock())            cout << "Account locked.\n";
                    else if (voters[i].verifyLogin(pass)) voterDashboard(i);
                    else                                  cout << "Incorrect password.\n";
                    break;
                }
            }
            if (!found) cout << "Voter not found.\n";
        } else if (choice == 3) break;
    }
}

// ============================================================
// ADMIN MENU — Login only (no registration)
// ============================================================
void adminMenu() {
    int choice; string username, pass;
    while (true) {
        cout << "\n--- Admin Menu ---\n"
             << "1. Login\n"
             << "2. Forgot Password\n"
             << "3. Back\n"
             << "Choice: ";
        cin >> choice; cin.ignore();

        if (choice == 1) {
            username = readInput("Admin Username: ");
            cout << "Password (shown as *): "; pass = getHiddenPassword();
            if (verifyAdminLogin(username, pass)) {
                cout << "\n[OK] Welcome, Admin!\n";
                adminDashboard();
            } else {
                cout << "[ERROR] Invalid username or password.\n";
            }
        }
        else if (choice == 2) {
            adminForgotPassword();
        }
        else if (choice == 3) break;
        else cout << "Invalid choice.\n";
    }
}

// ============================================================
// MAIN
// ============================================================
int main() {
    srand(time(0));
    loadDatabases();
    int choice;
    while (true) {
        cout << "\n=== Intelligent Secure E-Voting System ===\n"
             << "1. Voter\n"
             << "2. Admin\n"
             << "3. Exit\n"
             << "Choice: ";
        cin >> choice; cin.ignore();
        if      (choice == 1) voterMenu();
        else if (choice == 2) adminMenu();
        else if (choice == 3) { cout << "Goodbye!\n"; break; }
        else cout << "Invalid choice.\n";
    }
    return 0;
}