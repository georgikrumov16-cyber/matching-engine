#pragma once
#include <string>
#include <cstdint>
#include "matching_engine.hpp"

// Save the entire engine state to a file
void save_state(const MatchingEngine& engine,
                std::uint64_t next_id,
                const std::string& filename);

// Load engine state from a file (returns true on success)
bool load_state(MatchingEngine& engine,
                std::uint64_t& next_id,
                const std::string& filename);

// Replay a script of commands
void replay_script(MatchingEngine& engine,
                   std::uint64_t& next_id,
                   const std::string& filename);
