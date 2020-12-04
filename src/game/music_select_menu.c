#include "game/music_select_menu.h"
#include "sm64.h"
#include "text_strings.h"
#include "game/game_init.h"
#include "game/ingame_menu.h"
#include "game/print.h"
#include "game/segment2.h"
#include "game/music_select.h"
#include "include/course_table.h"
#include "area.h"
#include "audio/external.h"

// TODO
// Set no music

#define HALF_SCREEN_WIDTH SCREEN_WIDTH / 2
#define HALF_SCREEN_HEIGHT SCREEN_HEIGHT / 2

struct MenuState {
	s32 open;
	s32 horizontalScrollIndex;
	s32 horizontalScrollTimer;
	s32 horizontalScrollHoldTimer;
	s32 verticalScrollIndex;
	s32 verticalScrollTimer;
	s32 verticalScrollHoldTimer;
	enum Song courseMusicSelections[MUSIC_SELECT_COURSE_COUNT];
};

static struct MenuState menuState;

static u8 songNameTbl[][32] = {
    { TEXT_MENU_SONG_NO_MUSIC },
    { TEXT_MENU_SONG_TITLE_THEME },
    { TEXT_MENU_SONG_MAIN_THEME },
    { TEXT_MENU_SONG_INSIDE_CASTLE },
    { TEXT_MENU_SONG_DIRE_DIRE_DOCKS },
    { TEXT_MENU_SONG_LETHAL_LAVA_LAND },
    { TEXT_MENU_SONG_KOOPAS_THEME },
    { TEXT_MENU_SONG_SNOW_MOUNTAIN },
    { TEXT_MENU_SONG_SLIDER },
    { TEXT_MENU_SONG_HAUNTED_HOUSE },
    { TEXT_MENU_SONG_CAVE_DUNGEON },
    { TEXT_MENU_SONG_KOOPAS_ROAD },
    { TEXT_MENU_SONG_ULTIMATE_KOOPA },
    { TEXT_MENU_SONG_FILE_SELECT },
    { TEXT_MENU_SONG_STAGE_BOSS },
    { TEXT_MENU_SONG_MERRY_GO_ROUND },
    { TEXT_MENU_SONG_METALLIC_MARIO },
    { TEXT_MENU_SONG_POWERFUL_MARIO }
};

static void draw_menu_title(void) {
	const u8 str[32] = { TEXT_MUSIC_SELECT_MENU };

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);

    print_hud_lut_string(HUD_LUT_GLOBAL,
    	get_str_x_pos_from_center_scale(HALF_SCREEN_WIDTH, str, 12.0f), 81, str);

    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

static void draw_scroll_triangle(s16 x, s16 y, f32 angle) {
	create_dl_translation_matrix(MENU_MTX_PUSH, x, y, 0);
	create_dl_rotation_matrix(MENU_MTX_NOPUSH, angle, 0, 0, 1.0f);
	gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
	gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
	gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
}

static void draw_menu_box(s16 x, s16 y) {
	create_dl_translation_matrix(MENU_MTX_PUSH, x - 84, y, 0);
	create_dl_scale_matrix(MENU_MTX_NOPUSH, 1.3f, 0.8f, 1.0f);
	gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
	gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
	gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

	draw_scroll_triangle(x + 10, y + 4, 90.0f);
	draw_scroll_triangle(x - 6, y - 68, 270.0f);

	draw_scroll_triangle(x + 90, y - 38, 0.0f);
	draw_scroll_triangle(x - 90, y - 24, 180.0f);
}

static void draw_menu_course_text(s16 x, s16 y, s32 courseIndex) {
	if (courseIndex > COURSE_STAGES_MAX) {
		x -= 10.0f;
	}

	if (courseIndex == 0) {
		u8 courseName[] = { TEXT_MENU_CASTLE };
	    print_generic_string(get_str_x_pos_from_center(x, courseName, 10.0f), y - 20, courseName);
	} else {
	    void **courseNameTbl = segmented_to_virtual(seg2_course_name_table);
	    u8 *courseName = segmented_to_virtual(courseNameTbl[courseIndex - 1]);
	    print_generic_string(get_str_x_pos_from_center(x, courseName, 10.0f), y - 20, courseName);
	}
}

static void draw_menu_default_course_music_text(s16 x, s16 y, s32 courseIndex) {
    u8 defaultText[] = { TEXT_MUSIC_SELECT_DEFAULT };
    u8 *songText = songNameTbl[music_select_get_default_song(courseIndex)];

    print_generic_string(get_str_x_pos_from_center(x - 50, defaultText, 10.0f), y - 40, defaultText);
    print_generic_string(get_str_x_pos_from_center(x + 20, songText, 10.0f), y - 40, songText);
}

static void draw_menu_selected_course_music_text(s16 x, s16 y, s32 courseIndex, enum Song songId) {
    u8 selectedText[] = { TEXT_MUSIC_SELECT_SELECTED };
    u8 *songText = songNameTbl[songId];

    print_generic_string(get_str_x_pos_from_center(x - 50, selectedText, 10.0f), y - 60, selectedText);
    print_generic_string(get_str_x_pos_from_center(x + 20, songText, 10.0f), y - 60, songText);
}

