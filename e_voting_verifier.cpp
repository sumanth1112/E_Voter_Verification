#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>
#include <functional>
using namespace std;

// ======================= HASH FUNCTION =======================
string fnv1aHash(const string &data) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : data) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    stringstream ss;
    ss << hex << hash;
    return ss.str();
}

// ======================= MERKLE TREE STRUCTURE =======================
struct TreeNode {
    string hash;
    shared_ptr<TreeNode> left, right;
    TreeNode(const string &h) : hash(h) {}
};

shared_ptr<TreeNode> buildMerkleTree(vector<string> &votes) {
    if (votes.empty()) return nullptr;

    vector<shared_ptr<TreeNode>> nodes;
    for (const auto &v : votes)
        nodes.push_back(make_shared<TreeNode>(fnv1aHash(v)));

    while (nodes.size() > 1) {
        vector<shared_ptr<TreeNode>> newLevel;
        for (size_t i = 0; i < nodes.size(); i += 2) {
            auto parent = make_shared<TreeNode>(
                i + 1 < nodes.size() ? fnv1aHash(nodes[i]->hash + nodes[i + 1]->hash) : nodes[i]->hash);
            parent->left = nodes[i];
            if (i + 1 < nodes.size()) parent->right = nodes[i + 1];
            newLevel.push_back(parent);
        }
        nodes = newLevel;
    }
    return nodes[0];
}

// ======================= VERIFY VOTE =======================
bool verifyVote(shared_ptr<TreeNode> root, const string &regNumber) {
    if (!root) return false;
    string hashedReg = fnv1aHash(regNumber);
    vector<string> leafHashes;

    function<void(shared_ptr<TreeNode>)> findLeaves = [&](shared_ptr<TreeNode> node) {
        if (!node) return;
        if (!node->left && !node->right) {
            leafHashes.push_back(node->hash);
            return;
        }
        findLeaves(node->left);
        findLeaves(node->right);
    };

    findLeaves(root);
    return find(leafHashes.begin(), leafHashes.end(), hashedReg) != leafHashes.end();
}

// ======================= READ NAME FROM CSV =======================
string findNameByRegNumber(const string &filename, const string &regNumber) {
    ifstream file(filename);
    if (!file.is_open()) return "Error: Could not open file";

    string line, reg, name;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, reg, ',');
        getline(ss, name);
        if (reg == regNumber) return name;
    }
    return "Not found";
}

// ======================= PRINT MERKLE TREE =======================
void printTree(shared_ptr<TreeNode> node, int depth = 0, string side = "Root") {
    if (!node) return;
    string indent(depth * 4, ' ');
    cout << indent << "[" << side << "] " << node->hash << endl;
    printTree(node->left, depth + 1, "L");
    printTree(node->right, depth + 1, "R");
}

// ======================= MAIN PROGRAM =======================
int main() {
    vector<string> votes = {
        "12314545", "12314515", "12303398", "12303491", "12303843", "12304096", "12306198", "12306281",
        "12307019", "12307046", "12307739", "12309172", "12309577", "12310798", "12311606", "12312213",
        "12313977", "12314061", "12314137", "12314650", "12314828", "12315072", "12315331", "12316709",
        "12316890", "12316904", "12317006", "12317054", "12318355", "12318374", "12318467", "12319271",
        "12321373", "12321523", "12321571", "12321590", "12322351", "12323015", "12323485", "12324277",
        "12325009", "12325313", "12325414", "12325642"
    };

    cout << "=========================================\n";
    cout << "|------ Enhanced E-Voting Verifier ------|\n";
    cout << "=========================================\n";

    string regNumber;
    cout << "Enter Registration Number: ";
    cin >> regNumber;
    cout << "\n->Verification process initiated...\n";

    string name = findNameByRegNumber("data_for_cpp.csv", regNumber);
    cout << "->Name: " << name << endl;

    if (name == "Not found" || name.find("Error") != string::npos) {
        cout << "Exiting: Invalid registration number.\n";
        return 1;
    }

    auto root = buildMerkleTree(votes);
    if (!root) {
        cout << "Error: Merkle tree build failed.\n";
        return 1;
    }

    cout << "\n->Merkle Tree Structure:\n";
    printTree(root);
    cout << "\nRoot Hash: " << root->hash << endl;

    bool valid = verifyVote(root, regNumber);
    cout << "\nVerifying....\n->Registration Number " << regNumber
         << " (Hash: " << fnv1aHash(regNumber) << ")\n";
    cout << "->Name: " << name << endl;
    cout << "->Status: " << (valid ? "<---Verified--->" : "<---Not Found--->") << endl;
    cout << "\n========================================================\n";
    cout << "|---------------- Verification Completed ---------------|\n";
    cout << "========================================================\n";
    return 0;
}
