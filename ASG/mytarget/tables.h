#include <curses.h>
// ************************************************** //
// Parametres definis pour optimiser a la compilation //
// ************************************************** //
// defini lorsque le graphe comporte des evenements
#define EVENEMENT
// defini lorsque le graphe comporte des rendez-vous
#define RENDEZ_VOUS
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

// ************************************** //
// Definition des vecteurs de rendez-vous //
// ************************************** //
// nombre de rendez-vous dans le graphe
#define NB_RDVS 1
// tableau contenant le nombre de rendez-vous qu'il faut atteindre
unsigned char neededRdvs1[1] = {1};
// tableau contenant le nombre de rendez-vous effectivement atteint
unsigned char reachedRdvs1[1] = {0};

// *********************************** //
// Definition des vecteurs d'evenement //
// *********************************** //
#define NB_EVENEMENT 10
int EVENEMENTS1;

// ********************************************** //
// Declaration de l'ensemble des tables du graphe //
// ********************************************** //

etat E13, E21, E12, E4, E9, E19, E7, E16, E20, E15, E6, E11, E8, E10, E3, E5, E2, E17, E1, E14, E18;

etat* rootState = &E1;

// ********************************************* //
// Definition de l'ensemble des tables du graphe //
// ********************************************* //

/* ************************* Etat 13 : Process UnAck ************************* */

transition t13_1 = {0, 0x00, &E16};

transition* t13[1] = {&t13_1};

/*

*/

char action_E13(void) {

remove_and_prepare_unack_forward();
}

etat E13 = {&E1, 0x03, 0, t13, 0x01, 0, 0, action_E13, 0};


/* ************************* Etat 21 : Enqueued Msg ************************* */

transition t21_1 = {0, 0x00, &E20};

transition* t21[1] = {&t21_1};

char rdv21_1 = {1};
char* rdv21[1] = {&rdv21_1};

char action_E21(void) {

}

etat E21 = {&E1, 0x01, rdv21, t21, 0x11, 0, 0, action_E21, 0};


/* ************************* Etat 12 : Process Decline ************************* */

transition t12_1 = {0, 0x00, &E16};

transition* t12[1] = {&t12_1};

/*

*/

char action_E12(void) {

prepare_decline_forward();
}

etat E12 = {&E1, 0x03, 0, t12, 0x01, 0, 0, action_E12, 0};


/* ************************* Etat 4 : Renewing ************************* */

transition t4_1 = {0, 0x00, &E5};

transition* t4[1] = {&t4_1};

/*

*/

char action_E4(void) {


}

etat E4 = {&E1, 0x04, 0, t4, 0x01, 0, 0, action_E4, 0};


/* ************************* Etat 9 : Process Offer Msg ************************* */

transition t9_1 = {0, 0x00, &E16};

transition* t9[1] = {&t9_1};

/*

*/

char action_E9(void) {


}

etat E9 = {&E1, 0x03, 0, t9, 0x01, 0, 0, action_E9, 0};


/* ************************* Etat 19 : Enqueuing Msg ************************* */

transition t19_1 = {0, 0x00, &E21};

transition* t19[1] = {&t19_1};

/*

*/

char action_E19(void) {


}

etat E19 = {&E1, 0x01, 0, t19, 0x01, 0, 0, action_E19, 0};


/* ************************* Etat 7 : Process Ack Msg ************************* */

transition t7_1 = {0, 0x00, &E18};
transition t7_2 = {0, 0x00, &E6};

transition* t7[2] = {&t7_1, &t7_2};

/*

*/

char action_E7(void) {

look_into_table();
}

etat E7 = {&E1, 0x03, 0, t7, 0x02, 0, 0, action_E7, 0};


/* ************************* Etat 16 : Sending Msg ************************* */

transition t16_1 = {0, 0x00, &E14};

transition* t16[1] = {&t16_1};

/*

*/

char action_E16(void) {


}

etat E16 = {&E1, 0x03, 0, t16, 0x01, 0, 0, action_E16, 0};


/* ************************* Etat 20 : Waiting ************************* */

transition t20_1 = {512, 0x00, &E19};

transition* t20[1] = {&t20_1};

char action_E20(void) {

}

etat E20 = {&E1, 0x01, 0, t20, 0x01, 0, 0, action_E20, 0};


/* ************************* Etat 15 : Request from known ************************* */

transition t15_1 = {0, 0x00, &E16};

transition* t15[1] = {&t15_1};

/*

*/

char action_E15(void) {

prepare_ack_send_and_update_timers();
}

etat E15 = {&E1, 0x03, 0, t15, 0x01, 0, 0, action_E15, 0};


/* ************************* Etat 6 : Ack from Unknown ************************* */

transition t6_1 = {0, 0x00, &E16};

transition* t6[1] = {&t6_1};

/*

*/

char action_E6(void) {

store_informations_and_prepare_ack_send();
}

etat E6 = {&E1, 0x03, 0, t6, 0x01, 0, 0, action_E6, 0};


