# Wielowatkowy symulator restauracji

Projekt Qt/C++ przygotowany na podstawie wstepnego planu pracy. Aplikacja pokazuje uproszczony obieg zamowien w restauracji:

- klienci generuja nowe zamowienia,
- kelnerzy przekazuja zamowienia do kuchni i odbieraja gotowe dania,
- kucharze pracuja w osobnych watkach i pobieraja zadania ze wspolnej kolejki,
- GUI pokazuje stan kolejek, pracownikow i dziennik zdarzen.

## Wymagania

- Qt 5 lub Qt 6 z modulem Widgets,
- kompilator C++17,
- Qt Creator albo narzedzie `qmake`.

## Uruchomienie w Qt Creator

1. Otworz plik `RestaurantSimulator.pro`.
2. Wybierz zestaw kompilacji z Qt Widgets.
3. Zbuduj i uruchom projekt.

## Uruchomienie z terminala

```powershell
qmake RestaurantSimulator.pro
mingw32-make
.\RestaurantSimulator.exe
```

Nazwa polecenia budowania moze zalezec od zestawu Qt. Dla MSVC zwykle uzywa sie `nmake`, a dla MinGW `mingw32-make`.

## Najwazniejsze pliki

- `src/mainwindow.*` - interfejs graficzny i prezentacja stanu symulacji,
- `src/simulationengine.*` - watki, kolejki, mutexy, zmienne warunkowe i logika obiegu zamowien,
- `src/order.h` - prosty model zamowienia,
- `RestaurantSimulator.pro` - konfiguracja projektu Qt.

## Zastosowane mechanizmy wspolbiezne

- `std::thread` dla generatora zamowien, kucharzy i kelnerow,
- `std::mutex` do ochrony wspolnych kolejek,
- `std::condition_variable` do usypiania watkow do czasu pojawienia sie pracy,
- `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` do bezpiecznego przekazywania stanu z watkow roboczych do GUI.
