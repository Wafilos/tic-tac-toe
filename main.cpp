#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <iomanip>


struct Move {
    int row;
    int col;
    char player;
    double moveTimeSeconds;

    Move() = default;
    Move(int r, int c, char p, double t = 0.0) : row(r), col(c), player(p), moveTimeSeconds(t) {}
};

struct GameData {
    int boardSize;
    int winLength;
    std::string result;  // "human_win", "computer_win", "draw"
    int totalMoves;
    double durationSeconds;
    std::vector<Move> moveHistory;
};

class GameReport {
private:
    std::vector<GameData> games;
    std::chrono::steady_clock::time_point lastMoveTime;

public:
    void recordMove(int row, int col, char player) {
        if (!games.empty()) {
            auto currentTime = std::chrono::steady_clock::now();
            double moveTime = 0.0;

            if (games.back().moveHistory.empty()) {
                // First move in the game
                moveTime = 0.0;
            } else {
                moveTime = std::chrono::duration<double>(
                    currentTime - lastMoveTime
                ).count();
            }

            games.back().moveHistory.emplace_back(row, col, player, moveTime);
            lastMoveTime = currentTime;
        }
    }

    void startNewGame(int boardSize, int winLength) {
        GameData newGame;
        newGame.boardSize = boardSize;
        newGame.winLength = winLength;
        newGame.result = "";
        newGame.totalMoves = 0;
        newGame.durationSeconds = 0.0;
        games.push_back(newGame);
        lastMoveTime = std::chrono::steady_clock::now();
    }

    void endGame(
        const std::string& result,
        int totalMoves,
        double durationSeconds
    ) {
        if (!games.empty()) {
            games.back().result = result;
            games.back().totalMoves = totalMoves;
            games.back().durationSeconds = durationSeconds;
        }
    }

    void printReport() const {
        if (games.empty()) {
            std::cout << "\nBrak danych gier w raporcie.\n";
            return;
        }

        std::cout << "\n========== RAPORT Z GIER ==========\n";
        std::cout << "Liczba rozegranych gier: " << games.size() << "\n\n";

        // Summary statistics
        int humanWins = 0;
        int computerWins = 0;
        int draws = 0;
        double totalDuration = 0.0;
        int totalMovesSum = 0;

        for (const auto& game : games) {
            if (game.result == "human_win") {
                ++humanWins;
            } else if (game.result == "computer_win") {
                ++computerWins;
            } else {
                ++draws;
            }
            totalDuration += game.durationSeconds;
            totalMovesSum += game.totalMoves;
        }

        std::cout << "--- PODSUMOWANIE ---\n";
        std::cout << "Wygrane gracza:   " << humanWins << "\n";
        std::cout << "Wygrane komputera: " << computerWins << "\n";
        std::cout << "Remisy:           " << draws << "\n";
        std::cout << "Srednia liczba ruchow: " << std::fixed << std::setprecision(1)
                  << static_cast<double>(totalMovesSum) / games.size() << "\n";
        std::cout << "Sredni czas gry (s):   " << std::fixed << std::setprecision(2)
                  << totalDuration / games.size() << "\n";

        // Detailed game logs
        std::cout << "\n--- SZCZEGOLOWE LOGI GIER ---\n";
        for (size_t i = 0; i < games.size(); ++i) {
            printGameDetails(i + 1);
        }

        std::cout << "\n====================================\n";
    }

    void printGameDetails(size_t gameNumber) const {
        if (gameNumber < 1 || gameNumber > games.size()) {
            std::cout << "Niepoprawny numer gry.\n";
            return;
        }

        const auto& game = games[gameNumber - 1];

        std::cout << "\nGra " << gameNumber << ":\n";
        std::cout << "  Rozmiar planszy: " << game.boardSize << "x"
                  << game.boardSize << "\n";
        std::cout << "  Do wygranej potrzeba: " << game.winLength << " znakow\n";
        std::cout << "  Wynik: " << game.result << "\n";
        std::cout << "  Liczba ruchow: " << game.totalMoves << "\n";
        std::cout << "  Czas trwania: " << std::fixed << std::setprecision(2)
                  << game.durationSeconds << " sekund\n";

        std::cout << "  Historia ruchow:\n";
        double cumulativeTime = 0.0;
        for (size_t j = 0; j < game.moveHistory.size(); ++j) {
            const auto& move = game.moveHistory[j];
            cumulativeTime += move.moveTimeSeconds;
            std::cout << "    " << j + 1 << ". "
                      << (move.player == 'X' ? "Gracz" : "Komputer")
                      << " - wiersz " << move.row + 1
                      << ", kolumna " << move.col + 1
                      << " (czas ruchu: " << std::fixed << std::setprecision(2)
                      << move.moveTimeSeconds << "s, lacznie: "
                      << cumulativeTime << "s)\n";
        }
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            std::cout << "Blad: Nie mozna otworzyc pliku " << filename << "\n";
            return;
        }

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        file << "\n========== RAPORT Z DNIA "
             << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
             << " ==========\n";
        file << "Liczba gier: " << games.size() << "\n";

