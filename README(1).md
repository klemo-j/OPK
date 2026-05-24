<p align="center">
  <img src="https://img.shields.io/badge/C++-17%2F20-blue?style=for-the-badge&logo=cplusplus" />
  <img src="https://img.shields.io/badge/ROS2-Jazzy-green?style=for-the-badge&logo=ros" />
  <img src="https://img.shields.io/badge/STU_FEI-OPK-red?style=for-the-badge" />
</p>

# 🎓 OPK – Príprava na skúšku

> **Objektovo orientované programovanie v kybernetike**
> STU FEI · B-OPK · 2026

Kompletný test pokrývajúci **8 prednášok** + coding style dokument.
Obsahuje **~100 otázok** rôznych typov s odpoveďami na samokontrolu.

> 💡 **Odporúčaný postup:** Riešte po sekciách bez nahliadnutia do odpovedí. Odpovede sú na konci dokumentu.

---

## 📋 Obsah

| Sekcia | Typ | Počet |
|--------|-----|-------|
| [🅰️ Multiple Choice](#-sekcia-a--multiple-choice) | 1 správna z 4 | 25 |
| [🅱️ Pravda / Nepravda](#%EF%B8%8F-sekcia-b--pravda--nepravda) | P alebo N | 20 |
| [✏️ Krátke odpovede](#%EF%B8%8F-sekcia-c--krátke-odpovede--definície) | Definície, výpočty | 25 |
| [💻 Čítanie kódu](#-sekcia-d--čítanie-a-porozumenie-kódu) | Čo vypíše? Kde je chyba? | 10 |
| [⚖️ Porovnania](#%EF%B8%8F-sekcia-e--vysvetli-rozdiel--porovnaj) | A vs B | 10 |
| [🏗️ Návrhové vzory](#%EF%B8%8F-sekcia-f--návrhové-vzory--praktické-scenáre) | Aký vzor použiť? | 10 |
| [✅ Odpovede](#-odpovede) | Všetky riešenia | – |
| [🚀 Tipy na skúšku](#-tipy-na-skúšku) | Quick recap | – |

---

## 🅰️ SEKCIA A – Multiple Choice

<details>
<summary><b>Zobraziť otázky A1 – A25</b></summary>

**A1.** Ktorý zo základných princípov C++ znamená „za to čo nepoužiješ, neplatíš"?
- a) RAII
- b) Zero-cost abstractions
- c) Deterministické správanie
- d) Move semantics

**A2.** Aké je správne poradie udalostí pri vytvorení objektu triedy `Derived : public Base`?
- a) Derived constructor → Base constructor → Base destructor → Derived destructor
- b) Base constructor → Derived constructor → Derived destructor → Base destructor
- c) Base constructor → Derived constructor → Base destructor → Derived destructor
- d) Derived constructor → Base constructor → Derived destructor → Base destructor

**A3.** Čo vykoná operátor `new` v C++?
- a) Iba alokuje pamäť (ako `malloc`)
- b) Iba zavolá konštruktor
- c) Alokuje pamäť aj zavolá konštruktor
- d) Vytvorí smart pointer

**A4.** Ktorý smart pointer **NEDÁ SA** kopírovať?
- a) `std::shared_ptr`
- b) `std::unique_ptr`
- c) `std::weak_ptr`
- d) Žiaden, všetky sa dajú kopírovať

**A5.** Aký je problém s nasledovným kódom?
```cpp
int* p = nullptr;
std::cout << *p;
```
- a) Memory leak
- b) Dangling pointer
- c) Null dereference (crash)
- d) Double delete

**A6.** Čo je výsledkom prevodu `template<typename T> T max(T a, T b)` kompilátorom keď zavoláme `max(3, 5)` a `max(3.1, 4.2)`?
- a) Vytvorí sa jedna univerzálna funkcia
- b) Vytvoria sa dve špecializované funkcie (`int max(int,int)` a `double max(double,double)`)
- c) Funkcie sa generujú za behu programu
- d) Vyhodí chybu kompilácie pretože typy nie sú jednoznačné

**A7.** Aká je časová zložitosť operácie `push_back` v `std::vector`?
- a) O(n)
- b) O(log n)
- c) Amortizované O(1)
- d) O(1) vždy

**A8.** Aký kontajner použiť pre rýchly lookup hodnoty podľa kľúča s **NAJRÝCHLEJŠÍM** prístupom?
- a) `std::map`
- b) `std::unordered_map`
- c) `std::list`
- d) `std::set`

