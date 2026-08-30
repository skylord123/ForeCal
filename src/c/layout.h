#pragma once

#include <pebble.h>

// Per-display layout constants.
//
// ForeCal originally carried two hand-placed layouts inline via PBL_IF_RECT_ELSE():
// one for the 144x168 rectangular watches and one for 180x180 chalk. The newer
// PebbleOS platforms add two more display sizes, so the frames live here instead of
// being spelled out at each call site.
//
//   144x168 rect   aplite, basalt, diorite, flint
//   200x228 rect   emery    (Pebble Time 2)
//   180x180 round  chalk
//   260x260 round  gabbro
//
// The emery and gabbro blocks are the 144x168 / 180x180 designs scaled by the display
// ratio (200/144 and 228/168 for emery, 260/180 for gabbro) and then nudged so text
// sits on the calendar bands correctly. Keeping every frame as a compile-time literal
// means only one branch is ever compiled in, and the two original layouts keep their
// exact original numbers.
//
// Frames are expressed against PBL_DISPLAY_WIDTH/HEIGHT rather than a runtime
// layer_get_bounds() so they stay usable as constant initialisers.

#if defined(PBL_PLATFORM_EMERY)

// ---------------------------------------------------------------- emery, 200x228

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, 79)
#define LAYOUT_CLOCK_FRAME          GRect(-1, -18, 176, 70)
#define LAYOUT_PM_FRAME             GRect(171, 32, 28, 20)
#define LAYOUT_WEEK_FRAME           GRect(169, 32, 32, 20)
#define LAYOUT_DATE_FRAME           GRect(75, 45, 124, 32)
#define LAYOUT_DATE_ALIGN           GTextAlignmentRight
#define LAYOUT_BT_FRAME             GRect(176, 0, 17, 27)
#define LAYOUT_BATT_FRAME           GRect(173, 27, 24, 12)
#define LAYOUT_CURR_TEMP_FRAME      GRect(0, 45, 63, 32)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentLeft
#define LAYOUT_WIND_FRAME           GRect(63, 46, 63, 30)
#define LAYOUT_WIND_ALIGN           GTextAlignmentCenter

#define LAYOUT_FORECAST_FRAME       GRect(0, 78, PBL_DISPLAY_WIDTH, 88)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -5, PBL_DISPLAY_WIDTH, 22)
#define LAYOUT_STEPS_FRAME          GRect(0, -5, PBL_DISPLAY_WIDTH, 22)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -5, 89, 22)
#define LAYOUT_STATUS_FRAME         GRect(83, -5, 117, 22)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(1, 8, 14, 32)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(13, 8, 63, 32)
#define LAYOUT_LOW_LABEL_FRAME      GRect(1, 31, 14, 32)
#define LAYOUT_LOW_TEMP_FRAME       GRect(13, 31, 63, 32)
#define LAYOUT_SUN_TEXT_FRAME       GRect(138, 35, 62, 24)
#define LAYOUT_ICON_FRAME           GRect(90, 20, 48, 48)
#define LAYOUT_SUN_ICON_FRAME       GRect(159, 23, 30, 21)
#define LAYOUT_CONDITION_FRAME      GRect(0, 58, PBL_DISPLAY_WIDTH, 30)

#define LAYOUT_CAL_FRAME            GRect(0, 166, PBL_DISPLAY_WIDTH, 62)

// Calendar cell geometry. Cells are laid out at (d * LAYOUT_CAL_CELL_PITCH).
#define LAYOUT_CAL_CELL_PITCH       29
#define LAYOUT_CAL_CELL_W           27
#define LAYOUT_CAL_HDR_Y            (-5)
#define LAYOUT_CAL_TEXT_H           19
#define LAYOUT_CAL_HL_DY            5
#define LAYOUT_CAL_HL_H             15
#define LAYOUT_CAL_BAND1_Y          15
#define LAYOUT_CAL_BAND1_H          15
#define LAYOUT_CAL_BAND2_Y          45
#define LAYOUT_CAL_BAND2_H          18
#define LAYOUT_CAL_ROW1_Y           9
#define LAYOUT_CAL_ROW2_Y           24
#define LAYOUT_CAL_ROW3_Y           39

// The stock clock font tops out at 49px, which is lost on a 228px-tall display, so
// emery and gabbro load a scaled Roboto Bold digit subset instead. Defined to the
// resource id rather than to 1 so ForeCal.c can #ifdef on it and use it in one step.
// Each size is the 49px stock font scaled by that display's ratio (49 * 228/168 here,
// 49 * 260/180 for gabbro) so the clock keeps the same proportions as the small
// watches -- a shared compromise size left one display's clock crowding its date row.
#define LAYOUT_CLOCK_CUSTOM_FONT    RESOURCE_ID_FONT_CLOCK_66
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_18
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_28_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_28_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_18_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_18
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_28
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_24
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_24
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_18_BOLD

#elif defined(PBL_PLATFORM_GABBRO)

