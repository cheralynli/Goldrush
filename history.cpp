#include "history.hpp"

//Input: maximum number of entries allowed in history
//Output: initializes the history tracker with a size limit
//Purpose: sets up the object with a cap on stored actions
//Relation: defines the maximum capacity for the log system.
ActionHistory::ActionHistory(size_t maxEntries)
    : limit(maxEntries) {
}

//Input: string entry (action description)
//Output: modifies internal list (adds entry to front, removes oldest if over limit)
//Purpose: records a new action in history
//Relation: ensures history size stays within the defined limit; ignores empty entries.
void ActionHistory::add(const std::string& entry) {
    if (entry.empty()) {
        return;
    }
    entries.push_front(entry);
    while (entries.size() > limit) {
        entries.pop_back();
    }
}

//Input: none
//Output: empties the history list
//Purpose: resets the log to an empty state
//Relation: used when starting fresh or discarding past actions.
void ActionHistory::clear() {
    entries.clear();
}

//Input: none
//Output: vector of strings containing all current entries in order
//Purpose: retrieves a snapshot of recent actions
//Relation: used for displaying or saving history externally.
std::vector<std::string> ActionHistory::recent() const {
    return std::vector<std::string>(entries.begin(), entries.end());
}

//Input: vector of strings (snapshot of past actions)
//Output: replaces current history with snapshot contents
//Purpose: restores history from a saved state
//Relation: supports save/load functionality or undo/redo systems; uses add to rebuild entries in correct order.
void ActionHistory::restore(const std::vector<std::string>& snapshot) {
    clear();
    for (std::vector<std::string>::const_reverse_iterator it = snapshot.rbegin();
         it != snapshot.rend();
         ++it) {
        add(*it);
    }
}
