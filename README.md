# C++ a ROS2 — Sprievodca na skúšku OPK

> Kompletný študijný materiál pokrývajúci C++ kľúčové slová a ROS2 framework pre predmet **Objektovo-orientované programovanie a komponentové systémy**.

![C++](https://img.shields.io/badge/C%2B%2B-17%2F20-00599C?style=flat&logo=cplusplus&logoColor=white)
![ROS2](https://img.shields.io/badge/ROS2-Jazzy-22314E?style=flat&logo=ros&logoColor=white)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-E95420?style=flat&logo=ubuntu&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 📖 Obsah

- [Časť 1: C++ Kľúčové slová](#-časť-1-c-kľúčové-slová)
  - [Prístupové špecifikátory](#1-prístupové-špecifikátory)
  - [Konštruktory a špeciálne metódy](#2-konštruktory-a-špeciálne-metódy)
  - [Dedičnosť a polymorfizmus](#3-dedičnosť-a-polymorfizmus)
  - [Static (tri významy)](#4-static-tri-rôzne-významy)
  - [Const rodina](#5-const-rodina)
  - [Pamäť a smerníky](#6-pamäť-a-smerníky)
  - [Smart pointery](#7-smart-pointery)
  - [Templates a generické programovanie](#8-templates-a-generické-programovanie)
  - [Vlákna a synchronizácia](#9-vlákna-a-synchronizácia)
  - [Výnimky](#10-výnimky)
  - [Ostatné dôležité](#11-ostatné-dôležité)
- [Časť 2: ROS2](#-časť-2-ros2)
  - [Čo je ROS2?](#21-čo-je-ros2)
  - [Základné pojmy](#22-základné-pojmy)
  - [Štruktúra balíka](#23-štruktúra-ros2-balíka-podľa-opk)
  - [Konštruktor nodu](#24-konštruktor-ros2-nodu)
  - [Parametre](#25-načítanie-parametrov-opk-spôsob)
  - [Main súbor](#26-main-súbor-node)
  - [Launch súbor](#27-launch-súbor)
  - [Logovanie](#28-ros2-logovacie-makrá)
  - [Typické skúškové otázky](#29-typické-skúškové-otázky-s-ros2)
  - [CMakeLists.txt](#210-cmakeliststxt-pre-ros2-balík)
  - [Checklist](#211-rýchly-checklist--mám-všetko)

---

# 🔷 Časť 1: C++ Kľúčové slová

## 1. Prístupové špecifikátory

<details open>
<summary><b><code>public</code></b> — verejné rozhranie triedy</summary>

Čokoľvek označené ako `public` je prístupné odkiaľkoľvek — z `main()`, z inej triedy, odkiaľkoľvek. Je to verejné rozhranie triedy, cez ktoré komunikuje s okolím.

```cpp
class Robot
{
public:
    void move();  // ktokoľvek môže zavolať robot.move()
};
```
</details>

<details>
<summary><b><code>private</code></b> — zapuzdrenie</summary>

Čokoľvek označené ako `private` je prístupné **len zvnútra triedy** — ani podedená trieda sa k tomu nedostane. Slúži na **zapuzdrenie** — schovávame vnútorný stav pred vonkajším svetom, aby ho nikto nemohol nesprávne zmeniť.

```cpp
class Robot
{
private:
    int speed_;  // nikto zvonku nemôže napísať robot.speed_ = -100
};
```
</details>

<details>
<summary><b><code>protected</code></b> — prístupné pre potomkov</summary>

Správa sa ako `private`, ale s jednou výnimkou — **podedená trieda má prístup**. Používa sa keď chceš niečo schovať pred vonkajším svetom, ale zároveň to sprístupniť triedam, ktoré z teba dedia.

```cpp
class Robot
{
protected:
    int battery_;  // MobileRobot : public Robot môže čítať battery_
};

class MobileRobot : public Robot
{
    void check()
    {
        std::cout << battery_;  // OK, lebo protected
    }
};
```
</details>

<details>
<summary><b><code>friend</code></b> — kontrolované porušenie zapuzdrenia</summary>

Dáva **konkrétnej** externej funkcii alebo triede výnimočný prístup k `private` členom. Používa sa opatrne — hlavne pri testovaní (white-box testy). Porušuje zapuzdrenie, ale kontrolovane.

```cpp
class Robot
{
    friend void testRobot(Robot& r);  // táto funkcia vidí private
private:
    int speed_;
};

void testRobot(Robot& r)
{
    std::cout << r.speed_;  // OK vďaka friend
}
```
</details>

---

## 2. Konštruktory a špeciálne metódy

<details>
<summary><b><code>explicit</code></b> — zákaz implicitných konverzií</summary>

Zabraňuje kompilátoru robiť **implicitné konverzie** pri konštruktore s jedným parametrom. Bez neho by kompilátor mohol automaticky konvertovať `int` na `Robot`, čo je väčšinou nechcené a môže spôsobiť ťažko nájditeľné bugy.

```cpp
class Robot
{
public:
    explicit Robot(int speed);  // Robot r = 5; → CHYBA (dobre!)
                                // Robot r(5);  → OK
};
```
</details>

<details>
<summary><b><code>= default</code></b> — predvolená implementácia</summary>

Hovorí kompilátoru: *„Vygeneruj predvolenú implementáciu tejto metódy."* Používa sa pri konštruktoroch a deštruktoroch, keď nechceš písať prázdne telo, ale chceš byť explicitný, že metóda existuje.

```cpp
class Robot
{
public:
    Robot() = default;   // kompilátor vygeneruje prázdny konštruktor
    ~Robot() = default;  // kompilátor vygeneruje prázdny deštruktor
};
```
</details>

<details>
<summary><b><code>= delete</code></b> — zákaz metódy</summary>

Úplne **zakáže** použitie danej metódy. Ak sa ju niekto pokúsi zavolať, kompilátor vyhodí chybu. Typické použitie: zakázanie kopírovania (Singleton, `unique_ptr`).

```cpp
class Logger
{
public:
    Logger(const Logger&) = delete;             // ZAKÁŽ kopírovanie
    Logger& operator=(const Logger&) = delete;  // ZAKÁŽ priradenie
};

Logger a;
Logger b = a;  // CHYBA pri kompilácii – presne čo chceme
```
</details>

---

## 3. Dedičnosť a polymorfizmus

<details>
<summary><b><code>virtual</code></b> — dynamický dispatch</summary>

Označuje metódu v base triede, že sa **dá prepísať** v podedenej triede a pritom sa zavolá **správna verzia** aj keď pracuješ cez pointer na base typ. Bez `virtual` sa volá metóda podľa typu pointera (statický typ), nie podľa skutočného objektu (dynamický typ). Toto je kľúč k polymorfizmu.

```cpp
class Robot
{
public:
    virtual void stop()
    {
        std::cout << "Robot stop\n";
    }
};

class Drone : public Robot
{
public:
    void stop() override
    {
        std::cout << "Drone landing\n";
    }
};

Robot* r = new Drone();
r->stop();  // S virtual: "Drone landing" (správne)
            // Bez virtual: "Robot stop" (nesprávne!)
```
</details>

<details>
<summary><b><code>= 0</code> (pure virtual)</b> — abstraktná trieda</summary>

Robí z metódy **čisto virtuálnu** — trieda **neposkytuje implementáciu**, len predpis. Podedená trieda ju **MUSÍ implementovať**, inak sa tiež nedá inštancovať. Trieda s aspoň jednou pure virtual metódou sa nazýva **abstraktná trieda** — slúži len ako rozhranie.

```cpp
class Planner
{
public:
    virtual Path plan() = 0;  // neexistuje implementácia
};

Planner p;  // CHYBA – nemôžeš vytvoriť abstraktnú triedu
```
</details>

<details>
<summary><b><code>override</code></b> — bezpečnostná poistka</summary>

Bezpečnostný mechanizmus v podedenej triede. Hovorí kompilátoru: *„Táto metóda MUSÍ prepisovať virtuálnu metódu z base triedy — ak nie, vyhoď chybu."* Chráni pred preklepmi v názve metódy alebo zle napísanej signatúre. Nie je povinný, ale **vždy ho píš**.

```cpp
class RrtPlanner : public Planner
{
public:
    Path plan() override;    // OK – plan() existuje v Planner
    Path plaan() override;   // CHYBA – kompilátor odchytí preklep
};
```
</details>

---

## 4. Static (tri rôzne významy!)

> ⚠️ Pozor: `static` má v C++ **tri úplne odlišné významy** podľa kontextu.

<details>
<summary><b><code>static</code> metóda triedy</b></summary>

Metóda patrí **triede ako celku**, nie konkrétnemu objektu. Volá sa cez `NázovTriedy::metóda()`, netreba vytvárať objekt. Nemá prístup k `this` ani k nestatickým členom. Používa sa na utility funkcie alebo factory metódy.

```cpp
class Robot
{
public:
    static void printHelp()
    {
        std::cout << "Help\n";
    }
};

Robot::printHelp();  // volám bez objektu
```
</details>

<details>
<summary><b><code>static</code> členská premenná</b></summary>

Premenná je **zdieľaná medzi všetkými inštanciami** triedy. Existuje len jedna kópia, bez ohľadu na to, koľko objektov vytvoríš. Typické použitie: počítadlo inštancií, zdieľaná konfigurácia.

```cpp
class Robot
{
    static int count_;  // jedna premenná pre všetkých robotov
};
```
</details>

<details>
<summary><b><code>static</code> lokálna premenná</b></summary>

Premenná vo funkcii, ktorá **prežije koniec funkcie** a zachová si hodnotu medzi volaniami. Inicializuje sa len pri prvom volaní. Žije do konca programu.

```cpp
void tick()
{
    static int counter = 0;  // prvé volanie: 0, druhé: 1, tretie: 2...
    std::cout << counter++;
}
```
</details>

---

## 5. Const rodina

<details>
<summary><b><code>const</code> premenná</b></summary>

Hodnota sa **nedá zmeniť** po inicializácii. Kompilátor vyhodí chybu ak sa o to pokúsiš. Slúži ako ochrana pred neúmyselnou zmenou a ako dokumentácia úmyslu programátora.

```cpp
const int MAX_SPEED = 100;
MAX_SPEED = 200;  // CHYBA
```
</details>

<details>
<summary><b><code>const</code> metóda</b></summary>

Metóda **nemení stav objektu** — nemôže meniť žiadne členské premenné. Ak máš `const Robot r;`, môžeš na ňom volať len `const` metódy. Gettery by mali byť vždy `const`.

```cpp
class Robot
{
public:
    int getSpeed() const    // nemení objekt
    {
        return speed_;
    }

    void setSpeed(int v)    // MENÍ objekt – nie je const
    {
        speed_ = v;
    }

private:
    int speed_;
};

const Robot r;
r.getSpeed();   // OK – const metóda
r.setSpeed(5);  // CHYBA – const objekt nemôže volať ne-const metódu
```
</details>

<details>
<summary><b><code>const &</code> parameter</b></summary>

Parameter sa **nekopíruje** (referencia) a **nemení** (const). Štandardný spôsob odovzdávania veľkých objektov do funkcie. Šetrí pamäť a CPU, pretože sa nevytvára kópia.

```cpp
void process(const Path& path);  // path sa nekopíruje, nemení
```
</details>

<details>
<summary><b><code>const</code> a smerníky (tri kombinácie!)</b></summary>

```cpp
const int* p;        // nemôžem meniť HODNOTU (*p = 10 → CHYBA)
                     // ale môžem meniť KAM ukazuje (p = &b → OK)

int* const p;        // nemôžem meniť KAM ukazuje (p = &b → CHYBA)
                     // ale môžem meniť HODNOTU (*p = 10 → OK)

const int* const p;  // nemôžem meniť NIČ
```

> 💡 **Trik na zapamätanie:** čítaj sprava doľava. `const int*` = pointer na const int. `int* const` = const pointer na int.
</details>

<details>
<summary><b><code>constexpr</code></b> — compile-time výpočty</summary>

Funkcia alebo premenná **MÔŽE** byť vyhodnotená v čase kompilácie. Ak sú vstupy známe pri kompilácii, kompilátor vypočíta výsledok vopred (nulový runtime cost). Ak nie sú, funguje normálne za behu.

```cpp
constexpr int square(int x)
{
    return x * x;
}

int a = square(5);    // compile-time: kompilátor vloží 25
int b;
std::cin >> b;
int c = square(b);    // runtime: musí počítať za behu
```
</details>

<details>
<summary><b><code>consteval</code> (C++20)</b> — povinný compile-time</summary>

Funkcia **MUSÍ** byť vyhodnotená v čase kompilácie. Ak sa nedá (napr. vstup je runtime hodnota), kompilátor vyhodí chybu. Prísnejšia verzia `constexpr`.

```cpp
consteval int square(int x) { return x * x; }

int a = square(5);    // OK – compile-time
int b;
std::cin >> b;
int c = square(b);    // CHYBA – nedá sa vyhodnotiť pri kompilácii
```
</details>

---

## 6. Pamäť a smerníky

<details>
<summary><b><code>new</code> a <code>delete</code></b></summary>

`new` alokuje pamäť na **heape** a zavolá **konštruktor** objektu. Oproti C-čkovému `malloc` je bezpečnejšie, lebo rieši aj inicializáciu. Po `new` MUSÍ niekde nasledovať `delete`, inak memory leak.

`delete` zavolá **deštruktor** objektu a **uvoľní pamäť** na heape. Musí zodpovedať predchádzajúcemu `new`.

```cpp
Robot* r = new Robot();  // alokácia + konštruktor
delete r;                // deštruktor + uvoľnenie pamäte
```

> ⚠️ V modernom C++ sa `new`/`delete` nahradzuje smart pointermi.
</details>

<details>
<summary><b><code>new[]</code> / <code>delete[]</code></b> — polia na heape</summary>

Pre **polia** na heape. **NEMIEŠAJ** s bežným `delete` — `new[]` vyžaduje `delete[]`, inak undefined behavior!

```cpp
int* arr = new int[5];
delete[] arr;   // SPRÁVNE
// delete arr;  // ZLE – undefined behavior!
```
</details>

<details>
<summary><b><code>nullptr</code></b> — typovo bezpečný null</summary>

Špeciálna hodnota pointera znamenajúca *„neukazujem nikam"*. C++11 náhrada za C-čkové `NULL`. Typovo bezpečnejšie — `NULL` je vlastne `0` (int), `nullptr` je skutočne pointer.

```cpp
int* p = nullptr;
if (p != nullptr)    // kontrola pred použitím
    std::cout << *p;
```
</details>

<details>
<summary><b><code>this</code></b> — pointer na aktuálny objekt</summary>

Implicitný smerník na **aktuálny objekt** — objekt, v ktorom sa práve nachádza vykonávaný kód.

```cpp
class Robot
{
public:
    void setSpeed(int speed)
    {
        this->speed_ = speed;  // this-> odlíši členskú premennú
    }
private:
    int speed_;
};
```
</details>

---

## 7. Smart pointery

| Pointer | Vlastníctvo | Kopírovateľný | Overhead |
|---------|-------------|---------------|----------|
| `unique_ptr` | Jeden vlastník | ❌ (len move) | Žiadny |
| `shared_ptr` | Viacero vlastníkov | ✅ | Refcount |
| `weak_ptr` | Žiadne (pozorovateľ) | ✅ | Žiadny |

<details>
<summary><b><code>std::unique_ptr&lt;T&gt;</code></b> — default voľba</summary>

Inteligentný smerník s **jedným vlastníkom**. Keď `unique_ptr` zanikne (koniec scope), automaticky zavolá `delete`. Nedá sa kopírovať — len presunúť cez `std::move()`. Nulový runtime overhead oproti raw pointeru.

```cpp
std::unique_ptr<Robot> r = std::make_unique<Robot>();
// r zanikne na konci scope → automaticky delete
```
</details>

<details>
<summary><b><code>std::shared_ptr&lt;T&gt;</code></b> — zdieľané vlastníctvo</summary>

Inteligentný smerník s **viacerými vlastníkmi**. Používa **reference counting** — počíta koľko `shared_ptr`-ov ukazuje na ten istý objekt. Keď count klesne na 0, zavolá `delete`.

```cpp
std::shared_ptr<Robot> r1 = std::make_shared<Robot>();  // count = 1
std::shared_ptr<Robot> r2 = r1;                         // count = 2
r1.reset();                                             // count = 1
r2.reset();                                             // count = 0 → delete
```
</details>

<details>
<summary><b><code>std::weak_ptr&lt;T&gt;</code></b> — riešenie cyklov</summary>

Sleduje `shared_ptr` **bez zvýšenia refcount**. Rieši problém **cyklickej referencie**. Pred použitím musíš zavolať `.lock()`, ktorý vráti dočasný `shared_ptr`.

```cpp
std::shared_ptr<Robot> sp = std::make_shared<Robot>();
std::weak_ptr<Robot> wp = sp;  // nezvyšuje refcount

if (auto locked = wp.lock())   // existuje ešte?
{
    locked->move();            // áno, bezpečne použijem
}
```
</details>

<details>
<summary><b><code>make_unique</code> / <code>make_shared</code></b></summary>

Bezpečné vytvorenie smart pointera. `make_shared` alokuje objekt + control block **v jednej alokácii** (rýchlejšie). Oba chránia pred memory leakom v prípade výnimky.

```cpp
// PREFERUJ:
auto r = std::make_unique<Robot>(10);
// NAMIESTO:
std::unique_ptr<Robot> r(new Robot(10));  // menej bezpečné
```
</details>

<details>
<summary><b><code>std::move()</code></b> — presun namiesto kopírovania</summary>

Konvertuje **lvalue na rvalue referenciu**, čím umožní **presun** namiesto kopírovania. Samotný `move()` nič nepresúva — len hovorí kompilátoru *„tento objekt už nepotrebujem"*.

```cpp
std::unique_ptr<Robot> r1 = std::make_unique<Robot>();
std::unique_ptr<Robot> r2 = std::move(r1);  // r1 je teraz nullptr
```
</details>

---

## 8. Templates a generické programovanie

<details>
<summary><b><code>template&lt;typename T&gt;</code></b> — generátor kódu</summary>

Šablóna — **generátor kódu**. Kompilátor počas kompilácie zistí, s akými typmi sa šablóna používa, a vygeneruje špecializovaný kód pre každý typ. Zero runtime overhead.

```cpp
template<typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}

max(3, 5);        // kompilátor vytvorí int max(int, int)
max(3.1, 4.2);    // kompilátor vytvorí double max(double, double)
```
</details>

<details>
<summary><b><code>typename</code> vs <code>class</code></b></summary>

V `template<typename T>` a `template<class T>` sú **zameniteľné** — oba znamenajú *„neznámy typ"*. Konvencia: `typename` je modernejšie a čitateľnejšie.
</details>

<details>
<summary><b><code>auto</code></b> — odvodenie typu</summary>

Kompilátor sám **odvodí typ** z pravej strany výrazu.

```cpp
auto it = vec.begin();                    // namiesto std::vector<int>::iterator
auto r = std::make_unique<Robot>();       // namiesto std::unique_ptr<Robot>
auto lambda = [](int x){ return x*2; };   // typ lambdy je nevyjadriteľný
```
</details>

<details>
<summary><b><code>concept</code> (C++20)</b> — požiadavky na typ</summary>

Pomenovaná sada **požiadaviek na typ**. Nahradzuje SFINAE — čitateľnejšie, jasnejšie chybové hlásenia.

```cpp
template<typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::same_as<std::ostream&>;
};

template<Printable T>      // T musí byť vypísateľné
void print(const T& x)
{
    std::cout << x << "\n";
}
```
</details>

<details>
<summary><b><code>requires</code> (C++20)</b></summary>

Inline podmienka v šablóne — *„tento kód sa použije len ak typ spĺňa podmienku"*.

```cpp
template<typename T>
void printZ(const T& obj)
{
    if constexpr (requires { obj.z; })  // má objekt člen .z?
    {
        std::cout << obj.z;
    }
}
```
</details>

---

## 9. Vlákna a synchronizácia

<details>
<summary><b><code>std::thread</code> a <code>std::jthread</code></b></summary>

`std::thread` vytvorí **nové vlákno**. MUSÍŠ zavolať `join()` alebo `detach()`, inak program spadne.

```cpp
void worker(int id)
{
    std::cout << "Thread " << id << "\n";
}

std::thread t(worker, 1);  // spustí worker(1) v novom vlákne
t.join();                  // čakaj kým skončí
```

`std::jthread` (C++20) v deštruktore **automaticky zavolá `join()`**.

```cpp
{
    std::jthread t(worker, 1);  // spustí vlákno
}  // tu sa automaticky zavolá join()
```
</details>

<details>
<summary><b><code>std::mutex</code> a <code>lock_guard</code></b></summary>

`std::mutex` zabezpečuje, že v kritickej sekcii je **len jedno vlákno**.

```cpp
std::mutex m;
m.lock();     // zamkni
counter++;    // kritická sekcia
m.unlock();   // odomkni
```

`std::lock_guard` je RAII wrapper — vždy preferuj pred manuálnym lock/unlock.

```cpp
void increment()
{
    std::lock_guard<std::mutex> lock(m);  // lock() v konštruktore
    counter++;
}  // unlock() v deštruktore – automaticky, bezpečne
```
</details>

<details>
<summary><b><code>unique_lock</code> a <code>scoped_lock</code></b></summary>

`std::unique_lock` je flexibilnejší — umožňuje manuálne `lock()`, `unlock()`, `try_lock()`. Vyžaduje sa s `condition_variable`.

```cpp
std::unique_lock<std::mutex> lock(m);
cv.wait(lock);     // condition_variable vyžaduje unique_lock
lock.unlock();     // manuálne odomknutie
```

`std::scoped_lock` zamkne **viacero mutexov naraz** bez rizika deadlocku.

```cpp
std::scoped_lock lock(mutex1, mutex2);  // bezpečné zamknutie oboch
```
</details>

<details>
<summary><b><code>std::atomic&lt;T&gt;</code></b></summary>

Premenná, pri ktorej sú **základné operácie atomické** — čítanie, zápis, inkrement sú garantovane bezpečné bez mutexu.

```cpp
std::atomic<int> counter = 0;
counter++;  // atomické – bezpečné z viacerých vlákien
```
</details>

<details>
<summary><b><code>std::condition_variable</code></b> — čakanie a budenie</summary>

```cpp
// vlákno 1 (konzument): čaká na dáta
std::unique_lock<std::mutex> lock(m);
cv.wait(lock, []{ return !queue.empty(); });
auto data = queue.front();
queue.pop();

// vlákno 2 (producent): notifikuje
{
    std::lock_guard<std::mutex> lock(m);
    queue.push(data);
}
cv.notify_one();  // zobuď konzumenta
```
</details>

<details>
<summary><b><code>std::async</code>, <code>future</code>, <code>promise</code></b></summary>

`std::async` spustí úlohu **asynchrónne** a vráti `std::future` s výsledkom.

```cpp
std::future<int> f = std::async(std::launch::async, compute);
int result = f.get();  // čaká na výsledok
```

`std::promise` je ručný mechanizmus na **odovzdanie hodnoty** medzi vláknami.

```cpp
std::promise<int> p;
std::future<int> f = p.get_future();

std::thread t([&p]{
    p.set_value(42);  // producent nastaví výsledok
});

int result = f.get();  // konzument čaká a prečíta
t.join();
```
</details>

<details>
<summary><b><code>thread_local</code></b></summary>

Každé vlákno dostane **vlastnú kópiu** premennej.

```cpp
thread_local int counter = 0;

void worker()
{
    counter++;  // každé vlákno má svoj counter
}
```
</details>

---

## 10. Výnimky

<details>
<summary><b><code>throw</code>, <code>try</code>, <code>catch</code></b></summary>

```cpp
try
{
    SensorDriver sensor;  // konštruktor môže hodiť výnimku
    sensor.measure(data);
}
catch (const std::runtime_error& e)
{
    std::cout << "Runtime error: " << e.what() << "\n";
}
catch (...)
{
    std::cout << "Unknown error\n";
}
```

Výnimku vyhodíš cez `throw`:

```cpp
if (!sensor_ok)
    throw std::runtime_error("Sensor init failed");
```
</details>

### Kedy výnimky, kedy error kódy?

| Situácia | Použitie |
|----------|----------|
| Inicializácia, konfigurácia | Výnimky (`throw`) |
| Control loop, hardware IO | Error kódy (`return bool`) |
| Fatálny hardware failure | Výnimky (`throw`) |

---

## 11. Ostatné dôležité

<details>
<summary><b><code>using</code></b> — typový alias</summary>

Moderná náhrada za `typedef`.

```cpp
using Path = std::vector<Point>;
using Callback = std::function<void(int)>;
```
</details>

<details>
<summary><b><code>namespace</code></b></summary>

```cpp
namespace my_robot {
    class Planner { ... };
}
my_robot::Planner p;  // jednoznačné
```
</details>

<details>
<summary><b><code>#pragma once</code> a <code>#include</code></b></summary>

```cpp
#pragma once                          // ochrana pred duplicitným zahrnutím

#include <iostream>                   // systémová knižnica
#include "my_module/planner.hpp"      // projektový súbor
```
</details>

---

# 🔶 Časť 2: ROS2

## 2.1 Čo je ROS2?

**ROS2** (Robot Operating System 2) je framework pre robotické aplikácie. Nie je to operačný systém — je to **middleware** (komunikačná vrstva), ktorá umožňuje:

- 📡 komunikáciu medzi modulmi (procesmi) cez **topics** a **services**
- 🧩 modulárnu architektúru (každý modul = node)
- ⚙️ konfiguráciu cez **parametre** (YAML)
- 🚀 spúšťanie cez **launch** súbory

---

## 2.2 Základné pojmy

| Pojem | Vysvetlenie |
|-------|-------------|
| **Node** | Samostatný proces/modul, dedí z `rclcpp::Node` |
| **Topic** | Komunikačný kanál pre dáta (publish/subscribe) |
| **Publisher** | Odosiela správy na topic |
| **Subscriber** | Prijíma správy z topicu |
| **Service** | Synchronná komunikácia (request → response) |
| **Message (.msg)** | Definícia dátovej štruktúry pre topic |
| **Service (.srv)** | Definícia request + response štruktúry |
| **Parameter** | Konfiguračná hodnota (loadovaná z YAML) |
| **Launch file** | Skript na spúšťanie viacerých nodov naraz |
| **TF2** | Transformačný framework – vzťahy medzi súradnicovými systémami |

---

## 2.3 Štruktúra ROS2 balíka (podľa OPK)

```text
my_module/
├── include/
│   └── my_module/
│       ├── my_module_ros.hpp     ← trieda : public rclcpp::Node
│       └── my_class.hpp          ← čistá logika (BEZ ROS!)
├── src/
│   ├── my_module_node.cpp        ← iba main() + spin
│   ├── my_module_ros.cpp         ← ROS rozhranie (sub/pub/params)
│   └── my_class.cpp              ← implementácia logiky
├── config/
│   └── my_module.yaml            ← parametre
├── launch/
│   └── my_module.launch.py       ← spúšťací skript
├── msg/
│   └── MyMessage.msg             ← vlastná správa
├── srv/
│   └── MyService.srv             ← vlastná služba
├── CMakeLists.txt
├── package.xml
└── README.md
```

> 🔑 **Kľúčové pravidlo OPK:** ROS rozhranie a logika sú **ODDELENÉ**.

| Súbor | Obsahuje | Čo tam NEPATRÍ |
|-------|----------|-----------------|
| `*_node.cpp` | `main()` + `rclcpp::spin()` | Žiadna logika |
| `*_ros.cpp` | Subscribery, publishery, parametre | Žiadna logika |
| `*.cpp` (logika) | Algoritmy, výpočty | Žiadne ROS prvky |

**Prečo?** Aby si mohol testovať logiku bez ROS. Aby si mohol vymeniť ROS za iný framework.

---

## 2.4 Konštruktor ROS2 nodu

> ⭐ Toto je **veľmi pravdepodobná otázka na skúške**.

### Hlavičkový súbor

```cpp
// my_module_ros.hpp
#pragma once

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include "my_module/my_class.hpp"

class MyModuleROS : public rclcpp::Node
{
public:
    MyModuleROS();
    ~MyModuleROS() = default;

private:
    void topicCallback(const std_msgs::msg::String::SharedPtr msg);
    void loadParams();

    // ROS rozhranie
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscriber_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

    // Logika (NIE ROS)
    MyClass my_logic_;
};
```

### Implementácia konštruktora

```cpp
// my_module_ros.cpp
#include "my_module/my_module_ros.hpp"

MyModuleROS::MyModuleROS()
: Node("my_node_name")    // ← INICIALIZAČNÝ ZOZNAM: voláme konštruktor Node
{
    loadParams();

    // Vytvorenie subscribera
    subscriber_ = this->create_subscription<std_msgs::msg::String>(
        "input_topic",     // názov topicu
        10,                // queue size
        std::bind(&MyModuleROS::topicCallback, this, std::placeholders::_1)
    );

    // Vytvorenie publishera
    publisher_ = this->create_publisher<std_msgs::msg::String>(
        "output_topic",    // názov topicu
        10                 // queue size
    );

    // Timer (voliteľné)
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(100),
        [this]() { /* callback každých 100ms */ }
    );

    RCLCPP_INFO(this->get_logger(), "Node initialized");
}
```

### Callback pre subscriber

```cpp
void MyModuleROS::topicCallback(const std_msgs::msg::String::SharedPtr msg)
{
    RCLCPP_INFO(this->get_logger(), "Received: %s", msg->data.c_str());

    // Spracuj cez logiku (nie ROS!)
    auto result = my_logic_.process(msg->data);

    // Publikuj výsledok
    auto out_msg = std_msgs::msg::String();
    out_msg.data = result;
    publisher_->publish(out_msg);
}
```

---

## 2.5 Načítanie parametrov (OPK spôsob)

### Štruktúra na parametre v triede logiky

```cpp
// my_class.hpp
class MyClass
{
public:
    MyClass() = default;

    struct Params
    {
        int number;
        std::string message;
    };

    void setParams(const MyClass::Params& params)
    {
        params_ = params;
    }

private:
    Params params_;
};
```

### Načítanie v ROS triede

```cpp
// my_module_ros.cpp
void MyModuleROS::loadParams()
{
    MyClass::Params params;
    params.number = this->declare_parameter<int>("param_name", 42);
    params.message = this->declare_parameter<std::string>("message", "default");

    my_logic_.setParams(params);
}
```

### YAML konfigurácia

```yaml
# config/my_module.yaml
my_node_name:
  ros__parameters:
    param_name: 100
    message: "Hello from YAML"
```

> ⚠️ **Dôležité:** Názov uzla v YAML (`my_node_name`) musí zodpovedať názvu v `Node("my_node_name")`.

---

## 2.6 Main súbor (node)

```cpp
// my_module_node.cpp
#include <rclcpp/rclcpp.hpp>
#include "my_module/my_module_ros.hpp"

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<MyModuleROS>();

    rclcpp::spin(node);

    rclcpp::shutdown();
    return 0;
}
```

**Obsahuje IBA:** `init()`, vytvorenie nodu, `spin()`, `shutdown()`. **Žiadna logika!**

---

## 2.7 Launch súbor

```python
# launch/my_module.launch.py
import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    config_file = os.path.join(
        get_package_share_directory('my_module'),
        'config',
        'my_module.yaml'
    )

    my_node = Node(
        package='my_module',
        executable='my_node_executable',  # z CMakeLists.txt
        name='my_node_name',              # musí zodpovedať YAML
        parameters=[config_file],
        output='screen'
    )

    return LaunchDescription([my_node])
```

---

## 2.8 ROS2 logovacie makrá

> 🚫 **Vždy použi ROS makrá namiesto `std::cout`!** (OPK pravidlo)

```cpp
RCLCPP_INFO(this->get_logger(), "Správa: %d", value);        // informácia
RCLCPP_WARN(this->get_logger(), "Varovanie!");                // varovanie
RCLCPP_ERROR(this->get_logger(), "Chyba: %s", err.c_str());   // chyba
RCLCPP_DEBUG(this->get_logger(), "Debug info");               // debug
```

---

## 2.9 Typické skúškové otázky s ROS2

> 💬 **„Prečo tento kód nefunguje?"**

<details>
<summary><b>Chyba 1: Chýba <code>Node(...)</code> v inicializačnom zozname</b></summary>

```cpp
// ❌ ZLE
MyModuleROS::MyModuleROS()
{
    // konštruktor Node sa nezavolá správne
}

// ✅ DOBRE
MyModuleROS::MyModuleROS()
: Node("my_node_name")  // MUSÍ byť v inicializačnom zozname
{
}
```
</details>

<details>
<summary><b>Chyba 2: Nesúlad názvu uzla medzi kódom a YAML</b></summary>

```cpp
// V kóde:
: Node("robot_node")
```

```yaml
# V YAML:
my_node:            # ← NESÚLAD → parametre sa nenačítajú
  ros__parameters:
    speed: 10
```
</details>

<details>
<summary><b>Chyba 3: Zabudnutý <code>std::bind</code> v subscriberi</b></summary>

```cpp
// ❌ ZLE – callback je metóda triedy, nemôžeš ju dať priamo
subscriber_ = this->create_subscription<Msg>(
    "topic", 10, topicCallback  // CHYBA
);

// ✅ DOBRE
subscriber_ = this->create_subscription<Msg>(
    "topic", 10,
    std::bind(&MyModuleROS::topicCallback, this, std::placeholders::_1)
);

// ✅ DOBRE (lambda alternatíva)
subscriber_ = this->create_subscription<Msg>(
    "topic", 10,
    [this](const Msg::SharedPtr msg) { topicCallback(msg); }
);
```
</details>

<details>
<summary><b>Chyba 4: Logika v ROS triede</b></summary>

```cpp
// ❌ ZLE – porušenie OPK princípu
void MyModuleROS::topicCallback(const Msg::SharedPtr msg)
{
    // 200 riadkov algoritmu tu ← ZLE!
    for (auto& point : msg->data)
    {
        // filtrovanie, výpočty, plánovanie...
    }
}

// ✅ DOBRE – delegácia na triedu logiky
void MyModuleROS::topicCallback(const Msg::SharedPtr msg)
{
    auto result = my_logic_.process(msg->data);
    publisher_->publish(result);
}
```
</details>

<details>
<summary><b>Chyba 5: <code>std::cout</code> namiesto RCLCPP</b></summary>

```cpp
// ❌ ZLE
std::cout << "Speed: " << speed << std::endl;

// ✅ DOBRE
RCLCPP_INFO(this->get_logger(), "Speed: %d", speed);
```
</details>

<details>
<summary><b>Chyba 6: Zabudnutý <code>rclcpp::spin()</code></b></summary>

```cpp
// ❌ ZLE – node sa vytvorí ale hneď skončí, callbacky sa nikdy nezavolajú
int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MyModuleROS>();
    rclcpp::shutdown();  // subscribery nikdy nič neprijmú
    return 0;
}

// ✅ DOBRE
int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MyModuleROS>();
    rclcpp::spin(node);   // TOTO drží program nažive a spúšťa callbacky
    rclcpp::shutdown();
    return 0;
}
```
</details>

<details>
<summary><b>Chyba 7: Deklarácia parametra s nesprávnym typom</b></summary>

```yaml
# YAML:
speed: 10.5
```

```cpp
// ❌ ZLE – v kóde chceš int, ale v YAML je double
int speed = this->declare_parameter<int>("speed", 0);
// → runtime chyba alebo neočakávané pretypovanie

// ✅ DOBRE – typy musia zodpovedať
double speed = this->declare_parameter<double>("speed", 0.0);
```
</details>

---

## 2.10 CMakeLists.txt pre ROS2 balík

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_module)

# Nájdi závislosti
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# Spustiteľný súbor
add_executable(my_node_executable
    src/my_module_node.cpp
    src/my_module_ros.cpp
    src/my_class.cpp
)

# Include cesty
target_include_directories(my_node_executable PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ROS závislosti
ament_target_dependencies(my_node_executable
    rclcpp
    std_msgs
)

# Inštalácia
install(TARGETS my_node_executable
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY config launch
    DESTINATION share/${PROJECT_NAME}
)

ament_package()
```

---

## 2.11 Rýchly checklist — „Mám všetko?"

Pred odovzdaním/skúškou skontroluj:

- [ ] `*_ros.hpp` dedí z `rclcpp::Node`
- [ ] Konštruktor volá `Node("meno")` v inicializačnom zozname
- [ ] Subscriber má `std::bind` alebo lambdu
- [ ] `main()` obsahuje `init()` → `make_shared` → `spin()` → `shutdown()`
- [ ] Logika je v **samostatnej triede** (nie v ROS triede)
- [ ] Parametre cez `declare_parameter<T>("name", default)`
- [ ] YAML názov uzla = kódový názov uzla
- [ ] `RCLCPP_*` makrá namiesto `std::cout`
- [ ] ROS rozhranie v `*_ros.cpp`, logika v `*.cpp`, main v `*_node.cpp`

---

<div align="center">

**📚 Veľa šťastia na skúške OPK! 🚀**

Made with ☕ and 🤖

</div>
