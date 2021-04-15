//
// Created by swim on 2020/12/25.
//

#ifndef XTOS_H
#define XTOS_H
#endif //XTOS_H

enum  COMBO_FLAG{
    COMBO_FLAG_ROW = 1,
    COMBO_FLAG_COL = 2,
    COMBO_FLAG_FIRST = 4
};

enum GEM_FLAG{
    GEM_FLAG_NONE = 0,
    GEM_FLAG_FIRST = 1,
    GEM_FLAG_STACK = 2,
};

enum COMBO_TYPE{
    Normal=0,
    a2Gems=1,
    a4Gems=2,
    a5Gems=3,
    ROW=4,
    AllOut=5,
    b3Gems=6
};

enum COMBO_MODE_TYPE{
    QUICK = 0,
    BEST = 1
};
typedef struct SOPoint SOPoint;
struct SOPoint {
    char x;
    char y;
};
struct PARAMS
{
    int MaxSteps;
    int Complex;
    int mMode;
    int mComboMode;
    int TosColMode;
    int use3Gem;
    int useTurboMode;
    int GemTypeCondition;
    int ComboModetype;
    int MatchType;
    int tolsColModeGem;
    int firstBatch;
    int dropDirection;
    int firePathSize;
    int maxHurtCount;
    int isSupport8Dir;
};
typedef struct FirePath FirePath;
struct FirePath {
    char fpath[48];
    int hurting;
};

typedef struct Combo Combo;
struct Combo {
    char mGemType;
    char mGemCount;
    char mGemFlag;
    char flag[1];

};


typedef struct DoraResultData DoraResultData;
struct DoraResultData {
    FirePath *pfirePath;
    Combo combos[24];
    SOPoint initCursor;
    SOPoint nowCursor;
   unsigned char board[48];
    unsigned char dirs[80];
    char firstBatch;
    char Priority;
    char Weathering;
    char Reserve;
    char pass_board[30];
};


typedef struct DoraResult DoraResult;
struct DoraResult
{
    DoraResultData *data;
    short weight;
    short final_weight;
    unsigned char path_count;
    char isDone;
    unsigned char combos_length;
    char color_count;
    char pass_count;
};


typedef struct DoraResultArrayPointer DoraResultArrayPointer;
struct DoraResultArrayPointer
{
    DoraResult* ResultPointer;
    DoraResultData* resultData;
    FirePath* firepath;
    int MaxResultSize;
    int result_index;
};

struct Gem {
    int gemPrior;
    int gemComboType;
    int gemFlag;
    int flag;

};

typedef struct DoraConfig DoraConfig;
struct DoraConfig
{
    SOPoint startPoint[48];
    SOPoint endPoint[48];
    int maxGem;
    Gem gems[11];
    int x174;
    int x178;
    int x17C;
    int MaxPathSize;
    int isSupportDir;
    int x188;
    int x18C;
    int Complex_val;
    int x194;
    int mMode;
    int ComboMode;
    int ComboModeType;
    int Weathering;
    int Priority;
    int Reserve;
    int PuzzleShield;
    char puzzle_board[30];
    int GemCount[11];
    int x210;
    int useTosCol;
    int GemForRow;
    int GemTypeVal;
    int config_220;
    int use3Gem;
    int FirstBatchCount;
    int useTurboMode;
    int ROWS;
    int COLS;
    int flag_start_count;
    int flag_end_count;
    int GemsTypeCondition;
    int MatchType;
    int DropDirection;
    int firePathSize;
    short MaxHurtCount;
    char isTosCol[6];
    char tosCol[6];
    int firstTotal;
    int puzzleGemType;
    int flag_pass_count;
    int turnFirst;
    char pass_board[30];
    char fpass_board[30];
};