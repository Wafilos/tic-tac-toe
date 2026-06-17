#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>


class TicTacToe {
private:
    static constexpr char EMPTY = '.';
    static constexpr int INF = 1'000'000'000;
    static constexpr int WIN_SCORE = 100'000'000;

    int size;
    int winLength;
    int movesPlayed = 0;

    std::vector<std::vector<char>> board;

    const char human = 'X';
    const char computer = 'O';

    bool isInside(int row, int col) const {
        return row >= 0 && row < size &&
               col >= 0 && col < size;
    }

    bool isBoardFull() const {
        return movesPlayed == size * size;
    }

    bool isWinningMove(int row, int col, char mark) const {
        static const int directions[4][2] = {
            {1, 0},   // pionowo
            {0, 1},   // poziomo
            {1, 1},   // przekątna opadająca
            {1, -1}   // przekątna rosnąca
        };

        for (const auto& direction : directions) {
            const int dr = direction[0];
            const int dc = direction[1];

            int count = 1;

            for (int step = 1;
                 isInside(row + step * dr, col + step * dc) &&
                 board[row + step * dr][col + step * dc] == mark;
                 ++step) {
                ++count;
            }

            for (int step = 1;
                 isInside(row - step * dr, col - step * dc) &&
                 board[row - step * dr][col - step * dc] == mark;
                 ++step) {
                ++count;
            }

            if (count >= winLength) {
                return true;
            }
        }

        return false;
    }

    int lineValue(int marks) const {
        int value = 1;

        for (int i = 0; i < marks; ++i) {
            value = std::min(value * 10, 1'000'000);
        }

        if (marks == winLength - 1) {
            value = std::min(value * 10, 5'000'000);
        }

        return value;
    }

    int evaluateBoard() const {
        static const int directions[4][2] = {
            {1, 0},
            {0, 1},
            {1, 1},
            {1, -1}
        };

        long long score = 0;

        for (int row = 0; row < size; ++row) {
            for (int col = 0; col < size; ++col) {
                for (const auto& direction : directions) {
                    const int endRow =
                        row + (winLength - 1) * direction[0];

                    const int endCol =
                        col + (winLength - 1) * direction[1];

                    if (!isInside(endRow, endCol)) {
                        continue;
                    }

                    int computerMarks = 0;
                    int humanMarks = 0;

                    for (int step = 0; step < winLength; ++step) {
                        const char field =
                            board[row + step * direction[0]]
                                 [col + step * direction[1]];

                        if (field == computer) {
                            ++computerMarks;
                        } else if (field == human) {
                            ++humanMarks;
                        }
                    }

                    if (computerMarks > 0 && humanMarks == 0) {
                        score += lineValue(computerMarks);
                    } else if (humanMarks > 0 && computerMarks == 0) {
                        score -= lineValue(humanMarks);
                    }
                }
            }
        }

        // Niewielka premia za zajmowanie pól bliżej środka.
        const double center = (size - 1) / 2.0;

        for (int row = 0; row < size; ++row) {
            for (int col = 0; col < size; ++col) {
                if (board[row][col] == EMPTY) {
                    continue;
                }

                const int bonus = static_cast<int>(
                    size - (
                        std::abs(row - center) +
                        std::abs(col - center)
                    )
                );

                if (board[row][col] == computer) {
                    score += bonus;
                } else {
                    score -= bonus;
                }
            }
        }

        score = std::max<long long>(-WIN_SCORE / 2, score);
        score = std::min<long long>( WIN_SCORE / 2, score);

        return static_cast<int>(score);
    }

    bool hasOccupiedNeighbour(int row, int col) const {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) {
                    continue;
                }

                const int neighbourRow = row + dr;
                const int neighbourCol = col + dc;

