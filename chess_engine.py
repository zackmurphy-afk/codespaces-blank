from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

FILES = "abcdefgh"
RANKS = "12345678"
SQUARES = [f"{file}{rank}" for rank in RANKS for file in FILES]

PIECE_VALUES = {
    "P": 100,
    "N": 320,
    "B": 330,
    "R": 500,
    "Q": 900,
    "K": 20000,
}


@dataclass(frozen=True)
class Move:
    from_sq: str
    to_sq: str
    piece: str
    capture: Optional[str] = None
    promotion: Optional[str] = None
    is_castle: bool = False
    castle_side: Optional[str] = None
    en_passant: bool = False

    def __str__(self) -> str:
        if self.is_castle:
            return "O-O" if self.castle_side == "king" else "O-O-O"
        move = f"{self.from_sq}{self.to_sq}"
        if self.promotion:
            move += self.promotion.lower()
        return move


class ChessBoard:
    def __init__(self) -> None:
        self.board: Dict[str, str] = {}
        self.turn = "white"
        self.en_passant_target: Optional[str] = None
        self.move_history: List[Move] = []

    def clear(self) -> None:
        self.board.clear()
        self.en_passant_target = None
        self.move_history.clear()

    def reset(self) -> None:
        self.clear()
        for file in FILES:
            self.set_piece(f"{file}2", "wP")
            self.set_piece(f"{file}7", "bP")
        for piece, squares in {
            "wR": ["a1", "h1"],
            "wN": ["b1", "g1"],
            "wB": ["c1", "f1"],
            "wQ": ["d1"],
            "wK": ["e1"],
            "bR": ["a8", "h8"],
            "bN": ["b8", "g8"],
            "bB": ["c8", "f8"],
            "bQ": ["d8"],
            "bK": ["e8"],
        }.items():
            for sq in squares:
                self.set_piece(sq, piece)
        self.turn = "white"

    def copy(self) -> "ChessBoard":
        new_board = ChessBoard()
        new_board.board = dict(self.board)
        new_board.turn = self.turn
        new_board.en_passant_target = self.en_passant_target
        new_board.move_history = list(self.move_history)
        return new_board

    def set_piece(self, square: str, piece: str) -> None:
        if not self.is_valid_square(square):
            raise ValueError(f"Invalid square: {square}")
        if piece is None:
            self.board.pop(square, None)
        else:
            self.board[square] = piece

    def piece_at(self, square: str) -> Optional[str]:
        return self.board.get(square)

    def is_valid_square(self, square: str) -> bool:
        return square in SQUARES

    def color_of(self, piece: Optional[str]) -> Optional[str]:
        if not piece:
            return None
        return "white" if piece[0] == "w" else "black"

    def find_king(self, color: str) -> Optional[str]:
        king_piece = "wK" if color == "white" else "bK"
        for square, piece in self.board.items():
            if piece == king_piece:
                return square
        return None

    def in_bounds(self, file_idx: int, rank_idx: int) -> bool:
        return 0 <= file_idx < 8 and 0 <= rank_idx < 8

    def sq_to_coords(self, square: str) -> Tuple[int, int]:
        file = FILES.index(square[0])
        rank = int(square[1]) - 1
        return file, rank

    def coords_to_sq(self, file_idx: int, rank_idx: int) -> str:
        return f"{FILES[file_idx]}{rank_idx + 1}"

    def is_in_check(self, color: str) -> bool:
        king_sq = self.find_king(color)
        if not king_sq:
            return False
        return self.is_square_attacked(king_sq, "black" if color == "white" else "white")

    def is_square_attacked(self, square: str, by_color: str) -> bool:
        for sq, piece in self.board.items():
            if self.color_of(piece) != by_color:
                continue
            if self._piece_attacks_square(piece, sq, square):
                return True
        return False

    def _piece_attacks_square(self, piece: str, from_sq: str, target_sq: str) -> bool:
        color = self.color_of(piece)
        piece_type = piece[1]
        file_from, rank_from = self.sq_to_coords(from_sq)
        file_to, rank_to = self.sq_to_coords(target_sq)

        if piece_type == "P":
            direction = 1 if color == "white" else -1
            return (abs(file_to - file_from) == 1 and rank_to - rank_from == direction)

        if piece_type == "N":
            deltas = [(1, 2), (2, 1), (2, -1), (1, -2), (-1, -2), (-2, -1), (-2, 1), (-1, 2)]
            return (abs(file_to - file_from), abs(rank_to - rank_from)) in [(1, 2), (2, 1)]

        if piece_type in {"B", "R", "Q"}:
            directions = []
            if piece_type in {"B", "Q"}:
                directions.extend([(1, 1), (1, -1), (-1, 1), (-1, -1)])
            if piece_type in {"R", "Q"}:
                directions.extend([(1, 0), (-1, 0), (0, 1), (0, -1)])

            for df, dr in directions:
                fx, fy = file_from + df, rank_from + dr
                while self.in_bounds(fx, fy):
                    sq = self.coords_to_sq(fx, fy)
                    if sq == target_sq:
                        return True
                    if sq in self.board:
                        break
                    fx += df
                    fy += dr
            return False

        if piece_type == "K":
            return max(abs(file_to - file_from), abs(rank_to - rank_from)) == 1

        return False

    def generate_pseudo_legal_moves(self, color: str) -> List[Move]:
        moves: List[Move] = []
        for square, piece in list(self.board.items()):
            if self.color_of(piece) != color:
                continue
            piece_type = piece[1]
            if piece_type == "P":
                direction = 1 if color == "white" else -1
                start_rank = 2 if color == "white" else 7
                file_idx, rank_idx = self.sq_to_coords(square)
                forward_one = rank_idx + direction
                one_sq = self.coords_to_sq(file_idx, forward_one)
                if self.in_bounds(file_idx, forward_one) and one_sq not in self.board:
                    moves.append(Move(square, one_sq, piece, promotion=("Q" if forward_one in (7, 0) else None)))
                    if int(square[1]) == start_rank:
                        two_rank = rank_idx + 2 * direction
                        two_sq = self.coords_to_sq(file_idx, two_rank)
                        if self.in_bounds(file_idx, two_rank) and two_sq not in self.board:
                            moves.append(Move(square, two_sq, piece))
                for df in (-1, 1):
                    fx = file_idx + df
                    fy = rank_idx + direction
                    if not self.in_bounds(fx, fy):
                        continue
                    target = self.coords_to_sq(fx, fy)
                    if target in self.board and self.color_of(self.board[target]) != color:
                        moves.append(Move(square, target, piece, capture=self.board[target], promotion="Q"))
                    elif self.en_passant_target == target:
                        captured_square = self.coords_to_sq(fx, rank_idx)
                        captured_piece = self.board.get(captured_square)
                        if captured_piece and self.color_of(captured_piece) != color:
                            moves.append(Move(square, target, piece, capture=captured_piece, en_passant=True))
            elif piece_type == "N":
                file_idx, rank_idx = self.sq_to_coords(square)
                for df, dr in [(1, 2), (2, 1), (2, -1), (1, -2), (-1, -2), (-2, -1), (-2, 1), (-1, 2)]:
                    fx, fy = file_idx + df, rank_idx + dr
                    if not self.in_bounds(fx, fy):
                        continue
                    target = self.coords_to_sq(fx, fy)
                    occupant = self.board.get(target)
                    if occupant is None or self.color_of(occupant) != color:
                        moves.append(Move(square, target, piece, capture=occupant))
            elif piece_type in {"B", "R", "Q"}:
                file_idx, rank_idx = self.sq_to_coords(square)
                directions = []
                if piece_type in {"B", "Q"}:
                    directions.extend([(1, 1), (1, -1), (-1, 1), (-1, -1)])
                if piece_type in {"R", "Q"}:
                    directions.extend([(1, 0), (-1, 0), (0, 1), (0, -1)])
                for df, dr in directions:
                    fx, fy = file_idx + df, rank_idx + dr
                    while self.in_bounds(fx, fy):
                        target = self.coords_to_sq(fx, fy)
                        occupant = self.board.get(target)
                        if occupant is None:
                            moves.append(Move(square, target, piece))
                        else:
                            if self.color_of(occupant) != color:
                                moves.append(Move(square, target, piece, capture=occupant))
                            break
                        fx += df
                        fy += dr
            elif piece_type == "K":
                file_idx, rank_idx = self.sq_to_coords(square)
                for df in (-1, 0, 1):
                    for dr in (-1, 0, 1):
                        if df == 0 and dr == 0:
                            continue
                        fx, fy = file_idx + df, rank_idx + dr
                        if not self.in_bounds(fx, fy):
                            continue
                        target = self.coords_to_sq(fx, fy)
                        occupant = self.board.get(target)
                        if occupant is None or self.color_of(occupant) != color:
                            moves.append(Move(square, target, piece, capture=occupant))
                if color == "white":
                    if square == "e1" and self.board.get("h1") == "wR" and self.board.get("f1") is None and self.board.get("g1") is None and not self.is_in_check("white"):
                        moves.append(Move("e1", "g1", "wK", is_castle=True, castle_side="king"))
                    if square == "e1" and self.board.get("a1") == "wR" and self.board.get("d1") is None and self.board.get("c1") is None and self.board.get("b1") is None and not self.is_in_check("white"):
                        moves.append(Move("e1", "c1", "wK", is_castle=True, castle_side="queen"))
                if color == "black":
                    if square == "e8" and self.board.get("h8") == "bR" and self.board.get("f8") is None and self.board.get("g8") is None and not self.is_in_check("black"):
                        moves.append(Move("e8", "g8", "bK", is_castle=True, castle_side="king"))
                    if square == "e8" and self.board.get("a8") == "bR" and self.board.get("d8") is None and self.board.get("c8") is None and self.board.get("b8") is None and not self.is_in_check("black"):
                        moves.append(Move("e8", "c8", "bK", is_castle=True, castle_side="queen"))
        return moves

    def generate_legal_moves(self, color: str) -> List[Move]:
        legal_moves: List[Move] = []
        for move in self.generate_pseudo_legal_moves(color):
            next_board = self.copy()
            next_board.make_move(move)
            if not next_board.is_in_check(color):
                legal_moves.append(move)
        return legal_moves

    def make_move(self, move: Move) -> None:
        piece = self.board.get(move.from_sq)
        if piece is None:
            raise ValueError(f"No piece at {move.from_sq}")

        self.board.pop(move.from_sq, None)

        if move.en_passant:
            captured_rank = int(move.from_sq[1])
            captured_square = f"{move.to_sq[0]}{captured_rank}"
            self.board.pop(captured_square, None)

        if move.capture:
            self.board.pop(move.to_sq, None)

        if move.is_castle:
            if move.castle_side == "king":
                if move.from_sq == "e1":
                    self.board.pop("h1", None)
                    self.board["f1"] = "wR"
                elif move.from_sq == "e8":
                    self.board.pop("h8", None)
                    self.board["f8"] = "bR"
            elif move.castle_side == "queen":
                if move.from_sq == "e1":
                    self.board.pop("a1", None)
                    self.board["d1"] = "wR"
                elif move.from_sq == "e8":
                    self.board.pop("a8", None)
                    self.board["d8"] = "bR"

        destination_piece = piece
        if move.promotion:
            destination_piece = piece[0] + move.promotion
        self.board[move.to_sq] = destination_piece

        self.move_history.append(move)
        self.en_passant_target = None
        if piece[1] == "P" and abs(int(move.to_sq[1]) - int(move.from_sq[1])) == 2:
            self.en_passant_target = self.coords_to_sq((self.sq_to_coords(move.from_sq)[0]), (self.sq_to_coords(move.from_sq)[1] + (1 if piece[0] == "w" else -1)))

    def __str__(self) -> str:
        rows = []
        for rank in range(8, 0, -1):
            cells = []
            for file in FILES:
                sq = f"{file}{rank}"
                piece = self.board.get(sq, ".")
                cells.append(piece if piece != "." else "·")
            rows.append(f"{rank} {' '.join(cells)}")
        rows.append("  " + " ".join(FILES))
        return "\n".join(rows)


