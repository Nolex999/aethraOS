# 8. Structure du dépôt

Le répertoire `firmware/` du brief devient `src/` avec des plateformes séparées, parce que presque tout le code doit compiler pour le simulateur. L'ESP32 reste un projet distinct, créé seulement lorsque la connectivité sera débloquée.

```text
aethra/
  README.md  LICENSE  CONTRIBUTING.md  CLAUDE.md  AGENTS.md
  ROADMAP.md  TASKS.md  STATUS.md  RISKS.md  DECISIONS.md
  HARDWARE_MATRIX.md  TEST_PLAN.md
  CMakeLists.txt  CMakePresets.json  .clang-format  .gitignore
  .claude/   agents/  commands/  skills/  settings.json
  .github/   workflows/  ISSUE_TEMPLATE/
  docs/      architecture/ hardware/ boot/ graphics/ applications/
             connectivity/ security/ decisions/ research/
  src/
    core/      arenes, evenements, minuteries, journal, erreurs
    hal/       interfaces uniquement
    gfx/       primitives, texte, widgets, themes
    input/
    storage/   VFS et adaptateurs
    services/
    framework/
    shell/
    apps/
    boot_anim/
    platform/  sim/  eadk/
  sdk/       include/ examples/ templates/
  esp32/     (plus tard)
  tools/     build/ flash/ diagnostics/ packaging/ analysis/
  tests/     unit/ integration/ hardware/ regression/
  assets/    fonts/ icons/ themes/
  examples/  python/ cpp/
  scripts/
```

## Règles

- `.gitignore` exclut `*.bin`, `*.dfu`, `*.i64`, `*.idb`, `*.raw` et `build/`. Les dumps restent hors du dépôt.
- Seul `src/platform/` peut inclure `eadk.h`.
- `tools/flash/` ne contient, par défaut, que des commandes de lecture. Toute écriture exige une confirmation interactive et la vérification de l'appareil.

## Chaîne d'outils

| Outil | Version constatée sur cette machine |
|---|---|
| CMake | 4.3.2 |
| GCC hôte (TDM) | 10.3.0 |
| arm-none-eabi-gcc | 10.3-2021.10 |
| Python | 3.14.3 |
| SDL2, Ninja | Absents |

Le simulateur ouvre une fenêtre Win32 sans dépendance (décision D5), et son mode sans tête sert aux tests. `scripts/setup` signale ce qui manque et n'installe rien.

## Scripts

`setup`, `build`, `format`, `lint`, `test`, `sim`, `package`, `verify`, `diag`. Aucun script automatisé ne flashe : seul `tools/flash/aethra_flash.py` le peut, sur confirmation humaine (voir `docs/flash.md`).
