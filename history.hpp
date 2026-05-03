#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

//Purpose: provides a bounded, restorable log system for tracking actions (e.g., player moves, system events).
//Relation: used across the project to record and retrieve sequences of actions, supporting undo/redo or replay features.
class ActionHistory {
public:

    //Input: maximum number of entries allowed (default = 6)
    //Output: initializes history tracker with a size limit
    //Purpose: sets up the object with a cap on stored actions
    //Relation: defines how many past actions can be remembered.
    explicit ActionHistory(size_t maxEntries = 6);

    //Input: string entry (action description)
    //Output: modifies internal deque (adds entry to front, removes oldest if over limit)
    //Purpose: records a new action in history
    //Relation: ensures history size stays within the defined limit; ignores empty entries.
    void add(const std::string& entry);

    //Input: none
    //Output: empties the history list
    //Purpose: resets the log to an empty state
    //Relation: used when starting fresh or discarding past actions.
    void clear();

    //Input: none
    //Output: vector of strings containing all current entries in order
    //Purpose: retrieves a snapshot of recent actions
    //Relation: used for displaying or saving history externally.
    std::vector<std::string> recent() const;

    //Input: vector of strings (snapshot of past actions)
    //Output: replaces current history with snapshot contents
    //Purpose: restores history from a saved state
    //Relation: supports save/load functionality or undo/redo systems; uses add internally to rebuild entries in correct order.
    void restore(const std::vector<std::string>& snapshot);

private:
    //Purpose: stores action entries in order (front = most recent).
    //Relation: core container for history tracking.
    std::deque<std::string> entries;

    //Purpose: maximum number of entries allowed.
    //Relation: enforces bounded history size.
    size_t limit;
};