        int humanWins = 0;
        int computerWins = 0;
        int draws = 0;

        for (const auto& game : games) {
            if (game.result == "human_win") {
                ++humanWins;
            } else if (game.result == "computer_win") {
                ++computerWins;
            } else {
                ++draws;
            }
        }

        file << "Wygrane gracza: " << humanWins << "\n";
        file << "Wygrane komputera: " << computerWins << "\n";
        file << "Remisy: " << draws << "\n";

        file.close();
        std::cout << "\nRaport zapisany do pliku: " << filename << "\n";
    }
};


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

    GameReport& report;
    std::chrono::steady_clock::time_point gameStartTime;

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

        // Ruch wygrywający.
        board[row][col] = computer;

        if (isWinningMove(row, col, computer)) {
            score += 1'000'000;
        }

        // blokowanie gracza
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
            Najbardziej obiecujące ruchy sprawdzane jako pierwsze.
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
        return 6;
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
                // Validate square is empty before placing mark
                if (board[row][col] != EMPTY) {
                    continue;
                }

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
            if (board[row][col] != EMPTY) {
                continue;
            }

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
            // Validate square is empty before placing mark
            if (board[row][col] != EMPTY) {
                continue;
            }

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
    TicTacToe(int boardSize, int requiredMarks, GameReport& gameReport)
        : size(boardSize),
          winLength(requiredMarks),
          board(
              boardSize,
              std::vector<char>(boardSize, EMPTY)
          ),
          report(gameReport) {
        report.startNewGame(boardSize, requiredMarks);
    }

    void play() {
        gameStartTime = std::chrono::steady_clock::now();

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
            report.recordMove(humanRow, humanCol, human);

            printBoard();

            if (isWinningMove(humanRow, humanCol, human)) {
                auto gameEndTime = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double>(
                    gameEndTime - gameStartTime
                ).count();
                report.endGame("human_win", movesPlayed, duration);

                std::cout << "Wygrales!\n";
                system("pause"); 
                return;
            }

            if (isBoardFull()) {
                auto gameEndTime = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double>(
                    gameEndTime - gameStartTime
                ).count();
                report.endGame("draw", movesPlayed, duration);

                std::cout << "Remis.\n";
                system("pause"); 
                return;
            }

            const auto [computerRow, computerCol] =
                findBestComputerMove();

            board[computerRow][computerCol] = computer;
            ++movesPlayed;
            report.recordMove(computerRow, computerCol, computer);

            printBoard();

            if (isWinningMove(
                    computerRow,
                    computerCol,
                    computer
                )) {
                auto gameEndTime = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double>(
                    gameEndTime - gameStartTime
                ).count();
                report.endGame("computer_win", movesPlayed, duration);

                std::cout << "Komputer wygral.\n";
                system("pause"); 
                return;
            }

            if (isBoardFull()) {
                auto gameEndTime = std::chrono::steady_clock::now();
                double duration = std::chrono::duration<double>(
                    gameEndTime - gameStartTime
                ).count();
                report.endGame("draw", movesPlayed, duration);

                std::cout << "Remis.\n";
                system("pause"); 
                return;
            }
        }
    }
};

int main() {
    GameReport report;

    while (true) {
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

        TicTacToe game(boardSize, requiredMarks, report);
        game.play();

        // Offer to play again
        std::cout << "\nCzy chcesz zagrac jeszcze raz? (t/n): ";
        char choice;
        std::cin >> choice;

        if (choice != 't' && choice != 'T') {
            break;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Display report
    report.printReport();

    std::cout << "\nCzy chcesz zapisac raport do pliku? (t/n): ";
    char saveChoice;
    std::cin >> saveChoice;

    if (saveChoice == 't' || saveChoice == 'T') {
        report.saveToFile("tictactoe_raport.txt");
    }

    system("pause");
    return 0;
}