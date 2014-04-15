//moteur_ASG.c après avoir migré les données vers la ROM

#include <p18f452.h>

/* Set configuration bits for use with ICD2 / PICDEM2 PLUS Demo Board:
 *  - set HS oscillator
 *  - disable watchdog timer
 *  - disable low voltage programming
 */ 
#pragma romdata CONFIG
_CONFIG_DECL(_CONFIG1H_DEFAULT & _OSC_HS_1H,\
             _CONFIG2L_DEFAULT,\
             _CONFIG2H_DEFAULT & _WDT_OFF_2H,\
             _CONFIG3H_DEFAULT,\
             _CONFIG4L_DEFAULT & _LVP_OFF_4L,\
             _CONFIG5L_DEFAULT,\
             _CONFIG5H_DEFAULT,\
             _CONFIG6L_DEFAULT,\
             _CONFIG6H_DEFAULT,\
             _CONFIG7L_DEFAULT,\
             _CONFIG7H_DEFAULT);
#pragma romdata


int level;
int current_level;
int orderLevel;

void main2 (void);

void main(void){
	main2();
}

#include "tables.h"
//#include "moteur_ASG.h"

void main2 (void){
	processGraph();
} 

void activateState(const rom etat* state) {
	// définition des variables qui seront utilisées dans la procédure
	unsigned char prov1;
	unsigned char prov2;
	// On s'occupe de mettre à jour le père de l'état :
	prov1 = ((state->composantes) & 0x0F)-1;
	//prov1 est le numéro de la composante parallèle 
	//à laquelle appartient l'état 'state'
	if (state != rootState)
		(state->father->etatsActifs)[prov1] = state;
	
	// On met à jour le status de cet état :
	bitset(state->volatiledata->minvt, 7);
	state->volatiledata->time_minvt = state->volatiledata->minvt;
	if (state->action != 0x00)
		bitclr(state->volatiledata->time_minvt, 7);
	
	// On s'occupe des rendez-vous à la condition qu'il n'y ait pas
	// d'action :
	#ifdef RENDEZ_VOUS
	else {
		prov2 = (state->nb_rdvs_trans >> 4) & 0x0F;
		for (prov1=0; prov1 < prov2; prov1++)
			reachedRdvs1[(*(state->rdvs[prov1]))-1] +=1;
	}
	#endif

	// On s'occupe des ressources
	#ifdef RESSOURCE
	for (prov1 = 0; prov1 < state->nbRess; prov1++) 
		usedRessources1[state->ressources[prov1]->ressourceId] +=
			state->ressources[prov1]->ressourceQty;
	#endif

	#ifdef TIMEOUT
	state->volatiledata->time_timeout = state->timeout;
	#endif
	state->volatiledata->nbChildrenVisited = 0;
	state->volatiledata->childrenVisited = 0;
	
}

/* 
 * Méthode permettant de désactiver un état qui n'a pas d'état actif.
 * 
 * Cette méthode met à jour les états actifs de son père.
 *
 * De plus, elle se charge de rendre faux l'éventuel évènement de timeout
 * associé à l'état.
 */
void privateDesactivateState(const rom etat* state) {
	// variables temporaires utilisées
	unsigned char prov1;
	// on supprime l'état des états actifs du père.
	state->father->etatsActifs[((state->composantes) &  0x0F)-1] 
		= 0x00;
	// on supprime les rendez-vous de l'état
	#ifdef RENDEZ_VOUS
	for (prov1 = 0; prov1 < (((state->nb_rdvs_trans)>>4)&0x0F); prov1++)
		reachedRdvs1[(*(state->rdvs[prov1]))-1]--;
	#endif
	// on supprime les ressources de l'état
	#ifdef RESSOURCE
	for (prov1 = 0; prov1 < state->nbRess; prov1++)
		usedRessources1[state->ressources[prov1]->ressourceId] 
			-= state->ressources[prov1]->ressourceQty;
	#endif
}

void desactivateState(const rom etat* state) {
	rom etat* current_state = state;
	unsigned char prov1;
	unsigned char prov2;
	
	while (current_state != state->father) {
		prov1 =  ((current_state->composantes >> 4) & 0x0F);
		for (prov2 = 0; prov2 < prov1; prov2 ++) {
			// on se dirrige vers un état qui n'a plus de fils
			if (current_state->etatsActifs[prov2] != 0x00) {
				current_state = current_state->
					etatsActifs[prov2];
				break;
			}
		}
		// Quand il n'y a plus de fils, on le désactive;
		privateDesactivateState(current_state);
		// on remonte chez son père
		current_state = current_state->father;
	}	
}

