# 10. Feuille de route

« Fini » signifie vérifié selon la méthode indiquée. Du code généré n'est pas une fonctionnalité terminée.

| Phase | Objectif | Livrable vérifiable | Validation | Prérequis |
|---|---|---|---|---|
| 0 | Recherche | Ce dossier, analyse IDA complétée (PLL, MPU), matrice matérielle | Faits sourcés | Dump |
| 1 | Outils et simulateur | Dépôt, build, simulateur 320 x 240, clavier simulé, animation intégrée, tests | CI, capture d'écran | Phase 0 |
| 2 | Runtime | Arènes, événements, minuteries, journal, cycle de vie | Tests unitaires | 1 |
| 3 | Graphique et shell | Widgets, accueil, lanceur, réglages, thèmes, espaces | Images de référence | 2 |
| **3b** | **Première app T1** | `.nwa` minimale : animation, clavier, écran, mesure de RAM et de temps d'image | **Test manuel sur N0120 par l'utilisateur** | 2, partie de 3 |
| 4 | Calcul | Évaluateur, historique, variables, graphes, statistiques | Régression mathématique | 3 |
| 5 | Stockage et terminal | VFS, fichiers, terminal, configuration, journaux | Tests, coupure simulée | 3 |
| 6 | SDK | API, exemples, paquets, documentation | Exemple compilé depuis zéro | 5 |
| 7 | Python et édition | REPL, éditeur, scripts, erreurs | Tests, mesure du tas | 5 |
| 8 | Intégration matérielle | Pilotes T1 validés, alimentation, déploiement sûr | Matrice remplie à la main | 3b |
| 9 | ESP32 | Protocole UART, Wi-Fi, HTTPS | Validation électrique | **Bloquée** : accès aux broches à prouver |
| 10 | IA et avancé | Client distant en streaming | Tests réseau | 9 |
| 11 | Optimisation et publication | Profils, batterie, fiabilité, paquets | Mesures réelles | Selon l'avancement |

La phase 3b est ajoutée au brief : tester tôt sur l'appareil évite de construire tout le shell sur des hypothèses de RAM et de débit.

## Première tâches

| ID | Description | Priorité | Dépend | Agent | Fichiers | Vérification | Acceptation | État |
|---|---|---|---|---|---|---|---|---|
| P0-1 | Rebaser l'IDB, analyser le kernel | Haute | Aucune | hardware | `docs/hardware` | Revue des faits | PLL, MPU et câblage documentés ou marqués inconnus | Fait |
| P1-1 | Créer le dépôt et la CI | Haute | Aucune | firmware | Racine | Build propre | `build` et `test` verts | Fait (`sim`), CI jamais exécutée |
| P1-2 | Intégrer `boot_anim` | Haute | P1-1 | graphics | `src/boot_anim` | Tests et image de référence | Rendu identique | Fait (`sim`) |
| P1-3 | HAL et plateforme simulateur | Haute | P1-1 | firmware | `src/hal`, `src/platform/sim` | Mode sans tête | Écran et clavier simulés | Fait (`sim`) |
| P1-4 | Écran de démonstration | Moyenne | P1-3 | graphics | `src/shell` | Image de référence | Texte et rectangles corrects | Fait (`sim`) |
| P3b-1 | Construire le `.nwa` | Haute | P1-2 | firmware | `src/platform/eadk` | Test par l'utilisateur | L'animation tourne sur N0120 | À faire |

`TASKS.md` reprend ce format pour chaque nouvelle tâche, avec la dette technique et les blocages dans des sections dédiées.
