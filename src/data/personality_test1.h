#include "locale/personality_test1_usa.h"

ALIGNED(4) static const u8 gSeedModePrompt[] = _(
    "{CENTER_ALIGN}Choose how to set the dungeon seed.\n"
    "{CENTER_ALIGN}The seed randomizes tilesets and Pokemon."
);
static const MenuItem gSeedModeMenu[] = {
    { _("Random"), 0 },
    { _("Custom"), 1 },
    { NULL, -1 },
};

ALIGNED(4) static const u8 gSeedCustomPrompt[] = _("{CENTER_ALIGN}Enter the dungeon seed\n{CENTER_ALIGN}using the number pad.");
ALIGNED(4) static const u8 gRecruitAllPrompt[] = _(
    "{CENTER_ALIGN}Recruitment Settings:\n"
    "{CENTER_ALIGN}All Recruitable - Can recruit fully evolved\n"
    "{CENTER_ALIGN}No Recruitable - No in-dungeon recruits"
);
static const MenuItem gRecruitAllMenu[] = {
    { _("All Recruitable"), 0 },
    { _("No Recruitable"), 1 },
    { NULL, -1 },
};
ALIGNED(4) static const u8 gDifficultyPrompt[] = _(
    "{CENTER_ALIGN}Normal: Similar difficulty to the base game.\n"
    "{CENTER_ALIGN}Hard: Stronger enemies.\n"
    "{CENTER_ALIGN}Nightmare: Only the best will survive."
);
static const MenuItem gDifficultyMenu[] = {
    { _("Normal"), DIFFICULTY_NORMAL },
    { _("Hard"), DIFFICULTY_HARD },
    { _("Nightmare"), DIFFICULTY_NIGHTMARE },
    { NULL, -1 },
};
ALIGNED(4) static const u8 gDungeonCountPrompt[] = _(
    "{CENTER_ALIGN}How many dungeons?\n"
    "{CENTER_ALIGN}Choose the length of this run."
);
static const MenuItem gDungeonCountMenu[] = {
    { _("10"), MAX_DUNGEONS_10 },
    { _("15"), MAX_DUNGEONS_15 },
    { _("20"), MAX_DUNGEONS_20 },
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
ALIGNED(4) static const u8 gStarterItemPrompt[] = _(
    "{CENTER_ALIGN}Choose a starting item for your hero to hold."
);
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
