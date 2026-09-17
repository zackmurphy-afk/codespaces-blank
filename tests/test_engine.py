import unittest

from chess_engine import ChessBoard, Move, choose_best_move


class TestChessEngine(unittest.TestCase):
    def test_initial_legal_moves_count(self):
        board = ChessBoard()
        board.reset()
        self.assertEqual(len(board.generate_legal_moves("white")), 20)

    def test_detects_check(self):
        board = ChessBoard()
        board.clear()
        board.set_piece("e1", "wK")
        board.set_piece("e8", "bK")
        board.set_piece("e2", "wR")
        board.set_piece("f7", "bR")
        self.assertFalse(board.is_in_check("white"))
        board.set_piece("e7", "bQ")
        self.assertTrue(board.is_in_check("white"))

    def test_best_move_returns_valid_move(self):
        board = ChessBoard()
        board.reset()
        move = choose_best_move(board, "white", depth=1)
        self.assertIsInstance(move, Move)
        self.assertNotEqual(move.from_sq, move.to_sq)


if __name__ == "__main__":
    unittest.main()
