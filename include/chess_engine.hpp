#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace chess {

enum class Color {
    White,
    Black
};

enum class PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

struct Piece {
    PieceType type;
    Color color;
};

struct Move {
    int fromX = 0;
    int fromY = 0;
    int toX = 0;
    int toY = 0;
    std::optional<PieceType> promotion;
    bool capture = false;
};

class ChessBoard {
public:
    ChessBoard();

    void clear();
    void reset();

    void setPieceAt(int x, int y, const std::optional<Piece>& piece);
    std::optional<Piece> pieceAt(int x, int y) const;
    bool isInsideBoard(int x, int y) const;

    std::vector<Move> generateLegalMoves(Color side) const;
    bool isInCheck(Color side) const;
    void makeMove(const Move& move);

    std::string toString() const;

private:
    std::array<std::array<std::optional<Piece>, 8>, 8> board_{};

    std::vector<Move> generatePseudoLegalMoves(Color side) const;
    bool isSquareAttacked(int x, int y, Color bySide) const;
    bool isAttackedByPiece(const Piece& piece, int fromX, int fromY, int targetX, int targetY) const;
};

Color opposite(Color side);
std::string squareName(int x, int y);
int evaluateBoard(const ChessBoard& board, Color perspective);
std::optional<Move> chooseBestMove(const ChessBoard& board, Color side, int depth);

} // namespace chess
