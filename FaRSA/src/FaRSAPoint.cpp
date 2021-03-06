// Copyright (C) 2020 Frank E. Curtis, Daniel P. Robinson
//
// This code is published under the ??? License.
//
// Author(s) : Frank E. Curtis, Daniel P. Robinson

#include <cmath>

#include "FaRSAPoint.hpp"

namespace FaRSA
{

// Constructor, copy elements from input vector
Point::Point(std::shared_ptr<Problem> problem,
             std::shared_ptr<Vector> vector,
             double scale)
  : objective_evaluated_(false),
    gradient_evaluated_(false),
    scale_(scale),
    problem_(problem)
{

  // Declare new vector
  std::shared_ptr<Vector> new_vector(new Vector(vector->length()));

  // Set point's vector
  vector_ = new_vector;

  // Copy values to point's vector
  vector_->copy(*vector);

  // Set gradient pointer to null
  gradient_.reset();

} // end constructor

// Print
void Point::print(const Reporter* reporter,
                  std::string name) const
{
  vector_->print(reporter, name);
}

// Make new Point by adding "scalar1" times this Point's vector to "scalar2" times other Vector
std::shared_ptr<Point> Point::makeNewLinearCombination(double scalar1,
                                                       double scalar2,
                                                       const Vector& other_vector) const
{

  // Create new Vector
  std::shared_ptr<Vector> new_vector = vector_->makeNewLinearCombination(scalar1, scalar2, other_vector);

  // Create new Point
  std::shared_ptr<Point> new_point(new Point(problem_, new_vector, scale_));

  // Return
  return new_point;

} // end makeNewLinearCombination

// Determine scale
void Point::determineScale(Quantities& quantities)
{

  // Assert gradient has been evaluated
  ASSERT_EXCEPTION(gradient_evaluated_, FARSA_GRADIENT_EVALUATION_ASSERT_EXCEPTION, "Gradient should have been evaluated, but wasn't.");

  // Set scale
  if (gradient_->normInf() > quantities.scalingThreshold()) {
    scale_ = quantities.scalingThreshold() / gradient_->normInf();
  }

} // end determineScale

// Evaluate objective
bool Point::evaluateObjective(Quantities& quantities)
{

  // Check if objective has been evaluated already
  if (!objective_evaluated_) {

    // Set evaluation start time as current time
    clock_t start_time = clock();

    // Evaluate objective value for problem
    objective_evaluated_ = problem_->evaluateObjective(vector_->length(), vector_->values(), objective_);

    // Increment evaluation time
    quantities.incrementEvaluationTime(clock() - start_time);

    // Scale
    objective_ = scale_ * objective_;

    // Check for nan
    if (isnan(objective_)) {
      objective_evaluated_ = false;
    }

    // Increment function evaluation counter
    quantities.incrementFunctionCounter();

    // Check for function evaluation limit
    if (quantities.functionCounter() >= quantities.functionEvaluationLimit()) {
      THROW_EXCEPTION(FARSA_FUNCTION_EVALUATION_LIMIT_EXCEPTION, "Function evaluation limit reached.");
    }

  } // end if

  // Return
  return objective_evaluated_;

} // end evaluateObjective

// Evaluate gradient
bool Point::evaluateGradient(Quantities& quantities)
{

  // Check if gradient has been evaluated already
  if (!gradient_evaluated_) {

    // Declare gradient vector
    std::shared_ptr<Vector> gradient(new Vector(vector_->length()));

    // Set gradient vector
    gradient_ = gradient;

    // Declare temporary array
    double* g = new double[vector_->length()];

    // Set evaluation start time as current time
    clock_t start_time = clock();

    // Evaluate gradient value
    gradient_evaluated_ = problem_->evaluateGradient(vector_->length(), vector_->values(), g);

    // Increment evaluation time
    quantities.incrementEvaluationTime(clock() - start_time);

    // Evaluate gradient value
    gradient_->copyArray(g);

    // Scale
    gradient_->scale(scale_);

    // Delete temporary array
    if (g != nullptr) {
      delete[] g;
      g = nullptr;
    } // end if

    // Check for nan
    for (int i = 0; i < gradient_->length(); i++) {
      if (isnan(gradient_->values()[i])) {
        gradient_evaluated_ = false;
      }
    }

    // Increment gradient evaluation counter
    quantities.incrementGradientCounter();

    // Check for gradient evaluation limit
    if (quantities.gradientCounter() >= quantities.gradientEvaluationLimit()) {
      THROW_EXCEPTION(FARSA_GRADIENT_EVALUATION_LIMIT_EXCEPTION, "Gradient evaluation limit reached.");
    }

  } // end if

  // Return
  return gradient_evaluated_;

} // end evaluateGradient

} // namespace FaRSA