**A9.** Ktorý prístup k testovaniu **NEPORUŠUJE** zapuzdrenie triedy?
- a) White box
- b) Black box
- c) Friend tester
- d) Reflection

**A10.** Čo robí `std::lock_guard` v deštruktore?
- a) Zavolá `lock()`
- b) Zavolá `unlock()`
- c) Vyvolá výnimku
- d) Notifikuje condition variable

**A11.** Pri konvenciách v OPK – ktorý zápis je správny pre **členskú premennú** triedy?
- a) `MySpeed`
- b) `mySpeed`
- c) `my_speed_`
- d) `MY_SPEED`

**A12.** Čo je `concept` v C++20?
- a) Šablónová špecializácia
- b) Spôsob definovania požiadaviek, ktoré musí typ spĺňať
- c) Synonymum pre triedu
- d) Forma virtuálneho dedenia

**A13.** Ktorý návrhový vzor patrí do kategórie **Behavioral**?
- a) Adapter
- b) Singleton
- c) Strategy
- d) Decorator

**A14.** Pri ktorej launch policy `std::async` sa úloha začne vykonávať až pri zavolaní `future.get()`?
- a) `std::launch::async`
- b) `std::launch::deferred`
- c) `std::launch::immediate`
- d) Default (async | deferred)

**A15.** Aký kontajner má vnútorne implementáciu **Red-Black Tree**?
- a) `std::unordered_map`
- b) `std::vector`
- c) `std::map`
- d) `std::deque`

**A16.** Čo robí `std::move(x)`?
- a) Fyzicky presunie objekt v pamäti
- b) Vytvorí rvalue referenciu na lvalue (umožní move semantics)
- c) Vymaže objekt
- d) Skopíruje objekt do novej lokácie

**A17.** Ktorá z týchto operácií je v `constexpr` funkcii v C++11 **POVOLENÁ**?
- a) Cykly
- b) Lokálne premenné
- c) Iba jeden `return` výraz
- d) Dynamická alokácia

**A18.** Pri použití smart pointerov pre polymorfizmus je **POVINNÝ**:
- a) Friend constructor
- b) Virtuálny deštruktor v base triede
- c) Const metóda
- d) Explicit konštruktor

**A19.** Aký je rozdiel medzi `std::execution::par` a `std::execution::par_unseq`?
- a) `par_unseq` je sekvenčné
- b) `par` používa SIMD, `par_unseq` nie
- c) `par_unseq` kombinuje paralelizáciu + SIMD operácie
- d) Sú totožné

**A20.** Aké je hlavné riziko **Singleton** návrhového vzoru?
- a) Pomalý výkon
- b) Globálny stav a porušenie testovateľnosti
- c) Nadmerná pamäťová náročnosť
- d) Nedá sa použiť s polymorfizmom

**A21.** Pri kompilačnom procese, ktorá fáza vytvára **AST (Abstract Syntax Tree)**?
- a) Preprocesor
- b) Lexikálna analýza
- c) Syntaktická analýza
- d) Linker

**A22.** Aký kontajner sa interne realizuje ako **adaptér nad `std::vector`**?
- a) `std::queue`
- b) `std::priority_queue`
- c) `std::stack`
- d) `std::deque`

**A23.** V OPK štruktúre ROS balíka – súbor `*_ros.cpp` má obsahovať:
- a) Iba `main()` funkciu
- b) Iba čistú logiku bez ROS
- c) ROS rozhranie (subscribery, publishery, parametre) bez logiky
- d) Definíciu vlastných správ

**A24.** Pri lvalue/rvalue referenciách v C++ – ktoré tvrdenie je správne?
- a) Lvalue má identitu a adresu, rvalue je dočasná hodnota
- b) Rvalue má identitu, lvalue je dočasná
- c) Sú totožné, len iný zápis
- d) Lvalue je len pre primitívne typy

**A25.** Ktorý návrhový vzor sa najčastejšie kombinuje s ostatnými (Strategy, State, Command)?
- a) Singleton
- b) Decorator
- c) Factory
- d) Observer

</details>

---

## 🅱️ SEKCIA B – Pravda / Nepravda

<details>
<summary><b>Zobraziť otázky B1 – B20</b></summary>

