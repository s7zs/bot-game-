#include <vector>
#include <iostream>
#include <cstdlib>
#include <gl/glut.h>
#include <utility>
#include <algorithm>

using namespace std;

const int BOARD_SIZE = 5;
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

int selectedRow = 0; // Row of the currently selected token
int selectedCol = 0; // Column of the currently selected token


enum Player { RED, GREEN };
Player currentPlayer = RED;

struct Token {
    int row, col;
};

std::vector<Token> redTokens, greenTokens;
char board[BOARD_SIZE][BOARD_SIZE]; // Board state for AI logic
bool gameRunning = true;
void initTokens() {
    redTokens.clear();
    greenTokens.clear();

    for (int i = 1; i < BOARD_SIZE-1; i++) { // Red tokens start from row 1
        redTokens.push_back({ i, 0 }); // Red tokens on the left column
    }

    for (int j = 1; j < BOARD_SIZE-1; j++) { // Green tokens start from column 1
        greenTokens.push_back({ 0, j }); // Green tokens on the top row
    }

    // Initialize board state
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = '.';
        }
    }

    for (const auto& token : redTokens) {
        board[token.row][token.col] = 'R';
    }

    for (const auto& token : greenTokens) {
        board[token.row][token.col] = 'G';
    }

    // Ensure the top-left box is empty
    board[0][0] = '.';
}


// Check if coordinates are within bounds
bool isInside(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}
bool moveToken(char player, int row, int col) {
    int dr = 0, dc = 0;
    char opponent = (player == 'G') ? 'R' : 'G';

    // Set movement direction
    if (player == 'G') dr = 1; // Green moves down
    else dc = 1;               // Red moves right

    int nextRow = row + dr;
    int nextCol = col + dc;

    // Case 1: One-step forward
    if (isInside(nextRow, nextCol) && board[nextRow][nextCol] == '.') {
        if ((player == 'G' && nextRow == BOARD_SIZE - 1) ||
            (player == 'R' && nextCol == BOARD_SIZE - 1)) {
            board[row][col] = '.'; // Token disappears at the end
        }
        else {
            board[row][col] = '.';
            board[nextRow][nextCol] = player;
        }
        return true;
    }

    // Case 2: Jump over one opponent token
    int jumpRow = row + 2 * dr;
    int jumpCol = col + 2 * dc;

    if (isInside(nextRow, nextCol) && board[nextRow][nextCol] == opponent &&
        isInside(jumpRow, jumpCol) && board[jumpRow][jumpCol] == '.') {
        if ((player == 'G' && jumpRow == BOARD_SIZE - 1) ||
            (player == 'R' && jumpCol == BOARD_SIZE - 1)) {
            board[row][col] = '.'; // Token disappears after jump
        }
        else {
            board[row][col] = '.';
            board[jumpRow][jumpCol] = player;
        }
        return true;
    }

    return false;
}


bool checkWin(char player) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == player) {
                return false; // Player still has tokens on the board
            }
        }
    }
    return true; // All tokens are gone
}

// Minimax algorithm
int evaluateBoard() {
    if (checkWin('G')) return -10; // Green wins
    if (checkWin('R')) return 10;  // Red wins
    return 0; // Neutral state
}

int minimax(int depth, bool isMaximizing) {

    int score = evaluateBoard();

    if (score == 10 || score == -10) return score;
    if (depth == 0) return 0;

    if (isMaximizing) {
        int best = -1000;
        for (int i = 0; i < BOARD_SIZE - 1; i++) {
            for (int j = 0; j < BOARD_SIZE - 1; j++) {
                if (board[i][j] == 'R') {
                    int nextCol = j + 1;
                    if (isInside(i, nextCol) && board[i][nextCol] == '.') {
                        board[i][j] = '.';
                        board[i][nextCol] = 'R';
                        best = max(best, minimax(depth - 1, false));
                        board[i][nextCol] = '.';
                        board[i][j] = 'R';
                    }
                }
            }
        }
        return best;
    }
    else {
        int best = 1000;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == 'G') {
                    int nextRow = i + 1;
                    if (isInside(nextRow, j) && board[nextRow][j] == '.') {
                        board[i][j] = '.';
                        board[nextRow][j] = 'G';
                        best = min(best, minimax(depth - 1, true));
                        board[nextRow][j] = '.';
                        board[i][j] = 'G';
                    }
                }
            }
        }
        return best;
    }
}

std::pair<int, int> getbotMove() {
    int bestValue = -1000;
    std::pair<int, int> bestMove = { -1, -1 };

    for (int i = 0; i < BOARD_SIZE -1; i++) {
        for (int j = 0; j < BOARD_SIZE -1; j++) {
            if (board[i][j] == 'R') {
                int nextCol = j + 1;
                if (isInside(i, nextCol) && board[i][nextCol] == '.') {
                    board[i][j] = '.';
                    board[i][nextCol] = 'R';
                    int moveValue = minimax(3, false);
                    board[i][nextCol] = '.';
                    board[i][j] = 'R';

                    if (moveValue > bestValue) {
                        bestValue = moveValue;
                        bestMove = { i, j };
                    }
                }
            }
        }
    }

    return bestMove;
}

// Update tokens based on board state
void updateTokens() {
    redTokens.clear();
    greenTokens.clear();
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 'R') redTokens.push_back({ i, j });
            if (board[i][j] == 'G') greenTokens.push_back({ i, j });
        }
    }
}

void processbotMove() {
    auto aiMove = getbotMove();
    if (aiMove.first != -1) {
        moveToken('R', aiMove.first, aiMove.second);
        updateTokens();
        if (checkWin('R')) {
            std::cout << "AI wins!\n";
            gameRunning = false;
            return;
        }
        currentPlayer = GREEN;
        glutPostRedisplay();
    }
}



