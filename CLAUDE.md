# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**MVE** (Virtual Idol Studio and Concert Participant Interaction Project) is an Unreal Engine 5.6.1 C++ project that enables virtual idol concerts with real-time audience interaction. The project supports two distinct user roles:

- **Studio (Host)**: Creates and controls virtual idols, manages concerts
- **Audience (Client)**: Joins concerts, interacts with virtual idols and other participants

Key features include AI-generated character models (via Comfy UI), real-time motion capture from webcam, online multiplayer sessions via Steam, and chat interaction with AI-driven persona.

## Build & Development Commands

### Building the Project

```bash
# Open solution in Visual Studio
start MVE.sln

# Or open in Rider
rider64.exe MVE.sln
```

The project uses standard Unreal Engine 5.6.1 build workflow:
1. Right-click `MVE.uproject` → "Generate Visual Studio project files" if .sln is missing
2. Build from Visual Studio (Development Editor configuration)
3. Launch editor via Visual Studio debugger or by double-clicking `MVE.uproject`

### Common Commands

```bash
# Launch Unreal Editor
start MVE.uproject

# Package for Windows (from Unreal Editor)
# File → Package Project → Windows

# Clean intermediate files if build fails
rmdir /s /q Binaries Intermediate Saved
# Then regenerate project files
```

### PIE (Play In Editor) Testing

**PIE Setup for Multiplayer Testing:**
1. Open **Editor Preferences** → **Play** → **Multiplayer Options**
2. **Play Net Mode**: `Play As Listen Server`
3. **Number of Players**: `2` (or more)

**How MainLevel Works:**
- MainLevel is a **local menu** - each player has their own independent instance
- Even in PIE multiplayer mode, players don't interact in MainLevel
- When a player clicks "Studio" button, **only that player** moves to StageLevel via `ConsoleCommand("open StageLevel?listen")`
- Other players remain in MainLevel

**Testing Flow:**
```
PIE (Number of Players = 2):
  Player 0: MainLevel (local)
    → Clicks "Studio" → Moves to StageLevel (Listen Server)
  Player 1: MainLevel (local, stays here)
    → Clicks "Audience" → AudienceStation → Finds Player 0's session → Joins
```

## Architecture Overview

### Core Framework Pattern

The project uses a **role-based architecture** with dynamic PlayerController assignment:

**Entry Point (MainLevel - Local Menu):**
- `AMVE_PC_Master` - Handles main UI, login, mode selection
- `AMVE_GM_MainLevel` - Main menu GameMode
- **Important**: MainLevel is a local menu - each player has their own independent instance, no interaction between players

**Stage Level (Concert/Multiplayer):**
- `AMVE_GM_StageLevel` - Multiplayer map with single PlayerController
- `AMVE_PC_StageLevel` - Single PlayerController class for both roles:
  - **Host**: `HasAuthority() && IsLocalPlayerController()` → Shows StudioBroadcast UI
  - **Client**: `!HasAuthority() && IsLocalPlayerController()` → Shows AudienceConcertRoom UI

This enables entirely different UIs and gameplay experiences for host vs. audience using a single PlayerController class.

### Template System

The project includes a **template inheritance hierarchy** for reusable game variants:

**Base Classes** (`Source/MVE/Template/`):
- `AMVEGameMode` - Abstract GameMode base
- `AMVECharacter` - Abstract Character base (3rd person, camera, input)
- `AMVEPlayerController` - Abstract PlayerController base

**Three Variants** (each with GameMode, Character, PlayerController):

1. **Combat** (`Variant_Combat/`)
   - Combo attacks, charged attacks, damage system
   - Interfaces: `ICombatAttacker`, `ICombatDamageable`, `ICombatActivatable`
   - AI: State Trees, enemy controllers, ragdoll death

2. **Platforming** (`Variant_Platforming/`)
   - Double jump, wall jump, dash, coyote time
   - Physics-based advanced movement

3. **Side Scrolling** (`Variant_SideScrolling/`)
   - 2D/pseudo-3D movement
   - Pickups, moving platforms, jump pads
   - Interface: `ISideScrollingInteractable`

