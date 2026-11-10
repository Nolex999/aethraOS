# 3. Spécification matérielle

Les étiquettes sont définies dans `docs/README.md`. Les valeurs `[DUMP]` viennent du décompilateur ARM d'IDA sur le kernel et sur l'image liée en `0x08000000`, Epsilon 26.4.0 commit `cc9262e`. Bases d'analyse : `dfu-util-0.11-binaries/win64/ida_arm/`.

| Sujet | Valeur | Confiance |
|---|---|---|
| Modèle | N0120, chaîne `N0120` présente dans le kernel | `[DUMP]` |
| Variante | N0120EX, marché portugais | `[DOC]` |
| Version du logiciel | Epsilon 26.4.0 dans les deux slots | `[DUMP]` |
| MCU | STM32H725, Cortex-M7 | `[DOC]`, cohérent avec les registres |
| Référence exacte et boîtier | À lire sur la puce | `[?]` |
| **Fréquence CPU** | **540 MHz** (les 550 MHz publiés sont le maximum de la puce). SysTick est réglé par `sub_90001960(540)`, 67 500 ticks/ms | `[DUMP]` |
| PLL1 | M=4, N=270, P=1, Q=3, R=1 | `[DUMP]` |
| PLL2 | M=2, N=50, P=17, Q=1, R=2 | `[DUMP]` |
| Oscillateur externe | 8 MHz, déduit de 540 x 4 / 270 | `[HYP]` |
| Tension et flash | Échelle VOS0, 3 états d'attente, WRHIGHFREQ=3 | `[DUMP]` |
| Alimentation interne | Probablement SMPS seul | `[HYP]` |
| Flash interne | Région MPU de 512 Kio en `0x08000000`, lisible par le code non privilégié. Publié « 64 Ko » : il s'agit sans doute de ce qu'utilise le bootloader | `[DUMP]` |
| Flash externe | 8 Mio en `0x90000000`, OCTOSPI1 en lecture mappée | `[DUMP]` |
| Secteurs | `08*004K, 01*032K, 63*064K, 64*064K` | `[DUMP]` |
| RAM AXI | Région MPU de 256 Kio en `0x24000000`. 4 Kio réservés au kernel en `0x2403F000` | `[DUMP]` |
| Écran | 2,8 pouces, 320 x 240 | `[DOC]` |
| Bus d'écran | FMC banque 1 : commandes en `0x60000000`, données en `0x60020000` | `[DUMP]` |
| Contrôleur d'écran | **Famille ST7789.** Commandes `0x01, 0x11, 0x04, 0xC6, 0x35, 0xC4, 0xB7, 0x29, 0x36, 0x2B, 0x2C, 0x2E, 0x3A, 0x10`. L'identifiant est relu à l'init, plusieurs dalles sont possibles | `[DUMP]` modèle exact `[HYP]` |
| Clavier | Matrice 9 x 6 : lignes sur GPIOA en drain ouvert, colonnes sur GPIOC | `[DUMP]` |
| Lignes (ordre) | PA1, PA0, PA2, PA3, PA6, PA7, PA8, PA10, PA15 | `[DUMP]` |
| Colonnes | PC1 à PC6, **ou** PC1, PC9, PC11, PC4, PC5, PC6 selon le mot en `0x0807FA00` | `[DUMP]` |
| USB | USB1_OTG_HS `0x40040000` | `[DUMP]` |
| Batterie | ADC3 et ADC1 utilisés, canal exact à établir | `[DUMP]` / `[?]` |
| Minuteries | TIM1, 2, 3, 4, 6 ; rôles non établis | `[DUMP]` |
| UART, SPI, I2C | Aucune adresse correspondante dans les 345 fonctions décompilées | `[DUMP]` |
| Watchdog | Aucune référence à l'IWDG | `[DUMP]` |
| Rails, tensions, broches libres | Inconnus, mesure obligatoire | `[?]` |

## Découpage MPU du kernel

La MPU est active avec le mappage par défaut réservé au code privilégié : **tout accès non privilégié hors de ces régions provoque une faute.** Les apps externes s'exécutent en non privilégié.