| # | Otázka | Odpoveď |
|---|--------|---------|
| **B1** | V C++ je `struct` a `class` skoro to isté – jediný rozdiel je default prístupový špecifikátor. | `[ P / N ]` |
| **B2** | Referencia môže byť `nullptr`. | `[ P / N ]` |
| **B3** | `std::shared_ptr` používa **atomické** operácie pre reference counting, preto je drahší ako `unique_ptr`. | `[ P / N ]` |
| **B4** | Vlákno (thread) má **vlastný stack a registre**, ale **zdieľa heap** s ostatnými vláknami procesu. | `[ P / N ]` |
| **B5** | `assert()` je dostatočný nástroj pre profesionálne unit testovanie. | `[ P / N ]` |
| **B6** | STL algoritmus nepozná konkrétny kontajner – pracuje len cez **iterátory**. | `[ P / N ]` |
| **B7** | `constexpr` funkcia musí byť vždy vyhodnotená v čase kompilácie. | `[ P / N ]` |
| **B8** | `weak_ptr` zvyšuje reference count `shared_ptr`-a. | `[ P / N ]` |
| **B9** | Pri použití `std::shared_ptr` ako parametra funkcie cez `const &` sa reference counter **nemení**. | `[ P / N ]` |
| **B10** | `std::map` ukladá prvky **zoradené podľa kľúča**. | `[ P / N ]` |
| **B11** | Šablóna (template) má **runtime overhead** v porovnaní s ručne napísaným kódom. | `[ P / N ]` |
| **B12** | Implementácia šablóny musí byť v hlavičkovom súbore (okrem prípadu explicit instantiation). | `[ P / N ]` |
| **B13** | `std::atomic` vždy nahradí potrebu `std::mutex`. | `[ P / N ]` |
| **B14** | Návrhový vzor **Strategy** nás zbavuje veľkých `if/else` blokov. | `[ P / N ]` |
| **B15** | Pri **Black-box** testovaní môže vzniknúť problém kruhovej logiky. | `[ P / N ]` |
| **B16** | **Static** lokálna premenná v metóde si zachová hodnotu medzi volaniami funkcie. | `[ P / N ]` |
| **B17** | **`private` členy** základnej triedy sú prístupné v podedenej triede. | `[ P / N ]` |
| **B18** | **Visiaca referencia** (dangling reference) je bezpečnejšia ako visiaci smerník. | `[ P / N ]` |
| **B19** | `std::jthread` (C++20) sa automaticky `join`-ne v deštruktore. | `[ P / N ]` |
| **B20** | Lambda v C++ je v skutočnosti **syntaktický cukor nad funktorom** – kompilátor z nej vygeneruje triedu s `operator()`. | `[ P / N ]` |

</details>

---

## ✏️ SEKCIA C – Krátke odpovede / Definície

<details>
<summary><b>Zobraziť otázky C1 – C25</b></summary>

**C1.** Definuj princíp **RAII**.

**C2.** Vymenuj **4 typy konštruktorov** v C++.

**C3.** Aký je rozdiel medzi `public`, `private` a `protected` špecifikátorom prístupu **pri dedení**?

**C4.** Čo je **polymorfizmus** a ako sa v C++ aktivuje pre dynamický typ?

**C5.** Vysvetli, čo je **inicializačný zoznam** konštruktora a prečo je preferovaný pred priradením v tele konštruktora.

**C6.** Vymenuj **4 typy životnosti pamäte** v C++.

**C7.** Aké sú **5 chýb / rizík** pri použití surových (raw) smerníkov?

**C8.** Vysvetli rozdiel medzi `const int*`, `int* const` a `const int* const`.

**C9.** Čo robí `std::make_shared<T>(...)` oproti `std::shared_ptr<T>(new T(...))`?

**C10.** Vymenuj **5 typov volateľných (callable) objektov** v C++.

**C11.** Akú časovú zložitosť má `find` v `std::vector` a v `std::set`?

**C12.** Čo je **race condition** a kedy nastáva?

**C13.** Vysvetli rozdiel medzi `std::launch::async` a `std::launch::deferred`.

**C14.** Vymenuj **7 vlastností**, ktoré by mali spĺňať kvalitné unit testy.

**C15.** Čo znamená **CUT** a **SUT** v terminológii unit testov?

**C16.** Akú **návratovú konvenciu** používame pri parametroch funkcií podľa OPK štýlu?

**C17.** Definuj **SFINAE**.

**C18.** Aký je rozdiel medzi `constexpr` a `consteval` (C++20)?

**C19.** Vysvetli, čo robí **`override`** v C++.

**C20.** Vymenuj **3 kategórie návrhových vzorov** a uveď príklad pre každú.

**C21.** Čo je **stack unwinding** a kedy nastáva?