static void draw_menu_text(s16 x, s16 y) {
    s32 courseIndex = menuState.verticalScrollIndex;
    enum Song songId = menuState.horizontalScrollIndex;

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);

	draw_menu_course_text(x, y, courseIndex);

    draw_menu_default_course_music_text(x, y, courseIndex);

    draw_menu_selected_course_music_text(x, y, courseIndex, songId);

    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

static void check_allowed_input(s32 *allowVerticalInput, s32 *allowHorizontalInput) {
	menuState.verticalScrollTimer--;
	if (menuState.verticalScrollTimer <= 0) {
		if (menuState.verticalScrollHoldTimer == 0) {
			menuState.verticalScrollHoldTimer++;
			menuState.verticalScrollTimer = 10;
		} else {
			menuState.verticalScrollTimer = 3;
		}

		*allowVerticalInput = TRUE;
	}

	menuState.horizontalScrollTimer--;
	if (menuState.horizontalScrollTimer <= 0) {
		if (menuState.horizontalScrollHoldTimer == 0) {
			menuState.horizontalScrollHoldTimer++;
			menuState.horizontalScrollTimer = 10;
		} else {
			menuState.horizontalScrollTimer = 3;
		}

		*allowHorizontalInput = TRUE;
	}
}

static void check_input(void) {
	s32 allowVerticalInput = FALSE;
	s32 allowHorizontalInput = FALSE;

	check_allowed_input(&allowVerticalInput, &allowHorizontalInput);

	if (ABS(gPlayer1Controller->rawStickY) > 60) {
		if (allowVerticalInput) {
			play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);

			if (gPlayer1Controller->rawStickY >= 60) {
				s32 lastCourseIndex = menuState.verticalScrollIndex--;
				if (menuState.verticalScrollIndex < 0) {
					menuState.verticalScrollIndex = MUSIC_SELECT_COURSE_COUNT - 1;
				}

				menuState.courseMusicSelections[lastCourseIndex] = menuState.horizontalScrollIndex;
				menuState.horizontalScrollIndex = menuState.courseMusicSelections[menuState.verticalScrollIndex];
			} else {
				s32 lastCourseIndex = menuState.verticalScrollIndex++;
				if (menuState.verticalScrollIndex > MUSIC_SELECT_COURSE_COUNT - 1) {
					menuState.verticalScrollIndex = 0;
				}

				menuState.courseMusicSelections[lastCourseIndex] = menuState.horizontalScrollIndex;
				menuState.horizontalScrollIndex = menuState.courseMusicSelections[menuState.verticalScrollIndex];
			}
		}
	} else if (ABS(gPlayer1Controller->rawStickX) > 60) {
		if (allowHorizontalInput) {
			play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);

			if (gPlayer1Controller->rawStickX >= 60) {
				menuState.horizontalScrollIndex++;
				if (menuState.horizontalScrollIndex > SONG_MAX) {
					menuState.horizontalScrollIndex = 1;
				}
			} else {
				menuState.horizontalScrollIndex--;
				if (menuState.horizontalScrollIndex < 1) {
					menuState.horizontalScrollIndex = SONG_MAX;
				}
			}
		}
	} else {
		menuState.verticalScrollTimer = 0;
		menuState.verticalScrollHoldTimer = 0;
		menuState.horizontalScrollTimer = 0;
		menuState.horizontalScrollHoldTimer = 0;
	}
}

static void menu_on_open(void) {
	s32 i;
	for (i = 0; i < MUSIC_SELECT_COURSE_COUNT; i++) {
		menuState.courseMusicSelections[i] = music_select_get_song(i);
	}

	menuState.horizontalScrollIndex = music_select_get_song(gCurrCourseNum);
	menuState.horizontalScrollTimer = 0;
	menuState.horizontalScrollHoldTimer = 0;
	menuState.verticalScrollIndex = gCurrCourseNum;
	menuState.verticalScrollTimer = 0;
	menuState.verticalScrollHoldTimer = 0;
	menuState.open = TRUE;
}

static void menu_on_close(void) {
	menuState.courseMusicSelections[menuState.verticalScrollIndex] = menuState.horizontalScrollIndex;

	s32 i;
	for (i = 0; i < MUSIC_SELECT_COURSE_COUNT; i++) {
		music_select_set_song(i, menuState.courseMusicSelections[i]);
	}

	menuState.open = FALSE;
}

s32 music_select_menu_is_open(void) {
	return menuState.open;
}

void music_select_menu_toggle(void) {
	if (!menuState.open) {
		menu_on_open();
	} else {
		menu_on_close();
	}
}

void music_select_menu_update_and_draw(void) {
	check_input();

	s16 x = HALF_SCREEN_WIDTH;
	s16 y = HALF_SCREEN_HEIGHT;
	draw_menu_title();
	draw_menu_box(x, y);
	draw_menu_text(x, y);
}