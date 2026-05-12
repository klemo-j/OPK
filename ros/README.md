# OPK Robot – Duel mód (ROS 2)

Dvaja hráči súťažia o zbieranie odpadkov na mape. Hráč s vyšším skóre po uplynutí času vyhráva.

---

## Požiadavky

- ROS 2 Humble (alebo novší)
- Python balíček `pynput` (`pip install pynput`)
- Zostavený workspace (viď nižšie)

---

## 1. Zostavenie

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/humble/setup.bash
colcon build --symlink-install
source install/setup.bash
```

---

## 2. Spustenie hry (duel)

### Terminál 1 – hlavný launch (mapa, roboty, lidar, hra, RViz)

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/humble/setup.bash && source install/setup.bash
ros2 launch robot duel_launch.py
```

### Terminál 2 – ovládanie oboch hráčov naraz

```bash
cd ~/Desktop/OPK/ros
source /opt/ros/humble/setup.bash && source install/setup.bash
python3 src/robot/ros/scripts/teleop_duel.py
```

> Terminál 2 musí mať fokus operačného systému, aby pynput zachytával klávesy.

---

## Ovládanie

| Akcia      | Hráč 1 (modré šípky) | Hráč 2 (fialové WASD) |
|------------|----------------------|------------------------|
| Dopredu    | ↑                    | W                      |
| Dozadu     | ↓                    | S                      |
| Doľava     | ←                    | A                      |
| Doprava    | →                    | D                      |
| Ukončiť   | ESC                  | ESC                    |

Viac klávesov naraz funguje súčasne.

---

## Pravidlá hry

- Každý robot zbiera odpadky (valce na mape) najazdením na ne.
- Odpadky sa ukladajú do batoha (kapacita 5).
- Batoh treba vyprázdniť na **stanici** (modrý kruh blízko štartu).
- Za každý odpadok odovzdaný na stanici: **+1 bod**.
- Hra trvá **60 sekúnd**.
- Po uplynutí času vyhráva hráč s vyšším skóre.

---

## RViz – čo vidíte

| Objekt              | Popis                                              |
|---------------------|----------------------------------------------------|
| **Map**             | Čiernobiela mapa prostredia                        |
| **Environment**     | Steny, prekážky, stanica (MarkerArray)             |
| **Wastes**          | Aktívne odpadky na mape (farebné valce)            |
| **Scoreboard**      | Skóre, zostatok času, výsledok (texty vľavo hore) |
| **P1 / P2 Pose**   | Aktuálna pozícia a smer hráčov (šípky)            |
| **P1 / P2 Path**   | Trasa, ktorú hráči prešli                          |
| **P1 / P2 Scan**   | Lidar – body na stenách okolo robota               |

---

## Reset hry bez reštartu

```bash
ros2 service call /game/reset std_srvs/srv/Trigger
```

---

## Štruktúra balíčkov

```
src/
├── robot/
│   ├── cpp/          # čistá C++ logika (Lidar, Environment, GameLogic, Robot)
│   └── ros/          # ROS 2 nody, launch, rviz konfig, skripty
└── robot_msgs/       # vlastné ROS správy (GameState.msg)
```
