#include <curses.h>
// ************************************************** //
// Parametres definis pour optimiser a la compilation //
// ************************************************** //
// defini lorsque le graphe comporte des evenements
#define EVENEMENT
// defini lorsque le graphe comporte des rendez-vous
#define RENDEZ_VOUS
// defini lorsque le graphe comporte des minvts
#define MINVT
// defini lorsque le graphe comporte des ressources
// defini lorsque le graphe comporte des timeouts
// defini lorsque le graphe comporte des minvts ou des timeouts (ou les deux)
#define MINVT_OR_TIMEOUT


// *************************************************** //
// Inclusion des definitions des structures de donnees //
// *************************************************** //
// Les structures de donnees sont inclues ici car elles doivent etre
// optimises en fonction des differents elements se trouvant dans le graphe.
// Les parametres definis en haut permettent de determiner ces optimisations.
#include "moteur_ASG.h"

// ************************************** //
// Definition des vecteurs de rendez-vous //
// ************************************** //
// nombre de rendez-vous dans le graphe
#define NB_RDVS 4
// tableau contenant le nombre de rendez-vous qu'il faut atteindre
unsigned char neededRdvs1[4] = {1 ,1 ,1 ,1};
// tableau contenant le nombre de rendez-vous effectivement atteint
unsigned char reachedRdvs1[4] = {0 ,0 ,0 ,0};

// *********************************** //
// Definition des vecteurs d'evenement //
// *********************************** //
#define NB_EVENEMENT 3
int EVENEMENTS1;

// ********************************************** //
// Declaration de l'ensemble des tables du graphe //
// ********************************************** //

etat E8, E1, E14, E7, E2, E16, E15, E9, E11, E6, E4, E10, E5, E17, E13, E12, E3;

etat* rootState = &E1;

// ********************************************* //
// Definition de l'ensemble des tables du graphe //
// ********************************************* //

/* ************************* Etat 8 : LedOn ************************* */

transition t8_1 = {1, 0x00, &E5};

transition* t8[1] = {&t8_1};

/*

*/

char action_E8(void) {


}

etat E8 = {&E4, 0x01, 0, t8, 0x01, 0, 0, action_E8, 0};


/* ************************* Etat 1 : Main ************************* */


char action_E1(void) {
	activateState(&E17);

}

etat* AS1[1] = {0x00};

etat E1 = {0, 0x10, 0, 0, 0x00, 0, 0, action_E1, AS1};


/* ************************* Etat 14 : MRunning ************************* */

transition t14_1 = {1, 0x00, &E13};

transition* t14[1] = {&t14_1};

char rdv14_1 = {3};
char* rdv14[1] = {&rdv14_1};

char action_E14(void) {

}

etat E14 = {&E12, 0x01, rdv14, t14, 0x11, 0, 0, action_E14, 0};


/* ************************* Etat 7 : LedBlinkOff ************************* */

transition t7_1 = {0, 0x00, &E6};

transition* t7[1] = {&t7_1};

/*

*/

char action_E7(void) {


}

etat E7 = {&E5, 0x01, 0, t7, 0x01, 10, 0, action_E7, 0};


/* ************************* Etat 2 : AppMotor ************************* */


char action_E2(void) {
	activateState(&E16);
	activateState(&E9);
	activateState(&E3);

}

etat* AS2[3] = {0x00, 0x00, 0x00};

etat E2 = {&E1, 0x31, 0, 0, 0x00, 0, 0, action_E2, AS2};


/* ************************* Etat 16 : Moff ************************* */

transition t16_1 = {0, 0x08, &E12};

transition* t16[1] = {&t16_1};

char rdv16_1 = {2};
char* rdv16[1] = {&rdv16_1};

/*

*/

char action_E16(void) {


}

etat E16 = {&E2, 0x01, rdv16, t16, 0x11, 0, 0, action_E16, 0};


/* ************************* Etat 15 : MStarting ************************* */

transition t15_1 = {1, 0x00, &E13};
transition t15_2 = {0, 0x00, &E14};

