#ifndef MOTIONGATE_H
#define MOTIONGATE_H

#include "imotion.h"
namespace sdfibm {

class MotionGate:public IMotion, _creator<MotionGate>
{
public:
    // same signature for all motions
    virtual void constraint(
            const scalar& time,
            vector& velocity,
            vector& omega) override final;

    // update below
    MotionGate(const dictionary& para)
    {
        try {
            m_opentime = Foam::readScalar(para.lookup("opentime"));
            m_stoptime = Foam::readScalar(para.lookup("stoptime"));
            m_dy       = Foam::readScalar(para.lookup("dy"));
        } catch (const std::exception& e) {
            std::cout << "Problem in creating MotionGate" << ' '
                      << e.what() << std::endl;
            std::exit(1);
        }
        if (m_opentime >= m_stoptime - 1e-6)
        {
            // sanity check
            throw(std::invalid_argument("opentime should not be larger than stoptime!"));
        }
    }
    virtual ~MotionGate() override final {}
    TYPENAME("MotionGate")
    virtual std::string description() const override {return "model a gate";}
private:
    scalar m_opentime, m_stoptime, m_dy;
    scalar getGateVelocity(const scalar& time)
    {
        scalar gate_v = m_dy / (m_stoptime - m_opentime);
        // scalar t = fmod(time, 5.0);
        scalar t = time;
        scalar v = 0.0;
        // if (t > 1 && t < 2)
        //     v = -1.0;
        // if (t > 3 && t < 4)
        //     v =  1.0;
        if (t > m_opentime && t < m_stoptime)
        {
            v = gate_v;
        }
        return v;
    }
};

void MotionGate::constraint(const scalar &time, vector &velocity, vector &omega)
{
    velocity = vector(0,getGateVelocity(time),0);
    omega    = vector::zero;
}

}
#endif // MOTIONGATE_H
