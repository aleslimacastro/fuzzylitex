/**
 * \file training.h
 *
 * \brief Training algorithms for ANFIS models
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2015 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * This file is part of fuzzylite.
 *
 * fuzzylite is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * fuzzylite is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with fuzzylite.  If not, see <http://www.gnu.org/licenses/>.
 *
 * fuzzylite™ is a trademark of FuzzyLite Limited.
 *
 */

#ifndef FL_ANFIS_TRAINING_H
#define FL_ANFIS_TRAINING_H


#include <cstddef>
#include <deque>
#include <fl/anfis/engine.h>
#include <fl/dataset.h>
#include <fl/detail/rls.h>
#include <fl/fuzzylite.h>
#include <map>
#include <vector>


namespace fl { namespace anfis {

/**
 * Hybrid learning algorithm by (J.-S.R. Jang,1993)
 *
 * The hybrid learning algorithm uses a combination of the gradient-descent
 * backpropagation algorithm and least-squares estimation to identify the
 * parameters of the input and output membership functions of a single-output,
 * Sugeno-type fuzzy inference system.
 *
 * The hybrid learning algorithm has been proposed by J.-S.R. Jang in [1] and is
 * well explained in [2].
 * What follows is an extert of the description of the hybrid algorithm as
 * found in [2].
 * In the batch mode of the hybrid learning algorithm, each epoch is composed of
 * a <em>forward pass</em> and a <em>backward pass</em>.
 * In the forward pass, after an input vector is presented, the outputs of the
 * nodes in the ANFIS adaptive network are computed layer by layer in order to
 * build a row for matrices \f$A\f$ and \f$y\f$.
 * This process is repeated for all the training data pairs to form a complete
 * \f$A\f$ and \f$y\f$.
 * The the parameters \f$S_2\f$ of the output terms in the rule consequents are
 * identified by a least-squares method (e.g., the recursive least-squares
 * algorithm).
 * After the parameters \f$S_2\f$ are identified, the error measure (i.e., the
 * squared error) can be computed for each training data pair.
 * In the backward pass, the error signals (i.e., the derivative of the error
 * measure with respect to each node output) propagate from the output end
 * toward the input end.
 * The gradient vector is accumulated for each training data entry.
 * At the end of the backward pass for all training data, the parameters
 * \f$S_1\f$ of the input terms are updated according to the steepest descent.
 * For given fixed values of parameters \f$S_1\f$, the parameters \f$S_2\f$ thus
 * found are guaranteed to be the global optimum point in the \f$S_2\f$
 * parameter space because of the choice of the squared error measure.
 *
 * In the backward step, parameters \f$\alpha\f$ are updated according to the
 * generalized delta rule formula (typically used by the backpropagation
 * algorithm):
 * \f{align}
 *   \alpha &= \alpha + \Delta\alpha,\\
 *   \Delta\alpha &= -\eta\frac{\partial E}{\partial \alpha},\\
 *   \eta &= \frac{\kappa}{\sqrt{\sum_\alpha \frac{\partial E}{\partial a}}}
 * \f}
 * where:
 * - \f$\eta\f$ is the learning rate,
 * - \f$\kappa\f$ is the step size, representing the length of each transition
 *   along the gradient direction in the parameter space.
 * - \f$E\f$ is the error measure which is typically the sum of squared errors:
 *   \f[
 *     E=\sum_{k=1}^N (d_k-o_k)^2
 *   \f]
 *   where \f$d_k\f$ is the desired value and \f$o_k\f$ is the actual output.
 * .
 *
 * References
 * -# J.-S.R. Jang, "ANFIS: Adaptive-Network-based Fuzzy Inference Systems," IEEE Transactions on Systems, Man, and Cybernetics, 23:3(665-685), 1993.
 * -# J.-S.R. Jang et al., "Neuro-Fuzzy and Soft Computing: A Computational Approach to Learning and Machine Intelligence," Prentice-Hall, Inc., 1997.
 * .
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
class Jang1993HybridLearningAlgorithm
{
public:
	/**
	 * Constructor
	 *
	 * \param p_anfis Pointer to the ANFIS model to be trained
	 * \param ss The initial step size used in the parameter update formula
	 * \param ssDecrRate The step size decrease rate
	 * \param ssIncrRate The step size increase rate
	 * \param ff The forgetting factor used in the recursive least square
	 *  algorithm
	 */
    explicit Jang1993HybridLearningAlgorithm(Engine* p_anfis = fl::null,
											 fl::scalar ss = 0.01,
											 fl::scalar ssDecrRate = 0.9,
											 fl::scalar ssIncrRate = 1.1,
											 fl::scalar ff = 1);

