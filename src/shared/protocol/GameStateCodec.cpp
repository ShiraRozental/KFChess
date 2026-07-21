#include "shared/protocol/GameStateCodec.h"
#include <array>
#include <map>
#include <stdexcept>
#include <utility>
#include "logic/io/PieceNotation.h"
#include "shared/protocol/WireProtocol.h"
#include "nlohmann/json.hpp"

namespace {
    constexpr const char* kBoardKey = "board";
    constexpr const char* kRowsKey = "rows";
    constexpr const char* kColsKey = "cols";
    constexpr const char* kPiecesKey = "pieces";
    constexpr const char* kIdKey = "id";
    constexpr const char* kColorKey = "color";
    constexpr const char* kTypeKey = "type";
    constexpr const char* kRowKey = "row";
    constexpr const char* kColKey = "col";
    constexpr const char* kStateKey = "state";
    constexpr const char* kGameOverKey = "gameOver";
    constexpr const char* kWinnerKey = "winner";
    constexpr const char* kCooldownsKey = "cooldowns";
    constexpr const char* kProgressKey = "progress";
    constexpr const char* kInFlightKey = "inFlight";
    constexpr const char* kMovesKey = "moves";
    constexpr const char* kTimestampKey = "t";
    constexpr const char* kTextKey = "text";
    constexpr const char* kScoreKey = "score";

    constexpr std::array<std::pair<Piece::State, const char*>, 6> kStateNames{{
        {Piece::State::Idle, "Idle"},
        {Piece::State::Moving, "Moving"},
        {Piece::State::Jumping, "Jumping"},
        {Piece::State::ShortRest, "ShortRest"},
        {Piece::State::LongRest, "LongRest"},
        {Piece::State::Captured, "Captured"},
    }};

    const char* stateToName(Piece::State state) {
        for (const auto& entry : kStateNames) {
            if (entry.first == state) return entry.second;
        }
        throw std::out_of_range("No wire name registered for this piece state");
    }

    std::optional<Piece::State> stateFromName(const std::string& name) {
        for (const auto& entry : kStateNames) {
            if (name == entry.second) return entry.first;
        }
        return std::nullopt;
    }

    std::string colorKey(PieceColor color) {
        return std::string(1, wireColorToChar(color));
    }

    const nlohmann::json* field(const nlohmann::json& parent, const std::string& key) {
        if (!parent.is_object()) return nullptr;
        nlohmann::json::const_iterator found = parent.find(key);
        if (found == parent.end()) return nullptr;
        return &(*found);
    }

