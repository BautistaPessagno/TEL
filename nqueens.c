#include <stdio.h>
#include <assert.h>

#define DIRECTIONS 4

int isAttacked(int n, const char tablero[][n], int i_inicial, int j_inicial, int inc_i, int inc_j);
int nQueens(unsigned int n, const char tablero[][n]);

int main(void) {
    char board[][8] = {
        {'0', '0', '0', '0', '1', '0', '0', '0'},
        {'0', '0', '1', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '1', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '0'},
        {'0', '1', '0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '1', '0'},
        {'1', '0', '0', '0', '1', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '0', '0', '1'}
    };// por ejemplo, hay tres reinas en la columna 4 y dos en la fila 6
    assert(nQueens(8, board) == 0);
    
    char board2[][6] = {
        {'0', '0', '0', '1', '0', '0'},
        {'1', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '1', '0'},
        {'0', '1', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '1'},
        {'0', '0', '1', '0', '0', '0'}
    };// Hay 6 reinas y no se atacan, retorna true
    assert(nQueens(6, board2));
    
    char board3[][6] = {
        {'0', '0', '1', '0', '0', '0'},
        {'1', '0', '0', '0', '0', '0'},
        {'0', '1', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '1', '0'},
        {'0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '1'}
    };// Las reinas en (1,0) y (2,1) se amenazan en forma diagonal
    assert(nQueens(6, board3) == 0);

    char board4[][6] = {
        {'0', '0', '0', '1', '0', '0'},
        {'0', '0', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '1', '0'},
        {'0', '1', '0', '0', '0', '0'},
        {'0', '0', '0', '0', '0', '1'},
        {'0', '0', '1', '0', '0', '0'}
    };// No se amenazan pero hay menos de 6 reinas
    assert(nQueens(6, board4) == 0);
    // No hay solución para tableros de dimensión 2 o 3
    assert(nQueens(3, board2) == 0);
    assert(nQueens(2, board2) == 0);
    // En un tablero de 1x1 debe haber una reina
    char board1[][1] = {{'1'}};
    assert(nQueens(1, board1));

    puts("OK!");
    return 0;
}

int isAttacked(int n, const char tablero[][n], int i_inicial, int j_inicial, int inc_i, int inc_j) {
    int found = 0;
    int i = i_inicial + inc_i;
    int j = j_inicial + inc_j;
    
    while(0 <= i && i < n && 0 <= j && j < n && !found) {
        if(tablero[i][j] == '1') {
            found = 1;
        }
        i += inc_i;
        j += inc_j;
    }
    
    return found;
}

int nQueens(unsigned int n, const char tablero[][n]) {
    int isValid = 1, queens = 0;

    // no es necesario mirar hacia arriba ni horizontalmente a la izquierda ya que de haber una dama ahi ya la habria visto
    int dir[][2] = {
        {0, 1},
        {1, 0},
        {1, 1},
        {1, -1},
    };

    for(int i = 0; i < n && isValid; i++) {
        int foundQueen = 0;
        for(int j = 0; j < n && !foundQueen; j++) {
            if( tablero[i][j] == '1' ) {
                foundQueen = 1;
                queens++;
                for(int dirIdx = 0; dirIdx < DIRECTIONS && isValid; dirIdx++) {
                    if( isAttacked(n, tablero, i, j, dir[dirIdx][0], dir[dirIdx][1]) ) {
                        isValid = 0;
                    }
                }                
            }
        }
    }

    if(queens != n) {
        isValid = 0;
    }

    return isValid;
}
