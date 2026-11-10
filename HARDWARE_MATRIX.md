# Matrice matérielle

Chaque ligne indique ce qui a été réellement validé. Les détails sont dans `docs/03_hardware.md`.

| Élément | Simulateur | N0120, Epsilon 26.4.0 |
|---|---|---|
| Boot animé | Vérifié (`sim`) | Non testé |
| Écran d'accueil, focus, thèmes | Vérifié (`sim`) | Non testé |
| Lecture clavier via EADK | Simulé | Non testé |
| Temps d'image | Non pertinent | Non mesuré |
| RAM réellement disponible | Non pertinent | Non mesurée |
| Sortie vers Epsilon (touche Retour) | Non pertinent | Non testée |
| Désactivation en mode examen | Non pertinent | Non testée |
| Flasheur : detect | Testé contre un faux `dfu-util` | **Vérifié** : N0120, 26.4.0 dans les deux slots |
| Flasheur : backup, verify | Testé contre un faux `dfu-util` | **Vérifié** : sauvegarde identique au dump |
| Flasheur : install-nwa, restore-apps | Testé contre un faux `dfu-util` | Non testé |
