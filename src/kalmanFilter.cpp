/**
 * \file kalmanFilter.cpp
 *
 * Extended Kalman filter with sparse access governed by boost::indirect_array
 *
 *   \author: jsola@laas.fr
 *
 * \date 25/03/2010
 *
 *
 * \ingroup rtslam
 */

#include "rtslam/kalmanFilter.hpp"
#include "rtslam/observationAbstract.hpp"
#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;
		using namespace jmath;
		using namespace jblas;
		using namespace ublas;
		using namespace jmath::ublasExtra;

		ExtendedKalmanFilterIndirect::ExtendedKalmanFilterIndirect(size_t _size) :
			size_(_size), x_(size_), P_(size_)
		{
			x_.clear();
			P_.clear();
		}

		void ExtendedKalmanFilterIndirect::predict(const ind_array & ia_x, const mat & F_v, const ind_array & ia_v,
		    const mat & F_u, const sym_mat & U)
		{
			ind_array ia_invariant = ublasExtra::ia_complement(ia_x, ia_v);
			ixaxpy_prod(P_, ia_invariant, F_v, ia_v, ia_v, prod_JPJt(U, F_u));
		}

		void ExtendedKalmanFilterIndirect::predict(const ind_array & ia_x, const mat & F_v, const ind_array & ia_v,
		    const sym_mat & Q)
		{
			ind_array ia_inv = ublasExtra::ia_complement(ia_x, ia_v);
			ixaxpy_prod(P_, ia_inv, F_v, ia_v, ia_v, Q);
		}

		void ExtendedKalmanFilterIndirect::initialize(const ind_array & ia_x, const mat & G_v, const ind_array & ia_rs, const ind_array & ia_l, const mat & G_y, const sym_mat & R){
			ind_array ia_invariant = ia_complement(ia_x, ia_l);
			ixaxpy_prod(P_, ia_invariant, G_v, ia_rs, ia_l, prod_JPJt(R, G_y));
		}

		void ExtendedKalmanFilterIndirect::initialize(const ind_array & ia_x, const mat & G_v, const ind_array & ia_rs, const ind_array & ia_l, const mat & G_y, const sym_mat & R, const mat & G_n, const sym_mat & N){
			ind_array ia_invariant = ia_complement(ia_x, ia_l);
			ixaxpy_prod(P_, ia_invariant, G_v, ia_rs, ia_l, prod_JPJt(R, G_y) + prod_JPJt(N, G_n));
		}

		void ExtendedKalmanFilterIndirect::reparametrize(const ind_array & ia_x, const mat & J_l, const ind_array & ia_old, const ind_array & ia_new){
			ind_array ia_invariant = ia_complement(ia_x, ia_union(ia_old,ia_new));
			ixaxpy_prod(P_, ia_invariant, J_l, ia_old, ia_new);
		}

		void ExtendedKalmanFilterIndirect::computeKalmanGain(const ind_array & ia_x, Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl){
			PJt_tmp.resize(ia_x.size(),INN_rsl.size1());
			PJt_tmp = prod(project(P_, ia_x, ia_rsl), trans(INN_rsl));
			inn.invertCov();
			K = - prod(PJt_tmp, inn.iP_);
		}

		void ExtendedKalmanFilterIndirect::correct(const ind_array & ia_x, Innovation & inn, const mat & INN_rsl, const ind_array & ia_rsl)
		{
			// first the kalman gain
			computeKalmanGain(ia_x, inn, INN_rsl, ia_rsl);

			// mean and covariances update:
			ublas::project(x_, ia_x) += prod(K, inn.x());
			ublas::project(P_, ia_x, ia_x) += prod<sym_mat> (K, trans(PJt_tmp));
		}


	}
}
