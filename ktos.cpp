#include "ktos.h"
#include <cstdlib>
#include <cstring>
#include <math.h> 

DoraConfig *config;
DoraResultArrayPointer p_array0;
int status_interrupt;
int MaxCombo;
SOPoint *points0;
char *current_board;
char *match_board;
char *scatch_board;
size_t board_size;

char* substr(char* arr, int begin, int len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}



int isInterrupted(){
    return status_interrupt;
}

int compareWeight(const void* doraResult1, const void* doraResult2)
{
    struct DoraResult* r1 = (DoraResult*)doraResult1;
    struct DoraResult* r2 = (DoraResult*)doraResult2;
    return r2->weight - r1->weight;
}

int compareWL(const void* doraResult1, const void* doraResult2)
{
    struct DoraResult* r1 = (DoraResult*)doraResult1;
    struct DoraResult* r2 = (DoraResult*)doraResult2;
    if (r1->final_weight != r2->final_weight)
        return r2->final_weight -r1->final_weight;
    if (r1->combos_length == r2->combos_length)
        return r1->path_count - r2->path_count;
    else
        return r2->combos_length - r1->combos_length;
}


int  compareComboAndWeight(const void  *doraResult1, const void* doraResult2)
{
    struct DoraResult* r1 = (DoraResult*)doraResult1;
    struct DoraResult* r2 = (DoraResult*)doraResult2;
    if (r1->combos_length == r2->combos_length)
       return r2->weight - r1->weight;
    else
        return r2->combos_length - r1->combos_length;

}

int compareOnComboCount(const void* doraResult1, const void* doraResult2)
{
    struct DoraResult* r1 = (DoraResult*)doraResult1;
    struct DoraResult* r2 = (DoraResult*)doraResult2;
    if (r1->combos_length == r2->combos_length)
        return r2->final_weight - r1->final_weight;
    else
       return r2->combos_length - r1->combos_length;

}

int compareCombo(const void *combo1, const void* combo2)
{
    struct Combo* cb1 = (struct Combo*)combo1;
    struct Combo* cb2 = (struct Combo*)combo2;
    if (cb1->mGemType == cb2->mGemType)
        return cb2->mGemCount - cb1->mGemCount;
    else
        return cb1->mGemType - cb2->mGemType;
}

int weight_orb(int orbPrior, short *weight, short *finalweight, int orbType, int orbTypeComboCount, int orbCount, int combo_type, DoraConfig* config) {
    int diff_combo = orbCount;
    if (config->MatchType != 1)
    {
        if (orbTypeComboCount != 1) {
            if (config->MatchType != 2) {
                return orbPrior;
            }
            *weight -= 100;
            *finalweight -= 100;
            return orbPrior;
        }
        if (orbCount != combo_type)
        {
            if (config->GemsTypeCondition) {
                if (config->GemsTypeCondition != 1) {
                    return orbPrior;
                }
                *weight -= 100;
                *finalweight -= 100;
                return orbPrior;
            }
        }
        else if (orbCount == combo_type) {
            *weight += 4 * 5 * orbPrior;
            orbPrior = 2 * 5 * orbPrior;
            *finalweight += orbType * orbPrior;
            return orbPrior;
        }
        if (orbCount >= combo_type)
            diff_combo = combo_type - orbCount;
        *weight = (diff_combo * 2 * 5 * orbPrior) + *weight;
        *finalweight = (diff_combo * 2 * 5 * orbPrior) + *finalweight;
        return (diff_combo * 2 * 5 * orbPrior) + *weight;
    }
    if (orbTypeComboCount != 1) {
        if (orbCount != combo_type)
            return orbPrior;
        else if (orbCount == combo_type) {
            *weight += 4 * 5 * orbPrior;
            orbPrior = 2 * 5 * orbPrior;
            *finalweight += orbType * orbPrior;
            return orbPrior;
        }
    }
    return orbPrior;
}

SOPoint inPlaceMoveRC(SOPoint rc, int direction) {
    switch (direction)
    {
        case 0:
            rc.x++;
            break;
        case 1:
            rc.y++;
            rc.x++;
            break;
        case 2:
            rc.y++;
            break;
        case 3:
            rc.y++;
            rc.x--;
            break;
        case 4:
            rc.x--;
            break;
        case 5:
            rc.x--;
            rc.y--;
            break;
        case 6:
            rc.y--;
            break;
        case 7:
            rc.x++;
            rc.y--;
            break;
        default:
            break;
    }
    return rc;
}

void freeDynamicArray(DoraResultArrayPointer* p_array) {
    if (p_array)
    {
        if (p_array->ResultPointer) {
            free(p_array->ResultPointer);
        }
        if (p_array->resultData) {
            free(p_array->resultData);
        }
        if (p_array->firepath) {
            free(p_array->firepath);
        }
        p_array->resultData = 0;
        p_array->ResultPointer= 0;
        p_array->firepath = 0;
        p_array->MaxResultSize = 0;
        p_array->result_index = 0;
    }
}

void cleanDynamicArray(DoraResultArrayPointer *p_array) {
    if (p_array && p_array->MaxResultSize > 0)
    {
        if (p_array->ResultPointer)
            memset(p_array->ResultPointer,0,4);
        if (p_array->firepath)
            memset(p_array->firepath,0,4);
        if (p_array->resultData)
            memset(p_array->resultData,0,4);
        if (p_array->ResultPointer){
            if (p_array->resultData)
            {
                for (int i = 0;i < p_array->MaxResultSize;i++) {
                    p_array->ResultPointer[i].data = &p_array->resultData[i];
                    if (p_array->firepath) {
                        p_array->ResultPointer[i].data->pfirePath = &p_array->firepath[i];
                    }
                }
            }
        }
        p_array->result_index = 0;
    }
}

int initDynamicArray(DoraResultArrayPointer* p_array, int complex_final, bool isFirePath) {
    int _isFirePath = isFirePath;
    if (p_array == NULL) {
        return (long)p_array;
    }
    if (p_array->MaxResultSize ==complex_final) {
        cleanDynamicArray(p_array);
        return (long)p_array;
    }
    freeDynamicArray(p_array);
    p_array->ResultPointer = new DoraResult[complex_final];
    p_array->resultData = new DoraResultData[complex_final];
    if (isFirePath) {
        p_array->firepath = new FirePath[complex_final];
    }
    if (p_array->ResultPointer) {
        if (p_array->resultData && (!_isFirePath || p_array->firepath)){
            p_array->MaxResultSize = complex_final;
            cleanDynamicArray(p_array);
            return (long)p_array;
        }
        else {
            p_array = 0;
            if (p_array->resultData) {
                free(p_array->resultData);
                p_array->resultData = 0;
            }
            if (p_array->firepath) {
                free(p_array->firepath);
                p_array->firepath = 0;
            }
            cleanDynamicArray(p_array);
            p_array = 0;
            p_array->result_index = 0;
            p_array->MaxResultSize = 0;
        }
    };
    return (long)p_array;
}

