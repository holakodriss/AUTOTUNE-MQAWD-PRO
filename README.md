# AUTOTUNE MQAWD PRO

Windows VST3 vocal-processor prototype built with JUCE 8 + CMake.

## What is included
- VST3 and Standalone plugin targets
- Dark plugin editor UI
- 4-band vocal EQ
- Chorus, reverb and saturation
- Output gain
- Basic pitch detector / pitch display
- Key, scale, retune and humanize controls prepared in the interface

## Important limitation
This is an early DSP prototype, not a finished commercial-quality Auto-Tune replacement. The current source does **not** yet perform real-time automatic pitch correction. The pitch-correction engine and formant-preserving processing still need to be implemented and tested.

## Build without installing JUCE on your PC (GitHub Actions)
The repository includes `.github/workflows/build-windows.yml`. It builds on a hosted Windows machine, so JUCE and Visual Studio do not need to be installed on your computer.

1. Create a GitHub repository and upload the contents of this project ZIP to it.
2. Open the repository's **Actions** tab.
3. Select **Build Windows VST3** and click **Run workflow** (or push to the `main` branch).
4. When the run finishes, open it and download the artifact named **AUTOTUNE-MQAWD-PRO-Windows-VST3**.
5. Extract the ZIP. Copy the `.vst3` folder into a VST3 plugin folder that FL Studio scans.
6. In FL Studio, open **Options → Manage plugins → Find installed plugins**.

GitHub may ask you to sign in and confirm that Actions are enabled for the repository. The build is not guaranteed to succeed until the source has been compiled and any compiler errors fixed.

## Local build (optional)
Requires Visual Studio 2022 with Desktop development with C++, CMake 3.22+, and internet access for JUCE download:

```powershell
cmake -B build -S . -A x64
cmake --build build --config Release
```
