# 7. Inventaire des applications

| Identifiant | Application | Espace | Phase | Dépendances principales |
|---|---|---|---|---|
| `org.aethra.home` | Accueil | Tous | 3 | Shell |
| `org.aethra.launcher` | Lanceur | Tous | 3 | Framework |
| `org.aethra.settings` | Réglages | Tous | 3 | Services |
| `org.aethra.calc` | Calculatrice, historique, variables | Maths | 4 | Moteur maths |
| `org.aethra.graph` | Graphes, table de valeurs | Maths | 4 | Moteur maths, graphique |
| `org.aethra.solve` | Équations | Maths | 4 | Moteur maths |
| `org.aethra.stats` | Statistiques, probabilités | Maths | 4 | Moteur maths |
| `org.aethra.seq` | Suites | Maths | 4 | Moteur maths |
| `org.aethra.matrix` | Matrices, complexes | Maths | 4 | Moteur maths |
| `org.aethra.units` | Unités, constantes | Maths | 4 | Moteur maths |
| `org.aethra.files` | Fichiers | Productivité | 5 | VFS |
| `org.aethra.notes` | Notes | Productivité | 5 | VFS, éditeur |
| `org.aethra.term` | Terminal | Développement | 5 | VFS, commandes |
| `org.aethra.edit` | Éditeur de code | Développement | 7 | VFS |
| `org.aethra.python` | Python | Développement | 7 | MicroPython |
| `org.aethra.diag` | Diagnostics | Développement | 3 puis 8 | HAL, instrumentation |
| `org.aethra.net` | Réseau | Connectivité | 9 | **Bloqué** par le matériel |
| `org.aethra.ai` | Client IA distant | Connectivité | 10 | **Bloqué** par le réseau |

## Moteur mathématique

Les nombres décimaux et l'évaluateur doivent être testés sur les cas limites : infinis, NaN, domaines, arrondis, précision. Réutiliser `musl` pour les fonctions élémentaires et écrire l'analyseur d'expressions, petit et bien testé. Poincaré n'est pas réutilisable à ce jour. Le calcul formel n'est pas inclus au départ : KhiCAS existe déjà comme app séparée, sous GPL.

## Manifeste

```c
typedef struct {
    const char *id, *name, *version;
    const ae_app_ops *ops;
    uint32_t arena_bytes;
    uint16_t suspend_bytes;
    uint16_t permissions; /* AE_PERM_STORAGE, AE_PERM_NET ... */
} ae_manifest;
```

## Modèles d'applications

| Modèle | Sécurité | Mémoire | Décision |
|---|---|---|---|
| Liée statiquement | Code de confiance | Efficace | Phase initiale |
| Bytecode / Python | Bonne, limitée par l'interpréteur | Tas nécessaire | Phase 7, pour les apps tierces |
| Module natif chargé | Aucune isolation | Efficace, relocation à écrire | À étudier après l'audit |

## SDK

Documentation à fournir en phase 6 : manifeste, build, API UI, stockage, entrée, graphique, services, tests, format `.aep`, ABI, symboles de débogage, analyse de crash. Compilation croisée sur PC uniquement : un compilateur C complet sur l'appareil n'est pas un objectif.