**C22.** Vymenuj všetky **3 typy scope** v CMake a vysvetli, čo robia.

**C23.** Aký je rozdiel medzi **`add_executable`** a **`add_library`** v CMake?

**C24.** Čo je rozdiel medzi **statickou** a **dynamickou** knižnicou?

**C25.** Vymenuj **3 spôsoby paralelizácie** v C++.

</details>

---

## 💻 SEKCIA D – Čítanie a porozumenie kódu

<details>
<summary><b>Zobraziť otázky D1 – D10</b></summary>

**D1.** Čo vypíše tento kód?
```cpp
class Base {
public:
    Base()  { std::cout << "B "; }
    ~Base() { std::cout << "~B "; }
};
class Derived : public Base {
public:
    Derived()  { std::cout << "D "; }
    ~Derived() { std::cout << "~D "; }
};
int main() {
    Derived d;
    return 0;
}
```

**D2.** Skompiluje sa nasledovný kód? Prečo (nie)?
```cpp
class Robot {
public:
    int getSpeed() const { return speed_; }
    void setSpeed(int v) { speed_ = v; }
private:
    int speed_;
};
int main() {
    const Robot r;
    r.setSpeed(10);
    return 0;
}
```

**D3.** Čo je problém s týmto kódom?
```cpp
int* p;
{
    int x = 5;
    p = &x;
}
std::cout << *p;
```

**D4.** Aký bude výstup? *(Polymorfizmus)*
```cpp
class A {
public:
    void greet() { std::cout << "A"; }
};
class B : public A {
public:
    void greet() { std::cout << "B"; }
};
int main() {
    A* obj = new B();
    obj->greet();
    delete obj;
}
```

**D5.** A teraz čo vypíše tento upravený kód?
```cpp
class A {
public:
    virtual void greet() { std::cout << "A"; }
    virtual ~A() = default;
};
class B : public A {
public:
    void greet() override { std::cout << "B"; }
};
int main() {
    A* obj = new B();
    obj->greet();
    delete obj;
}
```

**D6.** Označ chyby v tomto kóde:
```cpp
int* p = new int[5];
p[0] = 10;
delete p;
```

**D7.** Čo je nesprávne pri tomto použití `shared_ptr`?
```cpp
int* raw = new int(5);
std::shared_ptr<int> p1(raw);
std::shared_ptr<int> p2(raw);
```

**D8.** Čo robí tento kód a aký bude výstup?
```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
auto it = std::find_if(v.begin(), v.end(),
    [](int x) { return x > 2; });
std::cout << *it;
```

**D9.** Identifikuj typ návrhového vzoru:
```cpp
class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }
    void log(const std::string& msg) { std::cout << msg; }
private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};
```

**D10.** Aký návrhový vzor demonštruje táto štruktúra?
```cpp
class Planner {
public:
    virtual Path plan(const State& s, const State& g) = 0;
    virtual ~Planner() = default;
};
class AStarPlanner : public Planner { /*...*/ };
class RRTPlanner : public Planner { /*...*/ };

class Robot {
    std::unique_ptr<Planner> planner_;
public:
    void setPlanner(std::unique_ptr<Planner> p) {
        planner_ = std::move(p);
    }
};
```

</details>

---

## ⚖️ SEKCIA E – Vysvetli rozdiel / Porovnaj

<details>
<summary><b>Zobraziť otázky E1 – E10</b></summary>

**E1.** Smerník vs Referencia – uveď minimálne 4 rozdiely.

**E2.** `std::vector` vs `std::list` – kedy zvoliť ktorý?

**E3.** `std::unique_ptr` vs `std::shared_ptr` – semantika vlastníctva, výkon, použitie.

**E4.** Strategy vs State – obidva používajú abstraktné rozhranie + viacero implementácií. V čom je rozdiel?

**E5.** Factory vs Builder – obidva sú „creational" patterny. Kedy zvoliť ktorý?

**E6.** `std::mutex` (lock_guard) vs `std::atomic` – kedy je ktoré vhodnejšie?

**E7.** White-box vs Black-box testovanie – výhody a nevýhody oboch.

**E8.** Statická vs Dynamická knižnica – výhody, nevýhody, kedy ktorú použiť.

**E9.** `std::async` vs `std::thread` – aké problémy `std::thread`-u rieši `std::async`?

**E10.** Šablónová špecializácia vs C++20 `concept` – ako sa líši filozofia obmedzovania typov?

</details>

---

