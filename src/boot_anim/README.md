# Animation de boot

Aperçu : `preview/boot_preview.gif`. Dans un terminal en couleurs vraies d'au moins 53 x 24 caractères : `build/host-debug/boot_anim_host` après `scripts/build`.

| Fichier | Rôle |
|---|---|
| `boot_anim.c/.h` | Moteur portable, sans flottant ni allocation |
| `spectrum_sprite.h` | Dessin du personnage, remplaçable |
| `host/host_main.c` | Lecture terminal et export d'images |

`src/shell/ae_system.c` la lance au démarrage et lui fournit les vraies étapes d'initialisation. Les tests sont dans `tests/unit/test_boot_anim.c`.
