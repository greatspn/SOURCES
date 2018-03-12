/*
 *  Torino, September 17, 1987
 *  program: comp_menc.c
 *  purpose: compute marking encoding for RG construction of a GSPN
 *           using M-invariants information.
 *  programmer: Giovanni Chiola
 *  notes:
 *    1) the net description must be in GreatSPN format
 */


/*
#define DEBUG
*/


#include <stdio.h>
#include <stdlib.h>

extern FILE *nfp, * ifp, * ccfp, * scfp, * ofp, * ssfp;

#include "const.h"

#include "comp.h"

extern struct place_obj *p_list;

extern struct I_header *M_invars, * unused;
extern int no_M_invars, no_alloc_inv, no_unused;
extern int no_alloc_pl;

extern struct C_mark *encoding, * cccp;
extern int no_cod, no_byte;

extern int	place_num, trans_num;

extern unsigned int nobits();

extern char *ecalloc(unsigned nbytes, unsigned no);

extern void load_places();

extern void load_Pinvars();

extern int inv_cmp();

void insert_pl(struct place_obj **plp, struct place_obj *pp);
void delete_place(struct I_header *list, int np);


unsigned int comp_code(tc, np)
unsigned int tc, np;
{
    unsigned cod_per_byte = 8 / nobits(np);
    unsigned nn = tc / cod_per_byte;

    if ((tc % cod_per_byte))
        ++nn;
    if (nn)
        return (nn);
    else
        return (1);
}

unsigned int long_code(tc, np)
unsigned int tc, np;
{
    unsigned cod_per_byte = 8 / nobits(tc);
    unsigned nn = np / cod_per_byte;

    if ((np % cod_per_byte))
        ++nn;
    if (nn)
        return (nn);
    else
        return (1);
}

void alloc_places() {
    struct I_header *I_p;
    struct place_obj *pp;
    ROWP rp;
    int np, ii;
    unsigned int cc, lc;
    unsigned int cod, num_inv;

#ifdef DEBUG
    printf("  Start of alloc_places\n");
#endif
    for (I_p = M_invars, unused = M_invars + 1, no_alloc_inv = 0 ;
            no_unused-- ; I_p = unused++) {
        unsigned int mult = I_p->tcount;

        no_alloc_inv++;
        num_inv = (I_p - M_invars) + 1;
        if (mult < MAX_mult) {
            if ((int)(I_p->nouncov) < (int)(*(I_p->rowp)))
                cc = comp_code(mult, (unsigned int)I_p->nouncov);
            else
                cc = comp_code(mult, (unsigned int)(I_p->nouncov - 1));
            if (cc < 8 && cc < (lc = long_code(mult, I_p->nouncov))) {
                /* COMPACT CODING */
                I_p->cod_type = mult;
                no_cod++;
                if (I_p->nouncov < * (I_p->rowp))
                    cod = I_p->nouncov;
                else
                    cod = I_p->nouncov - 1;
                I_p->max_c = cod;
                I_p->cod_type |= ((cc << 3) & 56);
            }
            else {
                /* LONG CODING */
                I_p->cod_type = 0;
                I_p->max_c = 0;
                cod = I_p->nouncov - 1;
                no_cod += cod + 1;
            }
        }
        else {
            /* LONG CODING */
            I_p->cod_type = 0;
            I_p->max_c = 0;
            cod = I_p->nouncov - 1;
            no_cod += cod + 1;
        }
        no_alloc_pl += (int)I_p->nouncov;
#ifdef DEBUG
        printf("invariant %d with coding %d\n", num_inv, (int)I_p->cod_type);
#endif
        for (ii = I_p->nouncov, rp = I_p->rowp + 2 ; ii-- ;) {
            for (np = *rp, pp = p_list + (np - 1) ; (int)pp->covered ;
                    rp += 2, np = *rp, pp = p_list + (np - 1));
            pp->mult = (*(rp - 1));
            pp->covered = num_inv;
            insert_pl(&(I_p->p_l), pp);
            pp->enc = (cod--);
            fprintf(ssfp, "double MatP%d[%d];\n", (int)(pp - p_list), mult + 1);
            fprintf(ssfp, "double Mat2P%d[%d];\n", (int)(pp - p_list), mult + 1);
#ifdef DEBUG
            printf("  place %d allocated to M-inv %d with coding %d\n",
                   np, (int)pp->covered, (int)pp->enc);
#endif
            if (no_unused)
                delete_place(unused, np);
        }
        if (no_unused) {
            int i;
            qsort(unused, no_unused, I_H_SIZE, inv_cmp);
#ifdef DEBUG
            printf("\nUnallocated M-invars :\n");
            {
                int i, j; struct I_header *rp; ROWP cp;
                for (rp = unused, i = no_unused; i-- ; rp++) {
                    printf("\ntc=%d, uncov=%d\n", (int)rp->tcount, (int)rp->nouncov);
                    for (cp = rp->rowp, j = (int)(*(cp++)) ; j-- ; cp++) {
                        if ((int)(*cp) != 1)
                            printf(" %d*", (int)(*cp));
                        else
                            printf("   ");
                        printf("%d", (int)(*(++cp)));
                    }
                }
                printf("\n");
            }
#endif
            for (i = no_unused, no_unused = 0, I_p = unused; i-- ; I_p++)
                if ((int)(I_p->nouncov))
                    no_unused++;
#ifdef DEBUG
            printf("\n    no_unused=%d\n", no_unused);
#endif
        }
    }
#ifdef DEBUG
    printf("  End of alloc_places\n");
#endif
}

