# Premier test sur la calculatrice

Cette procédure est faite par toi, à la main. L'étape 2 écrase KhiCAS, qui reste récupérable avec `restore-apps`.

Déjà fait et vérifié : construction du `.nwa`, `detect`, `backup`, `verify`, simulation d'installation.

1. Mettre la calculatrice sur l'écran de connexion USB.
2. Depuis un terminal, dans `aethra/`, lancer l'installation et taper `ECRIRE` quand c'est demandé :

   ```bash
   python tools/flash/aethra_flash.py install-nwa build/nwa/aethra.nwa --backup ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin --write
   ```

3. Remplir le tableau, puis remettre la calculatrice sur l'écran USB pour la vérification.
4. Pour revenir en arrière :

   ```bash
   python tools/flash/aethra_flash.py restore-apps --backup ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin --write
   ```

## À noter

| Contrôle | Résultat |
|---|---|
| L'app apparaît dans le menu | |
| L'animation se joue, couleurs correctes | |
| Une touche passe l'animation | |
| Flèches et focus fonctionnent | |
| Toolbox change le thème | |
| Retour revient à Epsilon | |
| Durée approximative de l'animation (7 s attendues) | |
| Fluidité, artefacts éventuels | |
| Comportement après un reset | |
| App désactivée en mode examen | |

Reporter les résultats dans `HARDWARE_MATRIX.md`, puis dans `STATUS.md` avec l'étiquette `device`.
