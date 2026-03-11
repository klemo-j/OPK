#include <gtest/gtest.h>
#include "fifo_lib/frame_fifo.hpp"



// 1. Black-box hypotéza: Novo vytvorený objekt
TEST(FrameFifoBlackBoxTest, EmptyAfterCreation) {
    // 1. Príprava Vytvoríme nový inštanciu FrameFifo
    FrameFifo fifo;

    // 2. Overenie  Metóda getSize() vracia hodnotu 0
    EXPECT_EQ(fifo.getSize(), 0);

    // 3. Overenie Metóda empty() vracia true
    EXPECT_TRUE(fifo.empty());
}


// 2. Black-box hypotéza: Pridanie dvoch prvkov
TEST(FrameFifoBlackBoxTest, PushBackTwiceCreatesTwoDifferentObjects) {

    FrameFifo fifo;
    CameraFrame dummy_frame;

    // Vykonanie: Zavoláme pushBack presne dvakrát s naším frame-om
    fifo.pushBack(dummy_frame);
    fifo.pushBack(dummy_frame);

    // Overenie 1: Metóda getSize() vracia hodnotu 2
    EXPECT_EQ(fifo.getSize(), 2);


    auto ptr0 = fifo.at(0).lock();
    auto ptr1 = fifo.at(1).lock();

    // Uistíme sa, že nám to naozaj niečo vrátilo (že pointre nie sú prázdne)
    EXPECT_NE(ptr0, nullptr);
    EXPECT_NE(ptr1, nullptr);

    // Overenie 2: Návratové hodnoty volania at(0) a at(1) ukazujú na dva rôzne objekty
    // .get() nám vráti čistú pamäťovú adresu, porovnáme, či sú rozdielne
    EXPECT_NE(ptr0.get(), ptr1.get());
}

// 3. Black-box hypotéza: zavolanie metódy popFront() na prázdny FrameFifo
TEST(FrameFifoBlackBoxTest, CallMethodPopFront) {
    FrameFifo fifo;


    EXPECT_EQ(fifo.popFront(), StatusCode::ERROR_EMPTY);
}

// 4. Black-box hypotéza: pointer metody at
TEST(FrameFifoBlackBoxtest, PointerReturn) {
    FrameFifo fifo;


    auto ptr = fifo.at(-1).lock();
    EXPECT_EQ(ptr, nullptr);

}

// 5. Black-box hypotéza: dve volania pushBack a overenie posunu
TEST(FrameFifoBlackBoxtest, TwoCallingPushBack) {
    FrameFifo fifo;
    CameraFrame dummy_frame;

    // 1. Pridáme dva prvky do FIFO
    fifo.pushBack(dummy_frame);
    fifo.pushBack(dummy_frame);

    // 2. Overíme, že veľkosť je 2 (POZOR: opravené z "fifo" na "fifo.getSize()")
    EXPECT_EQ(fifo.getSize(), 2);

    // 3. Uložíme si DRUHÝ prvok (index 1) ešte PREDTÝM, než niečo vymažeme.
    // 'auto' zabezpečí, že vôbec nemusíš vedieť, aký typ pointra to presne je.
    auto item_at_1_before_pop = fifo.at(1).lock();

    // 4. Odstránime prvý prvok z radu
    fifo.popFront();

    // 5. Overíme, že zostal už len jeden prvok
    EXPECT_EQ(fifo.getSize(), 1);

    // 6. Načítame NOVÝ prvý prvok (index 0) po vymazaní
    auto item_at_0_after_pop = fifo.at(0).lock();

    // 7. Porovnáme, či je to ten istý objekt (či sa naozaj posunul dopredu)
    EXPECT_EQ(item_at_0_after_pop, item_at_1_before_pop);
}

// 6. Black-box hypotéza:

TEST(FrameFifoBlackBoxtest,ATleastOneCall) {
    FrameFifo fifo;
    CameraFrame dummy_frame;

    fifo.pushBack(dummy_frame);

    auto saved_frame = fifo.at(0).lock();

}