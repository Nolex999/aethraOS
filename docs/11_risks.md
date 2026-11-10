# 11. Registre des risques

| ID | Risque | Probabilité | Impact | Mesure | Responsable |
|---|---|---|---|---|---|
| R1 | Calculatrice bloquée par une écriture | Faible si les règles sont suivies | Critique | Aucune écriture automatisée, T3 hors périmètre, plan de récupération pour T2 | security |
| R2 | Une mise à jour d'Epsilon casse l'approche ou verrouille plus | Moyenne | Élevé | Rester en 26.4.0 pendant le développement, relire la documentation avant toute mise à jour | hardware |
| R3 | ESP32 en UART irréalisable en T1 : la MPU interdit GPIO et UART aux apps. **Risque confirmé** | Certaine | Élevé | Connectivité reportée, test de faisabilité USB avant toute conception | connectivity |
| R4 | Mémoire T1 insuffisante | Moyenne | Élevé | Budget strict, mesure à chaque build, jalon 3b tôt | firmware |
| R5 | Stockage T1 trop petit (43 Ko) ou instable | Élevée | Moyen | Format compact, données critiques minimales, adaptateur isolé | application |
| R6 | App effacée après un crash ou un reset | Élevée | Moyen | Documenté à l'utilisateur, sauvegarde par USB à prévoir | application |
| R7 | Débit d'écran trop faible pour les effets | Moyenne | Moyen | Mesure en 3b, qualité adaptative | graphics |
| R8 | Réutilisation illégale de code Epsilon | Moyenne | Élevé | Pas de copie, licence à relire, dump exclu du dépôt | security |
| R9 | Publication du personnage Spectrum | Moyenne en cas de diffusion | Moyen | Sprite isolé, à remplacer dans une version publique | documentation |
| R10 | Faits matériels erronés issus d'une analyse partielle | Moyenne | Moyen | Étiquettes de confiance, mesures | hardware |
| R11 | Perte des données actuelles (KhiCAS, scripts) | Moyenne | Moyen | Conserver le dump, sauvegarder avant d'installer une app | utilisateur |
| R12 | Dérive du périmètre | Élevée | Moyen | Phases validées une à une, pas de phase en parallèle sans raison | architecture |
| R13 | Absence d'isolation : une app corrompt le système | Moyenne | Moyen | Revue mémoire, fuzz, watchdog, ne rien promettre | security |
| R14 | Atteinte à l'intégrité des examens | Faible | Élevé | Mode examen respecté, désactivation par le système conservée | security |

## Blocages actuels

- Connectivité : voir R3.
- Construction du `.nwa` : nécessite `nwlink` (npm), non installé.
- Validation sur appareil : à faire par l'utilisateur.