    std::optional<bool> boolField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_boolean()) return std::nullopt;
        return value->get<bool>();
    }

    std::optional<int> intField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_number_integer()) return std::nullopt;
        return value->get<int>();
    }

    std::optional<long long> timestampField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_number_integer()) return std::nullopt;
        return value->get<long long>();
    }

    std::optional<double> doubleField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_number()) return std::nullopt;
        return value->get<double>();
    }

    std::optional<std::string> stringField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_string()) return std::nullopt;
        return value->get<std::string>();
    }

    std::optional<char> charField(const nlohmann::json& parent, const std::string& key) {
        std::optional<std::string> text = stringField(parent, key);
        if (!text || text->length() != 1) return std::nullopt;
        return (*text)[0];
    }

    nlohmann::json encodePiece(const Piece& piece) {
        nlohmann::json node = nlohmann::json::object();
        node[kIdKey] = piece.id();
        node[kColorKey] = colorKey(piece.color());
        node[kTypeKey] = std::string(1, pieceTypeToChar(piece.kind()));
        node[kRowKey] = piece.cell().row;
        node[kColKey] = piece.cell().col;
        node[kStateKey] = stateToName(piece.state());
        return node;
    }

    std::optional<Piece> parsePiece(const nlohmann::json& node) {
        std::optional<int> id = intField(node, kIdKey);
        std::optional<char> colorChar = charField(node, kColorKey);
        std::optional<char> typeChar = charField(node, kTypeKey);
        std::optional<int> row = intField(node, kRowKey);
        std::optional<int> col = intField(node, kColKey);
        std::optional<std::string> stateName = stringField(node, kStateKey);
        if (!id || !colorChar || !typeChar || !row || !col || !stateName) return std::nullopt;

        std::optional<PieceColor> color = wireColorFromChar(*colorChar);
        std::optional<PieceType> kind = pieceTypeFromChar(*typeChar);
        std::optional<Piece::State> state = stateFromName(*stateName);
        if (!color || !kind || !state) return std::nullopt;

        Piece piece(*id, *color, *kind, Position{*row, *col});
        piece.setState(*state);
        return piece;
    }

    std::optional<Board> parseBoard(const nlohmann::json& node) {
        std::optional<int> rows = intField(node, kRowsKey);
        std::optional<int> cols = intField(node, kColsKey);
        const nlohmann::json* pieces = field(node, kPiecesKey);
        if (!rows || !cols || !pieces || !pieces->is_array()) return std::nullopt;
        if (*rows < 0 || *cols < 0) return std::nullopt;

        Board board(*rows, *cols);
        for (const nlohmann::json& entry : *pieces) {
            std::optional<Piece> piece = parsePiece(entry);
            if (!piece) return std::nullopt;
            if (!board.addPiece(piece->cell().row, piece->cell().col, *piece)) return std::nullopt;
        }
        return board;
    }

    std::optional<std::map<PieceId, double>> parseCooldowns(const nlohmann::json& array) {
        std::map<PieceId, double> cooldowns;
        for (const nlohmann::json& entry : array) {
            std::optional<int> id = intField(entry, kIdKey);
            std::optional<double> progress = doubleField(entry, kProgressKey);
            if (!id || !progress) return std::nullopt;
            cooldowns.emplace(*id, *progress);
        }
        return cooldowns;
    }

    std::optional<std::map<PieceId, BoardPoint>> parseInFlight(const nlohmann::json& array) {
        std::map<PieceId, BoardPoint> positions;
        for (const nlohmann::json& entry : array) {
            std::optional<int> id = intField(entry, kIdKey);
            std::optional<double> row = doubleField(entry, kRowKey);
            std::optional<double> col = doubleField(entry, kColKey);
            if (!id || !row || !col) return std::nullopt;
            positions.emplace(*id, BoardPoint{*row, *col});
        }
        return positions;
    }

    nlohmann::json encodeMoves(const std::vector<MoveLogEntry>& entries) {
        nlohmann::json array = nlohmann::json::array();
        for (const MoveLogEntry& entry : entries) {
            nlohmann::json node = nlohmann::json::object();
            node[kTimestampKey] = entry.timestampMs;
            node[kTextKey] = entry.text;
            array.push_back(node);
        }
        return array;
    }

    std::optional<std::vector<MoveLogEntry>> parseMoves(const nlohmann::json& parent,
                                                        PieceColor color) {
        const nlohmann::json* array = field(parent, colorKey(color));
        if (!array || !array->is_array()) return std::nullopt;

        std::vector<MoveLogEntry> entries;
        for (const nlohmann::json& node : *array) {
            std::optional<long long> timestampMs = timestampField(node, kTimestampKey);
            std::optional<std::string> text = stringField(node, kTextKey);
            if (!timestampMs || !text) return std::nullopt;
            entries.push_back(MoveLogEntry{*timestampMs, *text});
        }
        return entries;
    }

    std::optional<std::optional<PieceColor>> parseWinner(const nlohmann::json& root) {
        const nlohmann::json* value = field(root, kWinnerKey);
        if (!value) return std::optional<PieceColor>{};

        std::optional<char> winnerChar = charField(root, kWinnerKey);
        if (!winnerChar) return std::nullopt;
        std::optional<PieceColor> winner = wireColorFromChar(*winnerChar);
        if (!winner) return std::nullopt;
        return winner;
    }

    const nlohmann::json* arrayField(const nlohmann::json& parent, const std::string& key) {
        const nlohmann::json* value = field(parent, key);
        if (!value || !value->is_array()) return nullptr;
        return value;
    }
}

