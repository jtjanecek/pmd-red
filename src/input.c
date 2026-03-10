#include "global.h"
#include "globaldata.h"
#include "code_800C9CC.h"
#include "cpu.h"
#include "debug.h"
#include "input.h"
#include "main.h"

#define JUNK_INIT 0x4A14C1
#define JUNK_UPDATE 0x54A1C41
#define MENU_SOFT_RESET_COMBO (A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON)
#define SOFT_RESET_SEQUENCE_TIMEOUT 30
#define SOFT_RESET_RELEASE_GRACE 1
#define SOFT_RESET_DEBUG_LOG 0
#define KEY_TEST_LOG 1

EWRAM_DATA Inputs gRealInputs = {0}; // R=20255F0 | B=20F5CC0
static EWRAM_DATA UnusedInputStruct sUnusedInputsRelated = {0}; // R=2025600 | B=020F5CD0
static EWRAM_DATA u32 sUnusedScrambledInputJunk[3] = {0}; // R=202562C | B=020F5C88
static EWRAM_DATA Inputs sBufferedInputs = {0}; // R=2025638 | B=020F5C90
static EWRAM_DATA Inputs sCurrentInputs = {0}; // R=2025648 | B=020F5CB0
static EWRAM_DATA Inputs sPrevInputs = {0}; // R=2025658 | B=020F5CA0
static EWRAM_DATA InputTimers sInputTimers = {0}; // R=2025668 | B=020F5C8C
#if KEY_TEST_LOG
static EWRAM_DATA u16 sKeyTestPrevRaw = {0};
#endif
static EWRAM_DATA u16 sSoftResetHoldMask = {0};
static EWRAM_DATA u8 sSoftResetSequenceTimer = {0};
static EWRAM_DATA u8 sSoftResetReleaseFrames = {0};
static EWRAM_DATA u8 sSoftResetConfirmFrames = {0};
#if SOFT_RESET_DEBUG_LOG
static EWRAM_DATA u16 sSoftResetLogPrevHeld = {0};
static EWRAM_DATA u16 sSoftResetLogPrevPressed = {0};
static EWRAM_DATA u16 sSoftResetLogPrevMask = {0};
static EWRAM_DATA u16 sSoftResetLogPrevRawCombo = {0};
static EWRAM_DATA u8 sSoftResetLogPrevTimer = {0};
static EWRAM_DATA u8 sSoftResetLogPrevReleaseFrames = {0};
static EWRAM_DATA u8 sSoftResetLogPrevConfirmFrames = {0};
#endif

static void PerformSoftReset(void)
{
#if SOFT_RESET_DEBUG_LOG
    Log(0, "[SR] trigger");
#endif
    asm volatile("mov r0, #0xff\n\t"
                 "swi 0x1\n\t"
                 "mov r0, #0\n\t"
                 "swi 0x0");
    AgbMain();
    while (TRUE) {}
}

#if SOFT_RESET_DEBUG_LOG
static void SoftResetDebugLogState(u16 held, u16 pressed, u16 rawCombo, u16 mask, u8 timer, u8 releaseFrames, u8 confirmFrames)
{
    if (held == sSoftResetLogPrevHeld
        && pressed == sSoftResetLogPrevPressed
        && rawCombo == sSoftResetLogPrevRawCombo
        && mask == sSoftResetLogPrevMask
        && timer == sSoftResetLogPrevTimer
        && releaseFrames == sSoftResetLogPrevReleaseFrames
        && confirmFrames == sSoftResetLogPrevConfirmFrames)
        return;

    sSoftResetLogPrevHeld = held;
    sSoftResetLogPrevPressed = pressed;
    sSoftResetLogPrevRawCombo = rawCombo;
    sSoftResetLogPrevMask = mask;
    sSoftResetLogPrevTimer = timer;
    sSoftResetLogPrevReleaseFrames = releaseFrames;
    sSoftResetLogPrevConfirmFrames = confirmFrames;

    Log(0, "[SR] raw=%03x c=%03x held=%03x pr=%03x mask=%03x t=%d r=%d f=%d",
        gRawKeyInput, rawCombo, held, pressed, mask, timer, releaseFrames, confirmFrames);
}
#endif