#ifdef RESSOURCE
void privateSimulateDesactivateState(const rom etat* state) {
	unsigned char prov1;	
	// on supprime les ressources de l'état
	for (prov1 = 0; prov1 < state->nbRess; prov1++) {
		provRessources1[state->ressources[prov1]->ressourceId] 
			-= state->ressources[prov1]->ressourceQty;
	}
}

void simulateDesactivateState(const rom etat* state) {
	// on déclare les variables qui seront utilisées
	unsigned char prov1;
	const rom etat* current_state = state;
	// On met à jour le tableau provisoire
	for (prov1 = 0; prov1 < NB_RESSOURCES; prov1++)
		provRessources1[prov1] = usedRessources1[prov1];
	// On effectue la simulation
	while (current_state != state->father) {
		// on simule la suppression de l'état des états actifs
		privateSimulateDesactivateState(current_state);
		
		// on entre dans la première composante s'il y en a une
		if (((((current_state->composantes)>>4)&0x0F) != 0x00) &&
		(current_state->etatsActifs[0]!=0x00)) {
			current_state = current_state->etatsActifs[0];
			continue;
		}
		// S'il n'y a plus moyen de descendre, on remonte jusqu'à un
		// noeud dont on n'a pas exploré au moins un des fils
		do {
			prov1 = ((current_state->composantes)) & 0x0F;
			current_state = current_state->father;
		} while ((prov1 = (((current_state->composantes)>>4)&0x0F)) &&
		(current_state != state->father));
		// Si on n'est pas remonté trop haut, on descend dans le noeud
		// qu'on n'a pas encore exploré...
		if (current_state != state->father) 
			current_state = current_state->etatsActifs[prov1];
	}
		
}
#endif

/*
 * Cette méthode regarde l'ensemble des transitions de l'état state et
 * franchit celle qui est la plus prioritaire, s'il y en a une de
 * franchissable.
 *
 * Si elle franchit une transition, elle fait pointer state vers l'état où
 * arrive cette transition.
 *
 * Enfin, elle se charge de mettre à jour les rendez-vous, les ressources.
 * Elle vérifie que ces dernères sont là en suffisance avant de franchir la
 * transition.
 *
 * Une transition peut arriver dans l'état dans lequel elle part.
 *
 * Nous supposons ici que les transitions sont classées dans l'ordre de
 * priorité.
 */ 
