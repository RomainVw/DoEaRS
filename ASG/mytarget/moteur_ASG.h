//moteur_ASg.h apr!s avoir migré les données vers la ROM

// ********************************************************************* //
// Macros permettant de clearer ou de setter des bits dans des variables //
// ********************************************************************* //
#define bitset(var,bitno) ((var) |=1 << (bitno))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))

// ************************************ //
// définitions des structures utilisées //
// ************************************ //

//struct etatStruct;
typedef struct etatStruct etat;
typedef struct transitionStruct transition;
typedef struct ressourceStruct ressource;
typedef struct etatStructVolatile etatvolatile;

// ************************** //
// Structure d'une transition //
// ************************** //
struct transitionStruct {
#ifdef EVENEMENT
	// Identificateur de l'évènement associé à la transition
	unsigned char evt1;
#endif
#ifdef RENDEZ_VOUS
	// Identificateur du rendez-vous associé à la transition
	unsigned char rdv1;
#endif
	// Pointeur vers l'état dans lequel aboutit la transition
	rom etat* next_state;
};

#ifdef RESSOURCE
// ************************* //
// Structure d'une ressource //
// ************************* //
struct ressourceStruct {
	// Identificateur de la ressource
	unsigned char ressourceId;
	// Nombre de ressource utilisée.
	unsigned char ressourceQty;
};
#endif

// ******************* //
// Structure d'un état //
// ******************* //

struct etatStruct {

  // Pointeur vers l'état père de cet état.
  const rom etat* father;

  // nombre de composantes de cet état et N° de la composante du père
  //concaténés dans deux nibbles
  unsigned char composantes;

#ifdef RENDEZ_VOUS	
  // pointeur vers la liste des rendez-vous
  const rom char* const rom *rdvs;
#endif

  // pointeur vers la liste des transitions
  const rom transition* rom *trans;
  
  //nombre de rendez-vous + nombre de transitions
  //concaténés dans deux nibbles
  unsigned char nb_rdvs_trans;
  

  // pointeur vers l'action
  void (*action)(void);

  //ATTENTION, faut s'arranger pour placer ce tableau en RAM!
  // pointeur vers la structure des états actifs
  const rom etat* ram* rom etatsActifs;

#ifdef RESSOURCE
  // pointeur vers la liste de ressources utilisées.
  const rom ressource* rom *ressources;
  // nombre de ressources utilisées
  unsigned char nbRess;
#endif

#ifdef TIMEOUT
  // timeout
  unsigned char timeout;
#endif
	//RDL partie modifiable de l'état  
	ram etatvolatile* rom volatiledata;
};

//////////////////////////////////////////////////////////////////////////////:
struct etatStructVolatile{
  // status (acti-passif) + MinVT
  unsigned char minvt;
  // status (action) + Tps restant pour le minvt
  unsigned char time_minvt;  
#ifdef TIMEOUT
  // Tps restant pour le timeout
  unsigned char time_timeout;
#endif
  //données utilisées par l'interpréteur 
  //pour son parcours des états
  unsigned char nbChildrenVisited;
  unsigned char childrenVisited;
  char a;
};

// ********** //
// Moteur ASG //
// ********** //

/*
 * Procédure mettant à jour les évènements du diagramme ASG.
 *
 * Cette procédure, générée par le générateur de code, se charge de mettre à
 * jour l'ensemble des évènements du graphe ASG à l'exception des évènements
 * de type timeout.
 * 
 */

#ifdef EVENEMENT
void updateEvents(void);
#endif