These variants are **templates for game modes**, not directly used in the main concert feature.

### Subsystem Architecture

The project uses **GameInstance subsystems** for lifecycle management:

1. **`UUIManagerSubsystem`** (`Source/MVE/UI/Manager/`)
   - Centralized screen management via `EUIScreen` enum
   - Screens: Main, Login, Register, ModeSelect, StudioBroadcast, AudienceStation, etc.
   - Widget caching, modal background system, popup stacking
   - DataTable-based widget registration (`UIClassesTableAsset`, `PopUpClassesTableAsset`)
   - Access via: `UUIManagerSubsystem::Get(WorldContext)`

2. **`UMVE_GIS_SessionManager`** (`Source/MVE/Framework/`)
   - Online multiplayer session management via Steam OnlineSubsystem
   - Operations: `CreateSession()`, `FindSessions()`, `JoinSession()`, `LeaveSession()`
   - Delegates: `OnSessionCreated`, `OnSessionsFound`, `OnSessionJoined`
   - Uses Base64 encoding for session metadata (`FRoomInfo`)

3. **`UGenAISenderReceiver`**
   - HTTP communication with external AI server
   - Asset generation and download (Mesh/GLB, Audio/WAV, Video/MP4, Image/PNG)
   - Delegates: `OnAssetGenerated`, `OnDownloadProgress`

### Key Data Structures

- **`FRoomInfo`** (`Source/MVE/Data/RoomInfo.h`) - Session/room metadata (ID, title, broadcast time, thumbnail, viewer count, live status)
- **`EUIScreen`** (`UIManagerSubsystem.h`) - Type-safe screen enum for navigation

### Delegate-Based Communication Pattern

The architecture relies heavily on multicast delegates for loose coupling:

```
SessionManager → (OnSessionsFound) → AudienceStationModel → (broadcast) → UI Widgets
```

Example flow:
1. UI calls `SessionManager->FindSessions()`
2. SessionManager broadcasts `OnSessionsFound` delegate
3. Model layer updates internal state
4. UI widgets bind to model delegates for reactivity

### Animation-Driven Gameplay

Uses **AnimNotify classes** for gameplay events:
- `AnimNotify_CheckCombo`, `AnimNotify_DoAttackTrace` (Combat)
- `AnimNotify_EndDash` (Platforming)

This decouples animation from input, allowing animations to trigger gameplay logic.

## Code Organization

```
Source/MVE/
├── Framework/           # Core framework (GameMode, PlayerController, SessionManager)
│   ├── Public/         # Headers for AMVE_PC_Master, AMVE_GM_*, AMVE_*_PC_StageLevel
│   └── Private/        # Implementation files
├── UI/
│   ├── Manager/        # UIManagerSubsystem
│   ├── Widget/
│   │   ├── Main/       # Login, Register, ModeSelect screens
│   │   ├── Studio/     # Host UI (concert management)
│   │   ├── Audience/   # Client UI (concert list, room info)
│   │   └── PopUp/      # Modal dialogs
├── Template/           # Reusable variant templates
│   ├── Base/          # AMVEGameMode, AMVECharacter, AMVEPlayerController
│   ├── Variant_Combat/
│   ├── Variant_Platforming/
│   └── Variant_SideScrolling/
├── Data/              # FRoomInfo, DataTable rows, ScreenTypes
└── MVE.Build.cs       # Module dependencies

Content/
├── Blueprints/
│   ├── Framework/     # BP_GM_StageLevel, BP_*_PlayerController
│   └── UI/           # Widget blueprints (WBP_*)
└── Maps/
    ├── MainLevel      # Entry point (login/mode select)
    └── StageLevel     # Concert level (multiplayer)
```

## Module Dependencies

The project requires these key modules (see `MVE.Build.cs`):