void privateProcessTransitions(rom etat* state) {
	rom etat* leavedState;
	rom etat* current_state = state;
	unsigned char prov1;
	unsigned char prov2;
	// On n'exécute la transition que si l'action a été exécutée :
	if (state->volatiledata->time_minvt != 0x80)
		return;
	// On scanne l'ensemble des transitions de l'état
	transitions : for (prov1 = 0; prov1 < ((state->nb_rdvs_trans)&0x0F); 
	prov1++) {
		// On va tout d'abord vérifier que cette transition est
		// franchissable sur le plan des rendez-vous, de l'évènement
		// qui y sont associés.

		// évènement :
		#ifdef EVENEMENT
		if ((!(EVENEMENTS1 & (state->trans[prov1]->evt1)))
		&&(state->trans[prov1]->evt1 != 0x00))
			// Si l'évènement est défini et ne s'est pas produit,
			// on continue le parcours des transitions de l'état.
			continue;
		#endif
		// rendez-vous : 
		#ifdef RENDEZ_VOUS
		prov2 = state->trans[prov1]->rdv1;
		//RDL: ce truc est completement faux. une transition peut avoir plusieurs rdv 
		//RDL même si l'editeur ne le supporte pas, 
		//RDL le moteur et le générateur le supportent bien
		//if ((prov2 != 0)&&(neededRdvs1[prov2-1]>reachedRdvs1[prov2-1])){
		// s'il n'y en a pas assez, on continue le
		// parcours des transitions de l'état.
		//goto stop;
		//}
		if (prov2 !=0){
		  //il y a au moins un rdv nécessaire pour franchir cette transition
		  char i=0;
		  unsigned char mask=1;
		  while(i<8){
		    //on teste le bit numéro i+1 (cad le premier si i=0 et le huitième si i=7
		    //invariant: smallmask est un masque de sélection de ce bit là
		    if(prov2 & mask){
		      //ce rdv ci est attendu on vérifie donc qu'il et bien atteint
		      if (neededRdvs1[i]>reachedRdvs1[i]){
			//le rdv n'est pas atteint
			goto stop;
		      }//s'il est atteint, on continue à tourner pour tester les autres
		    }//end if: else ce rdv n'est pas nécessaire, on continue donc à tourner pour tester les autres
		    i++;
		    mask <<= 1; //on shifte le bit de une position vers le haut
		  }//end while
		}//end if
		//ici, tous les rdv nécessaires pour franchir la transition ont été vérifiée comme atteints
		#endif
		// On recherche le plus grand état quitté
		// On effectue la recherche en supposant que c'est l'état dans
		// lequel on entre	
		leavedState = state->trans[prov1]->next_state;
		while ((current_state != leavedState) && 
		(current_state != 0x00))
			current_state = current_state->father;
		// Si on n'a pas trouvé, c'est que le plus grand état quitté
		// est un de ses frères.
		if (current_state == 0x00) {
			// On recherche donc le père qui est en commun
			current_state = leavedState->father;
			leavedState = state;
			while (leavedState->father != current_state)
				leavedState = leavedState->father; 
		}
		// leavedState contient donc maintenant le plus grand état
		// quitté.
		// D'autre part, sur le plan des rendez-vous, de l'évènement
		// et de l'action, cette transition est franchissable.
		// On vérifie maintenant qu'il y a moyen de quitter ce
		// sous-graphe sur le plan des ressources.
		current_state = state->trans[prov1]->next_state;
		// Pour chaque ressource, on vérifie qu'il en resterait assez
		// si on quittait l'état leaved_state pour entrer dans l'état
		// current_state.
		#ifdef RESSOURCE
		simulateDesactivateState(leavedState);
		for (prov2 = 0; prov2 < current_state->nbRess; prov2++) {
			if ((((declaredRessources1[current_state->
			ressources[prov2]->ressourceId])-provRessources1[
			current_state->ressources[prov2]->ressourceId]))
			- current_state->ressources[prov2]->ressourceQty < 0)
				// S'il n'y en a pas assez, on continue à
				// parcourir les transitions de l'état.
				goto stop;
		}
		#endif
		// Si la transition est franchissable, on la franchit
		// En effet, d'après les restrictions qu'on a posé sur la
		// déclaration des transitions, on sait que c'est la plus
		// prioritaire.

		desactivateState(leavedState);
		activateState(current_state);

		// On met à jour la variable state et on finit la fonction
		state = current_state;
		return;
		stop: continue;
	}
}

/*
 * renvoie un nombre entre 0 et Max, nombre qui est aléatoire.
 *
 * Max doit être suppérieur ou égal à 0
 */
int nbloops = 0;
int privateRandom(int Max) {
	return nbloops % (Max+1);
}

void processTransitions(void) {
	unsigned char prov1, prov2, prov3, prov4;
	rom etat* current_state; 
	#ifdef EVENEMENT
	updateEvents();
	#endif
	current_state = rootState;
	// On effectue l'euler-tour
	while (current_state != 0x00) {
		// on effectue les transitions
		// La transition change l'état actif par l'état où arrive la
		// transition. De ce fait, on continuera à explorer l'arbre
		// qui est effectivement actif.
		privateProcessTransitions(current_state);
		current_state->volatiledata->nbChildrenVisited = 0;
		current_state->volatiledata->childrenVisited = 0;
		// on entre dans une composante s'il y en a une
		if( (((current_state->composantes>>4)&0x0F)!=0)) {
			
			prov1 = privateRandom(((current_state->composantes>>4)
					&0x0F)-
					(current_state->volatiledata->nbChildrenVisited+1));

			bitset(current_state->volatiledata->childrenVisited, prov1);
			current_state->volatiledata->nbChildrenVisited++;
			current_state = current_state->etatsActifs[prov1];
			continue;
		}
		// S'il n'y a plus moyen de descendre, on remonte jusqu'à un
		// noeud dont on n'a pas exploré au moins un des fils
		do {
			current_state = current_state->father;
		} while ((current_state != 0x00) &&
				(((current_state->composantes>>4)&0x0F)==
				current_state->volatiledata->nbChildrenVisited));
		// Si on n'est pas remonté trop haut, on descend dans le noeud
		// qu'on n'a pas encore exploré...
		if (current_state != 0x00) {
			prov1 = privateRandom(((current_state->composantes>>4)
					&0x0F)-
					(current_state->volatiledata->nbChildrenVisited+1));
			prov2=0x01;
			prov3=0xFF;
			prov4=0xFF;
			do {
				if (!(current_state->volatiledata->childrenVisited&prov2))
					prov3++;
				prov2=prov2<<1;
				prov4++;
			} while (prov3!=prov1);
			bitset(current_state->volatiledata->childrenVisited, prov4);
			current_state->volatiledata->nbChildrenVisited++;
			current_state = current_state->etatsActifs[prov4];
		}
	}
	#ifdef MINVT_OR_TIMEOUT
	updateTimes();
	#endif
}