void
insert_pl(plp, pp)
struct place_obj **plp;
struct place_obj *pp;
{
    pp->next = *plp;
    *plp = pp;
}

void
delete_place(list, np)
struct I_header *list;
int np;
{
    register ROWP cp;
    register int jj;
    int ii = no_unused;
    while (ii--) {
        if (list->nouncov > 0) {
            cp = list->rowp;
            for (jj = *(cp++), cp++ ; jj-- ; cp += 2)
                if ((int)(*cp) == np) {
                    list->nouncov -= 1;
                    goto next_inv;
                }
        }
next_inv:
        list++;
    }
}

unsigned u_exp(n1, n2)
unsigned n1, n2;
{
    if (n2 < 2)
        return (n1);
    else
        return (n1 * u_exp(n1, n2 - 1));
}

void
encode_markings() {
    extern int cod_cmp(), cod_cmp2();
    struct C_mark *ccp, * ncp;
    struct I_header *ip;
    struct place_obj *pp;
    int ii, jj;
    int kkk;

#ifdef DEBUG
    printf("  Start of encode_markings\n");
#endif
    encoding = (struct C_mark *)
               ecalloc((3 * no_cod) / 2 + place_num - no_alloc_pl + 1, C_M_SIZE);
    no_cod += (place_num - no_alloc_pl);
    for (cccp = ccp = encoding, ip = M_invars, ii = no_alloc_inv ;
            ii-- ; ip++) {
        if ((int)ip->cod_type) {
            unsigned ii = ip->cod_type;
            ccp->cod_type = ii;
            ccp->from0 = (ip->nouncov >= *(ip->rowp));
            ccp->max_val = (u_exp((unsigned)(ip->max_c + 1), ((ii & 7) / ((ii & 56) >> 3))) - 1);
            ccp->max_c = ip->max_c;
            ccp->p_l = pp = ip->p_l;
            if ((ii & 7) > 1) {
                unsigned char *op = ccp->offset;
                unsigned char *mp = ccp->mask;
                unsigned char mm = 1;
                unsigned int jj, nb = nobits(ip->max_c);
                unsigned int maxjj = 8 / nb;
                for (jj = nb - 1 ; jj-- ; mm |= (mm) << 1);
                *(op++) = 0;
                *(mp++) = mm;
#ifdef DEBUG
                printf("    place %d  mask=%d, offset=%d\n",
                       (int)(pp - p_list) + 1, (int) * (mp - 1), nb);
#endif
                for (jj = 1 ; jj < maxjj ;
                        * (op++) = (jj * nb),
                        *(mp++) = (mm << (nb * jj++)));
            }
            else
                ccp->nobit = nobits(ccp->max_val);
            ccp++;
            for (; pp != NULL ; pp = pp->next)
                pp->covered = (ccp - encoding);
        }
        else
            for (pp = ip->p_l ; pp != NULL ; pp = pp->next) {
                ccp->cod_type = 0;
                ccp->max_val = (ip->tcount / pp->mult);
                ccp->nobit = nobits(ccp->max_val);
                ccp->max_c = ccp->max_val;
                ccp->p_l = pp;
                ccp++;
                pp->covered = (ccp - encoding);
            }
    }
    if (no_alloc_pl < place_num)
        for (ii = place_num - no_alloc_pl, pp = p_list ; ii ; pp++)
            if (!(int)(pp->covered)) {
                --ii;
                ccp->cod_type = 0;
                ccp->max_c = (255);
                ccp->max_val = (255);
                ccp->nobit = 8;
                ccp->p_l = pp;
                fprintf(ssfp, "double MatP%d[256];\n", (int)(pp - p_list));
                fprintf(ssfp, "double Mat2P%d[256];\n", (int)(pp - p_list));
                ccp++;
                pp->covered = (ccp - encoding);
            }
    {
        int ii = 0;
        fprintf(ssfp, "double * MATPR[%d] = { 0\n", place_num + 1);
        for (; ii < place_num ; ++ii)
            fprintf(ssfp, ", MatP%d", ii);
        fprintf(ssfp, " };\n");
        fprintf(ssfp, "double * MATPR2[%d] = { 0\n", place_num + 1);
        for (ii = 0 ; ii < place_num ; ++ii)
            fprintf(ssfp, ", Mat2P%d", ii);
        fprintf(ssfp, " };\n");
    }
#ifdef DEBUG
    for (ccp = encoding, ii = no_cod ; ii-- ; ccp++) {
        printf("    coding #%d  type %d  range 0..%d (%d bit)\n",
               (no_cod - ii), (int)ccp->cod_type, (int)ccp->max_val, (int)ccp->nobit);
        if ((int)ccp->cod_type)
            for (pp = ccp->p_l ; pp != NULL ; pp = pp->next)
                printf("      place #%d as %d (%d)\n",
                       (int)(pp - p_list) + 1, (int)pp->enc, (int)pp->covered);
        else
            printf("      place #%d (%d)\n",
                   (int)(ccp->p_l - p_list) + 1, (int)ccp->p_l->covered);
    }
#endif
#ifdef DEBUG
    printf("    sorting %d codings\n", no_cod);
    {
        int old_cod = no_cod;
#endif
        qsort(encoding, no_cod, C_M_SIZE, cod_cmp);
        ncp = encoding + (no_cod - 1);
        ncp->link = NULL; cccp = encoding;
        for (ccp = encoding, ii = no_cod, no_cod = 0 ;
                (((ccp->cod_type) & 7) < 2) && (ccp->nobit < 6) && (ii--) ;
                ccp++) {
#ifdef DEBUG
            printf("      testing %d, cccp=%d\n", old_cod - ii, (cccp - encoding) + 1);
#endif
            if (ccp > cccp) {
                if (cccp->cod_type & 64) {
#ifdef DEBUG
                    printf("        already compact ");
#endif
                    if ((ncp->nobit + ccp->nobit) <= 8) {
#ifdef DEBUG
                        printf("fits\n");
#endif
                        ccp->cod_type |= 64;
                        ccp->link = ncp->link; ncp->link = ccp;
                        ncp->nobit += ccp->nobit;
                        ncp->max_val = (ncp->max_val + 1) * (ccp->max_val + 1) - 1;
                    }
                    else {
#ifdef DEBUG
                        printf("doesn't fit\n");
#endif
                        ++no_cod;
                    }
                    cccp = ccp;
                }
                else {
                    int nb = (cccp->nobit + ccp->nobit);
#ifdef DEBUG
                    printf("        not compact, nb=%d ", nb);
#endif
                    if (nb <= 8) {
#ifdef DEBUG
                        printf("fits\n");
#endif
                        ncp++;
                        ncp->nobit = nb;
                        ncp->max_val = (cccp->max_val + 1) * (ccp->max_val + 1) - 1;
                        ncp->link = ccp; ccp->link = cccp; cccp->link = NULL;
                        ccp->cod_type |= 64; cccp->cod_type |= 64;
                        ncp->cod_type = 128;
                        cccp = ccp;
                    }
                    else {
                        ++no_cod;
#ifdef DEBUG
                        printf("doesn't fit\n");
#endif
                        goto out_of_for;
                    }
                }
            }
            else
                ++no_cod;
        }
out_of_for:
        if (cccp->cod_type & 64)
            cccp++;
        for (; (ii--) > 0 ; ccp++) {
            if (ccp->cod_type)
                no_cod += (((ccp->cod_type) & 56) >> 3);
            else
                ++no_cod;
        }
#ifdef DEBUG
    }
    printf("   end testing, cccp=%d, ncp=%d\n",
           (cccp - encoding) + 1, (ncp - encoding) + 1);
#endif
    no_byte = (ncp - cccp) + 1;
#ifdef DEBUG
    printf("    re-sorting %d bytes\n", no_byte);
#endif
    qsort(cccp, no_byte, C_M_SIZE, cod_cmp2);
#ifdef DEBUG
    printf("    %d bytes re-sorted\n", no_byte);
#endif
    for (kkk = 0, ccp = cccp, ii = no_byte ; ii-- ; ccp++) {
#ifdef DEBUG
        printf("       for ii=%d\n", ii + 1);
#endif
        if (ccp->cod_type == 128) {
            struct C_mark *cccc = ccp->link;
            unsigned int offs = 0;
            for (; cccc != NULL ; cccc = cccc->link) {
                unsigned char mm = 1;
                unsigned int jj;
#ifdef DEBUG
                printf("           for cccc, nobit=\n", cccc->nobit);
#endif
                for (jj = cccc->nobit - 1 ; jj-- ; mm |= (mm) << 1);
                cccc->offset[0] = offs;
                cccc->mask[0] = (mm << offs);
                offs += cccc->nobit;
                cccc->covered = (ccp - encoding) + 1;
                if ((int)(cccc->cod_type & 63))
                    for (pp = cccc->p_l ; pp != NULL ; pp = pp->next)
                        pp->covered = ((cccc - encoding) + 1);
                else
                    cccc->p_l->covered = ((cccc - encoding) + 1);
#ifdef DEBUG
                printf("           end for cccc\n");
#endif
            }
        }
        else if (ccp->cod_type)
            for (pp = ccp->p_l ; pp != NULL ; pp = pp->next)
                pp->covered = ((ccp - encoding) + 1);
        else
            ccp->p_l->covered = ((ccp - encoding) + 1);
        ccp->displ = kkk;
        if ((((ccp->cod_type) & 56) >> 3) > 1)
            kkk += (((ccp->cod_type) & 56) >> 3);
        else
            ++kkk;
    }
#ifdef DEBUG
    for (ccp = cccp, ii = no_byte ; ii-- ; ccp++) {
        printf("    coding #%d  type %d  range 0..%d\n",
               (ccp - encoding) + 1, (int)ccp->cod_type, (int)ccp->max_val);
        if (ccp->cod_type == 128) {
            struct C_mark *cccc = ccp->link;
            for (; cccc != NULL ; cccc = cccc->link) {
                printf("      subcoding #%d  type %d  range 0..%d mask=%d, offset=%d\n",
                       (cccc - encoding) + 1, (int)cccc->cod_type, (int)cccc->max_val,
                       (int)cccc->mask[0], (int)cccc->offset[0]);
                if ((int)(cccc->cod_type & 63))
                    for (pp = cccc->p_l ; pp != NULL ; pp = pp->next)
                        printf("      place #%d as %d (%d)\n",
                               (int)(pp - p_list) + 1, (int)pp->enc, (int)pp->covered);
                else
                    printf("      place #%d (%d)\n",
                           (int)(cccc->p_l - p_list) + 1, (int)cccc->p_l->covered);
            }
        }
        else if ((int)ccp->cod_type)
            for (pp = ccp->p_l ; pp != NULL ; pp = pp->next)
                printf("      place #%d as %d (%d)\n",
                       (int)(pp - p_list) + 1, (int)pp->enc, (int)pp->covered);
        else
            printf("      place #%d (%d)\n",
                   (int)(ccp->p_l - p_list) + 1, (int)ccp->p_l->covered);
    }
    printf("\n  End of encode_markings\n");
#endif
}

int cod_cmp(p1, p2)
struct C_mark *p1, * p2;
{
    register int m1, m2;

    m1 = (((p1->cod_type) & 7) < 2);
    m2 = (((p2->cod_type) & 7) < 2);
    if (m1 != m2)
        return ((m1) ? -1 : 1);
    m1 = p1->max_val;
    m2 = p2->max_val;
    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}

int cod_cmp2(p1, p2)
struct C_mark *p1, * p2;
{
    register int m1, m2;

    m1 = p1->max_val;
    m2 = p2->max_val;
    if (m1 == m2)
        return (0);
    return ((m2 > m1) ? -1 : 1);
}

