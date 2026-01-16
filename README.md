# ARK-5
Next Generation Custom Firmware for PSP and Vita's ePSP.

## Installation
- Requires `ARK-4` to be previously installed.
- Copy `FLASH0.ARK` to `ARK_01234` savedata.
- If on PSP, run `ARK_Loader` to install the new flash0 files.

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