| # | Base | Taille | Accès non privilégié | Exécution | Rôle |
|---|---|---|---|---|---|
| 0 | `0x08000000` | 512 Kio | Lecture | Oui | Flash interne |
| 1 | `0x24000000` | 256 Kio | Lecture/écriture | Non | RAM AXI |
| 2 | `0x2403F000` | 4 Kio | Non établi | Non | RAM kernel |
| 3 | `0x40040000` | 256 Kio | **Lecture/écriture** | Non | **USB OTG_HS** |
| 4 | `0x00000000` | 4 Gio, sous-régions | Aucun | Non | Interdiction de `0x60000000-0x9FFFFFFF` |
| 5 | `0x60000000` | 256 Mio | Non établi | Non | Écran FMC |
| 6 | `0x90000000` | 8 Mio | Lecture | Oui | Flash externe |
| 7 | Slot actif | Variable | Lecture | Oui | Image en cours |

Conséquences :
- Pas de GPIO ni d'UART depuis une app : la région correspondante n'existe pas. **L'ESP32 en UART est impossible en T1.**
- Pas d'écriture directe en flash externe, donc pas de LittleFS sur flash brute en T1.
- L'USB, lui, est accessible depuis une app, ce qui rend possible un lien USB maison `[HYP]`.

## Appels système

60 numéros SVC (0 à 59), table de pointeurs en `0x90008D30`. Les numéros 36, 37, 38 et 58 sont refusés selon un drapeau en `0x2403FCAC` `[DUMP]`, probablement lié au logiciel tiers `[HYP]`.

## Le kernel écrit en flash interne

Une fonction du kernel recopie l'image de `slot + 0x10000` vers `0x08000000` si le bootloader installé n'est pas reconnu (magie `0xDEC0B007` en `0x08000244` et `0x0800024C`) ou est plus ancien. Elle met aussi à jour le mot `0x0807FA00` et une zone de bits à partir de `0x0807FA20` `[DUMP]`. Rôle probable : mise à jour du bootloader et compteur anti-retour `[HYP]`. **Installer un Epsilon plus récent peut donc modifier définitivement la flash interne.**

## Carte de la flash externe

Les deux slots ont la même structure, décalée de `0x400000`, chaque image étant liée à son adresse.

| Décalage | Contenu | Confiance |
|---|---|---|
| `0x000000` | En-tête : format 2, taille `0x17BFD8`, magie, version, commit, magie | `[DUMP]` |
| `0x000020` | Table de vecteurs du kernel | `[DUMP]` |
| `0x010000` | Image de bootloader, 64 Kio, liée en `0x08000000` | `[DUMP]` |
| `0x020000` | En-tête userland `FE ED C0 DE` | `[DUMP]` |
| `0x17BFD8` | Fin d'image, puis 128 octets à forte entropie | `[DUMP]` ; signature `[HYP]` |
| `0x180000-0x3F0000` | Zone des apps externes | `[DUMP]` |

## En-tête userland (slot A)

| Valeur | Interprétation |
|---|---|
| `0x2400657C`, taille `0xA800` | Stockage utilisateur en RAM, 43 008 octets |
| `0x90180000` à `0x903F0000` | Flash des apps externes, 2 496 Kio |
| `0x240118A4` à `0x24037000` | RAM des apps externes, 153 436 octets |
| `0x2403EFF8`, `0x90087E91` | Pile et point d'entrée du userland |

L'ordre de ces champs est confirmé par le code de `nwlink` 0.0.19, qui les lit au même emplacement. Le décalage du userland, `0x20000`, diffère de l'adresse `0x90010000` citée pour d'anciens modèles.

## Format observé d'une app externe

KhiCAS commence à `0x90190000` par un en-tête de 32 octets : `BA BE C0 DE`, un mot à 0, `0x20`, `0xA88`, `0x27`, `0xAFC`, `0x1BFC`, `BA BE C0 DE`. D'après le script de liaison de `nwlink`, ces champs sont : magie, niveau d'API, décalage du nom, taille de l'icône, décalage de l'icône, décalage du point d'entrée, taille totale, magie `[DOC]`.

## Contenu actuel de ta calculatrice

KhiCAS et ses scripts occupent la zone des apps. Une réinstallation peut les effacer : garde ton dump.

## Limites

- La flash interne n'a pas été lue. L'image de `slot + 0x10000` en donne probablement une copie, non vérifiée.
- Les droits des régions 2 et 5, et les rôles des minuteries, restent à établir.
- Aucune mesure électrique : tensions et broches accessibles restent inconnues.
