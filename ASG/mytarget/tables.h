#include <curses.h>
// ************************************************** //
// Parametres definis pour optimiser a la compilation //
// ************************************************** //
// defini lorsque le graphe comporte des evenements
// defini lorsque le graphe comporte des rendez-vous
// defini lorsque le graphe comporte des minvts
// defini lorsque le graphe comporte des ressources
// defini lorsque le graphe comporte des timeouts
// defini lorsque le graphe comporte des minvts ou des timeouts (ou les deux)


// *************************************************** //
// Inclusion des definitions des structures de donnees //
// *************************************************** //
// Les structures de donnees sont inclues ici car elles doivent etre
// optimises en fonction des differents elements se trouvant dans le graphe.
// Les parametres definis en haut permettent de determiner ces optimisations.
#include "moteur_ASG.h"

// ********************************************** //
// Declaration de l'ensemble des tables du graphe //
// ********************************************** //

etat E6, E1, E8, E2, E4, E9, E7, E5, E3;

etat* rootState = &E1;

// ********************************************* //
// Definition de l'ensemble des tables du graphe //
// ********************************************* //

/* ************************* Etat 6 : Sending Msg ************************* */


char action_E6(void) {

}

etat E6 = {&E1, 0x02, 0, 0x00, 0, 0, action_E6, 0};


/* ************************* Etat 1 : Storing Msg ************************* */


char action_E1(void) {
	activateState(&E9);
	activateState(&E7);
	activateState(&E5);

}

etat* AS1[3] = {0x00, 0x00, 0x00};

etat E1 = {0, 0x30, 0, 0x00, 0, 0, action_E1, AS1};


/* ************************* Etat 8 : Sending Msg ************************* */


char action_E8(void) {

}

etat E8 = {&E1, 0x01, 0, 0x00, 0, 0, action_E8, 0};


/* ************************* Etat 2 : Etat 10 ************************* */


char action_E2(void) {

}

etat E2 = {&E1, 0x03, 0, 0x00, 0, 0, action_E2, 0};


/* ************************* Etat 4 : Etat 8 ************************* */


char action_E4(void) {

}

etat E4 = {&E1, 0x03, 0, 0x00, 0, 0, action_E4, 0};


/* ************************* Etat 9 : Waiting ************************* */


char action_E9(void) {

}

etat E9 = {&E1, 0x01, 0, 0x00, 0, 0, action_E9, 0};


/* ************************* Etat 7 : Etat 5 ************************* */


char action_E7(void) {

}

etat E7 = {&E1, 0x02, 0, 0x00, 0, 0, action_E7, 0};


/* ************************* Etat 5 : Waiting ************************* */


char action_E5(void) {

}

etat E5 = {&E1, 0x03, 0, 0x00, 0, 0, action_E5, 0};


/* ************************* Etat 3 : Etat 9 ************************* */


char action_E3(void) {

}

etat E3 = {&E1, 0x03, 0, 0x00, 0, 0, action_E3, 0};