	/// Sets the ANFIS model to be trained
	void setEngine(Engine* p_anfis);

	/// Gets the ANFIS model to be trained
	Engine* getEngine() const;

	/// Sets the initial step size
	void setInitialStepSize(fl::scalar value);

	/// Gets the initial step size
	fl::scalar getInitialStepSize() const;

	/// Sets the step size decrease rate
	void setStepSizeDecreaseRate(fl::scalar value);

	/// Gets the step size decrease rate
	fl::scalar getStepSizeDecreaseRate() const;

	/// Sets the step size increase rate
	void setStepSizeIncreaseRate(fl::scalar value);

	/// Gets the step size increase rate
	fl::scalar getStepSizeIncreaseRate() const;

	/// Sets the momentum value
	void setMomentum(fl::scalar value);

	/// Gets the momentum value
	fl::scalar getMomentum() const;

	/// Sets the forgetting factor
	void setForgettingFactor(fl::scalar value);

	/// Gets the forgetting factor
	fl::scalar getForgettingFactor() const;

	/// Sets the online/offline mode for the learning algorithm
	void setIsOnline(bool value);

	/// Gets the online/offline mode of the learning algorithm
	bool isOnline() const;

	/**
	 * Trains the ANFIS model
	 *
	 * \param data The training set
	 * \param maxEpochs The maximum number of epochs
	 * \param errorGoal The error to achieve
	 *
	 * The error measure is the Root Mean Squared Error (RMSE).
	 *
	 * \return The achieve error
	 */
	fl::scalar train(const fl::DataSet<fl::scalar>& data,
					 std::size_t maxEpochs = 10,
					 fl::scalar errorGoal = 0);

    /// Trains the ANFIS model for a single epoch only using the given training set \a data
    fl::scalar trainSingleEpoch(const fl::DataSet<fl::scalar>& data);

	/// Resets the state of the learning algorithm
	void reset();

private:
	/// Initializes the training algorithm
	void init();

	/// Checks the correctness of the parameters of the training algorithm
	void check() const;

	/// Trains ANFIS for a signle epoch in offline (batch) mode
	fl::scalar trainSingleEpochOffline(const fl::DataSet<fl::scalar>& data);

	/// Trains ANFIS for a signle epoch in online mode
	fl::scalar trainSingleEpochOnline(const fl::DataSet<fl::scalar>& data);

	/// Updates parameters of input terms
	void updateInputParameters();

	/// Updates the step-size (and the learning rate as well)
	void updateStepSize();

	/// Resets state for single epoch training
	void resetSingleEpoch();

	/// Gets the number of parameters of each output term
	std::size_t numberOfOutputTermParameters() const;


private:
	Engine* p_anfis_; ///< The ANFIS model
	fl::scalar stepSizeInit_; ///< The initial value of the step size
	fl::scalar stepSizeDecrRate_; ///< The rate at which the step size must be decreased
	fl::scalar stepSizeIncrRate_; ///< The rate at which the step size must be increased
	fl::scalar stepSize_; ///< Step size to use in the parameter update formula representing the length of each transition along the gradient direction in the parameter space
	std::size_t stepSizeErrWindowLen_; ///< Length of the RMSE window used to update the step size
	std::deque<fl::scalar> stepSizeErrWindow_; ///< Window of RMSEs used to update the step size
	std::size_t stepSizeIncrCounter_; ///< Counter used to check when to increase the step size
	std::size_t stepSizeDecrCounter_; ///< Counter used to check when to decrease the step size
	bool online_; ///< \c true in case of online learning; \c false if offline (batch) learning
	fl::scalar momentum_;
	//bool useBias_; ///< if \c true, add a bias to handle zero-firing error
	//std::vector<fl::scalar> bias_; ///< The bias to use in the output
	fl::detail::RecursiveLeastSquaresEstimator<fl::scalar> rls_; ///< The recursive least-squares estimator
	std::map< Node*, std::vector<fl::scalar> > dEdPs_; ///< Error derivatives wrt node parameters
	std::map< Node*, std::vector<fl::scalar> > oldDeltaPs_; ///< Old values of parameters changes (only for momentum learning)
	//std::vector<fl::scalar> rlsPhi_; ///< RLS regressor of the last epoch
}; // Jang1993HybridLearningAlgorithm

}} // Namespace fl::anfis

#endif // FL_ANFIS_TRAINING_H
