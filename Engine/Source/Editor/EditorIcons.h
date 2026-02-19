// =============================================================================
// EditorIcons.h — Generated from IcoMoon selection.json
// Font: OctaveIcons.ttf | 74 icons | Range: U+F000..U+F049
// =============================================================================
#pragma once

#include "imgui.h"

// ---------------------------------------------------------------------------
// Glyph range
// ---------------------------------------------------------------------------
static constexpr ImWchar kIconMin = 0xF000;
static constexpr ImWchar kIconMax = 0xF049;

static const ImWchar kIconRanges[] = { 0xF000, 0xF049, 0 };

// ---------------------------------------------------------------------------
// Icon definitions (sorted alphabetically by identifier)
// ---------------------------------------------------------------------------
#define ICON_BASIL_EYE_CLOSED                        u8"\uF045"
#define ICON_MATH                                       u8"\uF049"
#define ICON_MOVE                                       u8"\uF044"
#define ICON_PACKAGE                                    u8"\uF043"
#define ICON_ARROW_TO_BOTTOM                           u8"\uF042"
#define ICON_GRID                                      u8"\uF00C"
#define ICON_NO_ACCESS                        u8"\uF041"
#define ICON_ARROW_DOWN                           u8"\uF040"
#define ICON_ARROW_UP                             u8"\uF03F"
#define ICON_NO_ALT                               u8"\uF03E"
#define ICON_GAMEPAD                                     u8"\uF03D"
#define ICON_BOT                           u8"\uF03C"
#define ICON_KEY                           u8"\uF046"
#define ICON_CUBE                         u8"\uF03A"
#define ICON_SPARKLE                       u8"\uF03B"
#define ICON_CUBES                            u8"\uF039"
#define ICON_MATERIAL                 u8"\uF038"
#define ICON_BASELINE_HOURGLASS_BOTTOM                   u8"\uF037"
#define ICON_BASELINE_LINK                               u8"\uF036"
#define ICON_BASELINE_LIST                               u8"\uF048"
#define ICON_BASELINE_PEOPLE                             u8"\uF035"
#define ICON_BASELINE_SEARCH                             u8"\uF034"
#define ICON_BASELINE_SHARE                              u8"\uF033"
#define ICON_BASELINE_STOP                               u8"\uF032"
#define ICON_SHARP_EXTENSION                             u8"\uF031"
#define ICON_PARK_INNER_SHADOW_BOTTOM_LEFT       u8"\uF030"
#define ICON_HAMMER_SHARP                               u8"\uF02F"
#define ICON_CODE                                        u8"\uF02E"
#define ICON_VIDEO_CAMERA                         u8"\uF02D"
#define ICON_VIDEO_FILE                           u8"\uF02C"
#define ICON_PAINT_DROP                      u8"\uF02B"
#define ICON_ROTATE_3D                               u8"\uF02A"
#define ICON_SCALE_UP                                  u8"\uF029"
#define ICON_ARROW_LEFT                    u8"\uF028"
#define ICON_ARROW_RIGHT                   u8"\uF027"
#define ICON_AUDIO_FILE_SHARP              u8"\uF026"
#define ICON_BOMB                          u8"\uF025"
#define ICON_CHECK                         u8"\uF024"
#define ICON_CHECK_CIRCLE                  u8"\uF023"
#define ICON_FILE_COPY_SHARP               u8"\uF022"
#define ICON_FOLDER_SHARP                  u8"\uF021"
#define ICON_LIGHT_FAST_FORWARD            u8"\uF015"
#define ICON_LIGHT_FAST_FORWARD_1          u8"\uF014"
#define ICON_LOCK_OPEN_SHARP               u8"\uF020"
#define ICON_LOCK_SHARP                    u8"\uF01F"
#define ICON_MIC                           u8"\uF01E"
#define ICON_PAUSE                         u8"\uF01D"
#define ICON_PERSON                        u8"\uF01C"
#define ICON_ROCKET_LAUNCH                 u8"\uF01B"
#define ICON_SAVE_SHARP                    u8"\uF01A"
#define ICON_SETTINGS                      u8"\uF019"
#define ICON_STAR                          u8"\uF018"
#define ICON_VIEW_TIMELINE_SHARP           u8"\uF017"
#define ICON_WIDGET_MEDIUM                 u8"\uF016"
#define ICON_CONTENT_PASTE                              u8"\uF013"
#define ICON_EYE                                        u8"\uF012"
#define ICON_MAGIC                                      u8"\uF011"
#define ICON_PEOPLE_ADD                                 u8"\uF010"
#define ICON_PLAY                                       u8"\uF00F"
#define ICON_THUMBS_DOWN                                u8"\uF00E"
#define ICON_THUMBS_UP                                  u8"\uF00D"
#define ICON_WEBCAM                                     u8"\uF00B"
#define ICON_PAPER_FILL                            u8"\uF00A"
#define ICON_PAPER_PLANE_TILT_FILL                       u8"\uF009"
#define ICON_SPHERE                                      u8"\uF008"
#define ICON_NO                                     u8"\uF007"
#define ICON_AUDIO                       u8"\uF006"
#define ICON_LOG                           u8"\uF005"
#define ICON_PLUMP_WORLD_REMIX                   u8"\uF004"
#define ICON_SHARP_NEW_FILE_REMIX                u8"\uF003"
#define ICON_CHAIR_DIRECTOR                          u8"\uF002"
#define ICON_IMAGE                           u8"\uF001"
#define ICON_TABLE                           u8"\uF047"
#define ICON_TRASH                                u8"\uF000"

// ---------------------------------------------------------------------------
// Semantic aliases for common editor toolbar icons
// ---------------------------------------------------------------------------
#define ICON_SCENE                  ICON_PLUMP_WORLD_REMIX          // World / Scene
#define ICON_STATIC_MESH            ICON_CUBE                // Single cube
#define ICON_INSTANCE_MESH          ICON_CUBES                   // Multiple cubes
#define ICON_TIMELINE               ICON_VIEW_TIMELINE_SHARP  // Timeline

// ---------------------------------------------------------------------------
// Helper: merge OctaveIcons.ttf into an existing ImGui font atlas
// ---------------------------------------------------------------------------
inline void MergeOctaveIcons(ImFontAtlas* atlas, float pixelSize, const char* ttfPath)
{
    ImFontConfig cfg;
    cfg.MergeMode        = true;
    cfg.PixelSnapH       = true;
    cfg.GlyphMinAdvanceX = pixelSize;
    atlas->AddFontFromFileTTF(ttfPath, pixelSize, &cfg, kIconRanges);
}
