# 9. Agents Claude Code

Chaque agent devient un fichier `.claude/agents/<nom>.md`. Règles communes : ne jamais annoncer un test non exécuté, ne pas modifier une interface publique sans décision enregistrée dans `DECISIONS.md`, et s'arrêter si l'acceptation exige un accès au matériel.

| Agent | Responsabilités | Fichiers autorisés | Livrables | Revue | Dépend de | Acceptation | Escalade |
|---|---|---|---|---|---|---|---|
| `architecture` | Interfaces, règles de dépendance, décisions | `docs/architecture`, `docs/decisions`, `DECISIONS.md`, en-têtes publics | Décisions, contrats | Sécurité pour tout ce qui touche au boot | Matériel | Test de dépendances vert | Conflit entre agents, changement d'ABI |
| `hardware` | Recherche STM32H725 et N0120, analyse du dump | `docs/hardware`, `docs/research`, `HARDWARE_MATRIX.md` | Faits étiquetés | Architecture | Aucun | Chaque fait porte une source et une confiance | Fait en contradiction, besoin d'une mesure |
| `firmware` | HAL, plateformes, démarrage, runtime | `src/core`, `src/hal`, `src/platform` | Pilotes, runtime | Sécurité, Test | Architecture, Matériel | Tests verts, pile et RAM mesurées | Toute action écrivant sur l'appareil |
| `graphics` | Rendu, widgets, thèmes, animation | `src/gfx`, `src/boot_anim`, `assets` | Moteur, composants | Test | Firmware | Tests d'images de référence verts, budget tenu | Budget dépassé |
| `mathematics` | Moteur et applications de calcul | `src/apps/math*`, `tests/regression/math` | Moteur, tests | Test | Graphics | Cas limites verts, licences vérifiées | Algorithme à forte complexité |
| `application` | Framework, shell, apps, SDK | `src/framework`, `src/shell`, `src/apps`, `sdk` | Apps, SDK | Architecture, Test | Graphics, Firmware | Cycle de vie testé, budgets respectés | Besoin d'une nouvelle API système |
| `connectivity` | Protocole et firmware ESP32 | `esp32`, `docs/connectivity` | Protocole, validation électrique | Sécurité, Matériel | Matériel | Tests du protocole verts, mesure électrique faite | Aucun branchement sans mesure validée |
| `security` | Mémoire, mises à jour, secrets, récupération | Lecture partout, écriture dans `docs/security` | Audits, modèle de menaces | Humain | Tous | Constats classés et suivis | Risque d'appareil bloqué, secret exposé |
| `testing` | Tests, simulateur, performances | `tests`, `.github/workflows` | Suites, rapports | Architecture | Tous | CI verte, rapports reproductibles | Test instable non expliqué |
| `documentation` | Documentation et guides | `docs`, `README.md`, `sdk/**/*.md` | Guides à jour | Agent propriétaire du sujet | Tous | Liens valides, commandes vérifiées | Documentation contredisant le code |

## Coordination

1. Une tâche appartient à un seul agent à la fois (`TASKS.md`).
2. Une modification d'interface passe d'abord par `architecture`.
3. `STATUS.md` n'accepte « Terminé » que si la méthode de vérification a réellement été exécutée, résultat à l'appui.
4. Deux agents ne modifient jamais le même fichier en parallèle.
5. Les validations matérielles portent le statut « non validé sur matériel » tant qu'un humain ne les a pas réalisées.
