/*
 * Character artwork for the boot animation, kept apart from the engine so it
 * can be swapped. Spectrum (Haunter) is a Pokemon character: this ASCII fan
 * drawing is fine for a personal build, but see docs/02_feasibility.md before
 * distributing it in a public release.
 *
 * Attributes: '.' empty, 'b' body, 'd' dark rim, 'e' eye, 'p' pupil,
 *             'm' mouth, 't' teeth, 'l' tongue.
 * The head is stored as its left half; the last column is the axis of symmetry.
 */
#ifndef AETHRA_SPECTRUM_SPRITE_H
#define AETHRA_SPECTRUM_SPRITE_H

#define SP_HALF 16
#define SP_W (SP_HALF * 2 - 1)
#define SP_H 20

static const char SP_HEAD[SP_H][SP_HALF + 1] = {
    "...............b",
    ".....d........db",
    ".....bd......dbb",
    "....dbb.....dbbb",
    "....dbbd...dbbbb",
    "...dbbbbddbbbbbb",
    "d.dbbbbbbbbbbbbb",
    "bddbbbbbbbbbbbbb",
    "dbbbeebbbbbbbbbb",
    ".dbbeeeeebbbbbbb",
    "..dbeeeeeeeebbbb",
    "..dbbeeppppeebbb",
    "...dbbbbeeeeebbb",
    "d.dbbbbbbbbbbbbb",
    "bddbmbbbbbbbbbbb",
    ".dbbmttttttttttt",
    "..dbbmmmmmmmmmmm",
    "...dbbmmmmmlllll",
    "....ddbbbmmlllll",
    "......d.ddbbllll",
};

#define SP_HAND_W 9
#define SP_HAND_H 6

static const char SP_HAND[SP_HAND_H][SP_HAND_W + 1] = {
    "d..d..d..",
    "bd.bd.bd.",
    "bb.bb.bb.",
    "bbbbbbbbd",
    "dbbbbbbd.",
    ".ddbbdd..",
};

#define SP_GRIN_W 15
#define SP_GRIN_H 5

static const char SP_GRIN[SP_GRIN_H][SP_GRIN_W + 1] = {
    ".ee.........ee.",
    ".eeee.....eeee.",
    "..eppe...eppe..",
    "t.............t",
    ".ttttttttttttt.",
};

#endif
