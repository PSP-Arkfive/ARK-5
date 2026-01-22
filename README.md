# ARK-5
Next Generation Custom Firmware for PSP and Vita's ePSP.

## Installation
- Download `FasterARK` from here: https://github.com/PSP-Arkfive/FasterARK
- If on `PSP`: extract `FasterARK_psp.zip` onto the Memory Stick.
- If on `Vita`: install `FasterARK_vita.vpk` via VitaShell. Requires latest version of `NoPspEmuDrm_mod`.
- Open `FasterARK` app.
- On PS Vita: you can install extra plugins such as right analog and ef0 support.
- On PSP: to make it a permanet CFW, use this: https://github.com/PSP-Arkfive/CustomIPL
- What? there's no `FasterARK` download? yeah that's because I'm not finished with it yet, give me a break.

## Building
- Requires the very `latest pspsdk` development build found here: https://github.com/pspdev/pspdev/releases/tag/latest
- Requires the latest `psp-cfw-sdk` package: `https://github.com/pspdev/psp-cfw-sdk/`

## Features

Main differences with ARK-4:
- Compiled with `latest PSPSDK`.
- Modular project for easier development (`Divide et Impera`).
- `XMBControl` is now a `Core` (mandatory) part of the CFW instead of an (optional) extra.
- `VSH Menu` has been deprecated in favor of `XMBControl`. *
- `Recovery Menu` has been deprecated in favor of `Vanilla Mode` and/or `Despertar del Cementerio`. **
- Code cleanup, refactor, restyling, fixes, improvements, etc.

<br>
<p>* `VSH Menu` can still be used via `L+R+Select` combo if it is still installed from previous ARK-4 installations.</p>
<p>** `Recovery Menu` can still be used via `R trigger during bootup` combo if it is still installed from previous ARK-4 installations.</p>
