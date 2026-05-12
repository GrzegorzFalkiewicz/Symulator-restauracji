# Wielowatkowy symulator restauracji

Projekt Qt/C++ przygotowany na podstawie wstepnego planu pracy. Aplikacja pokazuje uproszczony obieg zamowien w restauracji:

- klienci generuja nowe zamowienia,
- kelnerzy przekazuja zamowienia do kuchni i odbieraja gotowe dania,
- kucharze pracuja w osobnych watkach i pobieraja zadania ze wspolnej kolejki,
- GUI pokazuje stan kolejek, pracownikow i dziennik zdarzen.

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