def opponent(color: str) -> str:
    return "black" if color == "white" else "white"


def static_evaluation(board: ChessBoard, perspective: str = "white") -> int:
    score = 0
    for piece in board.board.values():
        value = PIECE_VALUES[piece[1]]
        if piece[0] == "w":
            score += value
        else:
            score -= value

    for square, piece in board.board.items():
        file_idx, rank_idx = board.sq_to_coords(square)
        if piece[0] == "w":
            score += (3 - abs(3 - file_idx)) // 2
            score += rank_idx * 2
        else:
            score -= (3 - abs(3 - file_idx)) // 2
            score -= (7 - rank_idx) * 2

    if board.is_in_check("white"):
        score -= 50
    if board.is_in_check("black"):
        score += 50

    if perspective == "black":
        return -score
    return score


def choose_best_move(board: ChessBoard, color: str, depth: int = 2) -> Optional[Move]:
    legal_moves = board.generate_legal_moves(color)
    if not legal_moves:
        return None

    best_move = legal_moves[0]
    best_score = -float("inf")

    alpha = -float("inf")
    beta = float("inf")

    for move in legal_moves:
        next_board = board.copy()
        next_board.make_move(move)
        score = minimax(next_board, opponent(color), depth - 1, alpha, beta)
        if color == "black":
            score = -score
        if score > best_score:
            best_score = score
            best_move = move
        if color == "white" and score > alpha:
            alpha = score

    return best_move


