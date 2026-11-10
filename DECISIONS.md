# Décisions

| ID | Date | Décision | Raison |
|---|---|---|---|
| D1 | 2026-11-10 | AETHRA reste le nom de travail | Voir `docs/01_naming_identity.md` |
| D2 | 2026-11-10 | Premier objectif matériel : app `.nwa` (T1), pas de firmware privilégié | Chaîne de confiance signée, voir `docs/02_feasibility.md` |
| D3 | 2026-11-10 | Boucle d'événements coopérative, pas de RTOS | Aucun gain en mode non privilégié, voir `docs/04_architecture.md` |
| D4 | 2026-11-10 | Pas de framebuffer complet | 153 600 octets dépassent la RAM T1 |
| D5 | 2026-11-10 | Fenêtre du simulateur en Win32 plutôt qu'en SDL2 | SDL2 absent de la machine, aucune dépendance à installer. SDL2 reste possible pour Linux |
| D6 | 2026-11-10 | ESP32 en UART abandonné en T1 | La MPU interdit GPIO et UART aux apps. Connectivité reportée, voir `docs/02_feasibility.md` |
| D7 | 2026-11-10 | Le flasheur écrit uniquement dans la zone des apps, et délègue l'installation à `nwlink` | Risque borné : le format d'installation relève de l'outil officiel |
| D8 | 2026-11-10 | Bouton Retour pour quitter l'app en T1 | Home et On/Off appartiennent au système |
| D9 | 2026-11-10 | Police déplacée dans `gfx`, partagée avec l'animation | Une seule source, couches respectées |
