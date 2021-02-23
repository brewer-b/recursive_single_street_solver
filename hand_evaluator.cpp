#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int HR[32487834];

int LookupHand(int const * pCards, int const * holeCards){
    static bool init = false;
    if (!init){
        // Load the HandRanks.DAT file and map it into the HR array
        printf("Loading HandRanks.DAT file...");
        memset(HR, 0, sizeof(HR));
        FILE * fin = fopen("HandRanks.dat", "rb");
        if (!fin)
            return false;
        fread(HR, sizeof(HR), 1, fin);   // get the HandRank Array
        fclose(fin);
        printf("complete.\n\n");
        init  = true;
    }
    int p = HR[53 + *holeCards++];
    p = HR[p + *holeCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
    p = HR[p + *pCards++];
   return HR[p + *pCards++];
}