void privateProcessAction(const rom etat* state) {
	unsigned char prov1;
	unsigned char prov2;
	if (!(state->volatiledata->time_minvt & 0x80)) {
		void (*theaction)(void) = state->action;
		theaction();
		state->volatiledata->time_minvt |= 0x80;
		
		#ifdef MINVT_OR_TIMEOUT
		updateTimes();
		#endif
		#ifdef RENDEZ_VOUS
			prov2 = (state->nb_rdvs_trans >> 4) & 0x0F;
			for (prov1=0; prov1 < prov2; prov1++)
				reachedRdvs1[(*(state->rdvs[prov1]))-1] +=1;
		#endif
	}
}

void processActions(void) {
	rom etat* current_state = rootState;
	unsigned char prov1;
	// On effectue l'euler-tour
	while (current_state != 0x00) {
		// on effectue les actions
		privateProcessAction(current_state);
		// on entre dans la première composante s'il y en a une
		if (( (((current_state->composantes) >> 4)&0x0F) != 0x00) &&
		(current_state->etatsActifs[0]!=0x00)) {
			current_state = current_state->etatsActifs[0];
			continue;
		}
		// S'il n'y a plus moyen de descendre, on remonte jusqu'à un
		// noeud dont on n'a pas exploré au moins un des fils
		do {
			prov1 = ((current_state->composantes)) & 0x0F;
			current_state = current_state->father;
		} while ((current_state != 0x00) && 
		(prov1 == (((current_state->composantes)>>4)&0x0F)) );
		// Si on n'est pas remonté trop haut, on descend dans le noeud
		// qu'on n'a pas encore exploré...
		if (current_state != 0x00)
			current_state = current_state->etatsActifs[prov1];
	}
}

#define false 0
#define true 1
//DEBUG
short int this_time = 0xFF;
int cycle = false;
//DEBUG


void processGraph(void) {
	#ifdef MINVT_OR_TIMEOUT
	initTimer();
	#endif
	activateState(rootState);
	while(1) {
		nbloops++; //pour mettre à jours le compteur aléatoire
		//DEBUG
		#ifdef MINVT_OR_TIMEOUT
		this_time --;
		if (this_time == 0)
			cycle = true;
		#endif
		//END DEBUG
		
		processActions();

		processTransitions();
	}
}

#ifdef MINVT_OR_TIMEOUT

int privateUpdateTimes(const rom etat* state) {
	#ifdef TIMEOUT
	if (state->volatiledata->time_timeout != 0x00) {
		state->volatiledata->time_timeout --;
	}
	#endif
	#ifdef MINVT
	if ((state->volatiledata->time_minvt & 0x7F) != 0x00) {
		state->volatiledata->time_minvt --;
	}
	#endif
}
void updateTimes(void) {
        char prov1;
	if (testTime()) {
	rom etat* current_state = rootState;
	// On effectue l'euler-tour
	while (current_state != 0x00) {
		// on effectue les actions
		privateUpdateTimes(current_state);
		// on entre dans la première composante s'il y en a une
		if (( (((current_state->composantes) >> 4)&0x0F) != 0x00) &&
		(current_state->etatsActifs[0]!=0x00)) {
			current_state = current_state->etatsActifs[0];
			continue;
		}
		// S'il n'y a plus moyen de descendre, on remonte jusqu'à un
		// noeud dont on n'a pas exploré au moins un des fils
		do {
			prov1 = ((current_state->composantes)) & 0x0F;
			current_state = current_state->father;
		} while ((current_state != 0x00) &&
		(prov1 == (((current_state->composantes)>>4)&0x0F)) );
		// Si on n'est pas remonté trop haut, on descend dans le noeud
		// qu'on n'a pas encore exploré...
		if (current_state != 0x00)
			current_state = current_state->etatsActifs[prov1];
	}
		
	}
}

// fonctions à mettre à jour en fonction de votre interruption
void initTimer(void){
;
}

int testTime(void) {
	//TODO
	if (cycle) {
		cycle = false;
		return true;
	}
	return false;
}

#endif