// --------------------------------------------------------------- gabbro, 260x260

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT)
#define LAYOUT_CLOCK_FRAME          GRect((PBL_DISPLAY_WIDTH - 182) / 2, 16, 182, 74)
#define LAYOUT_PM_FRAME             GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 182) / 2), 69, 29, 22)
#define LAYOUT_WEEK_FRAME           GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 173) / 2), 69, 35, 22)
#define LAYOUT_DATE_FRAME           GRect((PBL_DISPLAY_WIDTH - 129) / 2, 3, 129, 36)
#define LAYOUT_DATE_ALIGN           GTextAlignmentCenter
#define LAYOUT_BT_FRAME             GRect(225, 162, 17, 27)
#define LAYOUT_BATT_FRAME           GRect(12, 162, 24, 12)
#define LAYOUT_CURR_TEMP_FRAME      GRect((PBL_DISPLAY_WIDTH - 65) / 2, 216, 65, 36)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentCenter
#define LAYOUT_WIND_FRAME           GRect((PBL_DISPLAY_WIDTH - 87) / 2, 7, 65, 36)
#define LAYOUT_WIND_ALIGN           GTextAlignmentLeft

#define LAYOUT_FORECAST_FRAME       GRect(0, 91, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT - 91)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -6, PBL_DISPLAY_WIDTH, 25)
#define LAYOUT_STEPS_FRAME          GRect(0, -6, PBL_DISPLAY_WIDTH, 25)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -6, (PBL_DISPLAY_WIDTH / 2) - 29, 25)
#define LAYOUT_STATUS_FRAME         GRect((PBL_DISPLAY_WIDTH / 2) + 29, -6, (PBL_DISPLAY_WIDTH / 2) - 29, 25)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(3, 9, 14, 35)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(14, 9, 65, 35)
#define LAYOUT_LOW_LABEL_FRAME      GRect(176, 9, 14, 35)
#define LAYOUT_LOW_TEMP_FRAME       GRect(188, 9, 65, 35)
#define LAYOUT_SUN_TEXT_FRAME       GRect(144, 124, 68, 26)
#define LAYOUT_ICON_FRAME           GRect((PBL_DISPLAY_WIDTH - 50) / 2, 0, 50, 48)
#define LAYOUT_SUN_ICON_FRAME       GRect(72, 132, 30, 21)
#define LAYOUT_CONDITION_FRAME      GRect(0, 38, PBL_DISPLAY_WIDTH, 34)

#define LAYOUT_CAL_FRAME            GRect((PBL_DISPLAY_WIDTH - 180) / 2, 160, 180, 64)

#define LAYOUT_CAL_CELL_PITCH       26
#define LAYOUT_CAL_CELL_W           24
#define LAYOUT_CAL_HDR_Y            (-6)
#define LAYOUT_CAL_TEXT_H           20
#define LAYOUT_CAL_HL_DY            6
#define LAYOUT_CAL_HL_H             16
#define LAYOUT_CAL_BAND1_Y          16
#define LAYOUT_CAL_BAND1_H          16
#define LAYOUT_CAL_BAND2_Y          48
#define LAYOUT_CAL_BAND2_H          19
#define LAYOUT_CAL_ROW1_Y           10
#define LAYOUT_CAL_ROW2_Y           26
#define LAYOUT_CAL_ROW3_Y           42

#define LAYOUT_CLOCK_CUSTOM_FONT    RESOURCE_ID_FONT_CLOCK_70
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_18
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_28_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_28_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_18_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_18
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_28
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_24
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_24
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_18_BOLD

#elif defined(PBL_ROUND)

// ---------------------------------------------------------------- chalk, 180x180

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT)
#define LAYOUT_CLOCK_FRAME          GRect((PBL_DISPLAY_WIDTH - 126) / 2, 13, 126, 50)
#define LAYOUT_PM_FRAME             GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 126) / 2), 48, 20, 15)
#define LAYOUT_WEEK_FRAME           GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 120) / 2), 48, 24, 15)
#define LAYOUT_DATE_FRAME           GRect((PBL_DISPLAY_WIDTH - 89) / 2, 2, 89, 26)
#define LAYOUT_DATE_ALIGN           GTextAlignmentCenter
#define LAYOUT_BT_FRAME             GRect(156, 112, 11, 18)
#define LAYOUT_BATT_FRAME           GRect(8, 112, 16, 8)
#define LAYOUT_CURR_TEMP_FRAME      GRect((PBL_DISPLAY_WIDTH - 45) / 2, 150, 45, 26)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentCenter
#define LAYOUT_WIND_FRAME           GRect((PBL_DISPLAY_WIDTH - 60) / 2, 5, 45, 26)
#define LAYOUT_WIND_ALIGN           GTextAlignmentLeft

