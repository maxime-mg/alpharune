#pragma once
/// @file board_view.h
/// What one seat is allowed to see of a GameState, as JSON for the web board.
///
/// This is the single place that decides visibility. Everything the play
/// server sends about the board goes through `buildBoardView`, so a hidden
/// card can only leak through a bug here, and `test_board_view.cpp` checks
/// this file alone.
///
/// Rules (CR 107 public and private zones):
///   - Main Deck and Rune Deck: counts only, for everyone.
///   - Hand: the owner sees the cards, everyone else sees a count.
///   - Face-down cards: the controller sees the card, everyone else a back.
///   - Base, battlefields, trash, banishment, legend and champion zones,
///     the chain: public.
///   - The god seat sees everything, including deck order.
///
/// A card a seat may not see is sent as {"back": true} with no id: object ids
/// are allocated in deck order at setup, so an id alone can identify a card.

#include "core/types.h"

#include <nlohmann/json.hpp>

#include <optional>
#include <string>

namespace riftbound {

class CardDB;
struct GameState;

enum class Seat : uint8_t { Player1, Player2, Spectator, God };

/// Parses "p1", "p2", "spectator" or "god" (case-insensitive). Anything else
/// returns std::nullopt.
std::optional<Seat> parseSeat(const std::string& s);
const char* seatName(Seat s);

/// The player a seat plays as, or PlayerId::None for spectator and god.
PlayerId seatPlayer(Seat s);

/// The board as `viewer` may see it. See docs/play-api.md for the shape.
nlohmann::json buildBoardView(const GameState& state, const CardDB& db, Seat viewer);

}  // namespace riftbound
