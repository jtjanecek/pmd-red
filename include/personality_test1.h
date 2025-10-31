#ifndef GUARD_PERSONALITY_TEST1_H
#define GUARD_PERSONALITY_TEST1_H

#include "constants/personality_test.h"
#include "constants/difficulty.h"
#include "constants/global.h"
#include "structs/menu.h"

typedef u8 PersonalityEffects[NUM_PERSONALITY_TEST_EFFECTS];

// size: 0xC
typedef struct PersonalityQuestion
{
    /* 0x0 */ const u8 *question;
    /* 0x4 */ const MenuItem *answers;
    /* 0x8 */ const PersonalityEffects *effects;
} PersonalityQuestion;

// size: 0x45
typedef struct TeamBasicInfo
{
    /* 0x0 */ u32 unk0;
    /* 0x4 */ s16 StarterID;
    /* 0x6 */ s16 PartnerID;
    /* 0x8 */ u8 StarterName[20];
    /* 0x1C */ u8 PartnerNick[20];
    /* 0x30 */ u8 TeamName[TEAM_NAME_LENGTH + 1];
    /* 0x3C */ s32 customSeed;
    /* 0x40 */ u8 difficulty;
    /* 0x41 */ u8 skipCutscenes;
    /* 0x42 */ u8 skipBasicRescues;
    /* 0x43 */ u8 recruitAll;
} TeamBasicInfo;

typedef TeamBasicInfo PersonalityRelated;

// size: 0x74
#define PERSONALITY_TEST_SEED_BUFFER_SIZE 12
typedef struct PersonalityTestTracker
{
    /* 0x0 */ s32 FrameCounter;
    /* 0x4 */ PersonalityRelated unk4;
    /* 0x38 */ u32 TestState;
    /* 0x3C */ u32 playerGender; // 0 = Male, 1 = Female
    /* 0x40 */ TouchScreenMenuInput input;
    /* 0x4C */ s32 rngSeed;
    /* 0x50 */ bool8 seedChosen;
    /* 0x51 */ bool8 usingCustomSeed;
    /* 0x52 */ u8 seedBuffer[PERSONALITY_TEST_SEED_BUFFER_SIZE];
} PersonalityTestTracker;

bool8 CreateTestTracker(void);
void DeleteTestTracker(void);
u32 HandleTestTrackerState(void);

#endif // GUARD_PERSONALITY_TEST1_H