// arm9.bin::0200754C
void InitInput(void)
{
    gRealInputs.held = 0;
    gRealInputs.pressed = 0;
    gRealInputs.repeated = 0;
    gRealInputs.shortPress = 0;

    sBufferedInputs.held = 0;
    sBufferedInputs.pressed = 0;
    sBufferedInputs.repeated = 0;
    sBufferedInputs.shortPress = 0;

    sUnusedScrambledInputJunk[0] = JUNK_INIT;

    sUnusedInputsRelated.unk20 = 0;
    sUnusedInputsRelated.unk0 = 0xFFFF; // probably a mask
    sUnusedInputsRelated.unk2 = -1;
    sUnusedInputsRelated.unk4 = -1;
    sUnusedInputsRelated.unk6 = -1;
    sUnusedInputsRelated.unk1C = -1;
    sUnusedInputsRelated.unk1E = -1;
    sUnusedInputsRelated.unk8 = -1;
    sUnusedInputsRelated.unkA = -1;
    sUnusedInputsRelated.unkC = -1;
    sUnusedInputsRelated.unkE = -1;
    sUnusedInputsRelated.unk10 = -1;
    sUnusedInputsRelated.unk12 = -1;
    sUnusedInputsRelated.unk14 = -1;
    sUnusedInputsRelated.unk16 = -1;
    sUnusedInputsRelated.unk28 = 0;
    sUnusedInputsRelated.unk29 = 0;

    sInputTimers.holdTimerB = 0;
    sInputTimers.holdTimerR = 0;
#if KEY_TEST_LOG
    sKeyTestPrevRaw = 0xFFFF;
#endif
    sSoftResetHoldMask = 0;
    sSoftResetSequenceTimer = 0;
    sSoftResetReleaseFrames = 0;
    sSoftResetConfirmFrames = 0;
#if SOFT_RESET_DEBUG_LOG
    sSoftResetLogPrevHeld = 0xFFFF;
    sSoftResetLogPrevPressed = 0xFFFF;
    sSoftResetLogPrevRawCombo = 0xFFFF;
    sSoftResetLogPrevMask = 0xFFFF;
    sSoftResetLogPrevTimer = 0xFF;
    sSoftResetLogPrevReleaseFrames = 0xFF;
    sSoftResetLogPrevConfirmFrames = 0xFF;
#endif
}

// arm9.bin::02007200
void LoadBufferedInputs(void)
{
    gRealInputs = sBufferedInputs;

    sBufferedInputs.held = 0;
    sBufferedInputs.pressed = 0;
    sBufferedInputs.repeated = 0;
    sBufferedInputs.shortPress = 0;

    sUnusedInputsRelated.unk0 = 0xFFFF;
    sUnusedInputsRelated.unk2 = -1;
    sUnusedInputsRelated.unk4 = -1;
    sUnusedInputsRelated.unk6 = -1;
    sUnusedInputsRelated.unk8 = -1;
    sUnusedInputsRelated.unkA = -1;
    sUnusedInputsRelated.unkC = -1;
    sUnusedInputsRelated.unkE = -1;
    sUnusedInputsRelated.unk10 = -1;
    sUnusedInputsRelated.unk12 = -1;
    sUnusedInputsRelated.unk14 = -1;
    sUnusedInputsRelated.unk16 = -1;

    sUnusedInputsRelated.unk28 = 0;
    sUnusedInputsRelated.unk29 = 0;

    // This is way different in blue
}

// TODO: 2 funcs in blue. Not sure which they are in red, if they exist
// arm9.bin::020071CC
// arm9.bin::02007198

