# CSS Theme System

The Octave Editor supports importing custom themes via `.css` files. This system maps a simplified CSS syntax to ImGui style colors and properties.

## Quick Start

1. Create a `.css` file using the format described below
2. Open Edit > Preferences > Appearance > Theme
3. Click "Import CSS Theme..."
4. Select your `.css` file and give it a name
5. The theme appears in the dropdown and applies immediately

Imported themes are copied to the `Preferences/Themes/` directory and persist across sessions.

## CSS Format

### Variables (`:root`)

Define reusable color variables:

```css
:root {
    --bg: #2E3440;
    --accent: #81A1C1;
    --text: #ECEFF4;
}
```

Reference them with `var(--name)`.

### Style Properties (`style`)

Non-color layout properties:

```css
style {
    window-rounding: 4px;
    frame-rounding: 2px;
    frame-padding: 6px 4px;
    item-spacing: 8px 4px;
    scrollbar-size: 12px;
    tab-rounding: 2px;
}
```

All style properties are optional. Unspecified properties retain the Dark theme defaults.

**Available style properties:**
`alpha`, `disabled-alpha`, `window-padding`, `window-rounding`, `window-border-size`, `window-min-size`, `window-title-align`, `child-rounding`, `child-border-size`, `popup-rounding`, `popup-border-size`, `frame-padding`, `frame-rounding`, `frame-border-size`, `item-spacing`, `item-inner-spacing`, `cell-padding`, `indent-spacing`, `columns-min-spacing`, `scrollbar-size`, `scrollbar-rounding`, `grab-min-size`, `grab-rounding`, `tab-rounding`, `tab-border-size`, `button-text-align`, `selectable-text-align`

Values with two components (e.g., `6px 4px`) map to ImVec2. Single values are used for both components.

### Color Selectors

Color selectors map to ImGui color slots. Each selector represents an element group, and properties within map to specific ImGui colors.

```css
window { background: #2E3440; child-background: transparent; }
text { color: #ECEFF4; disabled: #4C566A; }
button { background: #81A1C1; }
button:hover { background: #88C0D0; }
button:active { background: #5E81AC; }
```

### Supported Color Formats

- `#RGB` - shorthand hex (e.g., `#FFF`)
- `#RRGGBB` - standard hex (e.g., `#2E3440`)
- `#RRGGBBAA` - hex with alpha (e.g., `#2E344080`)
- `rgb(r, g, b)` - RGB values 0-255 (e.g., `rgb(46, 52, 64)`)
- `rgba(r, g, b, a)` - RGBA, alpha 0.0-1.0 (e.g., `rgba(46, 52, 64, 0.5)`)
- `var(--name)` - reference a `:root` variable
- `transparent` - fully transparent (`rgba(0,0,0,0)`)

### Complete Selector Reference

| Selector | Property | ImGui Color |
|---|---|---|
| `window` | `background` | WindowBg |
| `window` | `child-background` | ChildBg |
| `popup` | `background` | PopupBg |
| `border` | `color` | Border |
| `border` | `shadow` | BorderShadow |
| `text` | `color` | Text |
| `text` | `disabled` | TextDisabled |
| `text` | `selected-background` | TextSelectedBg |
| `frame` | `background` | FrameBg |
| `frame:hover` | `background` | FrameBgHovered |
| `frame:active` | `background` | FrameBgActive |
| `title` | `background` | TitleBg |
| `title:active` | `background` | TitleBgActive |
| `title:collapsed` | `background` | TitleBgCollapsed |
| `menubar` | `background` | MenuBarBg |
| `scrollbar` | `background` | ScrollbarBg |
| `scrollbar` | `grab` | ScrollbarGrab |
| `scrollbar:hover` | `grab` | ScrollbarGrabHovered |
| `scrollbar:active` | `grab` | ScrollbarGrabActive |
| `button` | `background` | Button |
| `button:hover` | `background` | ButtonHovered |
| `button:active` | `background` | ButtonActive |
| `header` | `background` | Header |
| `header:hover` | `background` | HeaderHovered |
| `header:active` | `background` | HeaderActive |
| `separator` | `color` | Separator |
| `separator:hover` | `color` | SeparatorHovered |
| `separator:active` | `color` | SeparatorActive |
| `resize-grip` | `background` | ResizeGrip |
| `resize-grip:hover` | `background` | ResizeGripHovered |
| `resize-grip:active` | `background` | ResizeGripActive |
| `tab` | `color` | Dock tab label text |
| `tab` | `background` | Tab |
| `tab:hover` | `background` | TabHovered |
| `tab:active` | `background` | TabActive |
| `tab:unfocused` | `background` | TabUnfocused |
| `tab:unfocused-active` | `background` | TabUnfocusedActive |
| `checkbox` | `checkmark` | CheckMark |
| `slider` | `grab` | SliderGrab |
| `slider:active` | `grab` | SliderGrabActive |
| `plot` | `lines` | PlotLines |
| `plot:hover` | `lines` | PlotLinesHovered |
| `plot` | `histogram` | PlotHistogram |
| `plot:hover` | `histogram` | PlotHistogramHovered |
| `table` | `header-background` | TableHeaderBg |
| `table` | `border-strong` | TableBorderStrong |
| `table` | `border-light` | TableBorderLight |
| `table` | `row-background` | TableRowBg |
| `table` | `row-background-alt` | TableRowBgAlt |
| `drag-drop` | `target` | DragDropTarget |
| `nav` | `highlight` | NavHighlight |
| `nav` | `windowing-highlight` | NavWindowingHighlight |
| `nav` | `windowing-dim` | NavWindowingDimBg |
| `modal` | `dim-background` | ModalWindowDimBg |
| `panel-assets` | `background` | Assets panel background |
| `panel-scene` | `background` | Scene hierarchy background |
| `panel-properties` | `background` | Properties panel background |
| `panel-debug-log` | `background` | Debug Log panel background |

