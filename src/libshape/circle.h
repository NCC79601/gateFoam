#ifndef CIRCLE_H
#define CIRCLE_H

#include "ishape.h"
namespace sdfibm{

class Circle : public IShape, _shapecreator<Circle>
{
private:
    scalar m_radius;
    scalar m_radiusSQR;

public:
    // const static int shape_id = SHAPE::CIRC;
    Circle(const dictionary& para)
    {
        m_radius =  Foam::readScalar(para.lookup("radius"));
        m_com = para.lookupOrDefault("com", vector::zero);
        m_radiusSQR = m_radius * m_radius;

        // set inherited variables
        m_volume = M_PI*m_radiusSQR;
        m_volumeINV = 1.0/m_volume;
        scalar tmp = 0.5*m_volume*m_radiusSQR;
        m_moi[0] = tmp; m_moi[4] = tmp; m_moi[8] = tmp;
        m_moiINV = Foam::inv(m_moi);
        m_radiusB = m_radius;
    }
    inline scalar getRadius() const { return m_radius;}
    inline scalar getVolume() const { return m_volume;}
    inline void setRadius(scalar r) {m_radius = r;}

    // typename and description
    SHAPETYPENAME("Circle")
    virtual std::string description() const override {return "circle (x-y plane), r = " + std::to_string(m_radius);}

    // implement interface
    virtual inline bool isInside(const vector& p) const override
    {
        return sdf::circle_bool_fast(m_com + vector(p.x(), p.y(), 0.0), m_radiusSQR);
    }

    virtual inline scalar signedDistance(const vector& p) const override
    {
        // FIXME
        // debug info
        vector center_point = m_com + vector(p.x(), p.y(), 0.0);
        scalar result = sdf::filter(sdf::circle(m_com + vector(p.x(), p.y(), 0.0), m_radius));
        
        #ifdef GATEFOAM_DEBUG
        // debug info
        {
            printf(">>>>> Circle::signedDistance:\n");
            printf("    m_com = (%.3f, %.3f, %.3f)\n", m_com.x(), m_com.y(), m_com.z());
            printf("    input p = (%.3f, %.3f, %.3f)\n", p.x(), p.y(), p.z());
            printf("    center_point = (%.3f, %.3f, %.3f)\n", 
                center_point.x(), center_point.y(), center_point.z());
            printf("    radius = %.3f\n", m_radius);
            printf("    result = %.6e\n\n", result);
        }
        #endif

        return sdf::filter(sdf::circle(m_com + vector(p.x(), p.y(), 0.0), m_radius));
    }
};
}
#endif
