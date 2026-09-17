#include "chess_engine.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace chess {
namespace {

constexpr int kPieceValuePawn = 100;
constexpr int kPieceValueKnight = 320;
constexpr int kPieceValueBishop = 330;
constexpr int kPieceValueRook = 500;
constexpr int kPieceValueQueen = 900;
constexpr int kPieceValueKing = 20000;

int pieceValue(PieceType type) {
    switch (type) {
        case PieceType::Pawn: return kPieceValuePawn;
        case PieceType::Knight: return kPieceValueKnight;
        case PieceType::Bishop: return kPieceValueBishop;
        case PieceType::Rook: return kPieceValueRook;
        case PieceType::Queen: return kPieceValueQueen;
        case PieceType::King: return kPieceValueKing;
    }
    return 0;
}

int sqToFile(char file) {
    if (file >= 'a' && file <= 'h') {
        return file - 'a';
    }
    if (file >= 'A' && file <= 'H') {
        return file - 'A';
    }
    throw std::invalid_argument("Invalid chess file");
}

std::string toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

} // namespace

ChessBoard::ChessBoard() {
    clear();
}

void ChessBoard::clear() {
    for (auto& row : board_) {
        row.fill(std::nullopt);
    }
}

void ChessBoard::reset() {
    clear();
    for (int x = 0; x < 8; ++x) {
        board_[x][1] = Piece{PieceType::Pawn, Color::White};
        board_[x][6] = Piece{PieceType::Pawn, Color::Black};
    }

    const std::array<std::pair<int, Piece>, 8> backRankWhite = {
        std::pair<int, Piece>{0, Piece{PieceType::Rook, Color::White}},
        std::pair<int, Piece>{1, Piece{PieceType::Knight, Color::White}},
        std::pair<int, Piece>{2, Piece{PieceType::Bishop, Color::White}},
        std::pair<int, Piece>{3, Piece{PieceType::Queen, Color::White}},
        std::pair<int, Piece>{4, Piece{PieceType::King, Color::White}},
        std::pair<int, Piece>{5, Piece{PieceType::Bishop, Color::White}},
        std::pair<int, Piece>{6, Piece{PieceType::Knight, Color::White}},
        std::pair<int, Piece>{7, Piece{PieceType::Rook, Color::White}}
    };

    const std::array<std::pair<int, Piece>, 8> backRankBlack = {
        std::pair<int, Piece>{0, Piece{PieceType::Rook, Color::Black}},
        std::pair<int, Piece>{1, Piece{PieceType::Knight, Color::Black}},
        std::pair<int, Piece>{2, Piece{PieceType::Bishop, Color::Black}},
        std::pair<int, Piece>{3, Piece{PieceType::Queen, Color::Black}},
        std::pair<int, Piece>{4, Piece{PieceType::King, Color::Black}},
        std::pair<int, Piece>{5, Piece{PieceType::Bishop, Color::Black}},
        std::pair<int, Piece>{6, Piece{PieceType::Knight, Color::Black}},
        std::pair<int, Piece>{7, Piece{PieceType::Rook, Color::Black}}
    };

    for (const auto& [fileIdx, piece] : backRankWhite) {
        board_[fileIdx][0] = piece;
    }
    for (const auto& [fileIdx, piece] : backRankBlack) {
        board_[fileIdx][7] = piece;
    }
}

void ChessBoard::setPieceAt(int x, int y, const std::optional<Piece>& piece) {
    if (!isInsideBoard(x, y)) {
        throw std::out_of_range("Square out of bounds");
    }
    board_[x][y] = piece;
}

std::optional<Piece> ChessBoard::pieceAt(int x, int y) const {
    if (!isInsideBoard(x, y)) {
        return std::nullopt;
    }
    return board_[x][y];
}