std::string encodeGameState(const GameStateMessage& message) {
    const Board& board = message.snapshot.board();
    nlohmann::json pieces = nlohmann::json::array();
    nlohmann::json cooldowns = nlohmann::json::array();
    nlohmann::json inFlight = nlohmann::json::array();

    for (int row = 0; row < board.rowCount(); ++row) {
        for (int col = 0; col < board.colCount(); ++col) {
            const Piece* piece = board.pieceAt(row, col);
            if (!piece) continue;
            pieces.push_back(encodePiece(*piece));

            std::optional<double> progress = message.snapshot.cooldownProgressOf(piece->id());
            if (progress) {
                nlohmann::json node = nlohmann::json::object();
                node[kIdKey] = piece->id();
                node[kProgressKey] = *progress;
                cooldowns.push_back(node);
            }

            std::optional<BoardPoint> point = message.snapshot.inFlightPositionOf(piece->id());
            if (point) {
                nlohmann::json node = nlohmann::json::object();
                node[kIdKey] = piece->id();
                node[kRowKey] = point->row;
                node[kColKey] = point->col;
                inFlight.push_back(node);
            }
        }
    }

    nlohmann::json boardNode = nlohmann::json::object();
    boardNode[kRowsKey] = board.rowCount();
    boardNode[kColsKey] = board.colCount();
    boardNode[kPiecesKey] = pieces;

    nlohmann::json movesNode = nlohmann::json::object();
    movesNode[colorKey(PieceColor::White)] = encodeMoves(message.whiteMoves);
    movesNode[colorKey(PieceColor::Black)] = encodeMoves(message.blackMoves);

    nlohmann::json scoreNode = nlohmann::json::object();
    scoreNode[colorKey(PieceColor::White)] = message.whiteScore;
    scoreNode[colorKey(PieceColor::Black)] = message.blackScore;

    nlohmann::json root = nlohmann::json::object();
    root[kBoardKey] = boardNode;
    root[kGameOverKey] = message.snapshot.isGameOver();
    if (message.snapshot.winner()) {
        root[kWinnerKey] = colorKey(*message.snapshot.winner());
    }
    root[kCooldownsKey] = cooldowns;
    root[kInFlightKey] = inFlight;
    root[kMovesKey] = movesNode;
    root[kScoreKey] = scoreNode;
    return root.dump();
}

std::optional<GameStateMessage> parseGameState(const std::string& text) {
    nlohmann::json root = nlohmann::json::parse(text, nullptr, false);
    if (root.is_discarded() || !root.is_object()) return std::nullopt;

    const nlohmann::json* boardNode = field(root, kBoardKey);
    const nlohmann::json* cooldownsNode = arrayField(root, kCooldownsKey);
    const nlohmann::json* inFlightNode = arrayField(root, kInFlightKey);
    const nlohmann::json* movesNode = field(root, kMovesKey);
    const nlohmann::json* scoreNode = field(root, kScoreKey);
    std::optional<bool> gameOver = boolField(root, kGameOverKey);
    if (!boardNode || !cooldownsNode || !inFlightNode || !movesNode || !scoreNode || !gameOver) {
        return std::nullopt;
    }

    std::optional<Board> board = parseBoard(*boardNode);
    std::optional<std::optional<PieceColor>> winner = parseWinner(root);
    std::optional<std::map<PieceId, double>> cooldowns = parseCooldowns(*cooldownsNode);
    std::optional<std::map<PieceId, BoardPoint>> inFlight = parseInFlight(*inFlightNode);
    if (!board || !winner || !cooldowns || !inFlight) return std::nullopt;

    std::optional<std::vector<MoveLogEntry>> whiteMoves = parseMoves(*movesNode, PieceColor::White);
    std::optional<std::vector<MoveLogEntry>> blackMoves = parseMoves(*movesNode, PieceColor::Black);
    std::optional<int> whiteScore = intField(*scoreNode, colorKey(PieceColor::White));
    std::optional<int> blackScore = intField(*scoreNode, colorKey(PieceColor::Black));
    if (!whiteMoves || !blackMoves || !whiteScore || !blackScore) return std::nullopt;

    return GameStateMessage{
        GameSnapshot(std::move(*board), *gameOver, *winner,
                     std::move(*cooldowns), std::move(*inFlight)),
        std::move(*whiteMoves), std::move(*blackMoves), *whiteScore, *blackScore};
}
