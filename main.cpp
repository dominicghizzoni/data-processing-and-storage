#include <iostream>
#include <unordered_map>
#include <string>
#include <stdexcept>
using namespace std;

class InMemoryDB {
public:
    string get(const string &key) const {
        auto it = db.find(key);
        if (it == db.end()) {
            return "null";
        }
        return to_string(it->second);
    }

    void put(const string &key, int val) {
        if (!transaction_active) {
            throw runtime_error("put() called with no active transaction");
        }
        tx_changes[key] = val;
    }

    void begin_transaction() {
        if (transaction_active) {
            throw runtime_error("begin_transaction() called while a transaction is already active");
        }
        transaction_active = true;
        tx_changes.clear();
    }

    void commit() {
        ensure_transaction("commit");
        for (auto &p : tx_changes) {
            db[p.first] = p.second;
        }
        tx_changes.clear();
        transaction_active = false;
    }

    void rollback() {
        ensure_transaction("rollback");
        tx_changes.clear();
        transaction_active = false;
    }

private:
    unordered_map<string, int> db;
    unordered_map<string, int> tx_changes;
    bool transaction_active = false;

    void ensure_transaction(const char *opName) const {
        if (!transaction_active) {
            string msg = string(opName) + "() called with no active transaction";
            throw runtime_error(msg);
        }
    }
};

int main() {
    InMemoryDB inmemoryDB;

    cout << "get(A): " << inmemoryDB.get("A") << "\n";

    try {
        inmemoryDB.put("A", 5);
    } catch (const exception &e) {
        cout << "put(A,5) threw: " << e.what() << "\n";
    }

    inmemoryDB.begin_transaction();
    inmemoryDB.put("A", 5);

    cout << "get(A) in tx: " << inmemoryDB.get("A") << "\n";

    inmemoryDB.put("A", 6);
    inmemoryDB.commit();

    cout << "get(A) after commit: " << inmemoryDB.get("A") << "\n";

    try {
        inmemoryDB.commit();
    } catch (const exception &e) {
        cout << "commit() threw: " << e.what() << "\n";
    }

    try {
        inmemoryDB.rollback();
    } catch (const exception &e) {
        cout << "rollback() threw: " << e.what() << "\n";
    }

    cout << "get(B): " << inmemoryDB.get("B") << "\n";

    inmemoryDB.begin_transaction();
    inmemoryDB.put("B", 10);
    inmemoryDB.rollback();

    cout << "get(B) after rollback: " << inmemoryDB.get("B") << "\n";

    inmemoryDB.begin_transaction();
    inmemoryDB.put("A", 42);
    cout << "get(A) in tx (should show committed 6): " << inmemoryDB.get("A") << "\n";
    inmemoryDB.rollback();

    cout << "final get(A): " << inmemoryDB.get("A") << "\n";
    cout << "final get(B): " << inmemoryDB.get("B") << "\n";

    return 0;
}
