# Changelog

## v1.0 — 2024

### Initial Release

- System scanner: CPU, GPU, RAM, OS detection via WMI
- Core systems: Logger, Config, EventBus, String utilities
- Live monitor: CPU usage, RAM, temperature (4-tier fallback), FPS
- Game detector: Steam, Epic, GOG library scanning + running process detection
- Game database: 35 game profiles with fuzzy matching
- Optimizer: Process killer (Normal + Extreme mode), RAM freeing, CPU boost
- Restore engine: Full system restore after game closes
- UI: Dear ImGui + DirectX 11, Dashboard / Games / Monitor / Settings tabs
- Extreme Mode: Kill all non-essential apps, max CPU performance, fans ramp up
- Streamer Mode: Protects OBS, Streamlabs, Discord, camera apps
- Steam protocol: Launches via steam:// preserving all saved launch options
- Manual game add: File browser dialog for any launcher or exe
- System tray: Background mode + auto-detect games launched outside GameCore
- Single instance: Prevents duplicate processes
- Admin elevation: Full system access for memory and service optimization
