# 12. Stratégie de test

Un résultat en simulateur ne valide jamais le matériel. Les rapports distinguent `sim` et `device`.

| Niveau | Contenu | Outil | Fréquence |
|---|---|---|---|
| Unitaire | Arènes, files, évaluateur, analyseur de commandes, thèmes | Exécutable de test C sans dépendance | Chaque build |
| Dépendances | Une couche n'inclut pas une couche supérieure | Script sur les `#include` | Chaque build |
| Images de référence | Rendu sans tête comparé à des hachages | Simulateur | Chaque build |
| Intégration | Scénario de touches rejoué | Simulateur | Chaque build |
| Régression mathématique | Tables de cas, y compris limites | Tests générés et revus | Chaque build |
| Robustesse | Fuzz des entrées : expressions, fichiers, thèmes, paquets | Sanitizers sur l'hôte | Quotidien |
| Mémoire | Taille par section, pile, pic des arènes | Rapport de build | Chaque build ARM |
| Performance | Temps d'image, pixels poussés | Instrumentation, séparée par plateforme | Par version |
| Coupure | Écriture interrompue à chaque étape | VFS simulé | Stockage |
| Matériel | Liste manuelle dans `tests/hardware` | Humain | Par version |

## Déjà vérifié pour l'animation

| Vérification | Résultat |
|---|---|
| Compilation hôte `-Wall -Wextra` | Sans avertissement |
| Tests : déterminisme, ASCII imprimable, fin, chaînes hostiles, statut nul, framebuffer borné, progression fournie | Réussis |
| Cellules modifiées par image | Moyenne 166, pic 839 sur 1 272 |
| Compilation Cortex-M7 `-Os` | 5 293 octets, aucun symbole externe |
| Adaptateur EADK | Compilé contre un en-tête de substitution uniquement |
| Rendu | Contrôlé visuellement sur images extraites |
| Exécution sur N0120 | **Non faite** |

## Système et flasheur

Résultats à jour dans `STATUS.md`. Le flasheur est testé contre un faux `dfu-util` : allowlist, détection, sauvegarde, simulation, refus sans terminal, restauration, relecture, détection d'une atteinte aux zones protégées. Aucun test ne touche le matériel.

## Liste de contrôle sur l'appareil (phase 3b)

Installation par la voie officielle ; rendu et couleurs corrects ; lecture du clavier ; absence d'artefacts ; temps d'image mesuré ; RAM réellement disponible ; sortie propre vers Epsilon ; comportement au reset ; application désactivée en mode examen.
