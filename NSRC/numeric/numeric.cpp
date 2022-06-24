
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cfloat>
#include <queue>
#include <exception>
using namespace std;

#include <boost/smart_ptr.hpp>

#include "../platform/platform_utils.h"

#include "numeric.h"

//-----------------------------------------------------------------------------

const char *numerical::ELA_Names[numerical::ELA_NUM_ALGORITHMS] = {
    "JACOBI",
    "FORWARD GAUSS-SEIDEL",
    "BACKWARD GAUSS-SEIDEL",
    "SYMMETRIC GAUSS-SEIDEL",
    "GMRES", "BiCG-STAB", "CGS"
};

//-----------------------------------------------------------------------------

const char *numerical::ELA_Names_OR[numerical::ELA_NUM_ALGORITHMS] = {
    "JOR",
    "FORWARD SOR",
    "BACKWARD SOR",
    "SYMMETRIC SOR",
    "GMRES", "BiCG-STAB", "CGS"
};

//-----------------------------------------------------------------------------

bool numerical::ELA_Is_Krylov[numerical::ELA_NUM_ALGORITHMS] = {
    false, false, false, false,
    true, true, true
};

//-----------------------------------------------------------------------------

const char *numerical::ILA_Names[numerical::ILA_NUM_ALGORITHMS] = {
    "POWER METHOD", "GMRES", "BiCG-STAB", "CGS"
};

//-----------------------------------------------------------------------------

const char *numerical::PA_Names[numerical::PA_NUM_PRECONDITIONER_ALGO] = {
    "NO", "DIAGONAL", "ILU(0)", "ILUTK", /*"INNER/OUTER"*/
};

//-----------------------------------------------------------------------------

const bool numerical::IPS_isInnerOuter[numerical::IPS_NUM_PRECONDITIONER_SOURCES] = {
    false, true, true, false, false
};

//-----------------------------------------------------------------------------

const char *numerical::IPS_Names[numerical::IPS_NUM_PRECONDITIONER_SOURCES] = {
    "NO PRECONDITIONER",
    "INNER/OUTER WITH EXPLICIT EMC (SLOW)",
    "INNER/OUTER WITH SIMULATED EMC",
    "PRECONDITIONER BUILT FROM EXPLICIT EMC (SLOW)",
    "PRECONDITIONER BUILT FROM SIMULATED EMC",
};

//-----------------------------------------------------------------------------

const char *numerical::KED_Names[2] = {
    "FORWARD", "BACKWARD"
};

//-----------------------------------------------------------------------------

bool print_GUI_stat_enabled = false;
bool numerical::print_stat_for_gui() {
	return invoked_from_gui() && print_GUI_stat_enabled;
}

//-----------------------------------------------------------------------------

void numerical::enable_print_stat_for_gui() {
	print_GUI_stat_enabled = true;
}

//-----------------------------------------------------------------------------

numerical::multiorder_vector::multiorder_vector(size_t N, double maxVal,
        double minVal, int _stepExp) {
    int emin, emax;
    frexp(minVal, &emin);
    frexp(maxVal, &emax);
    stepExp = _stepExp;
    baseExp = emin;
    numOrders = max(1, (emax - emin + stepExp - 1) / stepExp);
    movec.resize(N, numOrders);
}

//-----------------------------------------------------------------------------

void numerical::multiorder_vector::pack(ublas::vector<double> &outVec) const {
    const size_t N = movec.size1();
    outVec.resize(N);
    fill(outVec.begin(), outVec.end(), 0.0);

    for (size_t n = 0; n < N; n++) {
        // Add small values first
        for (int m = 0; m < numOrders; m++) {
            outVec(n) += movec(n, m);
        }
    }
}

//-----------------------------------------------------------------------------

ostream &numerical::operator <<(ostream &os, const numerical::AlgoName an) {
    if (an.ilsa != ILA_NUM_ALGORITHMS) {
        os << ILA_Names[an.ilsa];
        if (an.ilsa == ILA_GMRES_M)
            os << "(" << an.param << ")";
    }
    if (an.elsa != ELA_NUM_ALGORITHMS) {
        os << ELA_Names[an.elsa];
        if (an.elsa == ELA_GMRES_M)
            os << "(" << an.param << ")";
    }
    return os;
}