void handleHumanMove(int row, int col) {
    if (currentPlayer != GREEN) return;

    if (moveToken('G', row, col)) {
        updateTokens();
        if (checkWin('G')) {
            std::cout << "You win!\n";
            gameRunning = false;
            return;
        }
        currentPlayer = RED;
        processbotMove();
    }
}


void selection(unsigned char key, int x, int y) {

    switch (key) {
    case 'w': // Move selection up
    case 'W':
        if (selectedRow > 0) selectedRow--;
        break;
    case 's': // Move selection down
    case 'S':
        if (selectedRow < BOARD_SIZE - 1) selectedRow++;
        break;
    case 'a': // Move selection left
    case 'A':
        if (selectedCol > 0) selectedCol--;
        break;
    case 'd': // Move selection right
    case 'D':
        if (selectedCol < BOARD_SIZE - 1) selectedCol++;
        break;
    case 13: // Enter key to move the token
    case 32: // Space key to move the token
        if (board[selectedRow][selectedCol] == 'G') { // Check if the selected token is valid
            if (moveToken('G', selectedRow, selectedCol)) {
                updateTokens();
                if (checkWin('G')) {
                    std::cout << "You win!\n";
                    gameRunning = false;
                    return;
                }
                currentPlayer = RED; // Switch to bot's turn
                processbotMove();     // Bot makes its move
            }
            else {
                std::cout << "Invalid move. Try again.\n";
            }
        }
        else {
            std::cout << "No token selected. Try again.\n";
        }
        break;
    }
    std::cout << "Selected Row: " << selectedRow << ", Selected Col: " << selectedCol << std::endl;
    glutPostRedisplay(); // Redraw the board to show the updated selection

}


void move(unsigned char key, int x, int y) {
    if (currentPlayer == GREEN) { // User's turn
        if (key == 13 || key == 32) { // Enter or Space key
            if (board[selectedRow][selectedCol] == 'G') { // Check if the selected token is valid
                if (moveToken('G', selectedRow, selectedCol)) {
                    updateTokens();
                    if (checkWin('G')) {
                        std::cout << "You win!\n";
                        gameRunning = false;
                        return;
                    }
                    currentPlayer = RED; // Switch to bot's turn
                    processbotMove();     // Bot makes its move
                }
                else {
                    std::cout << "Invalid move. Try again.\n";
                }
            }
            else {
                std::cout << "No token selected. Try again.\n";
            }
        }
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

    
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINES);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        glVertex2f(i, 0);
        glVertex2f(i, BOARD_SIZE);
        glVertex2f(0, i);
        glVertex2f(BOARD_SIZE, i);
    }
    glEnd();

   
    glColor3f(0.5f, 0.5f, 0.5f); // Gray color
    glBegin(GL_QUADS);
    
    glVertex2f(0, BOARD_SIZE - 1);
    glVertex2f(1, BOARD_SIZE - 1);
    glVertex2f(1, BOARD_SIZE);
    glVertex2f(0, BOARD_SIZE);

    // Top-right corner
    glVertex2f(BOARD_SIZE - 1, BOARD_SIZE - 1);
    glVertex2f(BOARD_SIZE, BOARD_SIZE - 1);
    glVertex2f(BOARD_SIZE, BOARD_SIZE);
    glVertex2f(BOARD_SIZE - 1, BOARD_SIZE);

    // Bottom-left corner
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);

    // Bottom-right corner
    glVertex2f(BOARD_SIZE - 1, 0);
    glVertex2f(BOARD_SIZE, 0);
    glVertex2f(BOARD_SIZE, 1);
    glVertex2f(BOARD_SIZE - 1, 1);
    glEnd();

    
    for (const auto& token : redTokens) {
        glColor3f(1.0f, 0.0f, 0.0f); // Red
        float centerX = token.col + 0.5f;
        float centerY = BOARD_SIZE - token.row - 0.5f;
        glBegin(GL_TRIANGLES);
        glVertex2f(centerX + 0.3f, centerY); // Right vertex
        glVertex2f(centerX - 0.3f, centerY + 0.3f); // Top-left vertex
        glVertex2f(centerX - 0.3f, centerY - 0.3f); // Bottom-left vertex
        glEnd();
    }

    // Draw the green tokens as triangles rotated by 180 degrees
    for (const auto& token : greenTokens) {
        glColor3f(0.0f, 1.0f, 0.0f); // Green
        float centerX = token.col + 0.5f;
        float centerY = BOARD_SIZE - token.row - 0.5f;
        glBegin(GL_TRIANGLES);
        glVertex2f(centerX, centerY - 0.3f); // Bottom vertex
        glVertex2f(centerX + 0.3f, centerY + 0.3f); // Top-right vertex
        glVertex2f(centerX - 0.3f, centerY + 0.3f); // Top-left vertex
        glEnd();
    }

    // Highlight the selected token
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow for selection
    float x1 = selectedCol;
    float y1 = BOARD_SIZE - selectedRow - 1;
    float x2 = selectedCol + 1;
    float y2 = BOARD_SIZE - selectedRow;
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glutSwapBuffers(); // Swap buffers for double buffering
}




// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("4x4 Board Game with bot");

   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, BOARD_SIZE, 0, BOARD_SIZE);
    glClearColor(1, 1, 1, 1);

    initTokens();

    
    glutDisplayFunc(display);

    
    glutKeyboardFunc(move);
    glutKeyboardFunc(selection);

    glutMainLoop();
    return 0;
}

