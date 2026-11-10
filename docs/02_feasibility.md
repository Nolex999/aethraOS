# 2. Faisabilité, boot et récupération

## Conclusion

Un environnement AETHRA complet est réalisable aujourd'hui **en simulateur** et **comme application externe (`.nwa`) sur Epsilon officiel**. Un firmware autonome qui possède le matériel n'est pas réalisable sans contourner la chaîne de confiance, ce que le brief interdit en l'état. Aucune méthode publique de firmware personnalisé pour le N0120 n'a été trouvée : le signalement le plus clair date de 2024 et doit être revérifié.

« Vrai OS » signifie donc, sur N0120 et à ce stade : un runtime complet en mode non privilégié, qui passe par le kernel NumWorks pour l'écran, le clavier et le temps.

## Paliers de déploiement

| Palier | Nature | Accès matériel | Survit à un reset | État |
|---|---|---|---|---|
| **T0** | Simulateur PC | Simulé | Oui | Cible immédiate |
| **T1** | App `.nwa` sur Epsilon officiel | API EADK, appels SVC | Non, les apps sont effacées au reset ou après un crash `[DOC]` | Premier objectif matériel |
| **T2** | Userland non signé dans un slot, kernel officiel conservé | SVC, mode dégradé, popup « UNOFFICIAL SOFTWARE » `[DUMP]` | À vérifier | Recherche, soumis à validation explicite |
| **T3** | Firmware privilégié autonome | Total | Oui | **Hors périmètre.** Exige de remplacer le bootloader et le kernel signés |

T1 ne remplace pas le vrai démarrage de la calculatrice : l'animation de boot jouera au lancement de l'app.

## Chaîne de démarrage

```text
Reset
  |
  v
Bootloader NumWorks, flash interne (source jamais publiée) [DOC]
  |  verifie la signature du slot A, puis du slot B
  |  aucun valide : DFU et ecran de secours [DOC]
  v
Kernel, privilegie, slot + 0x20 [DUMP]
  |  pilotes, MPU, appels SVC
  v
Userland Epsilon, non privilegie, slot + 0x20000 [DUMP]
  |
  +--> Applications Epsilon
  +--> Apps externes .nwa, zone 0x90180000-0x903F0000 [DUMP]  <-- AETHRA T1
```

## Ce que dit le dump

- Le kernel 26.4 contient trois messages structurants `[DUMP]` : « OFFICIAL UPGRADE REQUIRED », « THIRD PARTY APPLICATIONS » et « UNOFFICIAL SOFTWARE ». Il prévoit donc les apps tierces et un userland non officiel.
- Le texte précise que les apps tierces sont désactivées en mode examen, et effaçables par le bouton reset.
- 128 octets à forte entropie, différents entre les slots, suivent l'image `[DUMP]`. C'est compatible avec une signature `[HYP]`.

## Règles de sécurité non négociables

1. Aucune écriture en flash interne, aucun changement d'option bytes, aucun effacement de bootloader.
2. Aucune exploitation de faille pour obtenir le mode privilégié (la documentation mentionne le jailbreak Phi, qui visait le pilote d'écran : on ne s'en sert pas).
3. Le mode examen reste intact. AETHRA ne le désactive pas, ne le masque pas, ne simule pas son voyant.
4. Les scripts automatisés ne contiennent jamais de commande d'écriture DFU.
5. Le palier T2 exige un plan de récupération écrit, testé sur un appareil sacrifiable, et un accord explicite à chaque flash.

## Récupération

| Situation | Moyen | Confiance |
|---|---|---|
| App T1 défaillante | Reset ; l'app est effacée, Epsilon reste | `[DOC]` |
| Slot corrompu | Le bootloader tente l'autre slot | `[DOC]` |
| Aucun slot valide | Mode DFU, restauration d'Epsilon signé depuis numworks.com/rescue | `[DOC]`, combinaison de touches N0120 `[?]` |
| Bootloader endommagé | Aucun moyen connu | Cas à ne jamais provoquer |

