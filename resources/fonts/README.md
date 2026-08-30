# Fonts

`Roboto-Bold.ttf` — Roboto, © Google, licensed under the Apache License 2.0
(<https://www.apache.org/licenses/LICENSE-2.0>).

Only used on the larger displays (emery, gabbro), where the largest stock clock
font (`FONT_KEY_ROBOTO_BOLD_SUBSET_49`) is too small. Built as `FONT_CLOCK_66` (emery)
and `FONT_CLOCK_70` (gabbro) -- each the 49px stock size scaled by that display's
ratio -- and subset to `[0-9:]` so each costs a few KB rather than the whole face.
