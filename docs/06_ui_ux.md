# 6. Spécification UI/UX

## Principes

Écran de 320 x 240, sans tactile. Chaque écran a un seul élément focalisé et une action évidente sur OK. L'information prime sur le décor. Rien ne bouge au repos, pour la batterie.

## Structure d'écran

```text
+--------------------------------------------------+
| Barre d'etat 18 px : espace, heure, activite, batt|
+--------------------------------------------------+
|                                                  |
| Zone de contenu 204 px                           |
|                                                  |
+--------------------------------------------------+
| Barre d'actions 18 px : indications de touches   |
+--------------------------------------------------+
```

## Éléments

| Élément | Comportement |
|---|---|
| Accueil | Espace courant, 6 tuiles, calcul rapide, état système réel |
| Lanceur | Grille 4 x 3 filtrable à la frappe |
| Barre d'état | Modules configurables. Un état inconnu s'affiche `--`, jamais une valeur inventée |
| Centre de notifications | Liste, 16 entrées maximum |
| Réglages rapides | Thème, luminosité, veille, animation |
| Réglages | Listes par catégorie, avec aperçu |
| Fichiers | Liste, métadonnées, actions contextuelles |
| Espaces | Bande de 4 cartes, changement direct |
| Recherche | Apps, fichiers, commandes, réglages |
| Menus contextuels | Liste flottante sur la touche `toolbox` |
| Dialogues | Message court, 2 boutons au plus, focus sur le choix non destructif |
| Toasts | 2 secondes, sans vol de focus |
| Progression | Barre réelle, ou activité indéterminée si la durée est inconnue |
| Listes, onglets, formulaires | Défilement par cellule, focus visible |
| Clavier virtuel | Uniquement pour les caractères absents du clavier physique |
| Changement d'app | Liste des récentes |
| Récupération | Message, code d'erreur, options relancer, journal, quitter |

## Clavier

Les touches Home et On/Off sont gérées par le système en T1 et ne sont pas redéfinissables de façon sûre `[DOC]`. Le menu global d'AETHRA passe donc par une autre touche.

| Touche | Action |
|---|---|
| Flèches, OK, Retour | Navigation, validation, retour |
| `var` | Menu global d'AETHRA |
| `shift` puis 1 à 4 | Aller à l'espace 1 à 4 |
| `shift` puis `var` | Liste des applications récentes |
| `alpha` puis lettre | Saisie alphabétique |
| `toolbox` | Menu contextuel |
| Appui long (500 ms) sur OK | Actions secondaires |
| `shift` puis Retour | Réglages rapides |
| Toute touche pendant le boot | Passe l'animation |

**Accessibilité :** aucun raccourci à accord obligatoire, puisque `shift` et `alpha` sont séquentiels ; thème à fort contraste ; animations désactivables ; taille de police agrandie.

## Thèmes

Un thème est une table de jetons et de polices, chargée depuis des données et validée. En cas d'erreur, retour au thème par défaut.

| Thème | Surface | Encre | Accent | Style |
|---|---|---|---|---|
| AETHRA Default | Gris neutre très sombre | Blanc cassé | Violet discret | Compact, scientifique |
| Obsidian | Noir | Gris clair | Monochrome | Développeur |
| Arctic | Blanc | Presque noir | Bleu froid | Contraste élevé |
| Spectra | Sombre | Blanc | Dégradés, en surfaces réduites | Expressif |
| Terminal | Noir | Vert ou ambre | Une seule teinte | Dense, monospace |

Personnalisable : accents, fonds, préréglages typographiques, packs d'icônes, intensité d'animation, dispositions accueil et lanceur, modules de barre d'état, terminal, raccourcis, accessibilité. Stockage compact dans un seul enregistrement versionné, avec aperçu avant application.

## Animation de boot

| Temps | Séquence |
|---|---|
| 0 à 0,6 s | Brouillard, invite `> aethra: cold start` |
| 0,6 à 1,3 s | Deux yeux s'ouvrent dans le noir |
| 1,3 à 2,6 s | Spectrum se matérialise, gaz ascendant |
| 2,2 à 3,1 s | Les mains arrivent des bords |
| 3,3 à 4,6 s | Ball'Ombre chargée devant la bouche, pupilles rouges |
| 4,6 à 5,5 s | Explosion, le logo se décode |
| 5,6 à 6,7 s | Sourire au-dessus du logo, clin d'œil, barre de progression |
| 6,7 à 7 s | Fondu |

La barre affiche uniquement les valeurs fournies par l'appelant. N'importe quelle touche passe l'animation.
