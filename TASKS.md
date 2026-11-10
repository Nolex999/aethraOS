# Tâches

| ID | Description | Priorité | Dépend | Agent | Fichiers attendus | Vérification | Acceptation | État |
|---|---|---|---|---|---|---|---|---|
| P0-1 | Analyse IDA du kernel | Haute | - | hardware | `docs/03_hardware.md` | Revue des faits | Faits sourcés | Fait |
| P1-1 | Dépôt et build | Haute | - | firmware | Racine | `scripts/test` | Tests verts | Fait (`sim`) |
| P1-2 | Intégrer l'animation | Haute | P1-1 | graphics | `src/boot_anim` | Images de référence | Rendu stable | Fait (`sim`) |
| P1-3 | HAL et simulateur | Haute | P1-1 | firmware | `src/hal`, `src/platform/sim` | `test_replay` | Rejeu déterministe | Fait (`sim`) |
| P1-4 | Écran d'accueil | Moyenne | P1-3 | graphics | `src/shell` | Images de référence | Focus et thèmes | Fait (`sim`) |
| P1-5 | Flasheur sécurisé | Haute | P0-1 | firmware | `tools/flash` | `test_flash`, essais en lecture sur l'appareil | Refus hors zone d'apps | Lecture vérifiée (`device`), écriture testée contre un faux `dfu-util` |
| P3b-1 | Construire le `.nwa` | Haute | P1-4 | firmware | `build/nwa` | `scripts/package`, `nwa-elf` | Fichier produit | Fait |
| P3b-2 | Premier test sur N0120 | Haute | P3b-1 | testing | `HARDWARE_MATRIX.md` | `tests/hardware/first_device_test.md` | Liste remplie | À faire, par l'utilisateur |
| P2-1 | Cycle de vie des applications | Haute | P3b-2 | application | `src/framework` | Tests unitaires | Création, suspension, reprise | À faire |
| P4-1 | Évaluateur mathématique | Haute | P2-1 | mathematics | `src/apps/math` | Régression | Cas limites | À faire |
| P5-1 | Terminal | Moyenne | P2-1 | application | `src/apps/term` | Tests | Commandes de base | À faire |
| C-1 | Faisabilité d'un lien USB | Basse | P3b-2 | connectivity | `docs/connectivity` | Test sur appareil | Conclusion écrite | À faire |

## Dette technique

- La batterie reste affichée `--` : l'unité de `eadk_battery_level` n'est pas établie.
- La CI GitHub n'a jamais tourné.
- Le mode ASan n'a pas été exécuté : non disponible avec TDM-GCC.
- `clang-format` et `cppcheck` sont absents de la machine.

## Blocages

- Connectivité ESP32 : voir `docs/02_feasibility.md`.