bool ChessBoard::isInsideBoard(int x, int y) const {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

Color opposite(Color side) {
    return side == Color::White ? Color::Black : Color::White;
}

bool ChessBoard::isAttackedByPiece(const Piece& piece, int fromX, int fromY, int targetX, int targetY) const {
    const int dx = targetX - fromX;
    const int dy = targetY - fromY;

    switch (piece.type) {
        case PieceType::Pawn: {
            const int direction = piece.color == Color::White ? 1 : -1;
            return std::abs(dx) == 1 && dy == direction;
        }
        case PieceType::Knight:
            return (std::abs(dx) == 1 && std::abs(dy) == 2) || (std::abs(dx) == 2 && std::abs(dy) == 1);
        case PieceType::King:
            return std::max(std::abs(dx), std::abs(dy)) == 1;
        case PieceType::Bishop: {
            if (std::abs(dx) != std::abs(dy) || dx == 0) {
                return false;
            }
            const int stepX = dx > 0 ? 1 : -1;
            const int stepY = dy > 0 ? 1 : -1;
            int x = fromX + stepX;
            int y = fromY + stepY;
            while (x != targetX || y != targetY) {
                if (!isInsideBoard(x, y) || board_[x][y].has_value()) {
                    return false;
                }
                x += stepX;
                y += stepY;
            }
            return true;
        }
        case PieceType::Rook: {
            if (dx != 0 && dy != 0) {
                return false;
            }
            const int stepX = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
            const int stepY = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
            int x = fromX + stepX;
            int y = fromY + stepY;
            while (x != targetX || y != targetY) {
                if (!isInsideBoard(x, y) || board_[x][y].has_value()) {
                    return false;
                }
                x += stepX;
                y += stepY;
            }
            return true;
        }
        case PieceType::Queen: {
            if (dx == 0 && dy == 0) {
                return false;
            }
            if (dx == 0 || dy == 0 || std::abs(dx) == std::abs(dy)) {
                const int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
                const int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
                int x = fromX + stepX;
                int y = fromY + stepY;
                while (x != targetX || y != targetY) {
                    if (!isInsideBoard(x, y) || board_[x][y].has_value()) {
                        return false;
                    }
                    x += stepX;
                    y += stepY;
                }
                return true;
            }
            return false;
        }
        default:
            return false;
    }
}

bool ChessBoard::isSquareAttacked(int x, int y, Color bySide) const {
    for (int fromX = 0; fromX < 8; ++fromX) {
        for (int fromY = 0; fromY < 8; ++fromY) {
            const auto occupant = board_[fromX][fromY];
            if (!occupant || occupant->color != bySide) {
                continue;
            }
            if (isAttackedByPiece(*occupant, fromX, fromY, x, y)) {
                return true;
            }
        }
    }
    return false;
}

bool ChessBoard::isInCheck(Color side) const {
    const auto king = side == Color::White ? Piece{PieceType::King, Color::White} : Piece{PieceType::King, Color::Black};
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const auto occupant = board_[x][y];
            if (occupant && occupant->type == PieceType::King && occupant->color == side) {
                return isSquareAttacked(x, y, opposite(side));
            }
        }
    }
    return false;
}

