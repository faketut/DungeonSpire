
# DungeonSpire: modular dungeon crawler game + 3 DLC

**DungeonSpire** is a terminal-based, single-player dungeon crawler implemented in C++. The game features an endless tower structure with procedurally generated floors, turn-based combat, and support for modular gameplay extensions such as a dynamic weather system and a quest engine. It was developed to demonstrate object-oriented design principles, memory-safe architecture, and feature extensibility.

![Gif](./Animation.gif)
---

## Features

- **Endless Tower Progression**: Procedural generation of rooms, potions, enemies, and loot on each floor.
- **Race-Based Playstyles**: Players choose a character race with unique base attributes and passive effects.
- **Turn-Based Mechanics**: Game state updates upon each valid player command, ensuring deterministic behavior.
- **Potion System**: Potions have hidden effects until first use; some classes gain special interactions.
- **Combat Engine**: Includes dynamic enemy AI, factional hostility logic, and damage calculations.
- **Gold & Loot System**: Diverse treasure types with faction-specific effects (e.g., Orcs collect half gold).
- **Dynamic Weather (Optional)**: Adds environmental effects that impact visibility and movement.
- **Quest System (Optional)**: Supports side quests with reward structures and objective tracking.

---

## Build & Run Instructions

### Prerequisites
- C++ compiler supporting C++11 or later
- `make` utility

### Compilation
```bash
git clone https://github.com/yourusername/DungeonSpire.git
cd DungeonSpire
make
```

### Running the Game
```bash
./cc3k -enableWeather -enableQuest -file <floorfilename>
```

### Command-Line Options
- `-enableWeather`: Enables the dynamic weather system
- `-enableQuest`: Enables the side quest system
- `-file <filename>`: Loads a specific floor layout from file

---

## Controls

| Command          | Description                                |
|------------------|--------------------------------------------|
| `no`, `so`, `ea`, `we` | Move north, south, east, west          |
| `ne`, `nw`, `se`, `sw` | Move diagonally                        |
| `u <dir>`        | Use potion in specified direction          |
| `a <dir>`        | Attack enemy in specified direction        |
| `r`              | Restart the game                           |
| `q`              | Quit the game                              |

---

## Development Highlights

- Fully modular class hierarchy with polymorphic behavior for items, effects, and characters
- Memory-safe architecture verified using Valgrind
- Supports DLC-style optional features through command-line toggles
- Designed for scalability and unit testing

---

## License

This project is licensed under the MIT License.

---

## Author

Developed independently. For inquiries or collaboration, please contact [@faketut].
