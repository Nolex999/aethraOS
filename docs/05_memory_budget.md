# 5. Budget mémoire

## RAM en T1

Disponible d'après l'en-tête userland : 153 436 octets, soit 149,8 Kio `[DUMP]`. L'attribution exacte de cette plage à une app reste à confirmer sur l'appareil, et la pile peut être située ailleurs `[?]`.

| Poste | Kio | Remarque |
|---|---:|---|
| Piles | 8 | À mesurer par remplissage de motif |
| Runtime et services | 8 | |
| Bande graphique | 12 | 16 lignes en RGB565, plus l'état |
| Files et minuteries | 2 | |
| Arène de l'application active | 64 | Python y prend son tas de 48 Kio |
| Pool d'états suspendus | 16 | |
| Cache de stockage | 8 | |
| Journal | 2 | Anneau |
| Réserve | 29,8 | Ne pas consommer sans décision |
| **Total** | **149,8** | |

Mesuré sur la cible Cortex-M7 : 68 056 octets de `.bss` pour tout le système, dont l'arène de 64 Kio. L'animation y prend 10 176 octets (deux grilles), libérés avant le shell.

## Flash en T1

Zone des apps : 2 496 Kio `[DUMP]`, déjà occupée en partie par KhiCAS sur ta calculatrice.

| Poste | Kio |
|---|---:|
| Noyau, services, shell | 180 |
| Calcul, graphes, statistiques | 300 |
| MicroPython | 280 |
| Polices, icônes, thèmes | 120 |
| Terminal, fichiers, éditeur | 100 |
| Marge | Le reste |

Ces valeurs sont des objectifs à confirmer par le rapport de taille de chaque build. Mesuré à ce jour : 9,5 Ko de code ARM Cortex-M7 pour le runtime, le graphique, l'animation et l'accueil.

## Espaces de travail

Un espace est un état sauvegardé, pas un groupe de processus. Une seule application tourne à la fois.

| Règle | Valeur |
|---|---|
| Applications actives | 1 |
| État suspendu par application | 4 Kio maximum |
| Applications suspendues | 4 maximum |
| Éviction | La moins récemment utilisée, sauf épinglage |
| Application sans état sérialisable | Fermée au changement |
| Espaces | 4, description de 256 octets chacun |

Une application évincée reprend depuis son état sauvegardé. Une tâche de fond ne continue que si elle est découpée en pas coopératifs et déclarée dans le manifeste.

## Si T2 devenait possible

La RAM jusqu'à la pile du userland représente 258 040 octets `[DUMP]`, mais Epsilon en occupe une partie. Un framebuffer complet y tiendrait, sans être souhaitable : il prendrait plus de la moitié de l'espace.