## 🏗️ SEKCIA F – Návrhové vzory – praktické scenáre

<details>
<summary><b>Zobraziť scenáre F1 – F10</b></summary>

> Pre každý scenár urči **najvhodnejší návrhový vzor** a krátko zdôvodni.

**F1.** Robot dokáže plánovať trajektóriu pomocou rôznych algoritmov (A*, RRT, flood fill) a chceš vedieť za behu prepínať, ktorý sa použije.

**F2.** Lidar produkuje dáta a chceš, aby na ne reagovali tri komponenty: vizualizácia, mapping a obstacle avoidance. Pričom Lidar by nemal vedieť, kto sa naňho „pripája".

**F3.** Máš robotickú ruku, ktorá prechádza cez fázy: čaká, približuje sa, uchopuje, dvíha. Každá fáza má vlastné správanie a prechody do iných fáz.

**F4.** GUI má tlačidlá „Move", „Pick", „Stop". Chceš aby sa akcie dali ukladať do queue, undo/redo, logovať.

**F5.** Tvoja knižnica používa staré API lidaru, ktoré vracia `std::vector<float>`, ale chceš s ňou pracovať cez `pcl::PointCloud<pcl::PointXYZ>`.

**F6.** Musíš vytvoriť objekt `Robot` s mnohými voliteľnými parametrami (s/bez lidaru, s/bez kamery, rôzne thresholdy). Konštruktor s 10 argumentmi je neprehľadný.

**F7.** Máš pipeline na spracovanie point cloud: niekedy chceš len filtrovať, niekedy filtrovať + downsample-ovať, niekedy všetko vrátane outlier removal. Poradie a kombinácia má byť dynamická.

**F8.** Spustenie robota vyžaduje sekvenciu: inicializácia senzorov → kalibrácia → načítanie mapy → spustenie planneru. Chceš to schovať do jednoduchého `start()` volania.

**F9.** Chceš vytvárať rôzne typy lidarov (Velodyne, Ouster, simulovaný) na základe stringu z config súboru.

**F10.** V projekte existuje `Logger`, ku ktorému pristupuje veľa modulov. Nechceš mať 5 rôznych inštancií.

</details>

---

## ✅ ODPOVEDE

### Sekcia A – Multiple Choice

<details>
<summary><b>Zobraziť odpovede A1 – A25</b></summary>

| # | Odpoveď | Vysvetlenie |
|---|---------|-------------|
| A1 | **b)** | Zero-cost abstractions – C++ princíp, že abstrakcia nestojí runtime |
| A2 | **b)** | Base konštruktor sa volá ako prvý, deštruktory v opačnom poradí |
| A3 | **c)** | `new` alokuje aj volá konštruktor (oproti `malloc`) |
| A4 | **b)** | `unique_ptr` má semantiku jedného vlastníka, dá sa iba presunúť (`std::move`) |
| A5 | **c)** | Dereferencovanie `nullptr` = null dereference / crash |
| A6 | **b)** | Šablóna je generátor kódu – vytvorí špecializované funkcie pre každú kombináciu |
| A7 | **c)** | Amortizované O(1) – pri realokácii O(n), ale v priemere O(1) |
| A8 | **b)** | `unordered_map` má prístup O(1) (hash tabuľka), `map` má O(log n) |
| A9 | **b)** | Black box testuje len cez verejné rozhranie, nezasahuje do enkapsulácie |
| A10 | **b)** | RAII – deštruktor `lock_guard` volá `unlock()` |
| A11 | **c)** | OPK konvencia: členské premenné = `snake_case_` s podčiarknikom na konci |
| A12 | **b)** | Concept definuje požiadavky, ktoré musí typ spĺňať (C++20) |
| A13 | **c)** | Strategy je behavioral; Adapter/Decorator sú structural; Singleton je creational |
| A14 | **b)** | `deferred` = úloha sa spustí až pri `get()`/`wait()` |
| A15 | **c)** | `map` (aj `set`) sú implementované ako Red-Black Tree |
| A16 | **b)** | `std::move` len typovo konvertuje lvalue na rvalue referenciu |
| A17 | **c)** | V C++11 mala constexpr extrémne obmedzenia – iba jeden return |
| A18 | **b)** | Virtuálny deštruktor je povinný, inak sa nezavolá Derived destructor |
| A19 | **c)** | `par_unseq` = paralelne + SIMD; `par` len paralelne |
| A20 | **b)** | Globálny stav sťažuje testovanie a zvyšuje skryté závislosti |
| A21 | **c)** | AST vzniká pri syntaktickej analýze (parser) |
| A22 | **b)** | `priority_queue` = binárna halda nad `vector`. `queue` je nad `deque` |
| A23 | **c)** | OPK konvencia: `*_ros.cpp` má ROS rozhranie, **žiadna logika** |
| A24 | **a)** | Lvalue = má identitu a adresu, rvalue = dočasná |
| A25 | **c)** | Factory sa často kombinuje s ostatnými vzormi |

