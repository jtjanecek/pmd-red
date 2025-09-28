#include "locale/personality_test1_usa.h"

ALIGNED(4) static const u8 gSeedModePrompt[] = _("{CENTER_ALIGN}Choose how to set the exploration seed.");
static const MenuItem gSeedModeMenu[] = {
    { _("Random"), 0 },
    { _("Custom"), 1 },
    { NULL, -1 },
};

ALIGNED(4) static const u8 gSeedCustomPrompt[] = _("{CENTER_ALIGN}Enter the exploration seed\n{CENTER_ALIGN}using the number pad.");
ALIGNED(4) static const u8 gStarterPrompt[] = _("{CENTER_ALIGN}Select your character.");

ALIGNED(4) static const u8 sGender0[] = GENDER_Q;
static const MenuItem gGenderMenu[] = {
    { GENDER_A0, 0 },
    { GENDER_A1, 1 },
    { NULL, -1 },
};

ALIGNED(4) static const u8 gStarterReveal[] = STARTER_REVEAL;
ALIGNED(4) static const u8 gPartnerPrompt[] = PARTNER_PROMPT;
ALIGNED(4) static const u8 gPartnerNickPrompt[] = PARTNER_NICK_PROMPT;
ALIGNED(4) static const u8 gEndIntroText[] = END_TEXT;

static const WindowTemplate sUnknown_80F4244 = {
    0,
    5,
    12, 6,
    5, 5,
    5, 0,
    NULL
};
