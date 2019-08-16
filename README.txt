playground.riddles.io/competitions/ultimate-tic-tac-toe

IA utilisant l'algorithme d'exploration negamax (variante de minmax), l'élagage alpha beta,
le précalcul de la fonction de score qui comporte de nombreuses conditions,
j'utilise également un backtracking très efficace pour passer d'une configuration à la suivante.
un score prenant en compte l'intérêt tactique des petits blocs et l'intérêt
de la situation globale des macro blocs, prennant en compte que certains macro blocs ne peuvent pas être gagnés (sans forcément être pleins).
La fonction de score a été conçue pour choisir la défaite la plus loin possible, et prend en compte le bug qu'il y a dans le serveur qui fait que l'on perd si l'on ne peut plus jouer.
J'ai implémenté également de l'iterative deepening, avec un budget qui s'adapte selon le temps disponible.
L'exploration des coups possible est fait en explorant d'abord les coups qui donnent le meilleur score.

30% : possibleMoves()
28% : boardScore()
17% : std::sort()
13% : get et put de hachage