</details>

### Sekcia B – Pravda / Nepravda

<details>
<summary><b>Zobraziť odpovede B1 – B20</b></summary>

| # | Odpoveď | Vysvetlenie |
|---|---------|-------------|
| B1 | **P** | `struct` má default `public`, `class` default `private`, inak sú totožné |
| B2 | **N** | Referencia musí byť inicializovaná, nemôže byť `nullptr` |
| B3 | **P** | Atomické inkrement/dekrement refcount-u je drahší ako jednoduchý pointer |
| B4 | **P** | Vlákna zdieľajú dáta a heap, ale majú vlastný stack a registre |
| B5 | **N** | `assert` má veľa nevýhod: dá sa vypnúť (NDEBUG), zlá diagnostika, nemá pojem testu |
| B6 | **P** | STL algoritmy pracujú s rozsahmi `[first, last)` cez iterátory |
| B7 | **N** | `constexpr` MÔŽE byť vyhodnotená v compile-time; `consteval` MUSÍ byť |
| B8 | **N** | `weak_ptr` zvyšuje len weak count, nie reference count |
| B9 | **P** | `const &` nevytvára kópiu, takže refcount sa nemení |
| B10 | **P** | `map` interne Red-Black Tree → zoradenie podľa kľúča |
| B11 | **N** | Šablóny majú **zero runtime overhead** |
| B12 | **P** | Kompilátor potrebuje vidieť celú definíciu pri inštanciovaní |
| B13 | **N** | Pre zložené operácie (`if(x > 10) x = 0`) stále treba mutex |
| B14 | **P** | Strategy presúva logiku z if/else do polymorfných tried |
| B15 | **P** | Ak testujeme len cez public rozhranie, musíme niečomu dôverovať bez testu |
| B16 | **P** | `static` lokálna premenná inicializuje pri prvom volaní, žije do konca programu |
| B17 | **N** | `private` členy nie sú prístupné v Derived. `protected` áno |
| B18 | **N** | Visiaca referencia je rovnako nebezpečná |
| B19 | **P** | `jthread` v deštruktore automaticky volá `join()` |
| B20 | **P** | Kompilátor lambda preloží na anonymný funktor s `operator()` |

</details>

### Sekcia C – Krátke odpovede

<details>
<summary><b>Zobraziť odpovede C1 – C25</b></summary>

**C1.** **RAII** (Resource Acquisition Is Initialization) – zdroj sa získava v konštruktore a uvoľňuje v deštruktore. Vďaka deterministickému zničeniu sa pamäť/mutex/file handle vždy korektne uvoľní.

**C2.** Default `Robot()`, Parametrický `Robot(int v)`, Copy `Robot(const Robot& v)`, Move `Robot(Robot&& v)`

**C3.** `public` → zostane `public` v Derived; `protected` → prístupné v Derived ale nie zvonku; `private` → neprístupné v Derived

**C4.** Polymorfizmus = metódy v podedených triedach robia rôzne veci. Aktivuje sa kľúčovým slovom **`virtual`** v base triede; v podedenej sa používa **`override`**.

**C5.** Inicializačný zoznam (`Robot(int v) : speed_(v) {}`) inicializuje premenné priamo na požadovanú hodnotu (1 inštrukcia) namiesto default konštrukcia + priradenie (2 inštrukcie). Poradie musí zodpovedať poradiu deklarácie!

**C6.** Automatická (stack), Statická (globálne/static), Pamäť vlákna (thread_local), Dynamická (heap – new/delete)

**C7.** Memory leak, Dangling pointer, Double delete, Null dereference, Chýbajúca kontrola rozsahu

**C8.** `const int* p` – nemôžem meniť hodnotu; `int* const p` – nemôžem meniť kam ukazuje; `const int* const p` – nemôžem meniť nič

**C9.** `make_shared` alokuje objekt + control block v **jednej alokácii** (rýchlejšie, exception-safe)

**C10.** Function pointer, Funktor (`operator()`), `std::bind`, Lambda, `std::function`

