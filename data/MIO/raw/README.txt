Bonjour Cécile et François,

Suite à notre discussion, Térence et moi avons préparé l'archive en PJ pour un premier test de travail collaboratif.

Le réseau exemplaire consiste en un réseau "originel" de N = 8196 noeuds (= nombre de ligne ds le fichier .grid). Ce fichier .grid contient autant de lignes que de noeuds (1 ligne pour chaque noeud) et 4 colonnes: 1ere colonne = latitude du centre du noeud ; 2eme colonne = longitude du centre du noeud ; 3ème colonne = delta-lon ; 4ème colonne = land-ratio (vous pouvez ignorer cette colonne pour le moment).
Les noeuds sont des rectangles; leurs côtés (en latitude, delta-lat) sont constant avec delta-lat = 0.25 degré ; leurs côtés (en longitude) varient avec delta-lon (3ème colonne du fichier .grid).    

Dans notre approche, nous construisons une grille de N = 8196 noeuds qui couvre toute la surface de la Méditerranée. Or, le réseau "effectif" que nous vous proposons pour ce travail concerne la connectivité d'organismes benthiques divers qui ne vivent que dans un certain type d'habitat ("infralittoral"). Ces habitats sont "patchy" et n'existe pas partout... Du coup, nous avons réduit le réseau "originel" en un réseau "effectif" de Nred = 1170 noeuds en "annulant" toutes les connexions qui ne concernent pas ces noeuds d'habitat. Les noeuds qui ont été gardés sont indexés dans le fichier infralittoral_habitat_filter.txt (qui contient aussi 8196 lignes, comme le .grid) avec la valeur "1"; les noeuds enlevés (à ignorer pour vous) sont indexés avec la valeur "0". 

Les matrices de connectivité que nous vous transmettons sont donc des matrices de Nred * Nred. Cependant, nous vous les envoyons comme des "sparse matrix", c-a-d que seuls les éléments non nuls sont reportés. Les 2 fichiers .matrix ont autant de ligne que de connexions effectives ; ils ont 3 colonnes: 1ère colonne = numéro du noeud de départ ; 2éme colonne = numéro du noeud d'arrivée ; 3éme colonne = probabilité de connexion "forward-in-time" du noeud de départ vers le noeud d'arrivée. Ces probabilités moyennes sont calculées en considérant le transport de larves (~particules lagrangiennes) par les courants marins des années 2000 à 2010. Nous en avons préparé 2 différentes pour ces premiers tests avec 2 temps de dispersion "tau" de 10 et 30 jours. 

Par exemple, les premières lignes du fichier for_infra_all_2000_2010_for_vfield1_ndom1_depth3_node0250_plat002500_tau10_rk003.matrix  indiquent que les larves relarguées dans le noeud 85 ont ~8% de chance de rester dans le noeud 85, ~32.6% de chance de finir dans le noeud 86, ~16.4% de chance de finir dans le noeud 87, etc... Pour savoir où sont ces noeuds sur la carte, il suffit d'aller voir les lignes correspondantes (i.e. 85, 86, 87, etc...) dans le fichier .grid.

Enfin, parmis tous les noeuds Nred, il y a 8 noeuds (n1, n2,..., n8) qui sont déjà surveillés dans SEAMoBB. Ces 8 noeuds sont identifiés dans le fichier SEAMoBB_nodes.txt (1 ligne pour chaque site d'observation) qui a 4 colonnes: 1ère colonne = nom du site ;  2éme colonne = latitude réelle du site ; 3éme colonne = longitude réelle du site ; 4éme colonne = numéro du node correspondant (= le noeuds le plus proche du vrai site et peu couvert par la terre).

A l'aide de ces données et cela devrait vous permettre de tester votre algo d'optimisation (ici ce ne serait pas exactement basé sur votre indice "Equivalent Connected Area" mais un équivalent? on pourra en rediscuter...) pour répondre à la question suivante: ou doit-on optimalement placer d'autres sites d'observation si nous aurions plus de fonds?
"Optimalement" signifierait ici d'avoir des sites surveillées les moins connectés possibles entre-eux (puisque plus ils sont connectés, plus ils seraient redondants...). On pourrait par tester qqs valeurs types du nombre total de sites surveillés, par ex. en testant des facteurs multiplicatifs génériques comme 8 sites *2, *5 et *10... qu'en pensez vous?

Si besoin et suivant nos timings respectifs, il sera surement utile de refaire une réunion entre-nous en présentiel. Ce sera aussi l'occasion que François et Térence se rencontre.

Last but not least, pourriez-vous svp nous transmettre un PDF de vos slides sur le sujet (celles que vous m'aviez montré lors de la réunion à St Charles, pour notre lecture interne seulement) d'ici notre prochaine réunion svp. Ceci afin que Térence et moi puissions se familiariser avec vos travaux et les techniques d'optimisation en vue de raffiner les objectifs communs.

A bientot,

Vincent