std::vector<Move> ChessBoard::generatePseudoLegalMoves(Color side) const {
    std::vector<Move> moves;

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const auto occupant = board_[x][y];
            if (!occupant || occupant->color != side) {
                continue;
            }

            const Piece piece = *occupant;
            const int direction = side == Color::White ? 1 : -1;

            switch (piece.type) {
                case PieceType::Pawn: {
                    const int forwardY = y + direction;
                    if (isInsideBoard(x, forwardY) && !board_[x][forwardY].has_value()) {
                        Move nextMove{x, y, x, forwardY, std::nullopt, false};
                        if ((side == Color::White && forwardY == 7) || (side == Color::Black && forwardY == 0)) {
                            nextMove.promotion = PieceType::Queen;
                        }
                        moves.push_back(nextMove);

                        const int startRank = side == Color::White ? 1 : 6;
                        if (y == startRank) {
                            const int doubleY = y + 2 * direction;
                            if (isInsideBoard(x, doubleY) && !board_[x][doubleY].has_value()) {
                                moves.push_back(Move{x, y, x, doubleY, std::nullopt, false});
                            }
                        }
                    }

                    for (int dx : {-1, 1}) {
                        const int targetX = x + dx;
                        const int targetY = y + direction;
                        if (!isInsideBoard(targetX, targetY)) {
                            continue;
                        }
                        const auto target = board_[targetX][targetY];
                        if (target && target->color != side) {
                            Move nextMove{x, y, targetX, targetY, std::nullopt, true};
                            if ((side == Color::White && targetY == 7) || (side == Color::Black && targetY == 0)) {
                                nextMove.promotion = PieceType::Queen;
                            }
                            moves.push_back(nextMove);
                        }
                    }
                    break;
                }
                case PieceType::Knight: {
                    static const std::vector<std::pair<int, int>> offsets = {
                        {1, 2}, {2, 1}, {2, -1}, {1, -2},
                        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
                    };
                    for (const auto& [dx, dy] : offsets) {
                        const int targetX = x + dx;
                        const int targetY = y + dy;
                        if (!isInsideBoard(targetX, targetY)) {
                            continue;
                        }
                        const auto target = board_[targetX][targetY];
                        if (!target || target->color != side) {
                            moves.push_back(Move{x, y, targetX, targetY, std::nullopt, target.has_value()});
                        }
                    }
                    break;
                }
                case PieceType::Bishop:
                case PieceType::Rook:
                case PieceType::Queen: {
                    std::vector<std::pair<int, int>> validDirs;
                    if (piece.type == PieceType::Bishop || piece.type == PieceType::Queen) {
                        validDirs.push_back({1, 1}); validDirs.push_back({1, -1}); validDirs.push_back({-1, 1}); validDirs.push_back({-1, -1});
                    }
                    if (piece.type == PieceType::Rook || piece.type == PieceType::Queen) {
                        validDirs.push_back({1, 0}); validDirs.push_back({-1, 0}); validDirs.push_back({0, 1}); validDirs.push_back({0, -1});
                    }
                    for (const auto& [dx, dy] : validDirs) {
                        int targetX = x + dx;
                        int targetY = y + dy;
                        while (isInsideBoard(targetX, targetY)) {
                            const auto occupant = board_[targetX][targetY];
                            if (!occupant) {
                                moves.push_back(Move{x, y, targetX, targetY, std::nullopt, false});
                            } else {
                                if (occupant->color != side) {
                                    moves.push_back(Move{x, y, targetX, targetY, std::nullopt, true});
                                }
                                break;
                            }
                            targetX += dx;
                            targetY += dy;
                        }
                    }
                    break;
                }
                case PieceType::King: {
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            if (dx == 0 && dy == 0) {
                                continue;
                            }
                            const int targetX = x + dx;
                            const int targetY = y + dy;
                            if (!isInsideBoard(targetX, targetY)) {
                                continue;
                            }
                            const auto target = board_[targetX][targetY];
                            if (!target || target->color != side) {
                                moves.push_back(Move{x, y, targetX, targetY, std::nullopt, target.has_value()});
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::generateLegalMoves(Color side) const {
    std::vector<Move> legal;
    for (const auto& move : generatePseudoLegalMoves(side)) {
        ChessBoard nextBoard = *this;
        nextBoard.makeMove(move);
        if (!nextBoard.isInCheck(side)) {
            legal.push_back(move);
        }
    }
    return legal;
}

void ChessBoard::makeMove(const Move& move) {
    const auto source = board_[move.fromX][move.fromY];
    if (!source) {
        throw std::invalid_argument("Move source is empty");
    }

    auto destination = board_[move.toX][move.toY];
    if (destination && destination->color == source->color) {
        return;
    }

    board_[move.fromX][move.fromY] = std::nullopt;

    Piece movedPiece = *source;
    if (move.promotion.has_value()) {
        movedPiece.type = *move.promotion;
    }

    board_[move.toX][move.toY] = movedPiece;
}

std::string ChessBoard::toString() const {
    std::ostringstream out;
    for (int rank = 7; rank >= 0; --rank) {
        out << (rank + 1) << " ";
        for (int file = 0; file < 8; ++file) {
            const auto piece = board_[file][rank];
            if (!piece) {
                out << ". ";
                continue;
            }
            char symbol = '?';
            switch (piece->type) {
                case PieceType::Pawn: symbol = 'P'; break;
                case PieceType::Knight: symbol = 'N'; break;
                case PieceType::Bishop: symbol = 'B'; break;
                case PieceType::Rook: symbol = 'R'; break;
                case PieceType::Queen: symbol = 'Q'; break;
                case PieceType::King: symbol = 'K'; break;
            }
            if (piece->color == Color::Black) {
                symbol = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
            }
            out << symbol << " ";
        }
        out << '\n';
    }
    out << "  a b c d e f g h\n";
    return out.str();
}

std::string squareName(int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        throw std::out_of_range("Square out of range");
    }
    std::string name;
    name.push_back(static_cast<char>('a' + x));
    name.push_back(static_cast<char>('1' + y));
    return name;
}

int evaluateBoard(const ChessBoard& board, Color perspective) {
    int score = 0;
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const auto piece = board.pieceAt(x, y);
            if (!piece) {
                continue;
            }
            const int value = pieceValue(piece->type);
            int delta = piece->color == Color::White ? value : -value;
            score += delta;
        }
    }

    if (board.isInCheck(Color::White)) score -= 50;
    if (board.isInCheck(Color::Black)) score += 50;

    return perspective == Color::White ? score : -score;
}

std::optional<Move> chooseBestMove(const ChessBoard& board, Color side, int depth) {
    const auto moves = board.generateLegalMoves(side);
    if (moves.empty()) {
        return std::nullopt;
    }

    auto negamax = [&](auto&& self, const ChessBoard& state, Color turn, int remainingDepth, int alpha, int beta) -> int {
        const auto legalMoves = state.generateLegalMoves(turn);
        if (remainingDepth == 0 || legalMoves.empty()) {
            if (legalMoves.empty()) {
                if (state.isInCheck(turn)) {
                    return turn == Color::White ? -100000 + remainingDepth : 100000 - remainingDepth;
                }
                return 0;
            }
            return evaluateBoard(state, side);
        }

        int bestScore = std::numeric_limits<int>::min();
        for (const auto& move : legalMoves) {
            ChessBoard nextState = state;
            nextState.makeMove(move);
            const int score = -self(self, nextState, opposite(turn), remainingDepth - 1, -beta, -alpha);
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, bestScore);
            if (alpha >= beta) {
                break;
            }
        }
        return bestScore;
    };

    int bestScore = std::numeric_limits<int>::min();
    std::optional<Move> bestMove;
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    for (const auto& move : moves) {
        ChessBoard nextState = board;
        nextState.makeMove(move);
        const int score = -negamax(negamax, nextState, opposite(side), depth - 1, -beta, -alpha);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return bestMove;
}

} // namespace chess