Attention : le kernel réécrit le bootloader et un compteur en flash interne lors d'une mise à jour `[DUMP]`. Installer un Epsilon plus récent peut donc verrouiller davantage l'appareil et bloquer tout retour en arrière. Ne pas mettre à jour la calculatrice de développement.

## Format d'image proposé

Pour T1, le conteneur est le `.nwa` produit par `nwlink`. Pour les paquets internes d'applications AETHRA, un format `.aep` :

| Décalage | Taille | Champ |
|---|---|---|
| 0 | 4 | Magie `AEP1` |
| 4 | 2 | Version du format |
| 6 | 2 | Palier minimal requis |
| 8 | 4 | Taille de la charge |
| 12 | 4 | CRC32 de la charge |
| 16 | 32 | SHA-256 de la charge |
| 48 | 16 | Identifiant d'application |
| 64 | n | Charge |

Le CRC et le SHA-256 détectent la corruption. Ils **n'authentifient pas** l'auteur : une signature reste à concevoir si des paquets tiers apparaissent.

## Éditeur de liens et démarrage

- T1 : script fourni par la chaîne `nwlink`, code indépendant de la position, RAM limitée à la plage de l'en-tête userland.
- T2 `[HYP]` : image liée au slot + `0x20000`, avec un en-tête au format observé. Non implémenté.
- Exigences de démarrage communes : pas d'allocation avant l'initialisation des arènes, `.bss` nulle vérifiée, version imprimée au premier log, gestion explicite d'un retour d'application.

## Licences

| Composant | Licence | Décision |
|---|---|---|
| Epsilon (Poincaré, Escher, Ion) | La page du dépôt indique « All rights reserved » ; des sources plus anciennes citent CC BY-NC-SA | **Ne pas réutiliser** tant que le fichier de licence exact n'a pas été lu |
| Dump flash | Code propriétaire NumWorks, et données personnelles | **Jamais dans le dépôt** |
| MicroPython | MIT | Candidat |
| LittleFS | BSD-3 | Candidat |
| musl libm | MIT | Candidat pour les fonctions mathématiques |
| giac / KhiCAS | GPL | Seulement comme programme séparé, sinon contamination de licence |
| Spectrum (Pokémon) | Propriété de Nintendo, Game Freak et Creatures | Dessin de fan acceptable en usage personnel. Pour une version publique, remplacer `spectrum_sprite.h` par une créature originale |

## Matrice de compatibilité

| Modèle | T0 | T1 | T2 | T3 |
|---|---|---|---|---|
| N0120 (26.4.0) | Oui | À valider | Recherche | Non |
| N0120EX | Oui | `[?]` | `[?]` | Non |
| N0110, N0100 | Oui | Hors cible | Hors cible | Hors cible |

## Connectivité ESP32 : bloquée à ce stade

Le brief préfère l'UART. L'analyse du kernel l'exclut en T1 : la MPU n'ouvre aux apps ni les GPIO ni les UART, tout accès provoquerait une faute `[DUMP]`, et aucun pilote UART n'existe dans le code privilégié.

**La seule voie compatible avec T1 est l'USB**, dont les registres sont accessibles depuis une app `[DUMP]` :

| Option | Besoin | Problème |
|---|---|---|
| Calculatrice périphérique, module hôte (ex. ESP32-S3) | Pilote USB dans l'app, protocole maison | Autre puce que l'ESP32-C3 prévue. Le module doit avoir sa propre alimentation |
| Calculatrice hôte, ESP32-C3 périphérique | Mode hôte du contrôleur OTG, VBUS externe | Pilote complet à écrire, VBUS non fourni par la calculatrice, risque électrique à mesurer |

Dans les deux cas, reconfigurer l'USB pendant qu'Epsilon s'en sert peut entrer en conflit avec lui, et aucune de ces voies n'a été testée. Décision : l'ESP32-C3 en UART reste hors de portée tant que T3 l'est. La connectivité est reportée, avec un test de faisabilité USB avant toute conception.