#define LAYOUT_FORECAST_FRAME       GRect(0, 63, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT - 63)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_STEPS_FRAME          GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -4, (PBL_DISPLAY_WIDTH / 2) - 20, 17)
#define LAYOUT_STATUS_FRAME         GRect((PBL_DISPLAY_WIDTH / 2) + 20, -4, (PBL_DISPLAY_WIDTH / 2) - 20, 17)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(2, 6, 10, 24)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(10, 6, 45, 24)
#define LAYOUT_LOW_LABEL_FRAME      GRect(122, 6, 10, 24)
#define LAYOUT_LOW_TEMP_FRAME       GRect(130, 6, 45, 24)
#define LAYOUT_SUN_TEXT_FRAME       GRect(100, 86, 47, 18)
#define LAYOUT_ICON_FRAME           GRect((PBL_DISPLAY_WIDTH - 34) / 2, 0, 34, 32)
#define LAYOUT_SUN_ICON_FRAME       GRect(50, 92, 20, 14)
#define LAYOUT_CONDITION_FRAME      GRect(0, 26, PBL_DISPLAY_WIDTH, 24)

#define LAYOUT_CAL_FRAME            GRect((PBL_DISPLAY_WIDTH - 124) / 2, 111, 124, 44)

#define LAYOUT_CAL_CELL_PITCH       18
#define LAYOUT_CAL_CELL_W           16
#define LAYOUT_CAL_HDR_Y            (-4)
#define LAYOUT_CAL_TEXT_H           14
#define LAYOUT_CAL_HL_DY            4
#define LAYOUT_CAL_HL_H             11
#define LAYOUT_CAL_BAND1_Y          11
#define LAYOUT_CAL_BAND1_H          11
#define LAYOUT_CAL_BAND2_Y          33
#define LAYOUT_CAL_BAND2_H          13
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           18
#define LAYOUT_CAL_ROW3_Y           29

#define LAYOUT_CLOCK_FONT           FONT_KEY_ROBOTO_BOLD_SUBSET_49
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_14
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_14
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_14_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_14
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_18
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_14
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_14_BOLD

#else

// ------------------------------------- 144x168 rect: aplite, basalt, diorite, flint

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, 58)
#define LAYOUT_CLOCK_FRAME          GRect(-1, -13, 126, 50)
#define LAYOUT_PM_FRAME             GRect(123, 23, 20, 15)
#define LAYOUT_WEEK_FRAME           GRect(122, 23, 24, 15)
#define LAYOUT_DATE_FRAME           GRect(54, 30, 89, 26)
#define LAYOUT_DATE_ALIGN           GTextAlignmentRight
#define LAYOUT_BT_FRAME             GRect(128, 0, 11, 18)
#define LAYOUT_BATT_FRAME           GRect(126, 18, 16, 8)
#define LAYOUT_CURR_TEMP_FRAME      GRect(0, 30, 45, 26)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentLeft
#define LAYOUT_WIND_FRAME           GRect(45, 34, 45, 26)
#define LAYOUT_WIND_ALIGN           GTextAlignmentCenter

#define LAYOUT_FORECAST_FRAME       GRect(0, 57, PBL_DISPLAY_WIDTH, 64)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_STEPS_FRAME          GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -4, 64, 17)
#define LAYOUT_STATUS_FRAME         GRect(60, -4, 84, 17)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(1, 6, 10, 24)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(9, 6, 45, 24)
#define LAYOUT_LOW_LABEL_FRAME      GRect(1, 23, 10, 24)
#define LAYOUT_LOW_TEMP_FRAME       GRect(9, 23, 45, 24)
#define LAYOUT_SUN_TEXT_FRAME       GRect(101, 26, 47, 18)
#define LAYOUT_ICON_FRAME           GRect(66, 16, 32, 32)
#define LAYOUT_SUN_ICON_FRAME       GRect(115, 17, 20, 14)
#define LAYOUT_CONDITION_FRAME      GRect(0, 43, PBL_DISPLAY_WIDTH, 24)

#define LAYOUT_CAL_FRAME            GRect(0, 122, PBL_DISPLAY_WIDTH, 47)

#define LAYOUT_CAL_CELL_PITCH       21
#define LAYOUT_CAL_CELL_W           19
#define LAYOUT_CAL_HDR_Y            (-4)
#define LAYOUT_CAL_TEXT_H           14
#define LAYOUT_CAL_HL_DY            4
#define LAYOUT_CAL_HL_H             11
#define LAYOUT_CAL_BAND1_Y          11
#define LAYOUT_CAL_BAND1_H          11
#ifdef PBL_COLOR
#define LAYOUT_CAL_BAND2_Y          33
#define LAYOUT_CAL_BAND2_H          13
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           18
#define LAYOUT_CAL_ROW3_Y           29
#else
#define LAYOUT_CAL_BAND2_Y          35
#define LAYOUT_CAL_BAND2_H          11
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           19
#define LAYOUT_CAL_ROW3_Y           31
#endif

#define LAYOUT_CLOCK_FONT           FONT_KEY_ROBOTO_BOLD_SUBSET_49
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_14
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_14
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_14_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_14
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_18
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_14
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_14_BOLD

#endif

// The round layouts put the date across the top where the rect layouts put wind
// speed, so wind stays a rectangular-only element.
#ifdef PBL_ROUND
#define LAYOUT_SHOW_WIND_LAYER      0
#else
#define LAYOUT_SHOW_WIND_LAYER      1
#endif
