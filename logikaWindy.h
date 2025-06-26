#pragma once
#include <iostream>
#include<set>
#include<vector>
#include <chrono>
using namespace std::chrono;
using namespace std;

enum Kierunek
{
    dol, stop, gora
};

enum Stan
{
    spawn, czeka, jedzie, dojechal
};

time_point<steady_clock> czasZatrzymania;
bool odliczanieAktywne = false;
bool zjazdNaParter = false;

class Pasazer {
public:
    int pietroStart;
    int pietroKoniec;
    Kierunek kierunek;
    Stan stan;
    Pasazer(int pStart, int pKoniec) :pietroStart(pStart), pietroKoniec(pKoniec), stan(spawn) {
        kierunek = (pStart < pKoniec) ? gora : dol;
    }
};


class Winda {
public:
    int pietro;
    Kierunek kierunek;
    vector<Pasazer*> vectorPasazerow;
    int liczbaPasazerow;
    int waga;
    //mozliwa poprawka wagi
    int pierwszaKolej;
    set<int> kolejkaGora;
    set<int, greater<int>> kolejkaDol;
    bool cele[5];


    Winda() :pierwszaKolej(0), pietro(0), kierunek(stop), liczbaPasazerow(0) {
        waga = liczbaPasazerow * 70;
        for (int i = 0; i < 5; i++) cele[i] = 0;
    }

    void aktualizacjaWagi() {
        liczbaPasazerow = vectorPasazerow.size();
        waga = liczbaPasazerow * 70;
    }

    void wezwij(Pasazer& pasazer) {

        if (pasazer.stan == spawn) {
            if (pasazer.kierunek == gora) {
                kolejkaGora.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
            else if (pasazer.kierunek == dol) {
                kolejkaDol.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
        }

    }


    //odnbieramy tylko jak vector.size()<8
    void odbierz(Pasazer& pasazer) {
        if (vectorPasazerow.size() >= 8) return;
        if (pietro == pasazer.pietroStart && pasazer.stan == czeka && kierunek == pasazer.kierunek) {
            //winda open animacja
            pasazer.stan = jedzie;
            vectorPasazerow.push_back(&pasazer);
            if (pasazer.kierunek == gora) {
                kolejkaGora.erase(pasazer.pietroStart);
            }
            if (pasazer.kierunek == dol) {
                kolejkaDol.erase(pasazer.pietroStart);
            }
            cele[pasazer.pietroKoniec] = true;
            aktualizacjaWagi();
            //animacja pasazer idzie do pola okreslonego indeksem wektora
            //zamykamy winde
            return;
        }
    }

    void odstaw() {
        for (auto it = vectorPasazerow.begin(); it != vectorPasazerow.end(); ) {
            if ((*it)->pietroKoniec == pietro && (*it)->stan == jedzie) {
                (*it)->stan = dojechal;
                it = vectorPasazerow.erase(it);
                //animacja it
            }
            else {
                ++it;
            }
        }
        cele[pietro] = false;
        aktualizacjaWagi();
    }

    void ruchPierwszaKolej() {
        //animacja ruchu od do
        //pietro = pierwszaKolej;
        if (pierwszaKolej == pietro) return;
        pietro < pierwszaKolej ? pietro++ : pietro--;
    }

    //test
    void pierwszyRuchJeœliPotrzeba() {
        if (kierunek == stop) {
            if (!kolejkaGora.empty()) {
                pierwszaKolej = *kolejkaGora.begin();
                kierunek = gora;
                ruchPierwszaKolej();
            }
            else if (!kolejkaDol.empty()) {
                pierwszaKolej = *kolejkaDol.begin();
                kierunek = dol;
                ruchPierwszaKolej();
            }
            //else if (vectorPasazerow.empty() && pietro != 0) {
            //    kierunek = dol;
            //    pietro--; // powrót na parter
            //}
        }
    }

    void ruch() {


        if (kierunek == gora) {
            for (int i = pietro; i < 5; i++) {
                if (cele[i] == true) {
                    //animacja
                    pietro++;
                    return;
                }
            }
            for (int kolejka : kolejkaGora) {
                if (kolejka > pietro) {
                    //animacja
                    pietro++;
                    return;
                }
            }
        }
        if (kierunek == dol) {
            for (int i = pietro; i >= 0; i--) {
                if (cele[i] == true) {
                    //animacja
                    pietro--;
                    return;
                }
            }
            for (int kolejka : kolejkaDol) {
                if (kolejka < pietro) {
                    //animacja
                    pietro--;
                    return;
                }
            }
        }
        kierunek = stop;
        // Jeœli winda pusta i nie ma nikogo do obs³ugi
        if (vectorPasazerow.empty() && kolejkaGora.empty() && kolejkaDol.empty()) {
            if (!odliczanieAktywne) {
                czasZatrzymania = steady_clock::now();
                odliczanieAktywne = true;
                return;
            }

            auto teraz = steady_clock::now();
            auto uplynelo = duration_cast<seconds>(teraz - czasZatrzymania).count();
            if (uplynelo >= 5 && pietro != 0) {
                zjazdNaParter = true;
                kierunek = dol;
            }
        }

        // Obs³uga zjazdu na parter
        if (zjazdNaParter) {
            if (!vectorPasazerow.empty() || !kolejkaGora.empty() || !kolejkaDol.empty()) {
                odliczanieAktywne = false;
                zjazdNaParter = false;
            }
            if (pietro > 0) {
                pietro--;
                return;
            }
            else {
                // Dotar³a na parter
                zjazdNaParter = false;
                odliczanieAktywne = false;
                kierunek = stop;
                return;
            }
        }

        // Je¿eli przerwano stan oczekiwania (np. wezwano windê), resetuj zegar
        if (!vectorPasazerow.empty() || !kolejkaGora.empty() || !kolejkaDol.empty()) {
            odliczanieAktywne = false;
            zjazdNaParter = false;
        }
    }

};