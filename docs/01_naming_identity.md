# 1. Noms et identité

Aucune disponibilité de marque ou de domaine n'a été vérifiée. La colonne « Conflits » vient de connaissances générales et doit être contrôlée avant toute publication.

| Nom | Origine et sens | Prononciation | Identité visuelle | Conflits à vérifier |
|---|---|---|---|---|
| **AETHRA** | Grec *aithra*, ciel clair ; nymphe de la mythologie | Aé-tra | Le A en pointe, déjà dessiné | Usages commerciaux possibles |
| UMBRA | Latin, ombre ; cône d'ombre d'une éclipse | Oum-bra | Très cohérent avec Spectrum | Nom très répandu |
| NOEMA | Grec, objet de pensée | No-é-ma | Sobre, typographique | Magazine et produits existants |
| LEMMA | Résultat intermédiaire en mathématiques | Lé-ma | Symbole ∴ | Terme courant, outils de preuve |
| NABLA | Opérateur ∇ | Na-bla | Triangle inversé, très fort | Sociétés d'IA de ce nom |
| APSIS | Point extrême d'une orbite | Ap-sis | Ellipse et point | Faible à vérifier |
| SYZYGY | Alignement de trois astres | Si-zi-ji | Trois points alignés | Agence et jeux de ce nom |
| PHASOR | Vecteur tournant en physique | Fa-zor | Flèche en rotation | Évoque Star Trek |
| EIDOS | Grec, forme, idée | É-dos | Formes pures | Éditeur de jeux connu |
| ERGON | Grec, travail | Er-gon | Unité de travail | Produits divers |
| ORRERY | Modèle mécanique du système solaire | O-re-ri | Cercles concentriques | Faible à vérifier |
| VESPER | Latin, étoile du soir | Ves-per | Astre unique | Noms de projets nombreux |
| QUANTA | Pluriel de quantum | Kwan-ta | Grille discrète | Société informatique connue |
| PERIGEE | Point le plus proche d'une orbite | Pé-ri-jé | Arc tangent | Plusieurs entreprises |
| LACUNA | Latin, vide, manque | La-cou-na | Cercle ouvert | Logiciels et films |

**Recommandation :** garder AETHRA. Il tient en 6 lettres, marche comme nom de dépôt et de terminal (`aethra>`), et n'évoque ni un OS commercial ni Epsilon de trop près. UMBRA est le meilleur second choix si l'univers fantôme devient central.

## Système d'identité

| Élément | Décision |
|---|---|
| Logo | Mot AETHRA en blocs ASCII, les deux A dessinés comme une couronne à pointes |
| Animation de boot | Spectrum en ASCII : yeux, matérialisation, mains, Ball'Ombre, explosion qui écrit le logo, sourire et clin d'œil. Implémentée dans `src/boot_anim` |
| Splash statique | Dernière image de l'animation, sans le sourire |
| Typographie | Bitmap 5x7 en cellule 6x10 pour le terminal (53 x 24), 8x12 pour les titres |
| Icônes | Grille 24x24, deux couleurs par icône, trait de 2 px, coins nets |
| Couleurs | Jetons `surface`, `surface_alt`, `ink`, `ink_muted`, `accent`, `danger`, `ok`, définis par thème |
| Bannière terminal | `AETHRA <version> [<tier>] on <device>`, suivie de la mémoire libre réelle |
| Versions | `MAJEUR.MINEUR.CORRECTIF-tN`, le suffixe indiquant le palier de déploiement (`0.1.0-t0`). Les noms de publication utilisent l'alphabet grec, pas des noms de Pokémon |
| Applications | Identifiants inversés `org.aethra.calc`, nom affiché court d'un seul mot |
| Documentation | Français pour les décisions de conception, anglais pour l'API et le code. Faits étiquetés selon `docs/README.md` |