                if (isInside(neighbourRow, neighbourCol) &&
                    board[neighbourRow][neighbourCol] != EMPTY) {
                    return true;
                }
            }
        }

        return false;
    }

    int orderingScore(int row, int col) {
        int score = 0;

        // Ruch wygrywający powinien zostać sprawdzony jako pierwszy.
        board[row][col] = computer;

        if (isWinningMove(row, col, computer)) {
            score += 1'000'000;
        }

        // Następnie sprawdzamy, czy ruch blokuje gracza.
        board[row][col] = human;

        if (isWinningMove(row, col, human)) {
            score += 500'000;
        }

        board[row][col] = EMPTY;

        // Preferowanie pól bliżej środka.
        const double center = (size - 1) / 2.0;

        score -= static_cast<int>(
            10 * (
                std::abs(row - center) +
                std::abs(col - center)
            )
        );

        return score;
    }

    std::vector<std::pair<int, int>> generateMoves() {
        std::vector<std::pair<int, int>> moves;

        if (movesPlayed == 0) {
            moves.emplace_back(size / 2, size / 2);
            return moves;
        }

        const bool useNeighbourFilter = size >= 7;

        for (int row = 0; row < size; ++row) {
            for (int col = 0; col < size; ++col) {
                if (board[row][col] != EMPTY) {
                    continue;
                }

                if (!useNeighbourFilter ||
                    hasOccupiedNeighbour(row, col)) {
                    moves.emplace_back(row, col);
                }
            }
        }

        // Zabezpieczenie na wypadek pustej listy.
        if (moves.empty()) {
            for (int row = 0; row < size; ++row) {
                for (int col = 0; col < size; ++col) {
                    if (board[row][col] == EMPTY) {
                        moves.emplace_back(row, col);
                    }
                }
            }
        }

        /*
            Kolejność ruchów ma znaczenie dla alfa-beta.
            Najbardziej obiecujące ruchy sprawdzamy jako pierwsze.
        */
        std::sort(
            moves.begin(),
            moves.end(),
            [this](const auto& first, const auto& second) {
                return orderingScore(first.first, first.second) >
                       orderingScore(second.first, second.second);
            }
        );

        return moves;
    }

    int chooseSearchDepth() const {
        const int remainingMoves =
            size * size - movesPlayed;

        if (size == 3 && winLength == 3) {
            // Dla klasycznej gry sprawdzamy całe drzewo.
            return remainingMoves;
        }

        if (size <= 4) {
            return std::min(6, remainingMoves);
        }

        if (size <= 5) {
            return std::min(4, remainingMoves);
        }

        return std::min(3, remainingMoves);
    }

    int minimax(
        int depth,
        int alpha,
        int beta,
        bool maximizingPlayer,
        int lastRow,
        int lastCol,
        char lastMark
    ) {
        if (lastRow != -1 &&
            isWinningMove(lastRow, lastCol, lastMark)) {
            if (lastMark == computer) {
                return WIN_SCORE + depth;
            }

            return -WIN_SCORE - depth;
        }

        if (isBoardFull()) {
            return 0;
        }

        if (depth == 0) {
            return evaluateBoard();
        }

        const auto moves = generateMoves();

        if (maximizingPlayer) {
            int bestScore = -INF;

            for (const auto& [row, col] : moves) {
                board[row][col] = computer;
                ++movesPlayed;

                const int score = minimax(
                    depth - 1,
                    alpha,
                    beta,
                    false,
                    row,
                    col,
                    computer
                );

                board[row][col] = EMPTY;
                --movesPlayed;

                bestScore = std::max(bestScore, score);
                alpha = std::max(alpha, bestScore);

                if (beta <= alpha) {
                    break; // cięcie alfa-beta
                }
            }

            return bestScore;
        }

        int bestScore = INF;

        for (const auto& [row, col] : moves) {
            board[row][col] = human;
            ++movesPlayed;

            const int score = minimax(
                depth - 1,
                alpha,
                beta,
                true,
                row,
                col,
                human
            );

            board[row][col] = EMPTY;
            --movesPlayed;

            bestScore = std::min(bestScore, score);
            beta = std::min(beta, bestScore);

            if (beta <= alpha) {
                break; // cięcie alfa-beta
            }
        }

        return bestScore;
    }

    std::pair<int, int> findBestComputerMove() {
        const auto moves = generateMoves();
        const int depth = chooseSearchDepth();

        int bestScore = -INF;
        std::pair<int, int> bestMove = moves.front();

        for (const auto& [row, col] : moves) {
            board[row][col] = computer;
            ++movesPlayed;

            const int score = minimax(
                depth - 1,
                -INF,
                INF,
                false,
                row,
                col,
                computer
            );

            board[row][col] = EMPTY;
            --movesPlayed;

            if (score > bestScore) {
                bestScore = score;
                bestMove = {row, col};
            }
        }

        return bestMove;
    }

    void printBoard() const {
        std::cout << "\n    ";

        for (int col = 0; col < size; ++col) {
            std::cout << col + 1 << ' ';
        }

        std::cout << "\n";

        for (int row = 0; row < size; ++row) {
            std::cout << row + 1 << "   ";

            for (int col = 0; col < size; ++col) {
                std::cout << board[row][col] << ' ';
            }

            std::cout << "\n";
        }

        std::cout << "\n";
    }

    std::pair<int, int> readHumanMove() const {
        while (true) {
            int row;
            int col;

            std::cout << "Podaj wiersz i kolumne: ";

            if (!(std::cin >> row >> col)) {
                std::cin.clear();

                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(),
                    '\n'
                );

                std::cout
                    << "Niepoprawne dane. Wpisz dwie liczby.\n";

                continue;
            }

            --row;
            --col;

            if (!isInside(row, col)) {
                std::cout
                    << "Pole znajduje sie poza plansza.\n";

                continue;
            }

            if (board[row][col] != EMPTY) {
                std::cout
                    << "To pole jest juz zajete.\n";

                continue;
            }

            return {row, col};
        }
    }

