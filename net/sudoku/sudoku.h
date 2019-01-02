#ifndef CCNET_EXAMPLES_SUDOKU_SUDOKU_H
#define CCNET_EXAMPLES_SUDOKU_SUDOKU_H


#include "../../base/types.h"
#include "../../base/string_piece.h"

ccnet::string solveSudoku(const ccnet::StringPiece& puzzle);
const int kCells = 81;
extern const char kNoSolution[];

#endif
