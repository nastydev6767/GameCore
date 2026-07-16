<div align="center">

# GameCore

**Free, open source gaming optimizer for Windows**

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11-blue)](https://github.com/nastydev6767/GameCore/releases)
[![Release](https://img.shields.io/github/v/release/nastydev6767/GameCore)](https://github.com/nastydev6767/GameCore/releases/latest)

**[Website](https://nastydev6767.github.io/GameCore) · [Download](https://github.com/nastydev6767/GameCore/releases/latest/download/GameCore_Setup.exe) · [Report Bug](https://github.com/nastydev6767/GameCore/issues)**

</div>

---

## What is GameCore?

GameCore is a free, open source gaming optimizer for Windows. Before you launch a game it:

- Closes background applications (browsers, sync apps, updaters)
- Frees RAM using a 3-step standby list clear (typically 1–3 GB)
- Sets your CPU to high performance mode
- Prioritizes the game process for maximum CPU and memory
- Monitors CPU, RAM, and temperature while you play
- Restores everything when you're done

No subscription. No data collection. No bloat.

## Installation

1. Go to the [website](https://nastydev6767.github.io/GameCore)
2. Click **Download GameCore**
3. Run `GameCore_Setup.exe` — it downloads and installs GameCore automatically
4. Launch GameCore from the Start Menu or Desktop shortcut
5. **Run as Administrator** for full optimization features

## Features

| Feature | Description |
|---|---|
| Smart process killer | Closes known bloat, never touches system files or stream software |
| RAM optimizer | 3-step standby clear frees 1–3 GB |
| Extreme Mode | Close everything, max fans, 100% CPU for the game |
| Streamer Mode | Protects OBS, Discord, and camera apps |
| Steam integration | Launches via `steam://` preserving your saved launch options |
| Live monitor | CPU, RAM, temperature, FPS in real time |
| Full restore | Everything returns to normal after gaming |
| System tray | Auto-optimizes games launched outside GameCore |
| Manual add | Add any game exe with a file browser dialog |

## Building from Source

### Requirements
- Windows 10 or 11
- Visual Studio 2022 with C++ Desktop workload
- CMake 3.21+

### Steps

```powershell
git clone https://github.com/nastydev6767/GameCore.git
cd GameCore
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\bin\Release\GameCore.exe
```

## Contributing

Pull requests are welcome. Please open an issue first to discuss major changes.

## License

[Apache 2.0](LICENSE) — free to use, modify, and distribute.