**C11.** `vector` – O(n) lineárne; `set` – O(log n) (Red-Black Tree)

**C12.** Race condition vzniká keď viacero vlákien pristupuje k zdieľanej premennej **bez synchronizácie** a aspoň jedno ju mení.

**C13.** `async` – spustí nové vlákno okamžite; `deferred` – spustí sa až pri `get()`/`wait()`

**C14.** Opakovateľné, replikovateľné, správne, presné, spoľahlivé, hermetické, kalibrované

**C15.** CUT = Code Under Test; SUT = System Under Test

**C16.** Vstupné: `const T&`; Vstupno-výstupné: `T&`; Výstupné: `T*`; Základné typy (≤64-bit): hodnotou

**C17.** SFINAE (Substitution Failure Is Not An Error) – neúspešná substitúcia typu v šablóne nie je chyba, kompilátor vyradí danú šablónu z výberu.

**C18.** `constexpr` MÔŽE byť compile-time; `consteval` MUSÍ byť compile-time (inak chyba)

**C19.** `override` hovorí kompilátoru: táto metóda MUSÍ prepisovať virtual z base – ak nie, vyhoď chybu.

**C20.** Creational (Factory, Singleton), Structural (Adapter, Decorator), Behavioral (Strategy, Observer, State)

**C21.** Stack unwinding – pri výnimke program opúšťa funkcie, ničí lokálne objekty (volá deštruktory) a hľadá catch blok.

**C22.** PRIVATE – len vo vnútri targetu; PUBLIC – target + linkujúce targety; INTERFACE – len linkujúce targety

**C23.** `add_executable` = spustiteľný program (má `main()`); `add_library` = knižnica (nemá `main()`)

**C24.** Statická – vložená do exe, samostatný binár, treba rekompilovať. Dynamická – linkuje sa pri behu, zdieľaná, aktualizovateľná.

**C25.** Thread-level (std::thread), Task-level (std::async), Data-level (STL par, OpenMP)

</details>

### Sekcia D – Čítanie kódu

<details>
<summary><b>Zobraziť odpovede D1 – D10</b></summary>

**D1.** `B D ~D ~B` — Base konštruktor pred Derived; deštruktory v opačnom poradí.

**D2.** NESKOMPILUJE SA — `const Robot r` nemôže volať ne-const metódu `setSpeed()`.

**D3.** Dangling pointer — `x` zaniká po `}`, `p` ukazuje na uvoľnenú pamäť → undefined behavior.

**D4.** Výstup: `A` — bez `virtual` sa volá metóda podľa statického typu pointera (`A*`).

**D5.** Výstup: `B` — s `virtual` sa použije dynamický typ (polymorfizmus).

**D6.** `new[]` vyžaduje `delete[]`, nie `delete`. Správne: `delete[] p;`

**D7.** Dva `shared_ptr` z rovnakého raw pointera → dva control blocky → double delete.

**D8.** Výstup: `3` — `find_if` vráti iterátor na prvý prvok kde `x > 2`.

**D9.** **Singleton** — privátny konštruktor, statická inštancia, zakázané kopírovanie.

**D10.** **Strategy** — `Planner` rozhranie, implementácie, `Robot` (kontext) drží `unique_ptr<Planner>`.

</details>

### Sekcia E – Porovnania

<details>
<summary><b>Zobraziť odpovede E1 – E10</b></summary>

**E1. Smerník vs Referencia:**

| | Smerník | Referencia |
|--|---------|------------|
| Inicializácia | Môže byť nullptr | Musí byť hneď |
| Reassign | Áno | Nie |
| nullptr | Môže | Nemôže |
| Syntax | Treba `*p` | Bežná premenná |
| Vlastná adresa | Áno | Nie (alias) |

**E2.** `vector` = default (súvislá pamäť, O(1) prístup, cache friendly). `list` = zriedkavé (O(1) insert, O(n) prístup).

**E3.** `unique_ptr` = jeden vlastník, zero overhead, default voľba. `shared_ptr` = viacero vlastníkov, atomic refcount, drahší.

**E4.** Strategy = klient si volí algoritmus zvonku. State = objekt si mení stav sám zvnútra.

**E5.** Factory = „daj mi typ X" (prednastavený). Builder = „daj mi X s týmito vlastnosťami" (krok po kroku).

**E6.** Atomic = jednoduché operácie (priradenie, inkrement). Mutex = zložené kritické sekcie.