int findCombo(DoraResult *result, DoraConfig *config){

    memcpy(current_board, result->data->board, board_size);
    memset(result->data->combos,0,24*sizeof(Combo));
    result->combos_length = 0;
    result->color_count = 0;
    result->data->firstBatch = 0;
    result->data->Priority = 0;
    result->data->Weathering= 0;
    result->data->Reserve = 0;

    int isFirstCombo = 1;
    int y;
    int x;
    char prev_1_gem;
    char prev_2_gem;
    int offset;
    unsigned char cur_gem;

    int isInRowCount;
    int isInColCount;
    int cell_offset;
    int count;
    SOPoint rc;

    int tmp_index;

    while (2) {
        memset(match_board, 0x1F, 48);
        int current_combo_index = result->combos_length;
        //HORITONTAL
        y=0;
        do{
            prev_1_gem = 0x1E;
            prev_2_gem = 0x1E;
            x =0;
            do{ //
                offset = x + config->COLS * y;
                cur_gem = current_board[offset] & 0x1F;
                //int notQue = cur_gem != 8 ? 1 : cur_gem - 8;

                if (config->gems[cur_gem].gemComboType == a2Gems && cur_gem <= 5 && config->GemTypeVal && cur_gem == prev_2_gem) {
                    //2 gems
                    match_board[offset] =  cur_gem;
                    match_board[offset - 1] =  cur_gem;
                    prev_1_gem = prev_2_gem;
                    continue;
                }else if (prev_1_gem == prev_2_gem && prev_2_gem == cur_gem && cur_gem != 0x1E && cur_gem != 8 ) {
                    match_board[offset] =  cur_gem;
                    match_board[offset - 1] =  cur_gem;
                    match_board[offset - 2] =  cur_gem;
                }else if(config->gems[cur_gem].gemComboType == b3Gems && cur_gem <= 5 && cur_gem == prev_2_gem){
                    if  ( y + 1 <config->ROWS){ //L type
                        if((current_board[offset + config->COLS] & 0x1F) == cur_gem ){
                            match_board[offset] =  current_board[offset];
                            match_board[offset - 1] =  current_board[offset];
                            match_board[offset + config->COLS] =  current_board[offset];
                            prev_1_gem = prev_2_gem;
                            continue;
                        }else if((current_board[offset + config->COLS - 1] & 0x1F) == cur_gem ){
                            match_board[offset] =  current_board[offset];
                            match_board[offset - 1] =  current_board[offset];
                            match_board[offset + config->COLS - 1] =  current_board[offset];
                            prev_1_gem = prev_2_gem;
                            continue;
                        }
                    }else if (y-1>0){ //L type
                        if((current_board[offset - config->COLS] & 0x1F) == cur_gem ){
                            match_board[offset] =  cur_gem;
                            match_board[offset - 1] =  cur_gem;
                            match_board[offset - config->COLS] =  cur_gem;
                            prev_1_gem = prev_2_gem;
                            continue;
                        }else if((current_board[offset - config->COLS - 1] & 0x1F) == cur_gem ){
                            match_board[offset] =  cur_gem;
                            match_board[offset - 1] =  cur_gem;
                            match_board[offset - config->COLS - 1] =  cur_gem;
                            prev_1_gem = prev_2_gem;
                            continue;
                        }
                    }

                }
                prev_1_gem = prev_2_gem;
                prev_2_gem = cur_gem;
            }
            while(x++<config->COLS -1 );
        }
        while(y++<config->ROWS -1 );
        //VERITICAL
        x=0;
        do{
            prev_1_gem = 0x1E;
            prev_2_gem = 0x1E;
            y =0;
            do{
                offset = x + config->COLS * y;
                cur_gem = current_board[offset] & 0x1F;;
                //int notQue = cur_gem != 8 ? 1 : cur_gem - 8;
                if (config->gems[cur_gem].gemComboType == a2Gems && cur_gem <= 5 && config->GemTypeVal && cur_gem == prev_2_gem) {
                    //2 gems
                    match_board[offset] =  cur_gem;
                    match_board[offset - config->COLS] =  cur_gem;
                    prev_1_gem = prev_2_gem;
                    continue;
                }else if (prev_1_gem == prev_2_gem && prev_2_gem == cur_gem && cur_gem != 0x1E && cur_gem != 8 ) {
                    match_board[offset] =  cur_gem;
                    match_board[offset - config->COLS] = cur_gem;
                    match_board[offset - 2 * config->COLS] =  cur_gem;
                }
                prev_1_gem = prev_2_gem;
                prev_2_gem = cur_gem;
            }
            while(y++<config->ROWS - 1);
        }
        while(x++<config->COLS -1);


        memcpy(scatch_board, match_board, board_size);

        tmp_index= 0;

        for (x = 0;x < config->COLS;x++) {
            for (y = 0;y < config->ROWS;y++) {
                offset = x + config->COLS * y;
                cur_gem = scatch_board[offset];
                if (cur_gem == 0x1F)
                    continue;

                count = 0;
                //* points = new SOPoint[30];//***
                isInRowCount = 0;
                isInColCount = 0;
                cell_offset = 1;
                points0[0].x = x;
                points0[0].y = y;
                do{
                    rc = points0[cell_offset - 1];
                    int offset_c = rc.x + config->COLS * rc.y;
                    cell_offset--;
                    if (scatch_board[offset_c] != (cur_gem & 0x1F))
                        continue;
                    if (config->useTosCol && rc.x == x)
                        ++isInColCount;
                    if (config->GemForRow && rc.y == y)
                        ++isInRowCount;
                    count++;
                    scatch_board[offset_c] = 0x1F;
                    if (rc.y > 0) { points0[++cell_offset -1] = { rc.x,(char)(rc.y - 1) }; };
                    if (rc.y < config->ROWS-1) { points0[++cell_offset - 1] = { rc.x,(char)(rc.y + 1) }; }
                    if (rc.x > 0) { points0[++cell_offset - 1] = { (char)(rc.x - 1), rc.y }; }
                    if (rc.x < config->COLS-1) { points0[++cell_offset - 1] = { (char)(rc.x + 1),rc.y }; }
                } while (cell_offset  > 0);

                //delete[] points;

                int notInRow = x > 0 ? config->GemForRow - 1 <= 0 : 0;
                if (!notInRow) {
                    result->data->combos[current_combo_index+ tmp_index].mGemFlag &= -2;
                }
                else {
                    int OrbsINRow = count > 3 ? (isInRowCount - count) <= 0 : 0;
                    if (OrbsINRow && count <= config->COLS) {
                        result->data->combos[current_combo_index + tmp_index].mGemFlag |= COMBO_FLAG_ROW; //flag_row
                    }
                    else {
                        result->data->combos[current_combo_index + tmp_index].mGemFlag &= -2;
                    }
                }
                result->data->combos[current_combo_index + tmp_index].mGemFlag &= -3;

                int firstComboInCol = config->useTosCol == 1 ? isFirstCombo & 1 : 0;
                if (firstComboInCol && config->isTosCol[x] == 1) {
                    int isVCombo = isInColCount == count ? (y < config->ROWS - 2) : 0;
                    if (isVCombo && count > 2 && config->tosCol[x] + 3 >= count)// ????
                        result->data->combos[current_combo_index + tmp_index].mGemFlag |= COMBO_FLAG_COL;  // flag_col
                    result->data->combos[current_combo_index + tmp_index].mGemFlag |= COMBO_FLAG_FIRST;
                }
                else if (isFirstCombo)
                {
                    result->data->combos[current_combo_index + tmp_index].mGemFlag |= COMBO_FLAG_FIRST;
                }
                result->data->combos[current_combo_index + tmp_index].mGemCount = count;
                result->data->combos[current_combo_index + tmp_index].mGemType = cur_gem;
                tmp_index++;
            }

        }

        if (tmp_index) {
            //_inPlaceRemoveMatches
            for (x = 0;x < config->COLS;x++) {
                for (y = 0;y < config->ROWS;y++) {
                    offset = x + config->COLS * y;
                    cur_gem = match_board[offset];
                    if (cur_gem != 0x1F) {
                        if (config->Weathering && (current_board[offset] & 0x40))
                            ++result->data->Weathering;
                        if (config->Priority && (current_board[offset] & 0x80))
                            ++result->data->Priority;
                        if (config->Reserve && (current_board[offset] & 0x20))
                            ++result->data->Reserve;
                        //_inPlaceRemoveMatches
                        current_board[offset] = (char)0x1E;
                    }
                }
            }

            //_inPlaceDropEmptySpaces
            int dest_y;
            int offset_drop;
            for (x = 0;x < config->COLS;x++) {
                dest_y = config->ROWS - 1;
                for (y = config->ROWS - 1; y >= 0;y--) {

                    offset = x + config->COLS * y;
                    cur_gem = current_board[offset];
                    if (cur_gem != 0x1E) {
                        offset_drop = x + config->COLS * dest_y--;
                        current_board[offset_drop] = cur_gem;
                    }

                }
                if (dest_y >= 0) {
                    offset = x + config->COLS * dest_y;
                    cur_gem = current_board[offset];
                    for (; dest_y >= 0; --dest_y) {
                        offset_drop = x + config->COLS * dest_y;
                        current_board[offset_drop] = 0x1E;
                    }
                }
            }
            result->combos_length = (tmp_index + result->combos_length);
            if (isFirstCombo)
            {
                isFirstCombo = 0;
                result->data->firstBatch = (char)tmp_index;
                continue;
            }
            continue;
        }
        break;
    }


    //Counting Hurting
    if (config->firePathSize > 0) {
        offset = result->data->nowCursor.x + config->COLS * result->data->nowCursor.y;
        int offset0 = result->data->initCursor.x + config->COLS * result->data->initCursor.y;
        if (result->data->pfirePath->fpath[offset] > 0) {

            result->data->pfirePath->hurting ++;
            if( result->path_count < 0x1E - result->data->pfirePath->fpath[offset0]){
                int i = 0;
                i++;

            }
        }
        result->data->pfirePath->fpath[offset] = (char)config->firePathSize; //mark firepath
    }

    qsort((Combo*)result->data->combos, result->combos_length, sizeof(Combo), compareCombo);


    int cb_index = result->combos_length;
    int mode = config->mMode;
    int isHighCombo = mode <= 0;
    short wt = 0;
    short fwt = 0;
    int orbCounter[8];
    memset(&orbCounter, 0, sizeof(orbCounter));

    if (!cb_index) {
        wt = -50;
        fwt = -50;
    }
    if (config->ComboModeType != BEST || config->ComboMode <= 0 || cb_index != config->ComboMode)
    { //notBestSolu||Highcombo|| combo NOT EQ
        if (!cb_index) {
            wt = 0;
            fwt = 0;
        }
    }
    else
    {
        wt += 500;
        wt += 500;
    }
    // int combo_counter = 0;
    short wt_prior = 0;
    short fwt_prior = 0;
    //short wt_m;
    //short fwt_m;
    char tmp_orbType = 0x1F;
    unsigned char gemtype;
    char gemcount;
    bool notKeep;
    int i;
    int allout[6] = {0};
    int ftotalcount = 0;
    for (i = 0;i < result->combos_length;i++) {
        Combo cb = result->data->combos[i];
        gemtype = cb.mGemType;
        gemcount = cb.mGemCount;
        orbCounter[gemtype]++;
        if (config->use3Gem != 1) {
            if (config->gems[gemtype].gemPrior < 0)// KeepOrb
            {
                wt_prior = (200 * config->gems[gemtype].gemPrior) & -8;
                fwt_prior = (200 * config->gems[gemtype].gemPrior) & -8;
                notKeep = (200 * config->gems[gemtype].gemPrior & -8) > 0;
            }
            else {
                wt_prior = config->gems[gemtype].gemPrior;
                fwt_prior = config->gems[gemtype].gemPrior;
                notKeep = config->gems[gemtype].gemPrior > 0;
            }
        }
        else { //force 3 gems
            int post_OrbComboType = config->gems[gemtype].gemComboType > 1 ? 0 : 1 - config->gems[gemtype].gemComboType;
            //leave 3orbs, let other = 0;
            int needCut3 = cb.mGemCount > 3 ? post_OrbComboType & 1 : 0;
            int isOrbs3OK = cb.mGemCount > 2 ? config->gems[gemtype].gemComboType - 1 <= 0 : 0;
            if ((needCut3 || isOrbs3OK) && config->gems[gemtype].gemPrior > 0) {
                if (cb.mGemType <= 6) {
                    wt_prior = config->gems[gemtype].gemPrior * (3 - config->gems[gemtype].gemComboType - cb.mGemCount);
                    fwt_prior = config->gems[gemtype].gemPrior * (3 - config->gems[gemtype].gemComboType - cb.mGemCount);
                }
                else {
                    wt_prior = config->gems[gemtype].gemPrior;
                    fwt_prior = config->gems[gemtype].gemPrior;
                    notKeep = 1;
                }
            }
        }

        //couting color count
        if (!isHighCombo && cb.mGemType <= 4 && cb.mGemType != tmp_orbType)
        {
            tmp_orbType = cb.mGemType;
            result->color_count++;
        }

        //NotNormal OrbComboType
        if (config->gems[gemtype].gemComboType > a2Gems && notKeep)
        {

            if (orbCounter[gemtype] != 1 && config->MatchType != 1)
            {
                //def_6894
            }
            switch (config->gems[gemtype].gemComboType) {
                case a4Gems: //4orbs
                    if (gemcount > 4)
                        wt += 20;
                    break;
                case a5Gems://5 gems
                    if (gemcount > 4)
                        wt += 2 * 20;
                    break;
                case AllOut:
                    if (gemcount > 9) {
                        wt = wt - 180 + 20 * gemcount;
                        wt += 20 * gemcount + 20;
                    }

                    break;
                default:
                    break;
            }
            ////if(cb)
            int orbCountOver3 = gemcount > 3;

            if (config->gems[gemtype].gemComboType == AllOut && gemcount > 3)
            {
                orbCountOver3 = 1;
                //fwt = fwt - 60 + 20 * gemcount;
                //fwt += 200;
            }
            else
            {
                if (config->gems[gemtype].gemComboType == a4Gems) {
                    weight_orb(wt_prior, &wt, &fwt, 6, orbCounter[gemtype], gemcount, 4, config);
                }
                else if (config->gems[gemtype].gemComboType == a5Gems) {
                    weight_orb(wt_prior, &wt, &fwt, 6, orbCounter[gemtype], gemcount, 5, config);
                }
            }
            if (orbCountOver3 && config->gems[gemtype].gemComboType <= ROW)
            {
                if (gemcount < config->COLS && cb.mGemFlag & COMBO_FLAG_ROW)
                    wt += (100 * gemcount - 300) * wt_prior;
                else if (gemcount == config->COLS && cb.mGemFlag & COMBO_FLAG_ROW) { //forRow
                    wt += wt_prior * 300;
                    fwt += wt_prior * 300;
                }
            }
        }
        //Weighting TosCOL
        if (config->useTosCol == 1 && notKeep > 0 && cb.mGemFlag & COMBO_FLAG_COL) //stacking orb
        {
            wt = (cb.mGemCount - 2 ) * 120 + wt;
            fwt = (cb.mGemCount - 2 ) * 120 + fwt;
        }
        if(cb.mGemFlag & COMBO_FLAG_FIRST){
            ftotalcount += gemcount;
            if (config->gems[gemtype].gemFlag == GEM_FLAG_FIRST)
            {
                wt += 40;
                fwt += 40;
            }
            else if (config->gems[gemtype].gemFlag == GEM_FLAG_STACK)
            { //stack
                wt -= 40;
                fwt -= 40;
            }
            if(config->gems[gemtype].gemComboType == AllOut){
                allout[gemtype]  += gemcount;
            }

        }


        wt += wt_prior;
        fwt += fwt_prior;
    }
    //Weight ALL out
    for(i=0;i<6;i++){
        if(config->gems[i].gemComboType == AllOut && config->GemCount[i] == allout[i]){
            wt += 200;
            fwt += 200;
        }
    }
    if(config->firstTotal>0 && ftotalcount >0) {
        if (ftotalcount == config->firstTotal) {
            wt += 200;
            fwt += 200;
        }
    }
    //Combine Weight
    while(2) {
        wt = 4 * result->combos_length + wt ;
        fwt = 4 * result->combos_length + fwt ;


        //weighting colorcount
        if (!isHighCombo) {
            if (mode == 2&& result->color_count > 3)//4colors
            {
                fwt += 20;
                wt += 20;
            }
            else if (mode == 3&& result->color_count > 4) //5 colors
            {
                fwt += 20;
                wt += 20;
            }
            else if (mode == 1 && result->color_count > 2)
            {
                fwt += 10;
                wt += 10;
            }
        }
        //weighting puzzle shield
        if (config->PuzzleShield) {
            int PuzzleShiledCount = 0;
            int PuzzleGemSet[12] = {0 };
            char maxOrbType = -1;
            if (board_size <= 0)
                board_size = 0;
            if (board_size == 0 || board_size < 0) {
                PuzzleShiledCount = board_size;
            }
            else {
                for (unsigned int offset = 0;offset < board_size;offset++) {
                    if (config->puzzle_board[offset]) {
                        PuzzleShiledCount++;
                        int orbval = result->data->board[offset] & 0x1F;
                        PuzzleGemSet[orbval]++;
                    }
                }
            }

            int MaxGemCountInShiled = 0;
            for (int orbIndex = 0;orbIndex < 12;orbIndex++) {
                //find max
                if (MaxGemCountInShiled <= PuzzleGemSet[orbIndex] && PuzzleShiledCount <= config->GemCount[orbIndex]){
                    MaxGemCountInShiled = PuzzleGemSet[orbIndex];
                    maxOrbType = orbIndex;
                }

            }
            //Weighting Puzzle shield
            if (PuzzleShiledCount == MaxGemCountInShiled ) {
                //PuzzleShiledCount == MaxGemCountInShiled
                if(config->puzzleGemType == -1){
                    wt += 280;
                    fwt += 280;
                }
                for(int ind=0;ind<result->combos_length;ind++){ //Exact Number
                    if(result->data->combos[ind].mGemType == maxOrbType && result->data->combos[ind].mGemCount == PuzzleShiledCount){
                        if(maxOrbType == config->puzzleGemType){
                            wt += 280;
                            fwt += 280;
                        }else if(config->puzzleGemType == -1){
                            wt += 280;
                            fwt += 280;
                        }
                    }
                }
            }else if(MaxGemCountInShiled > 2 && MaxGemCountInShiled < PuzzleShiledCount){
                wt += 200 - 40*(PuzzleShiledCount - MaxGemCountInShiled);
                fwt += 200 - 40*(PuzzleShiledCount - MaxGemCountInShiled);
            }
        }

        //weighting FirstBatch
        if (config->firePathSize <= 0 || result->data->pfirePath->hurting <= config->MaxHurtCount ) {
            int w1 = 20 * (result->data->Priority + result->data->Weathering) - 40 * result->data->Reserve;
            wt = wt + w1;
            fwt = fwt + w1;
            if (fwt > 0 && wt > 0) {
                if (config->FirstBatchCount > 0) {
                    if (config->FirstBatchCount == 9) {
                        fwt = fwt + 100 * result->data->firstBatch;
                        wt = wt + 100 * result->data->firstBatch;
                    }else if(config->FirstBatchCount == result->data->firstBatch){
                        fwt = fwt + 800;
                        wt = wt + 800;
                    }else if (config->FirstBatchCount > result->data->firstBatch) {
                        fwt = fwt + 50 * result->data->firstBatch;
                        wt = wt + 50 * result->data->firstBatch;
                    }else{
                        fwt = 50 * (2* config->FirstBatchCount - result->data->firstBatch);
                    }
                }
            }else{
                fwt = -50;
                wt = -50;
            }

        }else{
            fwt = -50;
            wt = -50;
        }

        result->weight = wt;              // ffce=-50
        result->final_weight = fwt;

        if (config->firePathSize>0) {
            int x,y;
            for (x = 0;x < config->COLS;x++) {
                for (y = 0;y < config->ROWS;y++) {
                    int offset = x + config->COLS * y;
                    if (result->data->pfirePath->fpath[offset] > 0) {
                        result->data->pfirePath->fpath[offset]--;
                    }
                }
            }
        }

        int comboMode = config->ComboModeType;
        if (comboMode || config->ComboMode == 0 //no limit
            || config->ComboMode != result->combos_length
            || result->final_weight <= 0)
        {
            return 1;
        }
        if (config->flag_end_count > 0) {
            for (int i = 0;i < config->flag_end_count; i++) {
                if (((SOPoint)config->endPoint[i]).x != result->data->nowCursor.x || ((SOPoint)config->endPoint[i]).y != result->data->nowCursor.y) {
                    comboMode++;
                    if (comboMode == config->flag_end_count)
                        return comboMode;
                }
            }
        }
        else if(config->flag_end_count<=0)
            return config->ComboModeType;
    }
    return 0;
}

