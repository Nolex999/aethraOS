# Flasheur AETHRA

`tools/flash/aethra_flash.py` installe AETHRA et sauvegarde la calculatrice. Il prend en charge uniquement Epsilon 26.4.0 sur N0120, et s'arrête s'il voit autre chose.

**Sur la vraie calculatrice, `detect`, `backup`, `verify` et la simulation d'installation fonctionnent.** Aucune écriture réelle n'a encore été faite : `install-nwa --write` et `restore-apps --write` n'ont été testés que contre un faux `dfu-util`, et elles relèvent de ta décision.

## Ce qu'il peut et ne peut pas faire

| Zone | Accès |
|---|---|
| Zones d'apps `0x90180000-0x903F0000` et `0x90580000-0x907F0000` | Écriture possible, avec confirmation |
| Kernel, userland, signatures | Lecture et vérification seulement |
| Flash interne, option bytes, bootloader | Jamais |

Il n'installe pas de firmware complet : la chaîne de confiance l'interdit, et ce serait hors de portée sans risque de blocage définitif.

## Protections

1. Toutes les écritures passent par une seule fonction, qui refuse toute adresse hors des zones d'apps.
2. Sans `--write`, tout est simulé.
3. Une vraie écriture exige un terminal interactif et de taper `ECRIRE`.
4. Une sauvegarde complète, avec empreinte valide, est obligatoire.
5. La calculatrice branchée doit correspondre à la sauvegarde.
6. Après écriture : relecture, et vérification que les zones protégées n'ont pas changé.

## Utilisation

Mets la calculatrice sur l'écran de connexion USB, puis :

```bash
python tools/flash/aethra_flash.py detect
```

```bash
python tools/flash/aethra_flash.py backup --out ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin
```

Cette sauvegarde existe déjà : elle est identique octet pour octet à ton dump.

```bash
python tools/flash/aethra_flash.py install-nwa build/nwa/aethra.nwa --backup ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin
```

Cette dernière commande est une simulation. Ajoute `--write` pour installer réellement.

**L'installation écrase KhiCAS** : `nwlink` écrit l'app au début de la zone des apps, puis un secteur vide. Après l'écriture, la calculatrice quitte le mode USB : le flasheur attend 90 secondes qu'elle revienne sur l'écran de connexion pour vérifier les zones protégées. Sinon, lance `verify` plus tard.

Pour vérifier l'intégrité ou annuler une installation :

```bash
python tools/flash/aethra_flash.py verify --backup ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin
```

```bash
python tools/flash/aethra_flash.py restore-apps --backup ../../dfu-util-0.11-binaries/win64/sauvegarde_n0120.bin --write
```

`restore-apps` remet KhiCAS et tes scripts tels qu'ils étaient dans la sauvegarde.

## Limites

- L'installation est déléguée à `nwlink` 0.0.19, l'outil de NumWorks. `npx` le télécharge au premier usage. Une version 1.0.0 existe et n'a pas été essayée.
- Le flasheur vérifie après coup que `nwlink` n'a touché à aucune zone protégée, il ne peut pas l'en empêcher.
- Ne mets pas Epsilon à jour : le kernel peut alors modifier la flash interne de façon définitive, et le flasheur refusera toute version autre que 26.4.0.
