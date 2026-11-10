# AETHRA, dossier de conception (phase 0)

Ce dossier est le premier livrable demandé par `prompt.txt` (section 25). Il ne contient aucune promesse de fonctionnement sur calculatrice : rien n'a encore tourné sur le matériel.

| # | Livrable | Fichier |
|---|---|---|
| 1 | Noms et identité | [01_naming_identity.md](01_naming_identity.md) |
| 2 | Faisabilité, boot et récupération | [02_feasibility.md](02_feasibility.md) |
| 3 | Spécification matérielle | [03_hardware.md](03_hardware.md) |
| 4 | Architecture système | [04_architecture.md](04_architecture.md) |
| 5 | Budget mémoire | [05_memory_budget.md](05_memory_budget.md) |
| 6 | Spécification UI/UX | [06_ui_ux.md](06_ui_ux.md) |
| 7 | Inventaire des applications | [07_applications.md](07_applications.md) |
| 8 | Structure du dépôt | [08_repository.md](08_repository.md) |
| 9 | Agents Claude Code | [09_agents.md](09_agents.md) |
| 10 | Feuille de route | [10_roadmap.md](10_roadmap.md) |
| 11 | Registre des risques | [11_risks.md](11_risks.md) |
| 12 | Stratégie de test | [12_test_strategy.md](12_test_strategy.md) |
| 13 | Prompt d'exécution | [13_claude_code_prompt.md](13_claude_code_prompt.md) |
| + | Flasheur | [flash.md](flash.md) |

## Étiquettes de confiance

| Étiquette | Sens |
|---|---|
| `[DUMP]` | Observé directement dans `epsilon_flash_8mb.bin` (Epsilon 26.4.0, commit `cc9262e`) |
| `[ST]` | Documentation STMicroelectronics de la famille STM32H725 |
| `[DOC]` | Documentation communautaire ou source publique, pas forcément propre au N0120 |
| `[HYP]` | Hypothèse raisonnable, à confirmer |
| `[?]` | Inconnu |

## Sources

- Dump local : SHA-256 `eabae38b2f424062584c28d0857402c763b1a1d83a6bb707fbda300218cc8c5f`.
- [Nwagyu : processus de boot](https://nwagyu.org/reference/firmware/boot-process.html), [bootloader](https://nwagyu.org/reference/firmware/bootloader.html), [kernel](https://nwagyu.org/reference/firmware/kernel.html), [userland](https://nwagyu.org/reference/firmware/userland.html), [apps externes](https://nwagyu.org/reference/apps/creating-application.html), [DFU](https://nwagyu.org/reference/others/dfu.html).
- [Wikipédia : NumWorks](https://en.wikipedia.org/wiki/NumWorks) et [dépôt numworks/epsilon](https://github.com/numworks/epsilon).
- [Datasheet STM32H725](https://st.com/resource/en/datasheet/stm32h725ig.pdf).
- [Fiche revendeur N0120](https://www.bruneau.fr/product/calculatrice-graphique-langage-python-integre-n0120/8363714), pour l'écran.
- [Discussion communautaire sur Omega et le N0120 (2024)](https://axebps-redlib.hf.space/r/numworksomega/comments/1esddfu/is_there_any_way_to_install_omega_on_n0120) et [analyse du verrouillage](https://M4xi1m3.github.io/Numworks-lock/).