/* ************************* Etat 11 : Cheking type ************************* */

transition t11_1 = {8, 0x00, &E7};
transition t11_2 = {16, 0x00, &E8};
transition t11_3 = {32, 0x00, &E9};
transition t11_4 = {64, 0x00, &E12};
transition t11_5 = {128, 0x00, &E10};
transition t11_6 = {256, 0x00, &E13};

transition* t11[6] = {&t11_1, &t11_2, &t11_3, &t11_4, &t11_5, &t11_6};

/*

*/

char action_E11(void) {


}

etat E11 = {&E1, 0x03, 0, t11, 0x06, 0, 0, action_E11, 0};


/* ************************* Etat 8 : Process Requests ************************* */

transition t8_1 = {0, 0x00, &E15};
transition t8_2 = {0, 0x00, &E17};

transition* t8[2] = {&t8_1, &t8_2};

/*

*/

char action_E8(void) {

look_into_table();
}

etat E8 = {&E1, 0x03, 0, t8, 0x02, 0, 0, action_E8, 0};


/* ************************* Etat 10 : Process Discover Msg ************************* */

transition t10_1 = {0, 0x00, &E16};

transition* t10[1] = {&t10_1};

/*

*/

char action_E10(void) {


}

etat E10 = {&E1, 0x03, 0, t10, 0x01, 0, 0, action_E10, 0};


/* ************************* Etat 3 : Removing ************************* */

transition t3_1 = {0, 0x00, &E2};
transition t3_2 = {0, 0x00, &E5};

transition* t3[2] = {&t3_1, &t3_2};

char action_E3(void) {

}

etat E3 = {&E1, 0x04, 0, t3, 0x02, 0, 0, action_E3, 0};


/* ************************* Etat 5 : NSA activities ************************* */

transition t5_1 = {2, 0x00, &E4};
transition t5_2 = {4, 0x00, &E3};

transition* t5[2] = {&t5_1, &t5_2};

char action_E5(void) {

}

etat E5 = {&E1, 0x04, 0, t5, 0x02, 0, 0, action_E5, 0};


/* ************************* Etat 2 : No Client ************************* */

transition t2_1 = {1, 0x00, &E5};

transition* t2[1] = {&t2_1};

char action_E2(void) {

}

etat E2 = {&E1, 0x04, 0, t2, 0x01, 0, 0, action_E2, 0};


/* ************************* Etat 17 : Request from Unknow ************************* */

transition t17_1 = {0, 0x00, &E16};

transition* t17[1] = {&t17_1};

/*

*/

char action_E17(void) {

prepare_request_forward();
}

etat E17 = {&E1, 0x03, 0, t17, 0x01, 0, 0, action_E17, 0};


/* ************************* Etat 1 : Storing Msg ************************* */


char action_E1(void) {
	activateState(&E20);
	activateState(&E14);
	activateState(&E2);

}

etat* AS1[4] = {0x00, 0x00, 0x00};

etat E1 = {0, 0x30, 0, 0, 0x00, 0, 0, action_E1, AS1};


/* ************************* Etat 14 : Idle ************************* */

transition t14_1 = {0, 0x01, &E11};

transition* t14[1] = {&t14_1};

char action_E14(void) {

}

etat E14 = {&E1, 0x03, 0, t14, 0x01, 0, 0, action_E14, 0};


/* ************************* Etat 18 : Ack from known ************************* */

transition t18_1 = {0, 0x00, &E14};

transition* t18[1] = {&t18_1};

/*

*/

char action_E18(void) {

update_informations();
}

etat E18 = {&E1, 0x03, 0, t18, 0x01, 0, 0, action_E18, 0};




void updateEvents(void) {
/* bit 5:  */
if (request()) bitset(EVENEMENTS1,4); else bitclr(EVENEMENTS1,4);
/* bit 8:  */
if (discover()) bitset(EVENEMENTS1,7); else bitclr(EVENEMENTS1,7);
/* bit 1:  */
if (new_client()) bitset(EVENEMENTS1,0); else bitclr(EVENEMENTS1,0);
/* bit 2:  */
if (timer_expires()) bitset(EVENEMENTS1,1); else bitclr(EVENEMENTS1,1);
/* bit 4:  */
if (ack()) bitset(EVENEMENTS1,3); else bitclr(EVENEMENTS1,3);
/* bit 10:  */
if (0) bitset(EVENEMENTS1,9); else bitclr(EVENEMENTS1,9);
/* bit 6:  */
if (offer()) bitset(EVENEMENTS1,5); else bitclr(EVENEMENTS1,5);
/* bit 7:  */
if (decline()) bitset(EVENEMENTS1,6); else bitclr(EVENEMENTS1,6);
/* bit 9:  */
if (unack()) bitset(EVENEMENTS1,8); else bitclr(EVENEMENTS1,8);
/* bit 3:  */
if (0) bitset(EVENEMENTS1,2); else bitclr(EVENEMENTS1,2);
}