transition* t15[2] = {&t15_1, &t15_2};

char rdv15_1 = {1};
char* rdv15[1] = {&rdv15_1};

/*

*/

char action_E15(void) {


}

etat E15 = {&E12, 0x01, rdv15, t15, 0x12, 20, 0, action_E15, 0};


/* ************************* Etat 9 : Button Up ************************* */

transition t9_1 = {2, 0x00, &E11};

transition* t9[1] = {&t9_1};

char action_E9(void) {

}

etat E9 = {&E2, 0x02, 0, t9, 0x01, 10, 0, action_E9, 0};


/* ************************* Etat 11 : Button Pressed ************************* */

transition t11_1 = {0, 0x00, &E10};

transition* t11[1] = {&t11_1};

char rdv11_1 = {4};
char* rdv11[1] = {&rdv11_1};

char action_E11(void) {

}

etat E11 = {&E2, 0x02, rdv11, t11, 0x11, 0, 0, action_E11, 0};


/* ************************* Etat 6 : LedBlinkOn ************************* */

transition t6_1 = {0, 0x00, &E7};

transition* t6[1] = {&t6_1};

/*

*/

char action_E6(void) {


}

etat E6 = {&E5, 0x01, 0, t6, 0x01, 10, 0, action_E6, 0};


/* ************************* Etat 4 : LedAuto ************************* */

transition t4_1 = {0, 0x02, &E3};

transition* t4[1] = {&t4_1};

char action_E4(void) {
	activateState(&E8);

}

etat* AS4[1] = {0x00};

etat E4 = {&E2, 0x13, 0, t4, 0x01, 0, 0, action_E4, AS4};


/* ************************* Etat 10 : Button Down ************************* */

transition t10_1 = {4, 0x00, &E9};

transition* t10[1] = {&t10_1};

char action_E10(void) {

}

etat E10 = {&E2, 0x02, 0, t10, 0x01, 10, 0, action_E10, 0};


/* ************************* Etat 5 : LedBlink ************************* */

transition t5_1 = {0, 0x04, &E8};

transition* t5[1] = {&t5_1};

char action_E5(void) {
	activateState(&E7);

}

etat* AS5[1] = {0x00};

etat E5 = {&E4, 0x11, 0, t5, 0x01, 0, 0, action_E5, AS5};


/* ************************* Etat 17 : init ************************* */

transition t17_1 = {0, 0x00, &E2};

transition* t17[1] = {&t17_1};

/*

*/

char action_E17(void) {


}

etat E17 = {&E1, 0x01, 0, t17, 0x01, 0, 0, action_E17, 0};


/* ************************* Etat 13 : Moverload ************************* */

transition t13_1 = {0, 0x00, &E15};

transition* t13[1] = {&t13_1};

/*

*/

char action_E13(void) {


}

etat E13 = {&E12, 0x01, 0, t13, 0x01, 40, 0, action_E13, 0};


/* ************************* Etat 12 : Mon ************************* */

transition t12_1 = {0, 0x08, &E16};

transition* t12[1] = {&t12_1};

char action_E12(void) {
	activateState(&E15);

}

etat* AS12[1] = {0x00};

etat E12 = {&E2, 0x11, 0, t12, 0x01, 0, 0, action_E12, AS12};


/* ************************* Etat 3 : Led Off ************************* */

transition t3_1 = {0, 0x01, &E4};

transition* t3[1] = {&t3_1};

/*

*/

char action_E3(void) {


}

etat E3 = {&E2, 0x03, 0, t3, 0x01, 0, 0, action_E3, 0};




void updateEvents(void) {
/* bit 1:  */
if (0) bitset(EVENEMENTS1,0); else bitclr(EVENEMENTS1,0);
/* bit 3:  */
if (0) bitset(EVENEMENTS1,2); else bitclr(EVENEMENTS1,2);
/* bit 2:  */
if (0) bitset(EVENEMENTS1,1); else bitclr(EVENEMENTS1,1);
}