int solveBoardStep(DoraResultArrayPointer *p_array, DoraConfig *config, int isMaxPath) {
    int ret = 1;
    int dirstep = config->isSupportDir != 1 ? 2 : 1;
    int index = 0;
    char cur1;
    char cur2;
    int offset;
    int next_offset;

    SOPoint rc;
    DoraResult *fdr = &p_array->ResultPointer[index];
    DoraResult *new_dr;
    FirePath* pf;
    int direction;
    int canMoveOrb;
    int result_index;
    for (result_index = p_array->result_index;result_index>0;--result_index) {
        if (fdr->isDone != 1) {
            direction = 0;
            do{
                  if (!fdr->path_count || fdr->data->dirs[fdr->path_count-1] != ((direction + 4) & 7)) {
                      canMoveOrb = 0;
                      switch (direction)
                      {
                          case 0:
                              canMoveOrb = fdr->data->nowCursor.x < config->COLS - 1;
                              break;
                          case 1:
                              if (fdr->data->nowCursor.y < config->ROWS - 1 && fdr->data->nowCursor.x < config->COLS - 1)
                                  canMoveOrb = 1;
                              break;
                          case 2:
                              canMoveOrb = fdr->data->nowCursor.y < config->ROWS - 1;
                              break;
                          case 3:
                              if (fdr->data->nowCursor.y < config->ROWS - 1 && fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 4:
                              if (fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 5:
                              if (fdr->data->nowCursor.y && fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 6:
                              canMoveOrb = fdr->data->nowCursor.y;
                              if (fdr->data->nowCursor.y)
                                  canMoveOrb = 1;
                              break;
                          case 7:
                              if (fdr->data->nowCursor.y>0 && fdr->data->nowCursor.x < config->COLS - 1)
                                  canMoveOrb = 1;
                              break;
                          default:
                              break;
                      }
                      if (canMoveOrb) {
                          offset = config->COLS * fdr->data->nowCursor.y + fdr->data->nowCursor.x;
                          rc = inPlaceMoveRC(fdr->data->nowCursor, direction);
                          next_offset = config->COLS * rc.y + rc.x;
                          if (!config->Weathering || !(fdr->data->board[next_offset] & 0x40)) {
                              if (p_array->result_index <= p_array->MaxResultSize) {
                                  new_dr = &p_array->ResultPointer[p_array->result_index];
                                  p_array->result_index = p_array->result_index + 1;
                                  if (new_dr) {
                                      pf = new_dr->data->pfirePath;
                                      memcpy(new_dr->data, fdr->data, sizeof(DoraResultData));
                                      if(config->firePathSize>0){
                                          new_dr->data->pfirePath = pf;
                                          memcpy(new_dr->data->pfirePath,fdr->data->pfirePath,sizeof(FirePath));
                                      }
                                      new_dr->weight = fdr->weight;
                                      new_dr->path_count = fdr->path_count;
                                      new_dr->isDone = 0;
                                      new_dr->combos_length = 0;
                                      new_dr->weight = 0;
                                      new_dr->final_weight = 0;
                                      new_dr->pass_count = fdr->pass_count;
                                      new_dr->data->nowCursor = inPlaceMoveRC(new_dr->data->nowCursor, direction);

                                      next_offset = config->COLS * new_dr->data->nowCursor.y + new_dr->data->nowCursor.x;
                                      cur1 = new_dr->data->board[offset];
                                      cur2 = new_dr->data->board[next_offset];


                                      //swap orb;
                                      if(config->turnFirst>-1 && new_dr->path_count <5){
                                          new_dr->data->board[offset] = config->turnFirst ;
                                      }else
                                        new_dr->data->board[offset] = cur2;

                                      new_dr->data->board[next_offset] = cur1;
                                      new_dr->data->dirs[(unsigned int)new_dr->path_count] = direction;
                                      new_dr->path_count = new_dr->path_count + 1;
                                      ret = findCombo(new_dr, config);
                                      if(config->flag_pass_count > 0){
                                          if(config->fpass_board[next_offset]){
                                              if(new_dr->data->pass_board[next_offset] != 1){
                                                  new_dr->data->pass_board[next_offset] = 1;
                                                  new_dr->pass_count++;
                                              }else if(new_dr->data->pass_board[next_offset] ==1){
                                                  new_dr->data->pass_board[next_offset] = 0;
                                                  new_dr->pass_count--;
                                              }
                                          }
                                          if (config->flag_pass_count == new_dr->pass_count){
                                              new_dr->weight += 300;
                                              new_dr->final_weight += 300;
                                          }
                                      }


                                      if (!config->ComboModeType) {
                                          if (config->ComboMode && config->ComboMode < new_dr->combos_length) {
                                              if (p_array->result_index > 0)
                                                  p_array->result_index = p_array->result_index - 1;
                                          }
                                      }
                                      if (!ret) {
                                          fdr->isDone = 1;
                                          if(status_interrupt)
                                              return 0;
                                      }
                                  }
                              }
                          }
                      }
                  }
                  direction += dirstep;
              }while(direction <=7);
            fdr->isDone = 1;
            if (!ret)
                break;
            if (status_interrupt)
                return 0;
        }
        index++;
        fdr = &p_array->ResultPointer[index];
    }
    if (ret && p_array->result_index > config->Complex_val) {
        if (isMaxPath)
            return 1;
        if (config->ComboModeType || config->ComboMode <= 0)
            qsort(p_array->ResultPointer, p_array->result_index, sizeof(DoraResult), compareWeight);
        else
            qsort(p_array->ResultPointer, p_array->result_index, sizeof(DoraResult), compareComboAndWeight);
        p_array->result_index = p_array->result_index < config->Complex_val ? p_array->result_index : config->Complex_val;
        return 1;
    }
    else if (!isMaxPath) {
        p_array->result_index = config->Complex_val >= p_array->result_index? p_array->result_index : config->Complex_val;
        return 1;
    }
    return 0;
}

void solveBoard(char *mIdx,DoraConfig *config,DoraResultArrayPointer *p_array){
    status_interrupt = 0;
    //size_t b_size = config->ROWS * config->COLS;
    if(config->flag_start_count){
        for(int point_idx=0; point_idx<config->flag_start_count; point_idx++){
            SOPoint p0 = config->startPoint[point_idx];
            int offset = config->COLS*p0.y + p0.x;
            if((config->Weathering && (mIdx[offset] &0x40)) || (config->GemForRow && config->gems[(unsigned int)mIdx[offset]].gemComboType != 4)){
                continue;
            }
            if (p_array->result_index <= p_array->MaxResultSize) {
                DoraResultData *resultData = (DoraResultData *)p_array->ResultPointer[p_array->result_index].data;
                resultData->initCursor = { p0.x,p0.y };
                resultData->nowCursor = { p0.x,p0.y };
                memcpy(&p_array->ResultPointer[p_array->result_index].data->board, mIdx, board_size);
                if (config->firePathSize > 0) {
                    p_array->firepath[p_array->result_index].fpath[config->COLS * p0.y + p0.x] = (char)(config->firePathSize - 1);
                }
                p_array->result_index++;
            }
        }
    }
    else{
        for (int y = 0;y < config->ROWS;y++) {
            for (int x = 0;x < config->COLS;x++) {
                int offset = x + config->COLS * y;
                SOPoint p0 ={ (char)x,(char)y };
                if ( (config->Weathering && (mIdx[config->COLS * p0.y + p0.x] & 0x40)) || (config->GemForRow && config->gems[offset].gemComboType != 4))
                    continue;
                if (p_array->result_index <= p_array->MaxResultSize) {
                    DoraResultData *resultData = (DoraResultData *)p_array->ResultPointer[p_array->result_index].data;
                    resultData->initCursor = { p0.x,p0.y };
                    resultData->nowCursor = { p0.x,p0.y };
                    memcpy(&p_array->ResultPointer[p_array->result_index].data->board, mIdx, board_size);
                    if (config->firePathSize > 0) {
                        p_array->firepath[p_array->result_index].fpath[config->COLS * p0.y + p0.x] = (char)(config->firePathSize - 1);
                    }
                    p_array->result_index++;
                }
            }
        }
    }
    if(config->MaxPathSize>0){
        int path_index = 0;
        while(true){
            int ret = config->MaxPathSize-1==path_index?solveBoardStep(p_array,config,1):solveBoardStep(p_array,config,0);
            if(!ret)
                break;
            if(!status_interrupt){
                if(config->MaxPathSize>++path_index)
                    continue;
            }
            if(ret != 1)
                return;
            break;
        }
    }
    if(!status_interrupt){
        if(config->flag_end_count <= 0){
            if(config->ComboModeType || config->ComboMode<=0)
                qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareWL);
            else
                qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareOnComboCount);
        }else{
            if(p_array->result_index>0){
                for(int index = 0;index < p_array->result_index;index++){
                    for(int point_idx=0; point_idx< config->flag_end_count; point_idx++){
                        if(config->endPoint[point_idx].x == p_array->ResultPointer[index].data->nowCursor.x &&config->endPoint[point_idx].y == p_array->ResultPointer[index].data->nowCursor.y){
                            p_array->ResultPointer[index].final_weight += 1000;
                            p_array->ResultPointer[index].weight += 1000;

                        }
                    }
                }
            }
            qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareWL);
        }
    }
}
void solveBoard2(char *mIdx,DoraConfig *config,DoraResultArrayPointer *p_array){
    status_interrupt = 0;
    //size_t b_size = config->ROWS * config->COLS;
    if(config->flag_start_count){
        for(int point_idx=0; point_idx<config->flag_start_count; point_idx++){
            SOPoint p0 = config->startPoint[point_idx];
            int offset = config->COLS*p0.y + p0.x;
            if((config->Weathering && (mIdx[offset] &0x40)) || (config->GemForRow && config->gems[(unsigned int)mIdx[offset]].gemComboType != 4)){
                continue;
            }
            if (p_array->result_index <= p_array->MaxResultSize) {
                DoraResultData *resultData = (DoraResultData *)p_array->ResultPointer[p_array->result_index].data;
                resultData->initCursor = { p0.x,p0.y };
                resultData->nowCursor = { p0.x,p0.y };
                memcpy(&p_array->ResultPointer[p_array->result_index].data->board, mIdx, board_size);
                if (config->firePathSize > 0) {
                    p_array->firepath[p_array->result_index].fpath[config->COLS * p0.y + p0.x] = (char)(config->firePathSize - 1);
                }
                p_array->result_index++;
            }
        }
    }
    if(config->MaxPathSize>0){
        int path_index = 0;
        while(true){
            int ret = config->MaxPathSize-1==path_index?solveBoardStep(p_array,config,1):solveBoardStep(p_array,config,0);
            if(!ret)
                break;
            if(!status_interrupt){
                if(config->MaxPathSize>++path_index)
                    continue;
            }
            if(ret != 1)
                return;
            break;
        }
    }
    if(!status_interrupt){
        if(config->flag_end_count <= 0){
            if(config->ComboModeType || config->ComboMode<=0)
                qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareWL);
            else
                qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareOnComboCount);
        }else{
            if(p_array->result_index>0){
                for(int index = 0;index < p_array->result_index;index++){
                    for(int point_idx=0; point_idx< config->flag_end_count; point_idx++){
                        if(config->endPoint[point_idx].x == p_array->ResultPointer[index].data->nowCursor.x &&config->endPoint[point_idx].y == p_array->ResultPointer[index].data->nowCursor.y){
                            p_array->ResultPointer[index].final_weight += 1000;
                        }
                    }
                }
            }
            qsort(p_array->ResultPointer,p_array->result_index,sizeof(DoraResult),compareWL);
        }
    }
}