numerical::AlgoName numerical::AN_UNSPECIFIED = numerical::AlgoName();
numerical::AlgoName numerical::AN_POWER_METHOD = numerical::AlgoName(numerical::ILA_POWER_METHOD);
numerical::AlgoName numerical::AN_BICGSTAB = numerical::AlgoName(numerical::ILA_BICGSTAB);
numerical::AlgoName numerical::AN_CGS = numerical::AlgoName(numerical::ILA_CGS);

//-----------------------------------------------------------------------------

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

//-----------------------------------------------------------------------------

static std::map<std::tuple<string, double, double>, 
         std::vector<pair<double, double>>> computed_factors_mem;

const std::vector<pair<double, double>>&
ch_compute_alpha_factors_dbl(const char* fg_expr, const double q, const double accuracy)
{
	auto f_key = make_tuple(string(fg_expr), q, accuracy);

	if (computed_factors_mem.count(f_key) == 0) {
		// const size_t MAX_BUF = 2048;
		// char line[MAX_BUF];
		ostringstream cmdname;
		const char *appimg_dir = get_appimage_dir();
		if (appimg_dir != nullptr) {
			cmdname << appimg_dir << PATH_SEPARATOR_CH << "bin" << PATH_SEPARATOR_CH << "alphaFactory" << EXE_SUFFIX;
		}
		else {
			cmdname << "/usr/local/GreatSPN/bin/alphaFactory";
		}

		char tmpfname[1024];
		portable_mkstemp(tmpfname, "af_XXXXXXXX");
		// std::string tmpfname;
		// const char *tmpdir_env = getenv("GREATSPN_TEMP_DIR");
		// cout << "tmpdir_env=" << tmpdir_env << endl;
		// if (tmpdir_env != nullptr) {
		// 	tmpfname = tmpdir_env;
		// 	tmpfname += "af_XXXXXXXX";
		// }
		// else {
		// 	tmpfname = "af_XXXXXXXX";
		// }
		// mkstemp(tmpfname.data());

		// std::string tmpfname = std::tmpnam(nullptr);
		// cmdline << " \"" << fg_expr << "\" " << q << " " << accuracy << " " << tmpfname;
		std::vector<pair<double, double>> af;
		// cout << cmdline.str() << endl;
		// char temp [ PATH_MAX ];
		// cout << "getcwd=" << getcwd(temp, PATH_MAX) << endl;
		cout << "AlphaFactory:  f_g(x)=\"" << fg_expr << "\"  rate=" << q 
			 << "  acc=" << accuracy << " tmpfname=" << tmpfname << endl;

		// Call alphaFactory
		std::string cmdname_s = cmdname.str();
		std::string q_s = to_string_with_precision(q, 15);
		std::string acc_s = to_string_with_precision(accuracy, 15);
	    const char* const args[] = { cmdname_s.c_str(), fg_expr, q_s.c_str(), acc_s.c_str(), tmpfname, nullptr };
	    int ret = execp_cmd(args[0], args, 1);
		if (ret != 0) {
			cerr << "Cannot communicate with alphaFactory." << endl;
			throw program_exception("alphaFactory cannot be exec'd.");
		}

		// Read back the output file
		ifstream inf(tmpfname);
		size_t num_factors;
		inf >> num_factors;
		// cout << "num_factors=" << num_factors << endl;
		af.resize(num_factors);
		for (size_t ii=0; ii<num_factors; ii++)
			inf >> af[ii].first >> af[ii].second;
		if (!inf)
			throw program_exception("Count not read back the output file.");
		inf.close();

		/*FILE *proc = popen(cmdline.str().c_str(), "r");
		int num_factors, ii = -1;
		double factor;
		while (fgets(line, MAX_BUF, proc)) {
			// read the results!
			if (ii<0 && 1 == sscanf(line, "Factors: %d", &num_factors)) { 
				ii = 0;
				af.resize(num_factors);
				// cout << "factors = " << num_factors << endl;
			}
			else if (ii>=0 && 1 == sscanf(line, "%lf", &factor)) {
				if (ii < num_factors) {
					// cout << "af["<<ii<<"].first = " << factor << endl;
					af[ii++].first = factor;
				}
				else {
					// cout << "af["<<(ii- num_factors)<<"].second = " << factor << endl;
					af[ii++ - num_factors].second = factor;
				}
			}
		}
		if (0 != pclose(proc))
			throw "Cannot communicate with alphaFactory.";*/

		std::remove(tmpfname);

		computed_factors_mem[f_key] = af;
	}

	return computed_factors_mem[f_key];
}

