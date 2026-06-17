# Interaktywna scena podwodna — OpenGL / GLSL

Projekt zaliczeniowy GRK 2026. Aplikacja 3D w C++/OpenGL/GLSL przedstawiająca spójną
scenę podwodną: dno morskie z rafą i głazami, pływające stworzenia poruszające się po
splajnach, reflektor, smugi światła, kaustyki, bąble i zawiesina (marine snow) oraz
głębinowa mgła.

## Skład grupy

- Ruslan Trytsetskyi
- Daniel Stodulski
- Martyna Plenzer

## Wybrane metody

| Kategoria | ID | Metoda | Gdzie w projekcie |
|-----------|----|--------|-------------------|
| A (zaawansowana) | **A10** | Skeletal / vertex-shader swimming animation | `shaders/axolotl_vertex.glsl` — deformacja wierzchołków ciała falą sinusoidalną o rosnącej amplitudzie ku ogonowi |
| B (mniej zaawansowana) | **B04** | Fish/submarine movement along a spline path | `src/axolotl.cpp` — ruch po zamkniętym splajnie Catmulla-Roma, **orientacja z ramek transportu równoległego (PTF)** z bankowaniem w zakrętach |

Kombinacja: **A10 + B04**.

## Metody obowiązkowe

| Metoda | Realizacja |
|--------|-----------|
| **Normal mapping** | Dno morskie — dwa materiały (piasek + porost) z mapami normalnych i przestrzenią styczną TBN (`shaders/worldmesh_vertex.glsl`, `worldmesh_fragment.glsl`). |
| **PBR lighting** | Model metallic/roughness (Cook-Torrance GGX) na dnie, stworzeniach i rafie (`worldmesh_fragment.glsl`, `axolotl_fragment.glsl`, `reef_fragment.glsl`). Parametry sterowalne klawiszami 1–4. |
| **Quaternion camera control** | Kamera oparta na kwaternionach, bez gimbal lock (`src/camera.cpp`, `headers/camera.h`). |
| **Shadow mapping** | Mapa cienia 4096×4096 dla słońca, filtrowanie PCF 3×3 + slope-scaled bias; cienie rzucają teren, monument, stworzenia i rafa (`src/application.cpp` — `ShadowPass`). |
| **Parallel Transport Frames** | (1) generacja rury/kabla wzdłuż splajnu (`src/tube.cpp`), (2) stabilna orientacja stworzeń wzdłuż trajektorii pływania (`src/axolotl.cpp`). |
| **Underwater skybox/cubemap** | Cubemapa środowiskowa renderowana jako tło sceny, niezależna od ruchu kamery (`src/cubemap.cpp`, `assets/cubemap/`). |

## Dodatkowe techniki (podbicie jakości wizualnej)

Poza wymaganymi metodami scena wykorzystuje również:

- **Volumetric light shafts / god rays** — smugi światła w post-processie (`shaders/postprocess_fragment.glsl`).
- **Animowane kaustyki** rzutowane na zanurzoną geometrię (post-process).
- **Depth fog + color attenuation** — głębinowe tłumienie koloru zależne od dystansu (post-process).
- **ACES filmic tonemapping** + winieta.
- **System cząstek** — unoszące się bąble (CPU particles) oraz billboardowany plankton / marine snow.
- **OBJ model loading** — wczytywanie zewnętrznych modeli (stworzenia + rafa).
- **Ruchomy reflektor** podążający za stworzeniem prowadzącym.
- **Object picking** — wskazywanie stworzenia kursorem (lewy przycisk myszy).
- **Instancing/LOD terenu** — chunki dna z poziomami szczegółowości i frustum cullingiem.

## Sterowanie

Ruch kamery (nie liczy się jako interakcja):

| Klawisz | Akcja |
|---------|-------|
| Mysz | Rozglądanie (kamera kwaternionowa) |
| W / S | Przód / tył |
| A / D | Lewo / prawo |
| Q / E | Góra / dół |
| G | Skok do punktu widokowego przy monumencie |
| H | Spójrz na najbliższe stworzenie |

Interakcje ze sceną (wymagane, niezależne od kamery):

| Klawisz / akcja | Efekt |
|-----------------|-------|
| **L** | Włącz / wyłącz reflektor (zmiana oświetlenia sceny) |
| **5 / 6** | Mniejsza / większa gęstość mgły (widoczność pod wodą) |
| **7 / 8** | Słabszy / silniejszy prąd wodny (wpływa na bąble i zawiesinę) |
| **Spacja** | Pauza / wznowienie ruchu stworzeń |
| **↑ / ↓** | Prędkość pływania stworzeń |
| **Lewy przycisk myszy** | Wyceluj i kliknij — przyspiesza wskazane stworzenie (ray picking) |
| **C** | Przełącz tło: underwater cubemap / niebo |
| **1 / 2** | Roughness dna − / + (demonstracja PBR) |
| **3 / 4** | Metallic dna − / + (demonstracja PBR) |

## Budowanie

Wymagane: **CMake** oraz kompilator C++20 (MSVC / Visual Studio "Programowanie aplikacji
klasycznych w języku C++"). Biblioteki GLFW, GLM i GLAD są pobierane automatycznie przez
CMake (FetchContent).

```bash
cmake -B build
cmake --build build --config Release
```

Plik wykonywalny wraz z katalogami `shaders/` i `assets/` trafia do
`build/Release/`. Uruchom `computer_graphics.exe` z tego katalogu.

## Struktura

- `src/`, `headers/` — kod C++ (aplikacja, kamera, scena, mesh-e, stworzenia, rafa, cząstki).
- `shaders/` — shadery GLSL (teren, woda, stworzenia, rafa, cubemap, cząstki, post-process, depth).
- `assets/` — heightmapa, tekstury, cubemapa, modele OBJ.

## Prezentacja (YouTube)

Przegląd wszystkich wymaganych metod: https://www.youtube.com/watch?v=purJ5joKITE

## Zrzuty ekranu

> _Do uzupełnienia przed oddaniem — np. `docs/screenshot_reef.png`, `docs/screenshot_caustics.png`, `docs/screenshot_godrays.png`._

## Źródła i licencje zasobów

- **SkyBox** — „FREE - SkyBox Basic Sky" autorstwa *Paul* (Sketchfab),
  licencja **CC-BY-4.0**. This work is based on "FREE - SkyBox Basic Sky"
  (https://sketchfab.com/3d-models/free-skybox-basic-sky-b2a4fd1b92c248abaae31975c9ea79e2)
  by Paul (https://sketchfab.com/paul_paul_paul) licensed under CC-BY-4.0
  (http://creativecommons.org/licenses/by/4.0/).
- **Nature Assets – Marine Biome** (autor *lugiagames*, CGTrader),
  **Royalty Free License** — koralowce, skały, jeżowce, rozgwiazdy i muszle rozsiane
  po dnie (`assets/models/fauna/`, wczytywane z OBJ, poziom LOD0). Materiały PBR
  (BaseColor + Normal + Occlusion/Roughness/Metallic, pliki .tga z paczki Blendera)
  są mapowane per-materiał i próbkowane w `reef_fragment.glsl` (normal mapping z TBN
  liczonym z pochodnych ekranowych). Modele użyte zgodnie z licencją; repozytorium
  jest **prywatne**, surowe pliki nie są redystrybuowane publicznie.
- **Axolotl** — model stworzenia (OBJ + tekstury PBR). _(Uzupełnić źródło i licencję.)_
- Heightmapa i tekstury dna: zasoby projektowe / własne.