def minimax(board: ChessBoard, color: str, depth: int, alpha: float, beta: float) -> float:
    legal_moves = board.generate_legal_moves(color)
    if depth == 0 or not legal_moves:
        if not legal_moves:
            if board.is_in_check(color):
                return -100000 + (5 - depth) if color == "white" else 100000 - (5 - depth)
            return 0
        return static_evaluation(board, "white")

    best_score = -float("inf")
    for move in legal_moves:
        next_board = board.copy()
        next_board.make_move(move)
        score = minimax(next_board, opponent(color), depth - 1, alpha, beta)
        if color == "white":
            best_score = max(best_score, score)
            alpha = max(alpha, best_score)
        else:
            best_score = max(best_score, -score)
            beta = min(beta, best_score)
        if alpha >= beta:
            break
    return best_score


def parse_move(move_str: str) -> Move:
    if len(move_str) not in {4, 5}:
        raise ValueError("Move must be in the form e2e4 or e7e8q")
    from_sq = move_str[:2]
    to_sq = move_str[2:4]
    promotion = move_str[4].upper() if len(move_str) == 5 else None
    return Move(from_sq, to_sq, "P", capture=None, promotion=promotion)


def play_game() -> None:
    board = ChessBoard()
    board.reset()
    print("Welcome to CLI Chess. Enter moves like e2e4. Type 'quit' to exit.")
    while True:
        print(board)
        if board.turn == "white":
            move_str = input("White move: ")
            if move_str.strip().lower() == "quit":
                break
            start, end = move_str[:2], move_str[2:4]
            legal = board.generate_legal_moves("white")
            chosen = None
            for move in legal:
                if move.from_sq == start and move.to_sq == end:
                    chosen = move
                    break
            if chosen is None:
                print("Illegal move.")
                continue
            board.make_move(chosen)
            board.turn = "black"
        else:
            engine_move = choose_best_move(board, "black", depth=2)
            if engine_move is None:
                print("Black has no legal moves.")
                break
            print(f"Black plays {engine_move}")
            board.make_move(engine_move)
            board.turn = "white"


if __name__ == "__main__":
    play_game()