//-----------------------------------------------------------------------------

/*#include <gmm/gmm.h>

template <class Matrix>
void
gmm_ILUT(Matrix& M, typename Matrix::value_type threshold, size_t K)
{
	typedef typename Matrix::value_type value_type;
	typedef typename Matrix::size_type size_type;
	const size_type NR = M.size1(), NC = M.size2();

	gmm::row_matrix< gmm::wsvector<value_type> > M1(NR, NC);

	typename Matrix::const_iterator1 row(M.begin1()), endRows(M.end1());
	for (; row != endRows; ++row) {
		const size_t i = row.index1();
		typename Matrix::const_iterator2 A_ij(row.begin()), endRow_i(row.end());
		for (; A_ij != endRow_i; ++A_ij) {
			const size_t j = A_ij.index2();
			M1(i, j) = *A_ij;
		}
	}
	gmm::csr_matrix<value_type> M2;
	gmm::clean(M1, 1E-12);
	gmm::copy(M1, M2);

	gmm::ilut_precond< gmm::csr_matrix<value_type> > P(M2, K, threshold);

	numerical::matrix_map<double> newM(NR, NC);
	for (size_t m=0; m<2; m++) {
		typedef gmm::row_matrix<gmm::rsvector<value_type> > LU_matrix_t;
		const LU_matrix_t& PM = (m==0 ? P.L : P.U);
		typename gmm::linalg_traits<LU_matrix_t>::const_row_iterator row_it, row_ite;
		typename gmm::linalg_traits<LU_matrix_t>::const_sub_row_type row;

		row_it = mat_row_const_begin(PM);
		row_ite = mat_row_const_end(PM);
		for (size_t i=0; row_it != row_ite; ++row_it, ++i) {
			row = gmm::linalg_traits<LU_matrix_t>::row(row_it);
			typename gmm::linalg_traits< typename gmm::linalg_traits<LU_matrix_t>::const_sub_row_type >::const_iterator it, ite;
			it = vect_const_begin(row);
			ite = vect_const_end(row);

			for (; it != ite; ++it) {
				const size_t j = it.index();
				newM(i, j) = *it;
			}
		}
	}
	compress_matrix(M, newM);
}

//-----------------------------------------------------------------------------

void numerical::gmm_ILUT_d(ublas::compressed_matrix<double>& M, double threshold, size_t K)
{
	gmm_ILUT(M, threshold, K);
}*/

//-----------------------------------------------------------------------------




/*void test_multiorder_vector()
{
	cout << "test_multiorder_vector" << endl;
	multiorder_vector mv(2, 1.0, 1e-9, 3);
	ublas::vector<double> v, cv(2, 0.0);

#define ADD(i, val)   { mv.add(i, val); cv(i) += val; }

	ADD(1, 1.0);
	ADD(1, 1.0 / 2);
	ADD(1, 1.0 / 4);
	ADD(1, 1.0 / 8);
	ADD(1, 1.0 / 16);
	ADD(1, 1.0 / 32);
	ADD(1, 1.0 / 64);
	cout << endl;

	for (size_t k=0; k<100; k++)
		ADD(1, 1.0 / 64);

	cout << endl;
	ADD(0, 1.0);
	ADD(0, 2.0);
	ADD(0, 0.0001);
	ADD(0, 0.00009);
	ADD(0, 0.00005);
	ADD(0, 0.0009);

	cout << "------------------------" << endl;
	mv.pack(v);
	cout << v << cv << endl;
}*/