UNUSED static bool8 sub_80048B8(void)
{
    return FALSE;
}

bool8 sub_80048BC(void)
{
    return FALSE;
}

UNUSED static bool8 sub_80048C0(void)
{
    return FALSE;
}

UNUSED static bool8 sub_80048C4(void)
{
    return FALSE;
}

bool8 sub_80048C8(void)
{
    return FALSE;
}

UNUSED static bool8 sub_80048CC(void)
{
    return FALSE;
}

// arm9.bin::0200715C
void ResetRepeatTimers(void)
{
    gRealInputs.repeated = 0;
    sCurrentInputs.repeatTimerDpad = 0;
    sCurrentInputs.heldDpad = 0;
    sInputTimers.holdTimerB = 999;
    sInputTimers.holdTimerR = 999;
}

// arm9.bin::02007130
void UnpressButtons(void)
{
    gRealInputs.pressed = 0;
    sBufferedInputs.pressed = 0;
    sCurrentInputs.pressed = 0;
}

// arm9.bin::020070D4
void ResetUnusedInputStruct(void)
{
    sUnusedInputsRelated.unk20 = 5;
    sUnusedInputsRelated.unk24 = 0;
    sUnusedInputsRelated.unk0 = 0xFFFF;
    sUnusedInputsRelated.unk2 = -1;
    sUnusedInputsRelated.unk4 = -1;
    sUnusedInputsRelated.unk6 = -1;
    sUnusedInputsRelated.unk8 = -1;
    sUnusedInputsRelated.unkA = -1;
    sUnusedInputsRelated.unkC = -1;
    sUnusedInputsRelated.unkE = -1;
    sUnusedInputsRelated.unk10 = -1;
    sUnusedInputsRelated.unk12 = -1;
    sUnusedInputsRelated.unk14 = -1;
    sUnusedInputsRelated.unk16 = -1;

    sUnusedInputsRelated.unk28 = 0;
    sUnusedInputsRelated.unk29 = 0;
}