## Example: Nord Theme

```css
/* Nord-inspired theme for Octave Editor */

:root {
    --polar0: #2E3440;
    --polar1: #3B4252;
    --polar2: #434C5E;
    --polar3: #4C566A;
    --snow0: #D8DEE9;
    --snow1: #E5E9F0;
    --snow2: #ECEFF4;
    --frost0: #8FBCBB;
    --frost1: #88C0D0;
    --frost2: #81A1C1;
    --frost3: #5E81AC;
}

style {
    window-rounding: 4px;
    frame-rounding: 2px;
    frame-padding: 6px 4px;
    item-spacing: 8px 4px;
    scrollbar-size: 12px;
    tab-rounding: 2px;
    grab-rounding: 2px;
}

window { background: var(--polar0); child-background: transparent; }
popup { background: var(--polar1); }
text { color: var(--snow2); disabled: var(--polar3); selected-background: var(--frost3); }
border { color: var(--polar2); shadow: transparent; }

frame { background: var(--polar1); }
frame:hover { background: var(--polar2); }
frame:active { background: var(--frost2); }

title { background: var(--polar1); }
title:active { background: var(--polar2); }
title:collapsed { background: var(--polar1); }

menubar { background: var(--polar0); }

button { background: var(--frost2); }
button:hover { background: var(--frost1); }
button:active { background: var(--frost3); }

header { background: var(--polar1); }
header:hover { background: var(--polar2); }
header:active { background: var(--frost2); }

tab { color: var(--snow2); background: var(--polar1); }
tab:hover { background: var(--frost2); }
tab:active { background: var(--polar2); }
tab:unfocused { background: var(--polar0); }
tab:unfocused-active { background: var(--polar1); }

scrollbar { background: var(--polar0); grab: var(--polar3); }
scrollbar:hover { grab: #616E88; }
scrollbar:active { grab: var(--frost2); }

separator { color: var(--polar1); }
checkbox { checkmark: var(--frost0); }
slider { grab: var(--frost2); }
slider:active { grab: var(--frost1); }
```

## Example: Per-Panel Backgrounds

Each editor panel can have its own background color, independent of the global window background:

```css
/* Dark base with distinct panel tints */
window { background: #1E1E2E; }

panel-assets { background: #1E2030; }
panel-scene { background: #1E2E1E; }
panel-properties { background: #2E1E2E; }
panel-debug-log { background: #1E1E1E; }
```

Panel backgrounds are optional. When unset, panels fall back to the `window { background }` color.

## Example: Minimal Colors Only

Partial themes are supported. Only specified colors override the Dark base theme:

```css
/* Minimal red accent theme */
button { background: #C0392B; }
button:hover { background: #E74C3C; }
button:active { background: #922B21; }
header { background: #C0392B; }
header:hover { background: #E74C3C; }
header:active { background: #922B21; }
tab:active { background: #C0392B; }
checkbox { checkmark: #E74C3C; }
slider { grab: #C0392B; }
```

## Notes

- All colors and style properties are optional. Unspecified values use ImGui's default style values.
- Switching themes fully resets all style properties before applying, so there is no bleed between themes.
- CSS comments (`/* ... */` and `// ...`) are supported.
- The `style` block is a custom extension, not standard CSS.
- Theme files are stored in `AppData/Roaming/OctaveEditor/Preferences/Themes/` on Windows.
- Use the "Reimport" button to update a custom theme from a new or edited CSS file.
