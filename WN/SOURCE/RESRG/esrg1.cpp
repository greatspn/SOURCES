/***************************************************************************
 *   Copyright (C) 2006 by Marco Beccuti   *
 *   beccuti@mfn.unipmn.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "class.h"
#include <time.h>

using namespace std;

extern void Parser(std::string);
extern Q_list *Create_Q(char *);
extern X_list *Create_X_C(Q_list *&, C_list *&);
extern X_list *Create_X_C_all(Q_list *&, C_list *&);
extern X_list *Create_X_C_exact(Q_list *&, C_list *&);
extern void instance_macro(Marking *);
extern void Lump_test(Q_list *&, C_list *&, X_list  *&);
extern int call_instance_macro;
extern void test(Q_list_el *&);
extern void test_stampa(Q_list *&);
extern void test_stampa1(std::string, Q_list *&, int);
extern void gen_wngr_ctrs(std::string , Q_list *&);

int STRONG = false;
int ESRGtoUNIVERSO = FALSE;
int CTMC_GEN = FALSE, CTMC_SOLVE = FALSE;
int DEBUG = FALSE;
int PRESPLIT = TRUE;
int DTMC = FALSE;
int COUNT = FALSE;

int PEAK = 0;
Q_list *q = new Q_list;


int main(int  argc,  char  *argv[]) {

    time_t time_1, time_2, time_3, time_4;
    if (argv[1] == NULL) {
        cerr << "\n\nUSE: ESRG_CTMC netname [-s|-e|-u|-c|-m]\n\n\t -s compute test strong lumpability\n\n\t -e compute test exact lumpability\n\n\t -u init X=SRG\n\n\t -c compute CTMC in graphical way \n\n\t -m compute CTMC/DTMC \n\n\t -d solve DTMC\n\n";

        exit(EXIT_FAILURE);
    }
    int i = 2;
    cout << "START EXECUTION ON " << argv[1] << endl;
    cout << "\n\n***************************INITIAL PARAMETERS****************************\n\n";
    while (i < argc) {
        switch (argv[i][1]) {
        case 'd':
            DTMC = TRUE;
            cout << "DTMC\n";
            break;
        case 's':
            STRONG = TRUE;
            cout << "TEST STRONG\n";
            break;
        case 'u':
            ESRGtoUNIVERSO = TRUE;
            cout << "STARTING  PARTITION=SRG\n";
            break;
        case 'c':
            CTMC_GEN = TRUE;
            cout << "CTMC graphic generation\n";
            break;
        case 'm':
            CTMC_SOLVE = TRUE;
            cout << "CTMC generation\n";
            break;
        case 'e':
            cout << "TEST EXACT\n";
            break;
        case 'n':
            cout << "ESMs+EVs count\n";
            COUNT = TRUE;
            break;
        default:
            cout << "PARAMETER ERROR\n\n*************************************************************************\n\n";
            cerr << "\n\nUSE: ESRG_CTMC netname [-s|-e|-u|-c|-m|-d]\n\n\t -s compute test strong lumpability\n\t -e compute test exact lumpability\n\t -u init X=SRG\n\t -c compute CTMC in graphical way \n\t -n compute ESM+EV number\n\t -m compute CTMC/DTMC \n\t -d solve DTMC\n\n" << endl;
            exit(EXIT_FAILURE);
            break;
        }
        i++;
    }
    cout << "\n\n*************************************************************************\n\n";

    time(&time_1);
    std::string net_name(argv[1]);
    q = Create_Q(argv[1]);
    if (COUNT) {
        cout << "\n\n*************************************************************************\n\n";
        cout << "Inital ESMs and Evs number: " << q->Compute_ESMEV() << endl;
        cout << "\n\n*************************************************************************\n\n";
    }
    time(&time_2);
    X_list *x;
    C_list *c = NULL;
    // instance_macro(q->search(5));
    cout << "START PRESPLIT\n";
    if (!(ESRGtoUNIVERSO)) {
        if (!STRONG) {
            if (DTMC)
                Compute_denom(q);
            x = Create_X_C_exact(q, c);

        }
        else {
            if (DTMC)
                Compute_denom(q);
            x = Create_X_C(q, c);

        }
    }
    else {
        if (!STRONG)
            x = Create_X_C_all_exact(q, c);
        else
            x = Create_X_C_all(q, c);
    }
    cout<<"\tBlocks  in Q:"<<q->get_num_elem()<<endl;
    
    cout << "END PRESPLIT\n";
    //memorizzo il numero di volte che ho chiamato la funzione di istance_macro nel presplit
    int presplit_instance_macro = call_instance_macro;
    call_instance_macro = 0;
    //memorizzo il numero di volte che ho chiamato la funzione di istance_macro nel presplit
    time(&time_3);
    Lump_test(q, c, x);
#if TESTSTAMPA
    Q_list_el *block1 = q->get_block();
    while (block1 != NULL) {
        cout << "ID BLOCCO: " << block1->get_id() << "\n";
        try {
            //makepair(block1,in);
            cout << make_pair(block1, in) << endl;
        }
        catch (Exception obj) {
            cout << "\nException" << obj.get() << endl;
        }
        block1 = block1->get_pointer(nxt);
    }
    draw_graph(argv[1], q);
#endif
    time(&time_4);
    delete(x);

    PEAK = q->Compute_peak();
    if (DTMC) {
        std::string net = std::string(argv[1]) + ".bk";
        ofstream out(net.c_str(), ofstream::out);

        if (!out) {
            cerr << "Error opening output stream bk" << endl;
            exit(EXIT_FAILURE);
        }
        try {
            order_id_block(q);
            write_block(out, q);
            if (!STRONG)
                genEX_DTMC(argv[1], q);
            else
                genST_DTMC(argv[1], q);
        }
        catch (Exception obj) {
            cout << "\nException" << obj.get() << endl;
        }
    }
    if (CTMC_GEN) {
        order_id_block(q);
        try {
            genEX_DTMC(argv[1], q);
            test_stampa1(argv[1], q, 0);
        }
        catch (ExceptionIO) {
            exit(EXIT_FAILURE);
        }
    }
    if (CTMC_SOLVE) {
        order_id_block(q);
        try {
            gen_wngr_ctrs(argv[1], q);

        }
        catch (ExceptionIO) {
            exit(EXIT_FAILURE);
        }
    }
    cout << "END EXECUTION" << endl;
    cout << "\n=============================== ESRG TIME================================\n\n";
    cout << "Total time required: " << (time_4 - time_1) << "s\n\t Time Creat_Q() fuction: " << (time_2 - time_1) << "s\n\t Time Creat_X_C() fuction: " << (time_3 - time_2) << "s\n\t Time Lump_test() fuction: " << (time_4 - time_3) << "s\n";
    cout << "\nBlocks  in Q: " << q->get_num_elem() << endl;
    cout << "\nMemory peak: " << PEAK << endl;
#ifdef debug
    cout << "\nMemory peak with cache: " << q->Compute_Peak_withCache() << endl;
#endif
    cout << "\nNum instance_macro() calls: " << presplit_instance_macro << " in Creat_X_C() " << call_instance_macro << " in Lump_test()";
    cout << "\n\n=============================== ESRG ====================================\n\n";
}