// arm9.bin::02006EC0
void UpdateInput(void)
{
    sPrevInputs = sCurrentInputs;

    ReadKeyInput(&sCurrentInputs);

    sCurrentInputs.pressed = (sPrevInputs.held ^ sCurrentInputs.held) & sCurrentInputs.held;

#if KEY_TEST_LOG
    if (gRawKeyInput != sKeyTestPrevRaw) {
        sKeyTestPrevRaw = gRawKeyInput;
        Log(0, "[KEY] raw=%03x held=%03x pr=%03x", gRawKeyInput, sCurrentInputs.held, sCurrentInputs.pressed);
    }
#endif

    if (sCurrentInputs.held) {
        if ((sCurrentInputs.heldDpad & DPAD_ANY) == (sCurrentInputs.held & DPAD_ANY)) {
            if (sCurrentInputs.repeatTimerDpad < 50)
                sCurrentInputs.repeatTimerDpad++;
        }
        else {
            sCurrentInputs.heldDpad = sCurrentInputs.held & DPAD_ANY;
            sCurrentInputs.repeatTimerDpad = 1;
        }
    }
    else {
        sCurrentInputs.repeatTimerDpad = 0;
        sCurrentInputs.heldDpad = 0;
    }

    if (sCurrentInputs.repeatTimerDpad == 1)
        sCurrentInputs.repeated = (sCurrentInputs.heldDpad & DPAD_ANY) | sCurrentInputs.pressed;
    else if (sCurrentInputs.repeatTimerDpad == 48) {
        sCurrentInputs.repeatTimerDpad = 43;
        sCurrentInputs.repeated = (sCurrentInputs.heldDpad & DPAD_ANY) | sCurrentInputs.pressed;
    }
    else
        sCurrentInputs.repeated = 0;

    sCurrentInputs.shortPress = 0;

    if (sCurrentInputs.held & B_BUTTON) {
        if (sInputTimers.holdTimerB < 100)
            sInputTimers.holdTimerB++;
    }
    else if (1 < sInputTimers.holdTimerB && sInputTimers.holdTimerB < 12) {
        sCurrentInputs.shortPress = B_BUTTON;
        sInputTimers.holdTimerB = 0;
    }
    else
        sInputTimers.holdTimerB = 0;

    if (sCurrentInputs.held & R_BUTTON) {
        if (sInputTimers.holdTimerR < 100)
            sInputTimers.holdTimerR++;
    }
    else if (1 < sInputTimers.holdTimerR && sInputTimers.holdTimerR < 12) {
        sCurrentInputs.shortPress |= R_BUTTON;
        sInputTimers.holdTimerR = 0;
    }
    else
        sInputTimers.holdTimerR = 0;

    sBufferedInputs.held |= sCurrentInputs.held;
    sBufferedInputs.pressed |= sCurrentInputs.pressed;
    sBufferedInputs.repeated |= sCurrentInputs.repeated;
    sBufferedInputs.shortPress |= sCurrentInputs.shortPress;

    {
        u16 comboBits = gRawKeyInput & MENU_SOFT_RESET_COMBO;
        u16 comboPressedBits = sCurrentInputs.pressed & MENU_SOFT_RESET_COMBO;
        u16 prevHoldMask = sSoftResetHoldMask;
        u16 newPressedBits;

        if (prevHoldMask == 0) {
            sSoftResetReleaseFrames = 0;
            // Start a combo attempt only on freshly-pressed combo buttons.
            if (comboPressedBits != 0) {
                sSoftResetHoldMask = comboBits | comboPressedBits;
                sSoftResetSequenceTimer = SOFT_RESET_SEQUENCE_TIMEOUT;
            }
        }
        else {
            // Track currently held combo bits during an active attempt.
            sSoftResetHoldMask |= comboBits;

            newPressedBits = comboPressedBits & ~prevHoldMask;
            if (newPressedBits != 0) {
                sSoftResetHoldMask |= newPressedBits;
                // Refresh window only when progress is made.
                sSoftResetSequenceTimer = SOFT_RESET_SEQUENCE_TIMEOUT;
                sSoftResetReleaseFrames = 0;
            }
            else {
                if (comboBits == 0) {
                    if (sSoftResetReleaseFrames < 255)
                        sSoftResetReleaseFrames++;
                }
                else {
                    sSoftResetReleaseFrames = 0;
                }

                if (sSoftResetReleaseFrames >= SOFT_RESET_RELEASE_GRACE) {
                    sSoftResetHoldMask = 0;
                    sSoftResetSequenceTimer = 0;
                }
                else if (sSoftResetSequenceTimer != 0) {
                    sSoftResetSequenceTimer--;
                }
                else {
                    sSoftResetHoldMask = 0;
                }
            }
        }

        if (comboBits == MENU_SOFT_RESET_COMBO) {
            if (sSoftResetConfirmFrames < 255)
                sSoftResetConfirmFrames++;
        }
        else {
            sSoftResetConfirmFrames = 0;
        }

#if SOFT_RESET_DEBUG_LOG
        if (comboBits || sSoftResetSequenceTimer || sSoftResetHoldMask) {
            SoftResetDebugLogState(sCurrentInputs.held, sCurrentInputs.pressed, comboBits, sSoftResetHoldMask, sSoftResetSequenceTimer, sSoftResetReleaseFrames, sSoftResetConfirmFrames);
        }
#endif

        // ABSS-only trigger: either true simultaneous hold for 2 frames, or all
        // four bits collected within the active input window.
        if (sSoftResetConfirmFrames >= 2
            || ((sSoftResetHoldMask & MENU_SOFT_RESET_COMBO) == MENU_SOFT_RESET_COMBO)) {
            PerformSoftReset();
        }
    }

    sUnusedScrambledInputJunk[0] *= sCurrentInputs.held | JUNK_UPDATE;
}