int solveBoardStep2(DoraResultArrayPointer *p_array, DoraConfig *config, int isMaxPath) {
    int ret = 1;
    int dirstep = config->isSupportDir != 1 ? 2 : 1;
    int index = 0;
    char cur1;
    char cur2;
    int offset;
    int next_offset;

    SOPoint rc;
    DoraResult *fdr = &p_array->ResultPointer[index];
    DoraResult *new_dr;
    FirePath* pf;
    int direction;
    int canMoveOrb;
    int result_index;
    for (result_index = p_array->result_index;result_index>0;--result_index) {
        if (fdr->isDone != 1) {
            direction = 0;
            do{
                  if (!fdr->path_count || fdr->data->dirs[fdr->path_count-1] != ((direction + 4) & 7)) {
                      canMoveOrb = 0;
                      switch (direction)
                      {
                          case 0:
                              canMoveOrb = fdr->data->nowCursor.x < config->COLS - 1;
                              break;
                          case 1:
                              if (fdr->data->nowCursor.y < config->ROWS - 1 && fdr->data->nowCursor.x < config->COLS - 1)
                                  canMoveOrb = 1;
                              break;
                          case 2:
                              canMoveOrb = fdr->data->nowCursor.y < config->ROWS - 1;
                              break;
                          case 3:
                              if (fdr->data->nowCursor.y < config->ROWS - 1 && fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 4:
                              if (fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 5:
                              if (fdr->data->nowCursor.y && fdr->data->nowCursor.x>0)
                                  canMoveOrb = 1;
                              break;
                          case 6:
                              canMoveOrb = fdr->data->nowCursor.y;
                              if (fdr->data->nowCursor.y)
                                  canMoveOrb = 1;
                              break;
                          case 7:
                              if (fdr->data->nowCursor.y>0 && fdr->data->nowCursor.x < config->COLS - 1)
                                  canMoveOrb = 1;
                              break;
                          default:
                              break;
                      }
                      if (canMoveOrb) {
                          offset = config->COLS * fdr->data->nowCursor.y + fdr->data->nowCursor.x;
                          rc = inPlaceMoveRC(fdr->data->nowCursor, direction);
                          next_offset = config->COLS * rc.y + rc.x;
                          if (!config->Weathering || !(fdr->data->board[next_offset] & 0x40)) {
                              if (p_array->result_index <= p_array->MaxResultSize) {
                                  new_dr = &p_array->ResultPointer[p_array->result_index];
                                  p_array->result_index = p_array->result_index + 1;
                                  if (new_dr) {
                                      pf = new_dr->data->pfirePath;
                                      memcpy(new_dr->data, fdr->data, sizeof(DoraResultData));
                                      if(config->firePathSize>0){
                                          new_dr->data->pfirePath = pf;
                                          memcpy(new_dr->data->pfirePath,fdr->data->pfirePath,sizeof(FirePath));
                                      }

                                      new_dr->weight = fdr->weight;
                                      new_dr->path_count = fdr->path_count;
                                      if(new_dr->path_count>50)
                                        int y = 99;
                                      new_dr->isDone = 0;
                                      new_dr->combos_length = 0;
                                      new_dr->weight = 0;
                                      new_dr->final_weight = 0;
                                      new_dr->pass_count = fdr->pass_count;
                                      new_dr->data->nowCursor = inPlaceMoveRC(new_dr->data->nowCursor, direction);

                                      next_offset = config->COLS * new_dr->data->nowCursor.y + new_dr->data->nowCursor.x;
                                      cur1 = new_dr->data->board[offset];
                                      cur2 = new_dr->data->board[next_offset];


                                      //swap orb;
                                      if(config->turnFirst>-1 && new_dr->path_count <5){
                                          new_dr->data->board[offset] = config->turnFirst ;
                                      }else
                                        new_dr->data->board[offset] = cur2;

                                      new_dr->data->board[next_offset] = cur1;
                                      new_dr->data->dirs[(unsigned int)new_dr->path_count] = direction;
                                      new_dr->path_count = new_dr->path_count + 1;
                                      ret = findCombo(new_dr, config);
                                      if(config->flag_pass_count > 0){
                                          if(config->fpass_board[next_offset]){
                                              if(new_dr->data->pass_board[next_offset] != 1){
                                                  new_dr->data->pass_board[next_offset] = 1;
                                                  new_dr->pass_count++;
                                              }else if(new_dr->data->pass_board[next_offset] ==1){
                                                  new_dr->data->pass_board[next_offset] = 0;
                                                  new_dr->pass_count--;
                                              }
                                          }
                                          if (config->flag_pass_count == new_dr->pass_count){
                                              new_dr->weight += 300;
                                              new_dr->final_weight += 300;
                                          }
                                      }


                                      if (!config->ComboModeType) {
                                          if (config->ComboMode && config->ComboMode < new_dr->combos_length) {
                                              if (p_array->result_index > 0)
                                                  p_array->result_index = p_array->result_index - 1;
                                          }
                                      }
                                      if (!ret) {
                                          fdr->isDone = 1;
                                          if(status_interrupt)
                                              return 0;
                                      }
                                  }
                              }
                          }
                      }
                  }
                  direction += dirstep;
              }while(direction <=7);
            fdr->isDone = 1;
            if (!ret)
                break;
            if (status_interrupt)
                return 0;
        }
        index++;
        fdr = &p_array->ResultPointer[index];
    }
    if (ret && p_array->result_index > config->Complex_val) {
        if (isMaxPath)
            return 1;
        if (config->ComboModeType || config->ComboMode <= 0)
            qsort(p_array->ResultPointer, p_array->result_index, sizeof(DoraResult), compareWeight);
        else
            qsort(p_array->ResultPointer, p_array->result_index, sizeof(DoraResult), compareComboAndWeight);
        p_array->result_index = p_array->result_index < config->Complex_val ? p_array->result_index : config->Complex_val;
        return 1;
    }
    else if (!isMaxPath) {
        p_array->result_index = config->Complex_val >= p_array->result_index? p_array->result_index : config->Complex_val;
        return 1;
    }
    return 0;
}


DoraResultArrayPointer kora_solve(DoraResultArrayPointer p_array0,int mCols, int mRows, int mIdx[], int params[], int color_priority[], int startPoint[]) {

    int *params0 = params;
    int *color_priority0 = color_priority;
    int *mIdx0 = mIdx;
    char *cboard;

    config = new DoraConfig ;
    memset(config, 0, sizeof(DoraConfig));
    DoraResultArrayPointer *p_array;
    p_array = new DoraResultArrayPointer;
    memset(p_array, 0, sizeof(DoraResultArrayPointer));
    p_array0 = *p_array;

    //config = new DoraConfig;
    config->COLS = (int)mCols;
    config->ROWS = (int)mRows;

    //CUSTOM
    points0 = new SOPoint[30];

    board_size = config->ROWS * config->COLS;
    current_board = new char[board_size];
    match_board = new char[48];
    scatch_board = new char[board_size];

    if (board_size > 0) {
        cboard = new char[board_size];
        for (unsigned int i = 0;i < board_size;i++) {
            cboard[i] = (char)mIdx0[i];
            if (mIdx0[i] & 0x100) {
                config->Weathering++;
                cboard[i] = (cboard[i] | 0x40);
            }
            if(mIdx0[i] & 0x400) {
                config->Priority++;
                cboard[i] = (cboard[i] | 0x80);
            }
            if(mIdx0[i] & 0x800) {
                config->Reserve++;
                cboard[i] = (cboard[i] | 0x20);
            }
            if(mIdx0[i] & 0x200) {
                config->puzzle_board[i] = (char) 1;
                config->PuzzleShield = 1;
            }
            if(mIdx0[i] & 0x2000){
                config->endPoint[config->flag_end_count] = {(char) floor(i / config->COLS), (char)(i%config->COLS)};
                config->flag_end_count++;
            }
            if(mIdx0[i] & 0x4000){
                config->fpass_board[i] = 1;
                config->flag_pass_count++;
            }
            int orb_val = cboard[i] & 0x1F;
            config->GemCount[orb_val]++;
        }
        config->config_220 =1;
    }

    for (int i = 0;i < 8;i++) {
        config->gems[i].gemPrior = color_priority0[ i ];
        config->gems[i].gemComboType = color_priority0[i + 8 ];
        config->gems[i].gemFlag = color_priority0[i + 16];
    }
    config->gems[9].gemPrior = 1;
    config->gems[10].gemPrior = 1;
    config->MaxPathSize = params0[0];
    config->mMode = params0[2];
    config->ComboMode = params0[3];
    config->use3Gem = params0[5];
    config->ComboModeType = params0[8];
    config->FirstBatchCount = params0[11];
    config->useTurboMode = params0[6];
    config->GemsTypeCondition = params0[7];
    config->MatchType = params0[9];
    config->DropDirection = params0[12];
    config->firePathSize = params0[13];
    config->MaxHurtCount = params0[14];
    config->isSupportDir = params0[15];
    config->firstTotal = params0[16];
    config->puzzleGemType = params0[17];
    config->turnFirst = params0[18] - 1;
    if(config->useTurboMode){
        for(int i=0;i<8;i++){
            config->gems[i].gemPrior = 1;
            config->gems[i].gemComboType = config->gems[i].gemComboType > 1 ? 0 : 1;
        }
        config->PuzzleShield = 0;
        config->Complex_val = params0[1]>100?48000:480*params0[1];
    }else{
        config->Complex_val = 480*params0[1] * 4;
    }
    int maxCombo = 0;
    for (int i = 0;i < 6;i++) {
        if(config->gems[i].gemComboType == a2Gems)
            maxCombo += config->GemCount[i] / 2;
        else
            maxCombo += config->GemCount[i] / 3;
    }
    MaxCombo = maxCombo;
    if(config->useTurboMode){
        config->ComboMode = maxCombo;
        config->ComboModeType = 0;
    }else if(config->ComboMode != 9 && config->ComboMode != 10 && config->ComboMode<=maxCombo){

    }else{
        config->ComboMode =maxCombo;
    }
    if(!startPoint){
        for(int x =0 ;x<config->COLS;x++){
            for(int y=0;y<config->ROWS;y++)
            {
                int offset = config->COLS * y + x;
                if(mIdx0[offset] & 0x1000){ //detected question on board
                    config->startPoint[config->flag_start_count] = {(char)x, (char)y};
                    config->flag_start_count++;
                }
            }
        }
    }else{
        // if(sizeof(startPoint)/sizeof(startPoint[0]) <=0 || sizeof(startPoint)/sizeof(startPoint[0]) &1)
        //     config->flag_start_count = 0;
        // else{
        //     int *startPoint_ = startPoint;
        //     for(int i = 0 ;i<sizeof(startPoint)/sizeof(startPoint[0]);i++){
        //        config->startPoint[i] = {(char)(startPoint_[2*i]),(char)(startPoint_[2*i+1])};
        //        config->flag_start_count++;
        //     }
        //     delete startPoint_;
        // }
    }

    if(config->PuzzleShield)
        config->use3Gem = 0;
    config->useTosCol = 0;
    config->GemForRow = 0;

    if ( !config->useTurboMode ) {
        for (int x = 0; x<config->COLS; x++) {
            if ((params0[4] >> x) & 1)
                config->useTosCol = 1;
            config->isTosCol[x] = (params0[4] >> x) & 1;
            config->tosCol[x] = (params0[10] >> 2 * x) & 3;
        }
        for (int i = 0; i < 11; i++) {
            if (config->gems[i].gemComboType == 1)
                config->GemTypeVal = config->gems[i].gemComboType;
            else if (config->gems[i].gemComboType == 4)
                config->GemForRow = 1;
        }
    }
    int result_index = -1;
    int dirs = config->isSupportDir?8:4;
    if(initDynamicArray(&p_array0,dirs * config->Complex_val,config->firePathSize>0)){
        if(config->useTurboMode){
            int ri = 0;
            for(int index=0;index<7;index++){
                solveBoard(cboard,config,&p_array0);
                if(isInterrupted())
                    break;
                if(ri<p_array0.MaxResultSize){
                    ri = p_array0.MaxResultSize;
                }
                for(int i=0;i<6;i++){
                    int orbPriority = i==index?10:1;
                    config->gems[i].gemPrior = orbPriority;
                }

            }
            result_index = isInterrupted() == 0;
        }
        else{
            solveBoard(cboard,config,&p_array0);
            

            if(!isInterrupted())
                result_index = p_array0.result_index;
        }
    }
    delete [] points0;
    delete [] current_board;
    delete config;
    // delete  []params;
    // delete [] color_priority;
    // delete [] mIdx;

    return p_array0;
}
void interruptSolving(void){
    status_interrupt = 1;
}

void kora_clean_result(DoraResultArrayPointer p_array){
    freeDynamicArray(&p_array);
}
void kora_interrupt(){
    interruptSolving();
}


int test(int a, int b){
    return a + b;
}