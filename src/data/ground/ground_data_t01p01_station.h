// Pokémon Square scripts removed for roguelike mode.
// Provide an empty header so references to gGroundScript_gs1 resolve without dragging in the
// massive original script data (NPCs, events, etc.).

static const struct GroundLink sPokemonSquareLinks[] = {0};

const struct GroundScriptHeader gGroundScript_gs1 = {
    .nGroups = 0,
    .groups = NULL,
    .links = sPokemonSquareLinks,
};