/* 
 * Procédure permettant d'activer un état  
 * 
 * Cette procédure n'active que l'état sélectionné. C'est lorsque l'action
 * associée sera exécutée que les sous-états seront activés.
 *
 * Il n'y a par contre pas besoin de reinitialiser la liste des etats
 * actives de cet etat. Ca a du etre fait a l'initialisation et lorsque l'etat
 * a ete desactive.
 *
 * Elle se charge en outre de mettre a jour les informations de ressource.
 * Elle ne se charge pas de verifier que ces dernières sont disponnibles en
 * suffisance.
 *
 * Elle ne se charge par contre des rendez-vous que s'il n'y a pas d'action.
 * En effet, le rendez-vous ne peut être détecté que si l'action a été
 * exécutée. Une transition normale à laquelle a été associée un rendez-vous
 * ne peut être franchie que si les états qui déclarent le rendez-vous ont eu
 * leur rendez-vous exécuté.
 *
 * C'est elle qui s'ajoute dans la liste des états actifs de son père. 
 *
 * Dans le cas particulier ou aucune action n'est associée a l'état, ce
 * dernier est initialisé en supposant l'action effectuée.
 */
 
void activateState(const rom etat* state);

/*
 * Procedure permettant de desactiver un etat.
 * 
 * Pour chacun de ces etats, elle se charge de liberer les ressources et
 * rendez-vous utilises. 
 *
 * Elle se charge aussi de reinitialiser la liste des etats actives dans
 * chaque etat du sous-graphe. Ceci permettra de reactiver un sous-graphe
 * complet sans se soucier de reinitialiser cette liste.
 *
 * Enfin, elle se charge, le cas échéant, de mettre à faux l'évènement
 * timeout de l'état (s'il y en a un).
 *
 */
void desactivateState(const rom etat* state);

/*
 * Simule la libération d'un état en calculant le nombre de ressources que ca
 * libérerait. 
 * 
 * Le resultat de cette simulation est fourni en mettant a jour le tableau
 * freedRessources
 */
#ifdef RESSOURCE
void simulateDesactivateState(const rom etat* state);
#endif
/*
 * Cette procédure effectue l'ensemble des transitions qui peuvent être
 * franchies dans le graphe.
 *
 * Une transition ne peut être franchie que si l'action associée à l'état a
 * été exécutée (s'il n'y a pas d'action associée à l'état, ce dernier est
 * initialisé comme s'il avait effectué l'action).
 * 
 * Cet algorithme parcour le graphe en effectuant un EulerTour. A chaque fois
 * qu'une transition est franchie, il recommence l'EulerTour à l'état où la
 * transition aboutit.
 *
 * Il ne parcourt les fils d'un état que si l'action de l'état a été
 * effectuée. réalisées dans le graphe ASG. Pour se faire, il effectue un
 * EulerTour. 
 */
void processTransitions(void);

/*
 * Cette procédure effectue l'ensemble des actions qui peuvent être réalisées.
 *
 * Comme, lorsqu'il arrive dans un état, il en effectue l'action, il peut tout
 * de suite après, accéder aux fils actifs de l'état. En effet, c'est l'action
 * qui initialise les fils de l'état.
 *
 * A chaque action exécutée, si le minvt est échu, les rendez-vous de l'état
 * sont mis à jour. 
 *
 * Entre chaque action à effectuer, le système met à jour les timeouts.
 */
void processActions(void);

/*
 * C'est la fonction principale du moteur. 
 * 
 * Elle initialise l'état principal du graphe ainsi que le timer utilisé pour
 * les timeouts. Ensuite, elle effectue en
 * boucle les fonctions processTransitions et processActions.
 */
void processGraph(void);

/*
 * Méthode permettant de mettre à jour les minvt et les timeout de tous les
 * états actifs lorsqu'une unité de temps s'est écoulée. 
 * 
 * Lorsqu'un minvt que l'on vient de décroitre arrive à zéro, si l'action de
 * l'état correspondant a déjà été exécutée, alors, les rendez-vous sont mis à
 * jour.
 *
 * Lorsqu'un timeout est atteint, l'évènement associé est considéré comme
 * vrai.
 */
#ifdef MINVT_OR_TIMEOUT
void updateTimes(void);
void initTimer(void);
int testTime(void);
#endif
