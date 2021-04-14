#include "xtos.h"
#ifndef KTOS_H
#define KTOS_H
#endif

int compareWeight(const void* doraResult1, const void* doraResult2);
int compareWL(const void* doraResult1, const void* doraResult2);
int  compareComboAndWeight(const void  *doraResult1, const void* doraResult2);
int compareOnComboCount(const void* doraResult1, const void* doraResult2);
int compareCombo(const void *combo1, const void* combo2);
int weight_orb(int orbPrior, short *weight, short *finalweight, int orbType, int orbTypeComboCount, int orbCount, int combo_type, DoraConfig* config);
SOPoint inPlaceMoveRC(SOPoint rc, int direction);
void freeDynamicArray(DoraResultArrayPointer* p_array);
void cleanDynamicArray(DoraResultArrayPointer *p_array);
int findCombo(DoraResult *result, DoraConfig *config);
int solveBoardStep(DoraResultArrayPointer *p_array, DoraConfig *config, int isMaxPath);
void solveBoard(char *mIdx,DoraConfig *config,DoraResultArrayPointer *p_array);
int kora_solve(int mCols, int mRows, int mIdx[], int params[], int color_priority[], int startPoint[]);



