# OPK Robot – Duel mód (ROS 2)

Simulácia dvoch robotov súťažiacich o zbieranie odpadkov na 2D mape.
Projekt je implementovaný v **ROS 2 Jazzy** na **Ubuntu 24.04 LTS**.

---

## Obsah

- [Prehľad zadania](#prehľad-zadania)
- [Architektúra](#architektúra)
- [Požiadavky](#požiadavky)
- [Zostavenie](#zostavenie)
- [Spustenie – lokálne (RViz)](#spustenie--lokálne-rviz)
- [Spustenie – webová aplikácia](#spustenie--webová-aplikácia)
- [Pravidlá hry](#pravidlá-hry)
- [Lidar benchmark](#lidar-benchmark)
- [Štruktúra projektu](#štruktúra-projektu)

---

## Prehľad zadania

Cieľom zadania je implementovať systém riadenia robotov v prostredí ROS 2
s dôrazom na:

- **Softvérový dizajn** – čistá C++ logika oddelená od ROS vrstvy, návrhové vzory (Factory Method pre odpadky, Dependency Injection pre prostredie)
- **Simulácia robota** – fyzikálna simulácia pohybu s akceleráciou, detekciou kolízií a správnou synchronizáciou vlákien
- **Lidar** – optimalizovaný DDA ray-casting algoritmus priamo v pixel-priestore (27× rýchlejší ako step-based, ~0.28 ms / sken)
- **Herná logika** – duel dvoch hráčov, zbieranie a triedenie odpadkov, slot-based kapacita batoha
- **Vizualizácia** – RViz s MarkerArray scoreboardom + webová aplikácia s live canvas vizualizáciou cez WebSocket

---

## Architektúra

```
┌─────────────────────────────────────────────────────────────┐
│                        ROS 2 Graf                           │
│                                                             │
│  /p1/robot/cmd_vel ──► robot_node (P1) ──► /p1/robot/pose  │
│  /p2/robot/cmd_vel ──► robot_node (P2) ──► /p2/robot/pose  │
│                                                             │
│  /p1/robot/pose ──► lidar_node (P1) ──► /p1/scan           │
│  /p2/robot/pose ──► lidar_node (P2) ──► /p2/scan           │
│                                                             │
│  /p1/robot/pose ─┐                                         │
│  /p2/robot/pose ─┴► game_node ──► /game/state              │
│                             └───► /game/waste_markers       │
│                             └───► /game/scoreboard          │
│                                                             │
│  map_node ──► /map  ──► RViz2                               │
└─────────────────────────────────────────────────────────────┘

Ovládanie:
  teleop_duel.py  →  /p1(p2)/robot/cmd_vel   (klávesnica)
  web_server.py   →  /p1(p2)/robot/cmd_vel   (prehliadač)
```

### ROS nody

| Node | Popis |
|------|-------|
| `robot_node` | Fyzikálna simulácia robota (pohyb, kolízie, TF) |
| `lidar_node` | DDA ray-casting, publikuje `sensor_msgs/LaserScan` |
| `map_node` | Načíta PNG mapu, publikuje `nav_msgs/OccupancyGrid` |
| `game_node` | Herná logika – zbieranie, skóre, časomieral, scoreboard |
| `web_bridge` | ROS ↔ WebSocket most pre webovú aplikáciu |

---

## Požiadavky

| Závislosť | Verzia |
|-----------|--------|
| Ubuntu | 24.04 LTS |
| ROS 2 | Jazzy |
| Python | 3.12 |
| OpenCV | ≥ 4 |
| yaml-cpp | ≥ 0.7 |
| pynput | aktuálna |
| Flask + Flask-SocketIO | aktuálna (iba pre web app) |

```bash
# Python balíčky
pip install pynput flask flask-socketio --break-system-packages
```

---

## Zostavenie

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

> Stačí zostaviť raz. Vďaka `--symlink-install` sa Python skripty a konfiguračné
> súbory preberú bez opätovného zostavenia pri každej zmene.

---

## Spustenie – lokálne (RViz)

Klasické spustenie cez terminál a RViz2 vizualizáciu.

### Terminál 1 – hlavný launch

Spustí mapu, oboch robotov, oba lidary, hernú logiku a RViz2:

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/jazzy/setup.bash && source install/setup.bash
ros2 launch robot duel_launch.py
```

### Terminál 2 – ovládanie klávesnicou

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/jazzy/setup.bash && source install/setup.bash
python3 src/robot/ros/scripts/teleop_duel.py
```

> Terminál 2 musí mať **fokus operačného systému** (klikni do okna terminálu),
> inak `pynput` nezachytáva klávesy.

### Ovládanie

| Akcia | P1 (modrý) | P2 (fialový) |
|-------|-----------|-------------|
| Dopredu | `↑` | `W` |
| Dozadu | `↓` | `S` |
| Doľava | `←` | `A` |
| Doprava | `→` | `D` |
| Ukončiť | `ESC` | `ESC` |

Viac klávesov naraz funguje súčasne.

### Čo vidíte v RViz

| Vrstva | Popis |
|--------|-------|
| **Map** | PNG mapa prostredia (čiernobiela) |
| **Environment** | Geometrické prekážky a stanica |
| **Wastes** | Aktívne odpadky – farebné valce (žltá/oranžová/zelená) |
| **Scoreboard** | Skóre, batoh, čas – text markery nad mapou |
| **P1 / P2 Pose** | Šípka aktuálnej polohy a smeru robota |
| **P1 / P2 Path** | Prejdená trasa |
| **P1 / P2 Scan** | Lidar – body dopadu lúčov na steny |

### Reset bez reštartu

```bash
ros2 service call /game/reset std_srvs/srv/Trigger
```

---

## Spustenie – webová aplikácia

Alternatíva k RViz – ovládanie a vizualizácia cez prehliadač na `http://localhost:5000`.

### Terminál 1 – hlavný launch (rovnaký ako vyššie)

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/jazzy/setup.bash && source install/setup.bash
ros2 launch robot duel_launch.py
```

### Terminál 2 – web server

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/jazzy/setup.bash && source install/setup.bash
python3 src/robot/ros/scripts/web_server.py
```

Otvorte prehliadač na **http://localhost:5000**

### Čo ponúka webová aplikácia

- **Canvas** s PNG mapou, farebnými odpadkami a smerovými šípkami robotov
- **Live skóre** a stav batoha pre oboch hráčov
- **Časomieru** a výsledok po skončení hry
- **Klávesnica** – rovnaké klávesy ako teleop (šípky = P1, WASD = P2), funguje priamo v prehliadači
- **On-screen gamepad** – klikateľné tlačidlá, funguje aj na mobile / tablete
- **Tlačidlo Reset** – reštart hry bez reštartovania nodov

> Web server sa pripojí k bežiacemu ROS grafu automaticky.
> RViz môže bežať súbežne alebo ho môžete zavrieť – hra funguje bez neho.

---

## Pravidlá hry

### Odpadky

Každý typ odpadku má fixnú veľkosť, zaberie iný počet miest v batohu a má inú bodovú hodnotu:

| Typ | Farba | Veľkosť | Miesta v batohu | Body |
|-----|-------|---------|-----------------|------|
| Papier | žltá | malý (r = 0.12 m) | 1 | 1 |
| Plast | oranžová | stredný (r = 0.18 m) | 2 | 2 |
| Sklo | zelená | veľký (r = 0.25 m) | 3 | 3 |

### Kapacita batoha

Batoh má celkovú kapacitu **5 miest**. Príklady kombináciách:

| Kombinácia | Sloty | Zmestí sa? |
|------------|-------|-----------|
| 5× papier | 5/5 | ✅ |
| 2× plast + 1× papier | 5/5 | ✅ |
| 1× sklo + 1× papier | 4/5 | ✅ |
| 2× sklo | 6/5 | ❌ |
| 1× sklo + 1× plast | 5/5 | ✅ |

Ak sa odpadok nezmestí, robot ho **preskočí a skúsi zbierať menší**.

### Postup hry

1. Robot najazdí na odpadok → automaticky ho zoberie (ak sa zmestí)
2. Naplnený batoh odovzdá na **stanici** (zelený kruh pri štarte)
3. Každý odovzdaný odpadok pridá body zodpovedajúce jeho typu
4. Po **60 sekundách** vyhráva hráč s vyšším skóre

---

## Lidar benchmark

Optimalizovaný DDA lidar vs. pôvodný step-based prístup:

```bash
cd ~/Desktop/OPK/ros
./build/zadanie1/lidar_benchmark
```

Výsledok na HP Victus (Intel Core i5):

```
=== LIDAR BENCHMARK (500 skenov, 360 lúčov, max_range=5.0 m) ===

  DDA (optimalizovaný):    0.28 ms / sken
  Step-based (originál):   7.73 ms / sken
  Zrychlenie:              27.4x

  [PASS]  DDA sken 0.28 ms < 50 ms (ciel)
```

**Prečo je DDA rýchlejší:**
- Kráča pixel po pixeli – žiadne duplicitné kontroly toho istého pixelu
- `worldToPixel` a trigonometria sa počítajú raz na začiatku lúča, nie per-krok
- Prekážky sú vopred zapečené do `occupancy_` gridu – žiadna iterácia cez zoznam objektov počas skenu

---

## Štruktúra projektu

```
ros/
├── src/
│   ├── robot/
│   │   ├── cpp/                         # Čistá C++ logika (bez ROS)
│   │   │   ├── include/
│   │   │   │   ├── environment/
│   │   │   │   │   ├── Environment.hpp      # Načítanie mapy, isOccupied, isCollision
│   │   │   │   │   ├── GameEnvironment.hpp  # Prekážky, stanica, occupancy grid
│   │   │   │   │   └── Lidar.hpp            # DDA ray-casting
│   │   │   │   ├── game/
│   │   │   │   │   └── GameLogic.hpp        # Herná logika (zber, skóre, čas)
│   │   │   │   ├── robot/
│   │   │   │   │   └── Robot.hpp            # Fyzikálna simulácia, mutex, vlákno
│   │   │   │   ├── types/
│   │   │   │   │   ├── Geometry.hpp         # Point2d, RobotState, Twist
│   │   │   │   │   └── GameObjects.hpp      # GameObject, CircleObstacle, Station…
│   │   │   │   └── waste/
│   │   │   │       ├── Waste.hpp            # Abstraktná trieda + PaperWaste atď.
│   │   │   │       └── WasteFactory.hpp     # Factory Method pattern
│   │   │   ├── src/                         # Implementácie
│   │   │   ├── config/
│   │   │   │   ├── environment.yaml         # Mapa, prekážky, herné nastavenia
│   │   │   │   └── resources/opk-map.png   # PNG mapa (1536×1024 px, 0.02 m/px)
│   │   │   └── Test/
│   │   │       └── lidar_benchmark.cpp      # DDA vs. step-based benchmark
│   │   │
│   │   └── ros/                             # ROS 2 wrapper
│   │       ├── src/
│   │       │   ├── robot_node.cpp           # RobotNode – simulácia + TF + topics
│   │       │   ├── lidar_node.cpp           # LidarNode – scan + LaserScan msg
│   │       │   ├── map_node.cpp             # MapNode – OccupancyGrid + markery
│   │       │   └── game_node.cpp            # GameNode – logika + scoreboard
│   │       ├── launch/
│   │       │   └── duel_launch.py           # Spúšťa všetky nody + RViz
│   │       ├── rviz/
│   │       │   └── duel.rviz                # RViz konfigurácia
│   │       └── scripts/
│   │           ├── teleop_duel.py           # Klávesnicové ovládanie (pynput)
│   │           └── web_server.py            # Flask + SocketIO webová aplikácia
│   │
│   └── robot_msgs/                          # Vlastné ROS správy
│       └── msg/GameState.msg                # Skóre, čas, kapacita pre oboch hráčov
│
└── README.md
```
