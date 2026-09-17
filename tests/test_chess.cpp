#include "chess_engine.hpp"

#include <cassert>
#include <iostream>

using namespace chess;

int main() {
    ChessBoard start;
    start.reset();
    assert(start.generateLegalMoves(Color::White).size() == 20);

    ChessBoard checkBoard;
    checkBoard.clear();
    checkBoard.setPieceAt(4, 0, Piece{PieceType::King, Color::White});
    checkBoard.setPieceAt(4, 7, Piece{PieceType::King, Color::Black});
    checkBoard.setPieceAt(4, 6, Piece{PieceType::Queen, Color::Black});
    assert(checkBoard.isInCheck(Color::White));

    ChessBoard safeBoard;
    safeBoard.clear();
    safeBoard.setPieceAt(4, 0, Piece{PieceType::King, Color::White});
    safeBoard.setPieceAt(4, 7, Piece{PieceType::King, Color::Black});
    safeBoard.setPieceAt(4, 1, Piece{PieceType::Rook, Color::White});
    safeBoard.setPieceAt(5, 6, Piece{PieceType::Rook, Color::Black});
    assert(!safeBoard.isInCheck(Color::White));

    const auto best = chooseBestMove(start, Color::White, 1);
    assert(best.has_value());

    std::cout << "All chess engine checks passed.\n";
    return 0;
}
