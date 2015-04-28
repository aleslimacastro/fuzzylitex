/**
 * \file fl/detail/terms.cpp
 *
 * \brief Functionalities related to fuzzy terms
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2014 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include <cstddef>
#include <fl/commons.h>
#include <fl/detail/math.h>
#include <fl/detail/terms.h>
#include <fl/fuzzylite.h>
#include <fl/term/Term.h>
#include <fl/term/Bell.h>
#include <fl/term/Concave.h>
#include <fl/term/Constant.h>
#include <fl/term/Cosine.h>
#include <fl/term/Discrete.h>
#include <fl/term/Linear.h>
#include <fl/term/Ramp.h>
#include <fl/term/Sigmoid.h>
#include <fl/term/SShape.h>
#include <fl/term/Triangle.h>
#include <fl/term/ZShape.h>
#include <stdexcept>
#include <vector>


namespace fl { namespace detail {

std::vector<fl::scalar> GetTermParameters(const fl::Term* p_term)
{
	//FIXME: it would be a good idea to add a pure virtual method in fl::Term
	//       class that returns the vector of parameters, like:
	//         virtual std::vector<fl::scalar> getParameters() = 0;

	std::vector<fl::scalar> params;

	if (dynamic_cast<const fl::Bell*>(p_term))
	{
		const fl::Bell* p_realTerm = dynamic_cast<const fl::Bell*>(p_term);
		params.push_back(p_realTerm->getCenter());
		params.push_back(p_realTerm->getWidth());
		params.push_back(p_realTerm->getSlope());
	}
	else if (dynamic_cast<const fl::Concave*>(p_term))
	{
		const fl::Concave* p_realTerm = dynamic_cast<const fl::Concave*>(p_term);
		params.push_back(p_realTerm->getInflection());
		params.push_back(p_realTerm->getEnd());
	}
	else if (dynamic_cast<const fl::Constant*>(p_term))
	{
		const fl::Constant* p_realTerm = dynamic_cast<const fl::Constant*>(p_term);
		params.push_back(p_realTerm->getValue());
	}
	else if (dynamic_cast<const fl::Cosine*>(p_term))
	{
		const fl::Cosine* p_realTerm = dynamic_cast<const fl::Cosine*>(p_term);
		params.push_back(p_realTerm->getCenter());
		params.push_back(p_realTerm->getWidth());
	}
	else if (dynamic_cast<const fl::Discrete*>(p_term))
	{
		const fl::Discrete* p_realTerm = dynamic_cast<const fl::Discrete*>(p_term);
		const std::vector<fl::Discrete::Pair> pairs = p_realTerm->xy();
		const std::size_t np = pairs.size();
		for (std::size_t p = 0; p < np; ++p)
		{
			params.push_back(pairs[p].first);
			params.push_back(pairs[p].second);
		}
	}
	else if (dynamic_cast<const fl::Linear*>(p_term))
	{
		const fl::Linear* p_realTerm = dynamic_cast<const fl::Linear*>(p_term);
		params = p_realTerm->coefficients();
	}
	if (dynamic_cast<const fl::Ramp*>(p_term))
	{
		const fl::Ramp* p_realTerm = dynamic_cast<const fl::Ramp*>(p_term);
		params.push_back(p_realTerm->getStart());
		params.push_back(p_realTerm->getEnd());
	}
	if (dynamic_cast<const fl::Sigmoid*>(p_term))
	{
		const fl::Sigmoid* p_realTerm = dynamic_cast<const fl::Sigmoid*>(p_term);
		params.push_back(p_realTerm->getInflection());
		params.push_back(p_realTerm->getSlope());
	}
	else if (dynamic_cast<const fl::SShape*>(p_term))
	{
		const fl::SShape* p_realTerm = dynamic_cast<const fl::SShape*>(p_term);
		params.push_back(p_realTerm->getStart());
		params.push_back(p_realTerm->getEnd());
	}
	else if (dynamic_cast<const fl::Triangle*>(p_term))
	{
		const fl::Triangle* p_realTerm = dynamic_cast<const fl::Triangle*>(p_term);
		params.push_back(p_realTerm->getVertexA());
		params.push_back(p_realTerm->getVertexB());
		params.push_back(p_realTerm->getVertexC());
	}
	else if (dynamic_cast<const fl::ZShape*>(p_term))
	{
		const fl::ZShape* p_realTerm = dynamic_cast<const fl::ZShape*>(p_term);
		params.push_back(p_realTerm->getStart());
		params.push_back(p_realTerm->getEnd());
	}

	return params;
}

/**
 * Eval the derivative of the generalized bell function with respect to its parameters
 * \f{align}{
 *  \frac{\partial f(x,c,w,s)}{\partial x} &= -\frac{2s |\frac{x-c}{w}|^{2s-1}}{w (|\frac{x-c}{w}|^{2s}+1)^2},\\
 *  \frac{\partial f(x,c,w,s)}{\partial c} &=  \frac{2s |\frac{x-c}{w}|^{2s-1}}{w (|\frac{x-c}{w}|^{2s}+1)^2},\\
 *  \frac{\partial f(x,c,w,s)}{\partial w} &=  \frac{2s (x-c) |\frac{x-c}{w}|^{2s-1}}{w^2 (|\frac{x-c}{w}|^{2s}+1)^2},\\
 *  \frac{\partial f(x,c,w,s)}{\partial s} &= -\frac{2|\frac{x-c}{w}|^{2s} \log(|\frac{x-c}{w}|)}{(|\frac{x-c}{w}|^{2s}+1)^2}.
 * \f}
 *
 * Mathematica:
 *   f[x_, c_, w_, s_] := 1/(1 + Abs[(x - c)/w]^(2*s))
 *   D[f[x, c, w, s], {{x,c,w,s}}]
 */
std::vector<fl::scalar> EvalBellTermDerivativeWrtParams(const fl::Bell& term, fl::scalar x)
{
	const fl::scalar c = term.getCenter();
	const fl::scalar w = term.getWidth();
	const fl::scalar s = term.getSlope();

	const fl::scalar xn = (x-c)/w;
	const fl::scalar xnp = (xn != 0) ? std::pow(detail::Sqr(xn), s) : 0;
	const fl::scalar den = detail::Sqr(1+xnp);

	std::vector<fl::scalar> res(3);

	// Center parameter
	res[0] = (x != c)
			 ? 2.0*s*xnp/((x-c)*den)
			 : 0;
	// Width parameter
	res[1] = 2.0*s*xnp/(w*den);
	// Slope parameter
	res[2] = (x != c && x != (c+w))
			 ? -std::log(detail::Sqr(xn))*xnp/den
			 : 0;

	return res;
}

std::vector<fl::scalar> EvalTermDerivativeWrtParams(const fl::Term* p_term, fl::scalar x)
{
	if (dynamic_cast<const fl::Bell*>(p_term))
	{
		const fl::Bell* p_bell = dynamic_cast<const fl::Bell*>(p_term);
		return EvalBellTermDerivativeWrtParams(*p_bell, x);
	}

	FL_THROW2(std::runtime_error, "Derivative for term '" + p_term->className() + "' has not been implemented yet");
}

}} // Namespace fl::detail
