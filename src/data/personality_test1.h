#include "locale/personality_test1_usa.h"

ALIGNED(4) static const u8 gSeedModePrompt[] = _(
    "{CENTER_ALIGN}Choose how to set the dungeon seed.\n"
    "{CENTER_ALIGN}The seed randomizes tilesets and Pokemon."
);
static const MenuItem gSeedModeMenu[] = {
    { _("Vanilla"), 0 },
    { _("Random"), 1 },
    { _("Custom"), 2 },
    { NULL, -1 },
};

ALIGNED(4) static const u8 gSeedCustomPrompt[] = _("{CENTER_ALIGN}Enter the dungeon seed\n{CENTER_ALIGN}using the number pad.");
ALIGNED(4) static const u8 gDifficultyPrompt[] = _(
    "{CENTER_ALIGN}Vanilla: Similar difficulty to the base game..\n"
    "{CENTER_ALIGN}Hard: Stronger enemies. More bosses.\n"
    "{CENTER_ALIGN}Nightmare: Only the best will survive."
);
static const MenuItem gDifficultyMenu[] = {
    { _("Vanilla"), DIFFICULTY_VANILLA },
    { _("Hard"), DIFFICULTY_HARD },
    { _("Nightmare"), DIFFICULTY_NIGHTMARE },
    { NULL, -1 },
};
ALIGNED(4) static const u8 gStarterPrompt[] = _("{CENTER_ALIGN}Select your character.");
ALIGNED(4) static const u8 gStarterNickPrompt[] = STARTER_NICK_PROMPT;

ALIGNED(4) static const u8 sGender0[] = GENDER_Q;
static const MenuItem gGenderMenu[] = {
    { GENDER_A0, 0 },
    { GENDER_A1, 1 },
    { NULL, -1 },
};

ALIGNED(4) static const u8 gStarterReveal[] = STARTER_REVEAL;
ALIGNED(4) static const u8 gPartnerPrompt[] = PARTNER_PROMPT;
ALIGNED(4) static const u8 gPartnerNickPrompt[] = PARTNER_NICK_PROMPT;
ALIGNED(4) static const u8 gTeamNamePrompt[] = TEAM_NAME_PROMPT;
ALIGNED(4) static const u8 gEndIntroText[] = END_TEXT;

static const WindowTemplate sUnknown_80F4244 = {
    0,
    5,
    12, 6,
    5, 5,
    5, 0,
    NULL
};
