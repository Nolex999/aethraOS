# AETHRA

Environnement d'exploitation expérimental pour la calculatrice NumWorks N0120 (STM32H725, Cortex-M7).

**État : premier jalon en simulateur. Rien n'a encore tourné sur une calculatrice.** Voir [STATUS.md](STATUS.md).

AETHRA vise d'abord une application externe sur Epsilon officiel. Il ne remplace ni le bootloader ni le kernel signés, et ne touche pas au mode examen. Les raisons sont dans [docs/02_feasibility.md](docs/02_feasibility.md).

## Démarrer

```bash
scripts/setup
```

```bash
scripts/test
```

```bash
scripts/sim
```

Touches du simulateur : flèches, Entrée pour OK, Échap pour Retour, Tab pour Toolbox (change le thème).

## Installer sur la calculatrice

Voir [docs/flash.md](docs/flash.md). Le flasheur écrit uniquement dans la zone des apps externes.

## Documentation

Tout le dossier de conception est dans [docs/](docs/README.md).

## Licence

Aucune licence n'a encore été choisie : voir [LICENSE](LICENSE).
