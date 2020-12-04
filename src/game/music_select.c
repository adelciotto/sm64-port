#include "music_select.h"
#include "course_table.h"
#include "level_table.h"
#include "game/sound_init.h"
#include "area.h"

static const enum SeqId songToSeq[SONG_COUNT] = {
	SEQ_NONE,
    SEQ_MENU_TITLE_SCREEN,
    SEQ_LEVEL_GRASS,
    SEQ_LEVEL_INSIDE_CASTLE,
    SEQ_LEVEL_WATER,
    SEQ_LEVEL_HOT,
    SEQ_LEVEL_BOSS_KOOPA,
    SEQ_LEVEL_SNOW,
    SEQ_LEVEL_SLIDE,
    SEQ_LEVEL_SPOOKY,
    SEQ_LEVEL_UNDERGROUND,
    SEQ_LEVEL_KOOPA_ROAD,
    SEQ_LEVEL_BOSS_KOOPA_FINAL,
    SEQ_MENU_FILE_SELECT,
    SEQ_EVENT_BOSS,
    SEQ_EVENT_MERRY_GO_ROUND,
    SEQ_EVENT_METAL_CAP,
    SEQ_EVENT_POWERUP
};

struct MusicParams {
	u16 presetId;
	enum Song songId;
};

#define MUSIC_PARAMS(presetId, songId) ((struct MusicParams){(presetId), (songId)})

static const struct MusicParams courseMusicParams[MUSIC_SELECT_COURSE_COUNT] = {
	MUSIC_PARAMS(0x0001, SONG_INSIDE_CASTLE),     // (0)  Course Hub (Castle Grounds)
	MUSIC_PARAMS(0x0000, SONG_MAIN_THEME),        // (1)  Bob Omb Battlefield
	MUSIC_PARAMS(0x0005, SONG_MAIN_THEME),        // (2)  Whomp's Fortress
	MUSIC_PARAMS(0x0003, SONG_DIRE_DIRE_DOCKS),   // (3)  Jolly Rodger's Bay
	MUSIC_PARAMS(0x0000, SONG_SNOW_MOUNTAIN),     // (4)  Cool Cool Mountain
	MUSIC_PARAMS(0x0006, SONG_HAUNTED_HOUSE),     // (5)  Big Boo's Haunt
	MUSIC_PARAMS(0x0004, SONG_CAVE_DUNGEON),      // (6)  Hazy Maze Cave
	MUSIC_PARAMS(0x0000, SONG_LETHAL_LAVA_LAND),  // (7)  Lethal Lava Land
	MUSIC_PARAMS(0x0000, SONG_LETHAL_LAVA_LAND),  // (8)  Shifting Sand Land
	MUSIC_PARAMS(0x0003, SONG_DIRE_DIRE_DOCKS),   // (9)  Dire Dire Docks
	MUSIC_PARAMS(0x0000, SONG_SNOW_MOUNTAIN),     // (10) Snowman's Land
	MUSIC_PARAMS(0x0003, SONG_CAVE_DUNGEON),      // (11) Wet Dry World
	MUSIC_PARAMS(0x0000, SONG_MAIN_THEME),        // (12) Tall Tall Mountain
	MUSIC_PARAMS(0x0000, SONG_MAIN_THEME),        // (13) Tiny Huge Island
	MUSIC_PARAMS(0x0001, SONG_SLIDER),            // (14) Tick Tock Clock
	MUSIC_PARAMS(0x0000, SONG_SLIDER),            // (15) Rainbow Ride
	MUSIC_PARAMS(0x0000, SONG_KOOPAS_ROAD),       // (16) Bowser in the Dark World
	MUSIC_PARAMS(0x0000, SONG_KOOPAS_ROAD),       // (17) Bowser in the Fire Sea
	MUSIC_PARAMS(0x0000, SONG_KOOPAS_ROAD),       // (18) Bowser in the Sky
	MUSIC_PARAMS(0x0001, SONG_SLIDER),            // (19) Princess's Secret Slide
	MUSIC_PARAMS(0x0004, SONG_CAVE_DUNGEON),      // (20) Cavern of the Metal Cap
	MUSIC_PARAMS(0x0000, SONG_SLIDER),   		  // (21) Tower of the Wing Cap
	MUSIC_PARAMS(0x0000, SONG_SLIDER),   		  // (22) Vanish Cap Under the Moat
	MUSIC_PARAMS(0x0000, SONG_SLIDER),   		  // (23) Winged Mario over the Rainbow
	MUSIC_PARAMS(0x0003, SONG_DIRE_DIRE_DOCKS)    // (24) Secret Aquarium
};

// TODO: This will be loaded from savefile
static enum Song courseMusicOverrides[COURSE_END];

void music_select_init(void) {
	/* Init all course music to defaults. */
	s32 i;
	for (i = 0; i < MUSIC_SELECT_COURSE_COUNT; i++) {
		courseMusicOverrides[i] = SONG_DEFAULT;
	}
}

s32 music_select_check(s32 courseNumber, s32 levelId, u16 currentSeqArgs, struct MusicSelection *out) {
	enum Song songId;
	s32 notInCastle = levelId != LEVEL_CASTLE &&
					  levelId != LEVEL_CASTLE_GROUNDS &&
					  levelId != LEVEL_CASTLE_COURTYARD;

	/* Don't override music if COURSE_NONE is set and we're not in the castle. */
	if (notInCastle && courseNumber == COURSE_NONE) {
		return FALSE;
	}

	/* Don't override music for the star selection screen. */
	if (currentSeqArgs == SEQ_MENU_STAR_SELECT) {
		return FALSE;
	}

	songId = courseMusicOverrides[courseNumber];

	if (songId == SONG_DEFAULT) {
		return FALSE;
	}

	out->presetId = courseMusicParams[courseNumber].presetId;
	out->songId = songId;
	out->seqId = songToSeq[songId];
	return TRUE;
}

void music_select_set_song(s32 courseNumber, enum Song songId) {
	if (songId >= SONG_MIN && songId <= SONG_MAX) {
		if (songId != courseMusicParams[courseNumber].songId) {
			courseMusicOverrides[courseNumber] = songId;
			return;
		}
	}

	courseMusicOverrides[courseNumber] = SONG_DEFAULT;
}

enum Song music_select_get_song(s32 courseNumber) {
	enum Song songId = courseMusicOverrides[courseNumber];

	if (songId == SONG_DEFAULT) {
		return courseMusicParams[courseNumber].songId;
	}

	return songId;
}

enum Song music_select_get_default_song(s32 courseNumber) {
	return courseMusicParams[courseNumber].songId;
}

void music_select_update_background_music(s32 courseNumber) {
	enum Song songId = courseMusicOverrides[courseNumber];
	struct MusicParams musicParams = courseMusicParams[courseNumber];

	if (songId != SONG_DEFAULT) {
	    set_background_music(musicParams.presetId, songToSeq[songId], 0);
	} else {
	    set_background_music(gAreas[gCurrAreaIndex].musicParam, gAreas[gCurrAreaIndex].musicParam2, 0);
	}
}
