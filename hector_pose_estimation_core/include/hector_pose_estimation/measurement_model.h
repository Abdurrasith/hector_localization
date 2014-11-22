//=================================================================================================
// Copyright (c) 2011, Johannes Meyer and Martin Nowara, TU Darmstadt
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Flight Systems and Automatic Control group,
//       TU Darmstadt, nor the names of its contributors may be used to
//       endorse or promote products derived from this software without
//       specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//=================================================================================================

#ifndef HECTOR_POSE_ESTIMATION_MEASUREMENT_MODEL_H
#define HECTOR_POSE_ESTIMATION_MEASUREMENT_MODEL_H

#include <hector_pose_estimation/model.h>
#include <hector_pose_estimation/substate.h>
#include <hector_pose_estimation/input.h>

namespace hector_pose_estimation {

class MeasurementModel : public Model {
public:
  virtual ~MeasurementModel() {}

  virtual bool init(PoseEstimation& estimator, Measurement &measurement, State& state) { return true; }
  virtual int getDimension() const = 0;

  virtual SystemStatus getStatusFlags() { return SystemStatus(0); }
  virtual bool active(const State& state) { return !(state.getSystemStatus() & STATUS_ALIGNMENT); }

  virtual bool prepareUpdate(State& state, const MeasurementUpdate& update) { return true; }
  virtual void afterUpdate(State& state) {}
};

template <class Derived, int _Dimension> class MeasurementModel_;

namespace traits {

  template <class Derived, int _Dimension = Derived::MeasurementDimension>
  struct MeasurementModel {
    enum { MeasurementDimension = _Dimension };
    typedef ColumnVector_<MeasurementDimension> MeasurementVector;
    typedef SymmetricMatrix_<MeasurementDimension> NoiseVariance;
    typedef Matrix_<MeasurementDimension,Dynamic> MeasurementMatrix;
    typedef Matrix_<State::Covariance::RowsAtCompileTime,MeasurementDimension> GainMatrix;
    typedef ColumnVector_<State::Covariance::RowsAtCompileTime> UpdateVector;

    enum { InputDimension = traits::Input<Derived>::Dimension };
    typedef typename traits::Input<Derived>::Type InputType;
    typedef typename traits::Input<Derived>::Vector InputVector;
    typedef Matrix_<MeasurementDimension,InputDimension> InputMatrix;
  };

  #define MEASUREMENT_MODEL_TRAIT(Derived, _Dimension) \
    typedef typename traits::MeasurementModel<Derived, _Dimension> trait; \
    \
    enum { MeasurementDimension = _Dimension }; \
    typedef typename trait::MeasurementVector MeasurementVector; \
    typedef typename trait::NoiseVariance NoiseVariance; \
    typedef typename trait::MeasurementMatrix MeasurementMatrix; \
    typedef typename trait::GainMatrix GainMatrix; \
    typedef typename trait::UpdateVector UpdateVector; \
    \
    enum { InputDimension = trait::InputDimension }; \
    typedef typename trait::InputType InputType; \
    typedef typename trait::InputVector InputVector; \
    typedef typename trait::InputMatrix InputMatrix; \

} // namespace traits

template <class Derived, int _Dimension>
class MeasurementModel_ : public MeasurementModel {
public:
  MEASUREMENT_MODEL_TRAIT(Derived, _Dimension)
  virtual ~MeasurementModel_() {}

  virtual int getDimension() const { return trait::MeasurementDimension; }

  Derived *derived() { return static_cast<Derived *>(this); }
  const Derived *derived() const { return static_cast<const Derived *>(this); }

  virtual void getExpectedValue(MeasurementVector& y_pred, const State& state) {}
  virtual void getStateJacobian(MeasurementMatrix& C, const State& state, bool init) { if (init) C.setZero(); }
  virtual void getInputJacobian(InputMatrix& D, const State& state, bool init) { if (init) D.setZero(); }
  virtual void getMeasurementNoise(NoiseVariance& R, const State& state, bool init) { if (init) R.setZero(); }

  virtual void limitError(MeasurementVector& error) {}

  virtual const MeasurementVector* getFixedMeasurementVector() const { return 0; }
};

} // namespace hector_pose_estimation

#endif // HECTOR_POSE_ESTIMATION_MEASUREMENT_MODEL_H