public:
    TicTacToe(int boardSize, int requiredMarks)
        : size(boardSize),
          winLength(requiredMarks),
          board(
              boardSize,
              std::vector<char>(boardSize, EMPTY)
          ) {}

    void play() {
        std::cout << "Grasz jako X. Komputer gra jako O.\n";

        std::cout
            << "Aby wygrac, ustaw "
            << winLength
            << " znakow w jednym rzedzie.\n";

        printBoard();

        while (true) {
            const auto [humanRow, humanCol] =
                readHumanMove();

            board[humanRow][humanCol] = human;
            ++movesPlayed;

            printBoard();

            if (isWinningMove(humanRow, humanCol, human)) {
                std::cout << "Wygrales!\n";
                system("pause"); 
                return;
            }

            if (isBoardFull()) {
                std::cout << "Remis.\n";
                system("pause"); 
                return;
            }

            const auto [computerRow, computerCol] =
                findBestComputerMove();

            board[computerRow][computerCol] = computer;
            ++movesPlayed;

            printBoard();

            if (isWinningMove(
                    computerRow,
                    computerCol,
                    computer
                )) {
                std::cout << "Komputer wygral.\n";
                system("pause"); 
                return;
            }

            if (isBoardFull()) {
                std::cout << "Remis.\n";
                system("pause"); 
                return;
            }
        }
    }
};

int main() {
    int boardSize;
    int requiredMarks;

    std::cout
        << "Podaj rozmiar planszy N (N x N): ";

    std::cin >> boardSize;

    std::cout
        << "Podaj liczbe znakow potrzebnych do wygranej: ";

    std::cin >> requiredMarks;

    if (!std::cin ||
        boardSize < 3 ||
        boardSize > 7 ||
        requiredMarks < 3 ||
        requiredMarks > boardSize) {
        std::cout
            << "Niepoprawna konfiguracja. "
            << "Wymagane: 3 <= N <= 7 oraz 3 <= K <= N.\n";
            system("pause"); 

        return 1;
    }

    TicTacToe game(boardSize, requiredMarks);
    game.play();

    return 0;
}