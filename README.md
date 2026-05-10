# ScrapIt: Overload

**ScrapIt: Overload** is a Top-Down Action Roguelite built in Unreal Engine 5. It features a custom physics-based movement system where player power is physically represented by industrial scrap. As a gameplay programmer, my focus was on creating a robust, data-driven architecture that manages shifting physics states, modular weapon systems, and procedural world generation.

---

## Core Technical Implementation

### 1. Physics-Driven Mecha Locomotion
Rather than using the standard `CharacterMovementComponent`, I developed a custom movement solution directly on a **Static Mesh** to achieve a heavy mass feel.
* **Force-Based Movement:** Locomotion is calculated using forward/backward impulses and torque for steering.
* **Dynamic Handling:** Implemented a lateral friction/grip system to simulate mechanical inertia. A "Mass Tier System" scales these physics parameters in real-time based on scrap count, increasing linear damping and reducing acceleration as the mecha grows.
* **The Dash:** An impulse applied in the current velocity direction with a configurable duration window and cooldown.

### 2. The Magnet & Scrap Shield System
The core gameplay hook is managed through a specialized interaction layer:
* **Magnetic Pulse:** A radial attraction field that utilizes a speed penalty and increased linear damping while active. Scraps use a two-stage logic (Rise -> Seek) before being collected.
* **Procedural Armor:** Collected scrap acts as a health buffer. I implemented a "Damage Bleed" system where incoming hits consume scrap count proportionally; once depleted, damage is applied to the separate Core HP pool, accompanied by a **Hit Slow** (velocity reduction) for visceral feedback.

### 3. Modular Weapon & Socket Architecture
To support 4 independent weapon sockets (Front, Back, Left, Right), I built a decoupled weapon system:
* **Data-Driven Levels:** Using `UWeaponLevels` Data Assets, weapon stats (Damage, Fire Rate, Range) scale automatically based on the mecha's current Mass Tier.
* **Component-Based Logic:** * **Ranged (Nail Gun):** Features independent mesh rotation and fire-cone targeting logic.
    * **Melee (Screws):** Collision-based damage using a 60° forward cone overlap, implementing a custom knockback-slow on the player to simulate the physical impact of ramming.

### 4. AI & Shielding
Enemies are controlled via **Behavior Trees**.
* **SocketWitch AI:** A support-class enemy that projects a dynamic shield onto allies using **Niagara VFX**. 
* **Shield System:** A sophisticated damage-reduction layer that features warmup, ramp-up, and decay logic, with visual intensity driven by real-time C++ parameters.

### 5. Procedural Level Generation
The game features a multi-layered procedural pipeline:
* **Run Generation:** A graph-based algorithm generates level ranks with interconnected nodes. I implemented a **Safety Pass** to ensure every room is reachable.
* **Grid-Coordinate Mapping:** Standard levels generate 3 to N rooms on a coordinate grid (Start, Combat, Exit, Special).
* **Room Manager:** Dynamically handles cardinal-direction door sockets, obstacle randomization (40% spawn chance per slot), and player teleportation between room transitions.

### 6. Persistence & State Management
* **PersistentManager (Subsystem):** A `GameInstance` subsystem manages the full mecha state (scraps, health, weapons, tiers) across level loads, ensuring data survives the `ResetRun` logic.

---

## Technical Stack
* **Engine:** Unreal Engine 5
* **Primary Language:** C++
* **Secondary Language:** Blueprints (for UI, Feedback and Behavior Trees)
* **Systems:** AI Behavior Trees, Niagara VFX, Physics (Impulse/Torque)
* **Architecture:** Data Assets, GameInstance Subsystems, Modular Actor Components

---
*Developed by Rebecca Mastrostefano*
