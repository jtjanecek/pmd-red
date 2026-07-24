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
// Frames all four combo buttons must be held *simultaneously* to trigger.
#define SOFT_RESET_CONFIRM_FRAMES 4
#define SOFT_RESET_DEBUG_LOG 1
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
static EWRAM_DATA u8 sSoftResetConfirmFrames = {0};
#if SOFT_RESET_DEBUG_LOG
static EWRAM_DATA u16 sSoftResetLogPrevRawCombo = {0};
static EWRAM_DATA u8 sSoftResetLogPrevConfirmFrames = {0};
#endif

static void PerformSoftReset(void)
{
#if SOFT_RESET_DEBUG_LOG
    Log(0, "[SR] trigger");
#endif
    // Mirror the libagbsyscall SoftReset wrapper (kept inline to avoid pulling
    // SoftReset.o into the link): disable interrupts, restore SP to USER_STACK
    // (0x03007F00, inside the 0x200-byte IWRAM tail that RegisterRamReset
    // preserves), then RegisterRamReset(RESET_ALL) + BIOS SoftReset. The BIOS
    // never returns from swi 0x00, so the loop below is only a safety net.
    asm volatile(
        "ldr r0, =0x04000208\n\t" // REG_IME
        "movs r1, #0\n\t"
        "strb r1, [r0]\n\t"
        "ldr r0, =0x03007F00\n\t" // USER_STACK
        "mov sp, r0\n\t"
        "movs r0, #0xFF\n\t"      // RESET_ALL
        "swi 0x01\n\t"            // RegisterRamReset
        "swi 0x00\n\t"           // SoftReset
        ::: "r0", "r1", "memory");
    while (TRUE) {}
}

#if SOFT_RESET_DEBUG_LOG
static void SoftResetDebugLogState(u16 rawCombo, u8 confirmFrames)
{
    if (rawCombo == sSoftResetLogPrevRawCombo
        && confirmFrames == sSoftResetLogPrevConfirmFrames)
        return;

    sSoftResetLogPrevRawCombo = rawCombo;
    sSoftResetLogPrevConfirmFrames = confirmFrames;

    Log(0, "[SR] raw=%03x c=%03x f=%d", gRawKeyInput, rawCombo, confirmFrames);
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
    sSoftResetConfirmFrames = 0;
#if SOFT_RESET_DEBUG_LOG
    sSoftResetLogPrevRawCombo = 0xFFFF;
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

        // Only a true simultaneous hold of all four buttons counts. Requiring
        // several consecutive frames rejects transient glitches and prevents
        // rolling button sequences (e.g. running with B, mashing A) from ever
        // accumulating into a reset.
        if (comboBits == MENU_SOFT_RESET_COMBO) {
            if (sSoftResetConfirmFrames < 255)
                sSoftResetConfirmFrames++;
        }
        else {
            sSoftResetConfirmFrames = 0;
        }

#if SOFT_RESET_DEBUG_LOG
        if (comboBits || sSoftResetConfirmFrames)
            SoftResetDebugLogState(comboBits, sSoftResetConfirmFrames);
#endif

        if (sSoftResetConfirmFrames >= SOFT_RESET_CONFIRM_FRAMES)
            PerformSoftReset();
    }

    sUnusedScrambledInputJunk[0] *= sCurrentInputs.held | JUNK_UPDATE;
}
