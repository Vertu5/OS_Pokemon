# Projet "Quel est ce Pokémon ?" -

Bienvenue sur notre référentiel GitHub pour le projet "Quel est ce Pokémon ?". Notre équipe est ravie de présenter ce projet passionnant réalisé dans le cadre du cours INFO-F-201 - Systèmes d'exploitation.
## Lancer le projet
Dans `img-dist` utiliser dans le terminal la commande `make` puis tester avec `./img-dist -v ../img/1.bmp ../img/20.bmp` et `./img-dist ../img/1.bmp ../img/20.bmp`.

Dans `base-project/` `chmod +x launcher` et `list-file` pour donner la permission aux fichier bash puis `g++ img-search.cpp -o img-search` pour le code c++. Pour tester list file : `./list-file img/`. Pour launcher cfr projet `launcher [-i|--interactive|-a|--automatic] image [database_path]` et enfin pour img-search on doit encore faire le code hahaha.

## Objectif du Projet

En tant que fervents amateurs de Pokémon, nous avons entrepris de créer un système ingénieux permettant à nos amis d'identifier le numéro de Pokédex associé à un Pokémon à partir de son image, même dans des conditions où l'image pourrait être partiellement dégradée ou présenter des couleurs variées. Pour réaliser cette tâche de manière robuste, nous avons opté pour un système de hachage perceptif, qui nous est fourni, afin de comparer efficacement les différentes images de notre base de données.

Pour garantir que notre système puisse gérer un grand nombre d'images, nous avons choisi de mettre en œuvre ce projet en utilisant plusieurs processus pour comparer simultanément plusieurs images.

## Composition du Projet

Notre projet se compose des éléments suivants :

1. **img-dist** : Un programme en C qui génère les codes de hachage perceptif (ce programme est fourni, nous n'avons pas besoin de le développer).
2. **list-file** : Un script Bash destiné à répertorier les fichiers d'un dossier.
3. **img-search** : Un programme en C ou C++ qui reçoit le chemin vers une image et identifie l'image la plus similaire parmi celles de notre base d'images, transmises via stdin.
4. **launcher** : Un script Bash permettant de lancer le programme img-search.

## Fonctionnalités Prises en Charge

Nous avons la liberté d'utiliser des fonctionnalités de C++ telles que les vecteurs, les chaînes de caractères, les tables de hachage, les opérateurs new et delete, les classes, etc., pour l'implémentation de ce projet. Cependant, nous devons garder à l'esprit que l'objectif principal est de développer des compétences en programmation système.

## Détails du Projet

### Programme img-dist

Le programme `img-dist` est un composant clé de notre projet. Il est chargé de comparer les images en utilisant le hachage perceptif. Ce programme est fourni et nous n'avons pas à le développer. Nous devons simplement le compiler avant de l'utiliser. Pour compiler `img-dist`, nous devons nous rendre dans le répertoire `img-dist/` et exécuter la commande `make`.

Le programme `img-dist` prend deux chemins d'images en tant qu'arguments et renvoie une valeur entre 0 et 64, indiquant le degré de similarité entre les deux images. Plus la valeur de retour est élevée, moins les images sont similaires.

### Programme img-search

Le programme `img-search` est le cœur de notre projet. Il est chargé de comparer une image BMP spécifiée en entrée avec un ensemble d'autres images BMP. Les principales exigences pour ce programme sont les suivantes :

- Être écrit en C ou C++
- Être compilé à l'aide d'un Makefile
- Prendre l'image à comparer comme argument en ligne de commande
- Utiliser le programme `img-dist` pour les comparaisons d'images
- Créer deux processus supplémentaires pour effectuer des comparaisons concurrentes
- Gérer les signaux SIGUSR1, SIGUSR2 et SIGINT

### Script Bash list-file

Le script Bash `list-file` a pour tâche de répertorier les fichiers dans un dossier donné. Il prend le chemin du dossier en tant que paramètre et liste les fichiers contenus dans ce dossier, en ignorant les sous-dossiers. Le script gère divers scénarios, tels que l'absence de nom de dossier ou la spécification d'un chemin non valide.

### Script Bash launcher

Le script Bash `launcher` est un élément essentiel de notre projet. Il est responsable du lancement du programme `img-search` dans deux modes différents :

- Mode interactif : L'utilisateur fournit manuellement l'ensemble d'images via stdin.
- Mode automatique : Le script transmet automatiquement l'ensemble d'images au programme.

Le script permet également de spécifier l'image à comparer et le chemin de l'ensemble d'images.

## Évaluation du Projet

Notre projet sera évalué en fonction des critères suivants :

- Tests : 33% (Tests automatiques pour la fonctionnalité et la gestion des erreurs)
- Bash : 33% (Qualité du script list-file et launcher)
- Structure : 15% (Clarté et organisation du code)
- Rapport : 20% (Documentation et explications)

Il est impératif que notre projet compile avec succès, et nous devons gérer les erreurs de manière adéquate. Les retards dans la soumission entraîneront des déductions de points.

## Soumission du Projet

Pour soumettre notre projet, nous devons créer un fichier ZIP contenant :

- Le code source de `img-search`
- Les scripts Bash `launcher` et `list-file`
- Le Makefile
- Le rapport au format PDF, indiquant les noms des membres de l'équipe et leurs ULBIDs
- Tout test supplémentaire que nous aurions écrit (si applicable)

Nous ne devons pas inclure la banque d'images fournie ni les répertoires `img-dist/` et `test/` ainsi que leur contenu.

Nous devons soumettre notre projet sur l'Université Virtuelle avant la **date limite du 12 novembre 2023, à 23h59.**

