#include "chess_engine.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace chess;

namespace {

std::string trim(const std::string& input) {
    std::string result = input;
    while (!result.empty() && (result.front() == ' ' || result.front() == '\t' || result.front() == '\n' || result.front() == '\r')) {
        result.erase(result.begin());
    }
    while (!result.empty() && (result.back() == ' ' || result.back() == '\t' || result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

std::vector<std::string> tokenize(const std::string& input) {
    std::istringstream stream(input);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::optional<Move> parseUserMove(const std::string& text) {
    auto tokens = tokenize(trim(text));
    if (tokens.empty()) {
        return std::nullopt;
    }

    std::string compact = tokens[0];
    if (compact.size() == 4) {
        const std::string from = compact.substr(0, 2);
        const std::string to = compact.substr(2, 2);
        return Move{static_cast<int>(from[0] - 'a'), static_cast<int>(from[1] - '1'), static_cast<int>(to[0] - 'a'), static_cast<int>(to[1] - '1'), std::nullopt, false};
    }

    if (tokens.size() == 2 && tokens[0].size() == 2 && tokens[1].size() == 2) {
        const std::string from = tokens[0];
        const std::string to = tokens[1];
        return Move{static_cast<int>(from[0] - 'a'), static_cast<int>(from[1] - '1'), static_cast<int>(to[0] - 'a'), static_cast<int>(to[1] - '1'), std::nullopt, false};
    }

    return std::nullopt;
}

} // namespace

int main() {
    ChessBoard board;
    board.reset();
    Color sideToMove = Color::White;

    std::cout << "C++ Chess Engine\n";
    std::cout << "Commands: e2e4 or e2 e4 | quit\n\n";
    while (true) {
        std::cout << board.toString();
        std::cout << (sideToMove == Color::White ? "White to move: " : "Black to move: ");

        std::string input;
        std::getline(std::cin, input);
        if (trim(input) == "quit") {
            break;
        }

        if (sideToMove == Color::White) {
            const auto move = parseUserMove(input);
            if (!move) {
                std::cout << "Illegal move format. Try e2e4 or e2 e4.\n";
                continue;
            }

            const auto legalMoves = board.generateLegalMoves(Color::White);
            bool legal = false;
            for (const auto& legalMove : legalMoves) {
                if (legalMove.fromX == move->fromX && legalMove.fromY == move->fromY &&
                    legalMove.toX == move->toX && legalMove.toY == move->toY) {
                    legal = true;
                    break;
                }
            }

            if (!legal) {
                std::cout << "Illegal move.\n";
                continue;
            }

            board.makeMove(*move);
            sideToMove = Color::Black;
        } else {
            const auto engineMove = chooseBestMove(board, Color::Black, 2);
            if (!engineMove) {
                std::cout << "Black has no legal moves.\n";
                break;
            }
            std::cout << "Black plays: " << squareName(engineMove->fromX, engineMove->fromY)
                      << squareName(engineMove->toX, engineMove->toY) << "\n";
            board.makeMove(*engineMove);
            sideToMove = Color::White;
        }
    }

    return 0;
}