**E7.** White-box = priamy prístup k stavu, krehké testy. Black-box = odolné voči refactoringu, problém kruhovej logiky. BDD odporúčaný.

**E8.** Statická = samostatný binár, treba rekompilovať. Dynamická = zdieľaná, aktualizovateľná, runtime závislosť.

**E9.** `async` rieši: návratovú hodnotu (future), životný cyklus, thread explosion, synchronizáciu.

**E10.** SFINAE = skrýva podmienku v type traits. Concept = deklaratívne pomenuje požiadavky, čistejšie chyby.

</details>

### Sekcia F – Návrhové vzory

<details>
<summary><b>Zobraziť odpovede F1 – F10</b></summary>

| # | Vzor | Zdôvodnenie |
|---|------|-------------|
| F1 | **Strategy** | Rodina algoritmov, prepínanie za behu, `setPlanner()` |
| F2 | **Observer** | Jeden zdroj → mnoho odberateľov, loose coupling |
| F3 | **State** | Fázy s vlastným správaním, samy si prepínajú stav |
| F4 | **Command** | Akcia ako objekt, queue, undo/redo |
| F5 | **Adapter** | Prevod nekompatibilného rozhrania |
| F6 | **Builder** | Krok-po-kroku konštrukcia, fluent API |
| F7 | **Decorator** | Dynamické skladanie funkcionality, krabica v krabici |
| F8 | **Facade** | Jednoduché API nad zložitým subsystémom |
| F9 | **Factory** | Vytváranie objektov podľa stringu/typu |
| F10 | **Singleton** | Jedna inštancia, globálny prístup *(pozor – často zlý nápad!)* |

</details>

---

## 🚀 TIPY NA SKÚŠKU

### 1. OOP základ – musíš ovládať naspamäť
- **Konvencie**: `PascalCase` triedy, `camelCase()` metódy, `snake_case_` členské premenné, `UPPER_CASE` konštanty
- Konštruktory: default, parametrický, copy, move (+ inicializačný zoznam!)
- Dedenie + `virtual` + `override` (poradie konštruktorov a deštruktorov!)
- `public` / `private` / `protected` (aj v kontexte dedenia)

### 2. Pamäť a smart pointery
- 4 typy životnosti pamäte
- 5 typických chýb s raw pointermi
- `unique_ptr` (jeden vlastník), `shared_ptr` (refcount), `weak_ptr` (lock)
- Pri polymorfizme: **virtuálny deštruktor**!

### 3. Vlákna
- `mutex` (`lock_guard`) vs `atomic` (jednoduché ops)
- Race condition (čítaj-modifikuj-zapíš)
- `async` + `future` (návratová hodnota)
- `promise` + `future` (manuálne odovzdávanie hodnoty)

### 4. STL kontajnery – tabuľka prístupov

| Kontajner | Prístup | Insert | Vyhľadávanie |
|-----------|---------|--------|--------------|
| `vector` | O(1) | amort. O(1) | O(n) |
| `deque` | O(1) | O(1) | O(n) |
| `list` | O(n) | O(1) | O(n) |
| `set`/`map` | O(log n) | O(log n) | O(log n) |
| `unordered_*` | O(1)* | O(1)* | O(1)* |

### 5. Šablóny
- Generátor kódu (zero overhead)
- Implementácia v `.hpp` (nie `.cpp`)
- Variadic templates, default values, specialization, SFINAE, C++20 concepts

### 6. Návrhové vzory – „kedy ktorý"

| Vzor | Kedy |
|------|------|
| **Strategy** | prepínanie algoritmu |
| **Observer** | jeden zdroj → mnoho odberateľov |
| **State** | stav riadi správanie + prechody |
| **Command** | akcia ako objekt (queue, undo) |
| **Adapter** | prevod rozhrania |
| **Factory** | vytváranie objektov podľa typu |
| **Builder** | krok-po-kroku konfigurácia |
| **Decorator** | dynamické skladanie funkcionality |
| **Facade** | zjednodušené API nad subsystémom |
| **Singleton** | jedna inštancia *(pozor, často zlý nápad!)* |

### 7. CMake & testy
- `add_executable` vs `add_library` (static/shared)
- `PUBLIC` / `PRIVATE` / `INTERFACE` scope
- GoogleTest + `TEST(SuiteName, TestName)` + `EXPECT_EQ`
- Black-box + BDD prístup
- 7 vlastností kvalitných testov

---

<p align="center">
  <b>Veľa šťastia na skúške! 🍀</b>
</p>