- **EnhancedInput** - Modern input system
- **UMG, Slate** - UI framework
- **AIModule, StateTreeModule, GameplayStateTreeModule** - AI behavior
- **glTFRuntime** - Runtime GLB asset loading
- **IKRig** - Animation retargeting for generated characters
- **HTTP, Json, JsonUtilities** - External AI server communication
- **ImageWrapper** - Image loading from HTTP
- **OnlineSubsystem, OnlineSubsystemSteam** - Multiplayer sessions

## Logging

Use the project's logging macros defined in `MVE.h`:

```cpp
#include "MVE.h"

// Log with function name and line number
PRINTLOG(TEXT("Value: %d"), SomeValue);

// Session-specific logging
SESSIONPRINTLOG(TEXT("Session created: %s"), *SessionName);

// Just print call location
PRINTINFO();
```

Log categories: `LogMVE`, `SessionLogMVE`

## Naming Conventions

**Prefixes used throughout the codebase:**

- `AMVE_` - Actor classes (e.g., `AMVE_PC_Master`, `AMVE_GM_StageLevel`)
- `UMVE_` - UObject classes (e.g., `UMVE_GIS_SessionManager`)
- `IMVE_` - Interface classes
- `FMVE_` - Struct/plain data types
- `EMVE_` - Enum types
- `WBP_` - Widget Blueprints (e.g., `WBP_STD_Concert`, `WBP_AUD_Station`)
- `BP_` - Actor Blueprints (e.g., `BP_GM_StageLevel`)

**Role prefixes:**
- `STU_` - Studio (Host) related (e.g., `AMVE_STU_PC_StageLevel`)
- `AUD_` - Audience (Client) related (e.g., `AMVE_AUD_PC_StageLevel`)

**Template variant prefixes:**
- `Combat`, `Platforming`, `SideScrolling` - Variant template classes

## Important Patterns

### Accessing Subsystems

```cpp
// UIManager
auto UIManager = UUIManagerSubsystem::Get(GetWorld());
UIManager->ShowScreen(EUIScreen::AudienceStation);

// SessionManager
auto SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>();
SessionManager->FindSessions();
```

### Binding to Session Events

```cpp
auto SessionManager = GetGameInstance()->GetSubsystem<UMVE_GIS_SessionManager>();
SessionManager->OnSessionsFound.AddUObject(this, &UMyClass::OnSessionListReceived);

void UMyClass::OnSessionListReceived(bool bSuccess, const TArray<FRoomInfo>& Rooms)
{
    // Handle session list
}
```

### Screen Navigation

```cpp
// Navigate to a new screen
UUIManagerSubsystem::Get(this)->ShowScreen(EUIScreen::StudioBroadcast);

// Go back to previous screen
UUIManagerSubsystem::Get(this)->GoBack();

// Show modal popup
auto Popup = UUIManagerSubsystem::Get(this)->ShowPopup(TEXT("ConfirmJoinPopup"), true);
```

## Platform Notes

- **Windows only** - Project is configured for Win64
- **Unreal Engine 5.6.1** - Do not use API deprecated after this version
- **Steam Integration** - OnlineSubsystemSteam must be configured for multiplayer testing
- **MetaHuman Support** - Multiple MetaHuman plugins enabled for character customization

## Special Considerations

### glTFRuntime Integration

The project uses the third-party `glTFRuntime` plugin for runtime GLB loading. Plugin source is in `Plugins/glTFRuntime/`. When working with dynamic character loading, reference this plugin's API.

### Include Path Configuration

The build script auto-adds all `Public/` folders to include paths:

```cpp
// Can include directly without full path
#include "MVE_PC_Master.h"  // Instead of "Framework/Public/MVE_PC_Master.h"
```

Manual include paths are also set for variant folders (Variant_Platforming, Variant_Combat, Variant_SideScrolling).

### Session Metadata Encoding

`SessionManager` uses **Base64 encoding** for `FRoomInfo` fields to avoid OnlineSubsystem string limitations. When reading session data, it's automatically decoded.

### Korean Language Support

The codebase contains Korean comments and the project supports Korean UI strings. The GEMINI.md file indicates the team prefers Korean language responses.
