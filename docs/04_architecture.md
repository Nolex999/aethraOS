# 4. Architecture système

## Vocabulaire

| Terme | Sens dans AETHRA |
|---|---|
| Bootloader | Code NumWorks en flash interne. AETHRA ne le remplace pas |
| Noyau RTOS | Absent en T0, T1 et T2 |
| Runtime | Boucle d'événements, minuteries, arènes, journal, crashs |
| Framework d'applications | Cycle de vie, manifestes, budgets |
| Shell graphique | Accueil, lanceur, barre d'état, espaces de travail |

## Couches

Une couche ne dépend que des couches inférieures. La règle est vérifiée par un test de dépendances.

```text
12 Apps integrees     calc, graph, term, files, edit, settings, diag
11 Shell              accueil, lanceur, espaces, notifications
10 Services systeme   reglages, themes, journal, ressources
 9 Framework apps     manifeste, cycle de vie, budgets
 8 Stockage           VFS, adaptateurs par palier
 7 Entree             evenements, repetition, focus
 6 Graphique          primitives, texte, rectangles sales, widgets
 5 Evenements         file, minuteries, boucle
 4 Memoire            arenes, pools, statistiques
 3 Runtime            erreurs, assertions, journal, crash
 2 HAL                ecran, clavier, temps, stockage, alimentation
 1 Plateformes        sim | eadk | (t2 recherche)
```

La connectivité et les outils de développement sont des modules optionnels, branchés sur les services.

## Choix du modèle d'exécution

| Critère | Boucle nue | Coopératif | FreeRTOS |
|---|---|---|---|
| RAM | Minimale | Faible (un contexte par tâche, partagé ici) | Une pile par tâche, plusieurs Ko chacune |
| Interruptions | Non disponibles en T1 | Non disponibles en T1 | Impossibles sans mode privilégié |
| Latence | Bornée si les handlers sont courts | Bornée de même | Meilleure, mais inaccessible |
| Consommation | Bonne | Bonne | Bonne |
| Maintenance | Simple, se complique vite | Simple | Lourde |
| Isolation | Aucune | Aucune | Aucune sans MPU privilégiée |

**Décision :** boucle d'événements coopérative, avec des tâches découpées en étapes courtes. Un RTOS préemptif n'apporte rien en mode non privilégié et coûte de la RAM. À réévaluer seulement si T3 devenait légitime.

## Contrats de base

- Démarrage déterministe : arènes, journal, HAL, services, shell, avec animation alimentée par les vraies étapes.
- Allocation : arènes par application, libérées d'un bloc à la sortie. Pas de `malloc` général.
- Événements : file circulaire de taille fixe, débordement compté et visible dans `diagnostics`.
- Minuteries : table fixe, résolution de la boucle, aucune garantie temps réel.
- Cycle de vie : `create`, `resume`, `event`, `render`, `suspend`, `destroy`.
- Budget temps : un gestionnaire au-delà du seuil est journalisé. Un watchdog logiciel coopératif affiche l'écran de récupération.
- Erreurs : codes explicites, pas d'exception, assertions actives en simulateur.
- Crash : en T1, journal en RAM et écran de récupération si possible. La sauvegarde durable reste à valider, un crash pouvant effacer l'app `[DOC]`.
- Basse consommation : attente via `eadk_timing_msleep`, pas d'animation au repos.

**Limite assumée :** aucune isolation entre applications natives. Elles partagent un espace d'adressage. Ne pas la présenter comme une protection de processus.

## HAL (interface minimale)

L'interface réelle est dans `src/hal/ae_hal.h` : écran (`display_push`, `display_fill`), clavier, temps, attente, batterie (qui peut renvoyer « inconnu ») et demande d'arrêt. Le stockage sera ajouté en phase 5.

## Moteur graphique

- Pas de framebuffer complet : 153 600 octets dépassent les 153 436 disponibles en T1.
- Rendu par rectangles sales et bande de 16 lignes (10 Ko).
- Texte en cellules, avec invalidation au niveau de la cellule (déjà utilisée par l'animation).
- Dégradés limités aux surfaces réduites et mis en cache en flash si possible.
- Qualité adaptative : si une image dépasse le budget, désactivation des transitions.
- Instrumentation : temps par image et nombre de pixels poussés, affichés dans `diagnostics`.

Temps d'image : **non mesuré**. Cible : 33 ms pour une mise à jour partielle. Le débit du bus FMC est à mesurer sur l'appareil. Coût mesuré du boot en cellules : 166 en moyenne par image, pic de 839 sur 1 272.

## Stockage

| Chemin logique | T0 | T1 |
|---|---|---|
| `/system`, `/apps` | Répertoire hôte, lecture seule | Données liées dans l'app, lecture seule |
| `/user/**` | Répertoire hôte | Enregistrements du stockage Epsilon, 43 Ko au total |
| `/user/config` | Répertoire hôte | Un enregistrement compact |
| `/cache`, `/temp` | Répertoire hôte | RAM, perdus à la sortie |
| `/logs` | Répertoire hôte | Anneau RAM de 2 Ko |

LittleFS n'est utile que sur de la flash brute, à laquelle T1 n'a pas accès d'après ce qu'on sait `[HYP]`. Il est prévu pour T0 afin de valider le code, et pour T2 si l'écriture s'avère possible. L'accès au stockage depuis une app n'est pas officiellement supporté `[DOC]` : l'adaptateur doit être isolé et testé, et les écritures rester rares pour ménager les données de l'utilisateur.

## Terminal

Analyseur, historique, complétion, alias, variables, répertoire courant, codes de retour, scripts. Tubes uniquement si la mémoire le permet. Chaque commande affiche clairement ce qu'elle ne peut pas faire : `wifi` répond « non disponible » tant que le matériel n'existe pas. Ce n'est pas POSIX, et la documentation le dit.

API de commande stable :

```c
typedef int (*ae_cmd_fn)(ae_term *t, int argc, const char *const *argv);
typedef struct { const char *name, *help; ae_cmd_fn fn; uint8_t perms; } ae_cmd;
```

## Python

MicroPython (MIT) est le candidat. Jalon minimal : REPL, `math`, interruption par touche, rapport mémoire, tas de 48 Ko pris dans l'arène active. Aucune compatibilité CPython générale n'est promise.

## Sécurité

| Menace | Mesure | Limite |
|---|---|---|
| Fichier corrompu | Validation de taille et CRC avant usage | Pas d'authentification |
| Débordement de tampon | Longueurs bornées, fuzz en simulateur | C reste non sûr |
| Application fautive | Budgets, watchdog coopératif | Pas d'isolation |
| Clés API | Jamais dans le firmware, stockage après consentement | Stockage non chiffré en T1 |
| Envoi de données à une IA | Consentement explicite par envoi | Dépend de la connectivité, bloquée |
