#ifndef MUSIC_SELECT_H
#define MUSIC_SELECT_H

#include "types.h"
#include "seq_ids.h"

#define MUSIC_SELECT_COURSE_COUNT COURSE_END - 1

enum Song {
	SONG_DEFAULT,
    SONG_TITLE_THEME,
    SONG_MAIN_THEME,
    SONG_INSIDE_CASTLE,
    SONG_DIRE_DIRE_DOCKS,
    SONG_LETHAL_LAVA_LAND,
    SONG_KOOPAS_THEME,
    SONG_SNOW_MOUNTAIN,
    SONG_SLIDER,
    SONG_HAUNTED_HOUSE,
    SONG_CAVE_DUNGEON,
    SONG_KOOPAS_ROAD,
    SONG_ULTIMATE_KOOPA,
    SONG_FILE_SELECT,
    SONG_STAGE_BOSS,
    SONG_MERRY_GO_ROUND,
    SONG_METALLIC_MARIO,
    SONG_POWERFUL_MARIO,
	SONG_COUNT,
	SONG_MIN = SONG_DEFAULT + 1,
	SONG_MAX = SONG_COUNT - 1
};

struct MusicSelection {
	u16 presetId;
	enum Song songId;
	enum SeqId seqId;
};

void music_select_init(void);
s32 music_select_check(s32 courseNumber, s32 levelId, u16 currentSeqArgs, struct MusicSelection *out);
void music_select_set_song(s32 courseNumber, enum Song songId);
enum Song music_select_get_song(s32 courseNumber);
enum Song music_select_get_default_song(s32 courseNumber);
void music_select_update_background_music(s32 courseNumber);


#endif // MUSIC_SELECT_H