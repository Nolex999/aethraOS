# État du projet

Dernière mise à jour : 2026-11-10. `sim` désigne un résultat en simulateur, `device` un résultat sur la calculatrice.

## Vérifié

| Élément | Preuve | Portée |
|---|---|---|
| Build hôte `-Wall -Wextra -Werror` | `scripts/build`, TDM-GCC 10.3.0 | `sim` |
| Build Cortex-M7 | `scripts/build arm-cortex-m7-release`, arm-none-eabi-gcc 10.3 | Compilation |
| App `.nwa` | `scripts/package` avec le vrai `eadk.h` de nwlink 0.0.19, liaison contrôlée par `nwa-elf` | Compilation et liaison |
| Tests automatiques | `scripts/test` : 5 suites sur 5 réussies, dont 27 tests du flasheur | `sim` |
| Rejeu déterministe | `test_replay` : 10 images de référence | `sim` |
| Couches respectées | `check_layers.py` : 0 violation | Code |
| Fenêtre Win32 | Lancée sans plantage pendant 4 s, rendu non contrôlé à l'œil | `sim` |
| Rendu boot et accueil | Contrôlé sur images extraites | `sim` |
| `detect` | N0120, Epsilon 26.4.0 dans les deux slots | `device` |
| `backup` | 8 Mio en 11 s, identiques octet pour octet au dump | `device` |
| `verify` | Zones protégées identiques à la sauvegarde | `device` |
| Simulation `install-nwa` | Préparation complète, aucune écriture | `device` |

## Mesures (Cortex-M7, `-Os`)

- App : 10 506 octets de code, 1 471 d'icône, fichier `.nwa` de 28 524 octets.
- RAM statique : 68 056 octets, dont l'arène de 64 Kio. Budget T1 : 153 436 octets.
- Pic d'arène pendant le boot : 10 176 octets.

## Non vérifié

- Aucune exécution d'AETHRA sur la calculatrice : l'installation écrase KhiCAS et exige une confirmation tapée par l'utilisateur.
- `install-nwa --write` et `restore-apps --write` n'ont pas été exécutés sur l'appareil.
- CI GitHub jamais lancée, mode ASan non exécuté, `cppcheck` absent.
- Temps d'image et RAM réelle sur l'appareil.

## Prochaine étape

Suivre `tests/hardware/first_device_test.md`.
